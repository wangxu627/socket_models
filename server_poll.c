#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include "socket_hdr.h"
#include "base64.h"

#define IP_ADDRESS "0.0.0.0"
#define PORT		20000
#define BUF_SIZE	64
#define MAX_POLL_SIZE 1024

int find_empty_postion_poll(const struct pollfd* arr, size_t len) {
    for(int i = 0; i < len;i++) {
        if(arr[i].fd == 0) {
            return i;
        }
    }
    return -1;
}

int main() {
    int svr_sock;                //套接字句柄
    int clifd;
    int result;
    int pos;
    char buf[BUF_SIZE];
    char* pB64;
    struct sockaddr_in addr;
    struct pollfd pollfds[MAX_POLL_SIZE];
    int timeout = 5000;
    int yes = 1;
    
    memset(pollfds, 0, sizeof(pollfds));

	if (init_socket() == -1) {
		printf("init socket error");
		return -1;
	}

	if ((svr_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0){
		printf("create socket failed::%d\n", errno);
		return -1;
	}
    
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
    
    pollfds[0].fd = svr_sock;                //设置监控sockfd
    pollfds[0].events = POLLIN | POLLPRI;            //设置监控的事件
    for(;;){
        result = poll(pollfds, MAX_POLL_SIZE, timeout);
        switch(result){        //开始监控
            case -1:                    //函数调用出错
                printf("poll error \r\n");
                break;
            case 0:
                printf("time out \r\n");
                break;
            default:                    //得到数据返回
                for(int i = 0;i < MAX_POLL_SIZE;i++) {
                    if(i == 0 && pollfds[i].revents & POLLIN) {
                        clifd = accept(svr_sock, NULL, NULL);
                        printf("client accpet\n");
                        if((pos = find_empty_postion_poll(pollfds, MAX_POLL_SIZE)) == -1) {
                            printf("poll list full\n");
                        } else {
                            pollfds[pos].fd = clifd;
                            pollfds[pos].events = POLLIN|POLLPRI; 
                        }
                    } else if(i > 0 && pollfds[i].revents & POLLIN){
                        int n = recv(pollfds[i].fd, buf, 64, 0);
                        printf("n ====>> %d\n", n);
                        if(n == 0) {
                            close(pollfds[i].fd);
                            pollfds[i].fd = 0;
                             pollfds[i].revents = 0;
                       } else if(n == -1) {
                            close(pollfds[i].fd);
                            pollfds[i].fd = 0;
                             pollfds[i].revents = 0;
                        } else {
                            buf[n] = 0;
                            pB64 = base64_encode(buf);
				            printf("%s <> %s\n", pB64, buf);
                            sprintf(buf, "%s", base64_decode(pB64));
                            send(pollfds[i].fd, buf, strlen(buf), 0);
                        }
                    }
                }
                break;
        }
    }
}
