#include <stdio.h>
#include <time.h>
#include <errno.h>
#include "socket_hdr.h"

#define IP_ADDRESS "0.0.0.0"
#define PORT		20000
#define BUF_SIZE	64
#define MAX(x, y)	((x) > (y) ? (x) : (y))

int main() {
	int svr_sock;
	int cli_sock;
	struct sockaddr_in addr;
	int socks[FD_SETSIZE];
	char buf[BUF_SIZE];
	int optval = 1;
	int maxfds = 0;
	int result = -1;
	int pos = -1;
	int i, n = 0;
	fd_set fds_read;

	FD_ZERO(&fds_read);
	memset(&socks, 0, sizeof(socks));

	if (init_socket() == -1) {
		printf("init socket error");
		return -1;
	}

	if ((svr_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0){
		printf("create socket failed::%d\n", errno);
		return -1;
	}
	socks[0] = svr_sock;

	if (setsockopt(svr_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
		printf("setsockopt failed::%d\n", errno);
		return -1;
	}
	
	memset(&addr, 0x00, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
	addr.sin_port = htons(PORT);

	if (bind(svr_sock, (struct sockaddr*)&addr, sizeof(addr)) == -1){
		printf("bind error::%d\n", errno);
		return -1;
	}

	if (listen(svr_sock, 5) == -1) {
		printf("listen error::%d\n", errno);
		return -1;
	}


	while (1) {
		maxfds = -1;
		FD_ZERO(&fds_read);
		for (i = 0; i < FD_SETSIZE; i++) {
			if (socks[i] != 0) {
				FD_SET(socks[i], &fds_read);
				maxfds = MAX(socks[i], maxfds);
			}
		}

		result = select(maxfds + 1, &fds_read, NULL, NULL, NULL);
		for (i = 0; i < FD_SETSIZE; i++) {
			if (socks[i] == 0) {
				continue;
			}

			if (FD_ISSET(socks[i], &fds_read)) {
				if (i == 0) {
					cli_sock = accept(socks[i], NULL, NULL);
					if ((pos = find_empty_postion(socks, FD_SETSIZE)) == -1) {
						printf("fdset is full, can not accept!\n");
					} else {
						socks[pos] = cli_sock;
					}
				} else {
					// can only receive less than BUF_SIZE bytes
					n = recv(socks[i], buf, BUF_SIZE, 0);
					if (n == -1) {
						printf("recv error::%d\n", errno);
					} else if (n == 0) {
						printf("client closed\n");
						close(socks[i]);
						socks[i] = 0;
					} else if (n > 0) {
						send(socks[i], buf, n, 0);
					}
				}
			}
		}
	}
}