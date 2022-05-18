#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h> //memset bzero
#include <unistd.h> //read write
#include "util.h"
#include <sys/epoll.h> //linux独有
#include <fcntl.h>
#include <errno.h> //EAGAIN EINTR errno

#define MAX_EVENTS 1024
#define READ_BUFFER 1024

void setnonblocking(int fd)
{
    // F_SETFL设置文件状态标记
    // 获取文件状态标记F_GETFL
    // O_NONBLOCK 非阻塞I/O
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

// 使用epoll 实现I/O复用 参考视频https://www.bilibili.com/video/BV1qJ411w7du
/*
   1. epoll_create  创建一个白板 存放fd_events
   2. epoll_ctl 用于向内核注册新的描述符或者是改变某个文件描述符的状态。
   3. 调用 epoll_wait() 可以得到事件完成的文件描述符数目
基本思想为：
    在创建了服务器socket fd后，将这个fd添加到epoll，只要这个fd上发生可读事件，
    表示有一个新的客户端连接。然后accept这个客户端并将客户端的socket fd添加到epoll，
    epoll会监听客户端socket fd是否有事件发生，如果发生则处理事件。
*/

int main()
{
    // 服务器socket fd 欢迎套接字
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    errif(sockfd == -1, "socket create error");

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(8888);
    errif(bind(sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) == -1, "socket bind error");
    errif(listen(sockfd, SOMAXCONN) == -1, "socket listen error");

    // 创建一个epoll文件描述符
    int epfd = epoll_create1(0);
    errif(epfd == -1, "epoll create error");

    struct epoll_event events[MAX_EVENTS], ev;
    bzero(&events, sizeof(events));
    bzero(&ev, sizeof(ev));

    ev.data.fd = sockfd;
    // EPOLLIN 事件：有新的连接请求；接收到普通数据（且接收缓冲区没满）；客户端正常关闭连接
    // EPOLLET 边缘触发模式，但必须搭配非阻塞式socket使用。 从无事件到有事件的变化会通知内核一次，之后就不会再次通知内核。
    ev.events = EPOLLIN | EPOLLET; //监听事件

    setnonblocking(sockfd);
    // epoll监听事件的描述符会放在一颗红黑树上
    // 将要监听的IO口放入epoll红黑树中，就可以监听该IO上的事件
    epoll_ctl(epfd, EPOLL_CTL_ADD, ev.data.fd, &ev);
    
    while (true)
    {
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        errif(nfds == -1, "epoll wait error");
        for (int i = 0; i < nfds; ++i)
        {
            if (events[i].data.fd == sockfd) //发生事件的fd是服务器socket fd，表示有新客户端连接
            {
                // 创建对应的连接套接字
                struct sockaddr_in clnt_addr;
                memset(&clnt_addr, 0, sizeof(clnt_addr));
                socklen_t clnt_addr_len = sizeof(clnt_addr);

                int clnt_sockfd = accept(sockfd, (sockaddr *)&clnt_addr, &clnt_addr_len);
                errif(clnt_sockfd == -1, "socket accept error");
                printf("new client fd %d! IP: %s Port: %d\n", clnt_sockfd, inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));

                bzero(&ev, sizeof(ev));
                ev.data.fd = clnt_sockfd;
                ev.events = EPOLLIN | EPOLLET;
                setnonblocking(clnt_sockfd);
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sockfd, &ev);
            }
            else if (events[i].events & EPOLLIN) //发生事件的是客户端，并且是可读事件
            {
                char buf[READ_BUFFER];
                while (true)
                { //由于使用非阻塞IO，读取客户端buffer，一次读取buf大小数据，直到全部读取完毕
                    bzero(&buf, sizeof(buf));
                    ssize_t bytes_read = read(events[i].data.fd, buf, sizeof(buf));
                    if (bytes_read > 0)
                    {
                        printf("message from client fd %d: %s\n", events[i].data.fd, buf);
                        write(events[i].data.fd, buf, bytes_read);
                    }
                    else if (bytes_read == -1 && errno == EINTR)
                    { //客户端正常中断、继续读取
                        printf("continue reading");
                        continue;
                    }
                    else if (bytes_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
                    { //非阻塞IO，这个条件表示数据全部读取完毕
                        printf("finish reading once, errno: %d\n", errno);
                        break;
                    }
                    else if (bytes_read == 0)
                    { // EOF，客户端断开连接
                        printf("EOF, client fd %d disconnected\n", events[i].data.fd);
                        close(events[i].data.fd); //关闭socket会自动将文件描述符从epoll树上移除
                        break;
                    }
                }
            }
            else
            { //其他事件，之后的版本实现
                printf("something else happened\n");
            }
        }
    }
    close(sockfd);
    return 0;
}