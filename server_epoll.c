#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/epoll.h>
#include "socket_hdr.h"
#include "base64.h"

#define IP_ADDRESS "0.0.0.0"
#define PORT		20000
#define BUF_SIZE	64
#define MAX_EPOLL_SIZE 1024



int main() {
    int svr_sock;                //套接字句柄
    int clifd;
    int result;
    int pos;
    char buf[BUF_SIZE];
    char* pB64;
    struct sockaddr_in addr;
    int timeout = 5000;
    int yes = 1;
    struct epoll_event event;
    struct epoll_event* events;
    int efd;
    
	if (init_socket() == -1) {
		printf("init socket error");
		return -1;
	}

	if ((svr_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0){
		printf("create socket failed::%d\n", errno);
		return -1;
	}
    
    efd = epoll_create1(0);
    if (efd == -1){
        printf("create epoll error");
		return -1;
    }
    event.events = EPOLLIN;
    event.data.fd = svr_sock;
    if (epoll_ctl(efd, EPOLL_CTL_ADD, svr_sock, &event) == -1) {
        perror("epoll_ctl: listen_sock");
        return -1;
    }
    events = malloc(MAX_EPOLL_SIZE * sizeof(event));

    setsockopt(svr_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);
    if(bind(svr_sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        printf("bind socket error\n");
        return -1;
    }
    
    if(listen(svr_sock, 5) == -1) {
        printf("listen socket error\n");
        return -1;
    }
    
    for(;;){
        // result = poll(pollfds, MAX_POLL_SIZE, timeout);
        result = epoll_wait(efd, events, MAX_EPOLL_SIZE, timeout);
        switch(result){        //开始监控
            case -1:                    //函数调用出错
                printf("epoll error \r\n");
                break;
            case 0:
                printf("time out \r\n");
                break;
            default:                    //得到数据返回
                for(int i = 0;i < result;i++) {
                    if(events[i].data.fd == svr_sock) {
                        clifd = accept(svr_sock, NULL, NULL);
                        printf("client accpet\n");
                        event.events = EPOLLIN | EPOLLET;
                        event.data.fd = clifd;
                       if (epoll_ctl(efd, EPOLL_CTL_ADD, clifd, &event) == -1) {
                           printf("epoll_ctl: conn_sock");
                           break;
                       }
                    } else if(events[i].events & EPOLLIN){
                        int n = recv(events[i].data.fd, buf, 64, 0);
                        printf("n ====>> %d\n", n);
                        if(n == 0) {
                            // close
                            close(events[i].data.fd);
                       } else if(n == -1) {
                           // error
                            close(events[i].data.fd);
                        } else {
                            buf[n] = 0;
                            pB64 = base64_encode(buf);
				            printf("%s <> %s\n", pB64, buf);
                            sprintf(buf, "%s", base64_decode(pB64));
                            send(events[i].data.fd, buf, strlen(buf), 0);
                        }
                    }
                }
                break;
        }
    }

    free(events);
    close(svr_sock);
    return 0;
}