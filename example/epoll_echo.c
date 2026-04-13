// 一个使用epoll实现的echo服务器，绑定8080端口
// 编译：gcc epoll_echo.c -o epoll_echo
// 执行 ./epoll_echo

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#define PORT 8080
#define MAX_EVENTS 1024
#define BUF_SIZE 1024

static int create_listen_fd(void) {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(listen_fd, 128) == -1) {
        perror("listen");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    return listen_fd;
}

int main(void) {
    int listen_fd = create_listen_fd();

    int epfd = epoll_create1(0);
    if (epfd == -1) {
        perror("epoll_create1");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;   // 监听可读事件
    ev.data.fd = listen_fd;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev) == -1) {
        perror("epoll_ctl: listen_fd");
        close(listen_fd);
        close(epfd);
        exit(EXIT_FAILURE);
    }

    struct epoll_event events[MAX_EVENTS];

    printf("epoll echo server is listening on 127.0.0.1:%d\n", PORT);

    while (1) {
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < nfds; ++i) {
            int fd = events[i].data.fd;

            if (fd == listen_fd) {
                // 有新连接到来
                struct sockaddr_in cli_addr;
                socklen_t cli_len = sizeof(cli_addr);
                int conn_fd = accept(listen_fd, (struct sockaddr *)&cli_addr, &cli_len);
                if (conn_fd == -1) {
                    perror("accept");
                    continue;
                }

                printf("new client: fd=%d, ip=%s, port=%d\n",
                       conn_fd,
                       inet_ntoa(cli_addr.sin_addr),
                       ntohs(cli_addr.sin_port));

                struct epoll_event client_ev;
                client_ev.events = EPOLLIN;   // 只监听读事件
                client_ev.data.fd = conn_fd;

                if (epoll_ctl(epfd, EPOLL_CTL_ADD, conn_fd, &client_ev) == -1) {
                    perror("epoll_ctl: conn_fd");
                    close(conn_fd);
                    continue;
                }
            } else {
                // 客户端套接字可读
                char buf[BUF_SIZE];
                ssize_t n = read(fd, buf, sizeof(buf));

                if (n == -1) {
                    perror("read");
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                } else if (n == 0) {
                    // 客户端关闭连接
                    printf("client fd=%d disconnected\n", fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                } else {
                    // 回显给客户端
                    ssize_t total = 0;
                    while (total < n) {
                        ssize_t m = write(fd, buf + total, n - total);
                        if (m == -1) {
                            perror("write");
                            epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                            close(fd);
                            break;
                        }
                        total += m;
                    }

                    if (total == n) {
                        printf("echo to fd=%d: %.*s\n", fd, (int)n, buf);
                    }
                }
            }
        }
    }

    close(listen_fd);
    close(epfd);
    return 0;
}