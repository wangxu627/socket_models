#include <stdio.h>
#include <time.h>
#include <errno.h>
#include "socket_hdr.h"
#include "base64.h"

#define IP_ADDRESS "0.0.0.0"
#define PORT		20000
#define BUF_SIZE	64

typedef enum
{
	soREVC,
	soSEND,
}SOCKETOPERATE;

typedef struct 
{
	WSAOVERLAPPED		overlapped;
	WSABUF              buf;
	char                sMessage[BUF_SIZE];
	DWORD               dwBytes;
	DWORD				flag;
	SOCKETOPERATE		socketType;
}SOCKETDATA;

void Clear(SOCKETDATA* pData, SOCKETOPERATE type)
{
	ZeroMemory(pData, sizeof(SOCKETDATA));
	pData->buf.buf = pData->sMessage;
	pData->buf.len = BUF_SIZE;
	pData->socketType = type;
}


DWORD WINAPI SocketProcMain(LPVOID pParam)
{
	HANDLE hIocp = (HANDLE)pParam;
	DWORD dwBytes;
	SOCKETDATA *lpSocketData;
	SOCKET clientSocket;
	const char* pB64;

	while (1)
	{
		GetQueuedCompletionStatus(hIocp, &dwBytes, (PULONG_PTR)&clientSocket, (LPOVERLAPPED*)&lpSocketData, INFINITE);
		if (dwBytes == 0xFFFFFFFF)
		{
			return 0;
		}

		if (lpSocketData->socketType == soREVC)
		{
			if (dwBytes == 0)
			{
				close(clientSocket);
				free(lpSocketData);
			}
			else
			{
				lpSocketData->sMessage[dwBytes] = 0;
				printf("%x\t:%s\n", (DWORD)clientSocket, lpSocketData->sMessage);
				pB64 = base64_encode(lpSocketData->sMessage);
				printf("%s\n", pB64);
				Clear(lpSocketData, soSEND);
				sprintf_s(lpSocketData->sMessage, BUF_SIZE, base64_decode(pB64));
				lpSocketData->dwBytes = strlen(lpSocketData->sMessage);
				WSASend(clientSocket, &lpSocketData->buf, 1, &lpSocketData->dwBytes, NULL, &lpSocketData->overlapped, NULL);
			}
		}
		else if (lpSocketData->socketType == soSEND) {
			Clear(lpSocketData, soREVC);
			WSARecv(clientSocket, &lpSocketData->buf, 1, &lpSocketData->dwBytes, &lpSocketData->flag, &lpSocketData->overlapped, NULL);
		}
	}
}

int main()
{
	int svr_sock;
	int cli_sock;
	struct sockaddr_in addr;
	char optval = 1;
	int i = 0;
	HANDLE hIocp;
	SYSTEM_INFO systemInfo;

	if (init_socket() == -1) {
		printf("init socket error");
		return -1;
	}

	if ((svr_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		printf("create socket failed::%d\n", errno);
		return -1;
	}

	if (setsockopt(svr_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval)) == -1) {
		printf("setsockopt failed::%d\n", errno);
		return -1;
	}

	memset(&addr, 0x00, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
	addr.sin_port = htons(PORT);

	if (bind(svr_sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("bind error::%d\n", errno);
		return -1;
	}

	if (listen(svr_sock, 5) == -1) {
		printf("listen error::%d\n", errno);
		return -1;
	}

	hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	GetSystemInfo(&systemInfo);
	for (i = 0; i < systemInfo.dwNumberOfProcessors; i++)
	{
		CreateThread(NULL, NULL, &SocketProcMain, hIocp, NULL, NULL);
	}

	while (1)
	{
		cli_sock = accept(svr_sock, NULL, NULL);
		printf("client accept\n");
		CreateIoCompletionPort((HANDLE)cli_sock, hIocp, (DWORD)cli_sock, INFINITE);
		SOCKETDATA *lpSocketData = (SOCKETDATA*)malloc(sizeof(SOCKETDATA));
		Clear(lpSocketData, soREVC);
		WSARecv(cli_sock, &lpSocketData->buf, 1, &lpSocketData->dwBytes, &lpSocketData->flag, &lpSocketData->overlapped, NULL);
	}

	CloseHandle(hIocp);
	close(svr_sock);
	WSACleanup();
	return 0;
}