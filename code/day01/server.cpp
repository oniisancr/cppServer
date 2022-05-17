#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h> //memset

/*
    服务器端
        "调用 socket函数创建服务端套接字"
        "调用 bind函数分配ip和端口"
        "调用 listen函数将套接字设置为可接收状态"
        "调用 socket函数创建保存客户端socket的套接字"
        "调用 accept函数接受连接"
    这两个步骤
*/

int main()
{
    // 文件描述符
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // 套接字信息
    struct sockaddr_in serv_addr;
    // 初始化
    memset(&serv_addr,0,sizeof(serv_addr));
    // 设置地址族、IP地址和端口
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(8888);
    // 将socket地址与文件描述符绑定
    bind(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr));
    // 最大监听队列长度SOMAXCONN
    listen(sockfd, SOMAXCONN);

    //接受连接时需要保存客户端的socket地址信息
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_len = sizeof(clnt_addr);
    bzero(&clnt_addr, sizeof(clnt_addr));
    // accept函数会阻塞当前程序，直到有一个客户端socket被接受后程序才会往下运行
    int clnt_sockfd = accept(sockfd, (sockaddr*)&clnt_addr, &clnt_addr_len);
    printf("new client fd %d! IP: %s Port: %d\n", clnt_sockfd, inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));
    
    return 0;
}