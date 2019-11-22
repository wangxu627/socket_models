#include "socket_hdr.h"
#include <errno.h>
#include <stddef.h>

int init_socket() {
#ifdef _WIN32
	WSADATA  ws;
	if (WSAStartup(MAKEWORD(2, 2), &ws) != 0) {
		printf("init windows socket failed::%d\n", errno);
		return -1;
	}
#else
	return 0;
#endif
}

#ifdef _WIN32
int close(int socket) {
	closesocket(socket);
}
#endif

int find_empty_postion(int arr[], size_t size) {
	int i = 0;
	for (; i < size; i++) {
		if (arr[i] == 0) {
			return i;
		}
	}
	return -1;
}