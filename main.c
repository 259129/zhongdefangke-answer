#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#define DEVICE "/dev/mycdev_ioctl"
#define IOCTL_ALLOC_MEM _IOW('x', 1, size_t)
#define IOCTL_STORE_DATA _IOW('x', 2, size_t)
#define IOCTL_EXPORT_DATA _IOR('x', 3, size_t)
int main(int argc, char const *argv[])
{
    int fd;
    size_t size;
    char *buffer;
    // 打开设备文件
    fd = open(DEVICE,O_RDWR);
    if(fd == -1){
        perror("open");
        return -1;
    }
    // 读取文件大小
    FILE *file = fopen("hello1", "rb");
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    printf("size = %ld\n", size);
    // 申请内核内存
    if (ioctl(fd, IOCTL_ALLOC_MEM, size) < 0) {
        perror("Failed to allocate memory...");
        close(fd);
        return -1;
    }

    // 读取文件内容到buffer
    buffer = (char*)malloc(size);
    fread(buffer, 1, size, file);
    fclose(file);

    // 将文件内容写入内核内存
    if (ioctl(fd, IOCTL_STORE_DATA, (size_t)buffer) < 0) {
        perror("Failed to store data...");
        free(buffer);
        close(fd);
        return -1;
    }

    // 从内核内存读取数据
    if (ioctl(fd, IOCTL_EXPORT_DATA, (size_t)buffer) < 0) {
        perror("Failed to export data...");
        free(buffer);
        close(fd);
        return -1;
    }

    // 将数据写入到另一个文件
    file = fopen("hello2", "wb");
    fwrite(buffer, 1, size, file);
    fclose(file);
    free(buffer);
    return 0;
}
