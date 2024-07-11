#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#define IOCTL_ALLOC_MEM _IOW('x', 1, size_t)
#define IOCTL_STORE_DATA _IOW('x', 2, size_t)
#define IOCTL_EXPORT_DATA _IOR('x', 3, size_t)
#define GET_CMD_SIZE(cmd)   (((cmd) >> 16) & 0x3FFF)  
dev_t devno;
int major = 0;
int minor = 0;
struct device *dev = NULL;
struct class *cls = NULL;
struct cdev *cdev = NULL;
static void* mem_area = NULL;
static size_t mem_size = 0;
int my_open(struct inode *inode, struct file *filp){
    printk(KERN_ERR "%s:%s:%d\n", __FILE__,__FUNCTION__,__LINE__);
    return 0;
}
long my_ioctl(struct file *filp, unsigned int magic_cmd, unsigned long arg){
    printk(KERN_ERR "%s:%s:%d\n", __FILE__,__FUNCTION__,__LINE__);
    switch (magic_cmd) {
        case IOCTL_ALLOC_MEM: {
            size_t size = (size_t)arg;
            if (mem_area) {
                kfree(mem_area);
            }
            printk(KERN_ERR "Allocating memory of size %zu bytes\n", size);
            mem_area = kmalloc(size, GFP_KERNEL);
            if (!mem_area) {
                return -ENOMEM;
            }
            mem_size = size;
            break;
        }
        case IOCTL_STORE_DATA: {
            size_t size = GET_CMD_SIZE(magic_cmd);
            if (copy_from_user(mem_area, (void*)arg, size)) {
                return -EFAULT;
            }
            break;
        }
        case IOCTL_EXPORT_DATA: {
           size_t size = GET_CMD_SIZE(magic_cmd);
            if (copy_to_user((void*)arg, mem_area, size)) {
                return -EFAULT;
            }
            break;
        }
        default:
            return -EINVAL;
    }
    return 0;
}
int my_release(struct inode *inode, struct file *filp){
    printk(KERN_ERR "%s:%s:%d\n", __FILE__,__FUNCTION__,__LINE__);
    return 0;
}
struct file_operations fops = {
    .open = my_open,
    .unlocked_ioctl = my_ioctl,
    .release = my_release,
};
int create_cdev_device(void){
    int retval = 0;
    cdev = cdev_alloc();
    if(cdev == NULL){
        printk(KERN_ERR "cdev_alloc failed\n");
        return -1;
    }
    if(major > 0){
        retval = register_chrdev_region(MKDEV(major,minor), 1, "mycdev_ioctl");
        if(retval){
            printk(KERN_ERR "register_chrdev_region failed\n");
            return -1;
        }
    }else{
        retval = alloc_chrdev_region(&devno,minor,1,"mycdev_ioctl");
        if(retval){
            printk(KERN_ERR "alloc_chrdev_region failed\n");
            return -1;
        }
        major = MAJOR(devno);
        minor = MINOR(devno);
        printk(KERN_ERR "major: %d minor: %d\n", major, minor);
    }
    cdev_init(cdev,&fops);
    retval = cdev_add(cdev,MKDEV(major,minor),1);
    if(retval){
        printk(KERN_ERR "cdev_add failed\n");
        return retval;
    }
    cls = class_create(THIS_MODULE,"mycdev_ioctl");
    if(IS_ERR(cls)){
        retval = PTR_ERR(cls);
        printk(KERN_ERR "class_create failed\n");
        return retval;
    }
    dev = device_create(cls,NULL,MKDEV(major,minor),NULL,"mycdev_ioctl");
    if(IS_ERR(dev)){
        retval = PTR_ERR(dev);
        printk(KERN_ERR "device_create failed\n");
        return retval;
    }

    return 0;
}
void destory_cdev(void){
    cdev_del(cdev);
    device_destroy(cls,MKDEV(major,minor));
    class_destroy(cls);
    unregister_chrdev_region(MKDEV(major,minor),1);
    kfree(cdev);
    printk(KERN_ERR "%s:%s:%d\n", __FILE__,__FUNCTION__,__LINE__);
}
static int __init mycdev_ioctl_init(void){
    int err = 0;
    err = create_cdev_device();
    if(err){
        printk(KERN_ERR "create_cdev_device failed\n");
        return err;
    }
    return 0;
}
static void __exit mycdev_ioctl_exit(void){
    destory_cdev();
    if(mem_area){
        kfree(mem_area);
    }
}
module_init(mycdev_ioctl_init);
module_exit(mycdev_ioctl_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("2591293022@qq.com");
MODULE_DESCRIPTION("This is a character device driver about IOCTL");
