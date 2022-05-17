#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h> //memset bzero
#include <unistd.h> //read write
#include "util.h"

int main()
{
    // 文件描述符
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    errif(sockfd == -1, "socket create error");

    // 套接字信息
    struct sockaddr_in serv_addr;
    // 初始化
    memset(&serv_addr, 0, sizeof(serv_addr));
    // 设置地址族、IP地址和端口
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(8888);
    // 将socket地址与文件描述符绑定
    errif(bind(sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) == -1, "socket bind error");
    errif(listen(sockfd, SOMAXCONN) == -1, "socket listen error");

    //接受连接时需要保存客户端的socket地址信息
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_len = sizeof(clnt_addr);
    bzero(&clnt_addr, sizeof(clnt_addr));
    // accept函数会阻塞当前程序，直到有一个客户端socket被接受后程序才会往下运行
    int clnt_sockfd = accept(sockfd, (sockaddr *)&clnt_addr, &clnt_addr_len);
    errif(clnt_sockfd == -1, "socket accept error");
    printf("new client fd %d! IP: %s Port: %d\n", clnt_sockfd, inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));
    while (true)
    {
        char buf[1024];
        bzero(&buf, sizeof(buf));
        ssize_t read_bytes = read(clnt_sockfd, buf, sizeof(buf));
        if (read_bytes > 0)
        {
            printf("message from client fd %d: %s\n", clnt_sockfd, buf);
            // write(clnt_sockfd, buf, sizeof(buf));
            write(clnt_sockfd, buf, read_bytes);
        }
        else if (read_bytes == 0)
        {
            printf("client fd %d disconnected\n", clnt_sockfd);
            close(clnt_sockfd);
            break;
        }
        else if (read_bytes == -1)
        {
            close(clnt_sockfd);
            errif(true, "socket read error");
        }
    }
    close(sockfd);
    return 0;
}