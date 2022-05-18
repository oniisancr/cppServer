#include <arpa/inet.h>
#include <string.h> //bzero
#include <sys/socket.h>
#include <unistd.h> //read write
#include "util.h"
#include <stdio.h>

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    errif(sockfd == -1, "socket create error");

    struct sockaddr_in serv_addr;
    // 初始化这个结构体
    bzero(&serv_addr, sizeof(serv_addr));
    // 设置地址族、IP地址和端口
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(8888);

    errif(connect(sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) == -1, "socket connect error");

    while (true)
    {
        char buf[1024]; //定义缓冲区
        bzero(&buf, sizeof(buf));
        scanf("%s", buf);   //要传到服务器的数据
        ssize_t write_bytes = write(sockfd, buf, sizeof(buf)); //阻塞 返回已发送数据大小
        if (write_bytes == -1)
        {
            printf("socket already disconnected, can't write any more!\n");
            break;
        }
        bzero(&buf, sizeof(buf));
        ssize_t read_bytes = read(sockfd, buf, sizeof(buf)); //阻塞 从服务器socket读到缓冲区，返回已读数据大小
        if (read_bytes > 0)
        {
            printf("message from server: %s\n", buf);
        }
        else if (read_bytes == 0) // read返回0，表示EOF
        {
            printf("server socket disconnected!\n");
            break;
        }
        else if (read_bytes == -1)
        {
            close(sockfd);
            errif(true, "socket read error");
        }
    }
    close(sockfd);

    return 0;
}
