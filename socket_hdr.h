#ifndef __SOCKET_HDR_H__
#define __SOCKET_HDR_H__


#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "WS2_32.lib")
extern int close(int socket);
#endif // _WIN32_WINDOWS

extern int init_socket();
extern int find_empty_postion(int arr[], size_t size);


#endif