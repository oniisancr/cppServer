#include <arpa/inet.h>  //这个头文件包含了<netinet/in.h>，不用再次包含了
#include <string.h> //bzero
#include <sys/socket.h>

/*
    客户端只有
        "调用 socket函数创建套接字"
        "调用 connect函数向服务器端发送连接请求"
    这两个步骤
*/
int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in serv_addr;
    // 初始化这个结构体
    bzero(&serv_addr, sizeof(serv_addr));
    // 设置地址族、IP地址和端口
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(8888);
    
    connect(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr));  

    return 0;
}
