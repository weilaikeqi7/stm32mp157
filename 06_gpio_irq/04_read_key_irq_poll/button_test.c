
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <poll.h>

int main(int argc, char **argv)
{   
    int fd;
    int val;
    struct pollfd fds[1];
    int timeout_ms = 5000;
    int ret;

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

    fds[0].fd = fd;
    fds[0].events = POLLIN;

    while (1)
    {
        ret = poll(fds, 1, timeout_ms);
        if ((ret == 1) && (fds[0].revents & POLLIN))
        {
            read(fd, &val, 4);
            printf("get button : 0x%x\n", val);
        }
        else
        {
            printf("timeout\n");
        }
    }

    close(fd);

    return 0;
}