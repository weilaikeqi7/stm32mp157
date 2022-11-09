#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{   
    int fd;
    int val;

    if (argc != 2)
    {
        printf("Usage: %s <dev>\n", argv[0]);
        return -1;
    }

    fd = open(argv[1], O_RDWR);
    if (-1 == fd)
    {
        printf("can not open file %s\n", argv[1]);
        return -1;
    }

    while (1)
    {
        read(fd, &val, 4);
        printf("get button : 0x%x\n", val);
    }

    close(fd);

    return 0;
}