// voip_client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#pragma comment(lib, "ws2_32.lib")

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <stdio.h>
#include <iostream>

#define SERVER_IP "127.0.0.1"
#define RECEIVING_PORT 9696
#define SEND_PORT 6969

#define MAXLEN	65000

DWORD WINAPI ReceiverThreadFunc(LPVOID lpParameter);
DWORD WINAPI SenderThreadFunc(LPVOID lpParameter);


int main()
{
	WSADATA stWSAData;
	WORD wVersionRequested = MAKEWORD(2, 2);

	// Initialize the DLL with version Winsock 2.2
	if (WSAStartup(wVersionRequested, &stWSAData) != 0) 
	{
		printf("%d\n", WSAGetLastError());
		exit(1);
	}

	HANDLE ReceiverThread;
	DWORD ReceiverThreadId;
	if ((ReceiverThread = CreateThread(NULL, 0, ReceiverThreadFunc, (LPVOID)0, 0, &ReceiverThreadId)) == NULL)
	{
		printf("CreateThread failed with error %d\n", GetLastError());
		return 1;
	}

	HANDLE SenderThread;
	DWORD SenderThreadId;
	if ((SenderThread = CreateThread(NULL, 0, SenderThreadFunc, (LPVOID)0, 0, &SenderThreadId)) == NULL)
	{
		printf("CreateThread failed with error %d\n", GetLastError());
		return 1;
	}

	WaitForSingleObject(ReceiverThread, INFINITE);
	WaitForSingleObject(SenderThread, INFINITE);

	WSACleanup();  
	exit(0);
}

DWORD WINAPI ReceiverThreadFunc(LPVOID lpParameter)
{
	SOCKET sock;
	struct sockaddr_in server;
	int data_size;
	struct sockaddr_in client;
	char buf[MAXLEN];

	// Create a datagram socket
	if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("Can't create a socket");
		exit(1);
	}

	// Bind an address to the socket
	memset((char *)&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(RECEIVING_PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sock, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		perror("Can't bind name to socket");
		exit(1);
	}

	while (TRUE) 
	{
		int client_len = sizeof(client);
		if ((data_size = recvfrom(sock, buf, MAXLEN, 0, (struct sockaddr *)&client, &client_len)) < 0)
		{
			perror("recvfrom error");
			exit(1);
		}

		printf("Received %d bytes\t", data_size);
		printf("From host: %s\n", inet_ntoa(client.sin_addr));
	}

	closesocket(sock);
}

DWORD WINAPI SenderThreadFunc(LPVOID lpParameter)
{
	SOCKET sock;
	char buf[MAXLEN] = "hello";
	int data_size = 64;
	struct sockaddr_in client;
	HOSTENT *hp;

	// Create a datagram socket
	if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("Can't create a socket");
		exit(1);
	}

	memset((char *)&client, 0, sizeof(client));
	client.sin_family = AF_INET;
	client.sin_port = htons(SEND_PORT);

	int client_len = sizeof(client);

	if ((hp = gethostbyname("127.0.0.1")) == NULL)
	{
		fprintf(stderr, "Can't get server's IP address\n");
		exit(1);
	}

	memcpy((char *)&client.sin_addr, hp->h_addr, hp->h_length);

	while (TRUE)
	{
		if (sendto(sock, buf, data_size, 0, (struct sockaddr *)&client, client_len) != data_size)
		{
			perror("sendto error");
			exit(1);
		}
		printf("Sent data");
	}

	closesocket(sock);
}


