#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#include <include/oslib/platform.h>

typedef struct OSLIB_Socket
{
	SOCKET winsocket;
} OSLIB_Socket;

typedef struct OSLIB_NetworkAddress
{
	struct addrinfo *info;
} OSLIB_NetworkAddress;

void InitNetwork()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,0), &wsaData);
}

void CloseNetwork()
{
	WSACleanup();
}

OSLIB_NetworkAddress *ConfigureNetworkAddress(const char *location, const char *port)
{
	OSLIB_NetworkAddress *addr = Allocate(sizeof(OSLIB_NetworkAddress));

	struct addrinfo hints;
	struct addrinfo *result = NULL;

	ZeroMemory(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(location, port, &hints, &result);

	addr->info = result;

	return addr;
}

OSLIB_Socket *CreateSocket()
{
	OSLIB_Socket *sock = Allocate(sizeof(OSLIB_Socket));

	struct addrinfo hints;

	ZeroMemory(&hints, sizeof(struct addrinfo));
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	sock->winsocket = INVALID_SOCKET;
	sock->winsocket = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);

	if (sock->winsocket == INVALID_SOCKET)
	{
		return NULL;
	}

	return sock;
}

void ConnectSocket(OSLIB_Socket *sock, OSLIB_NetworkAddress *addr)
{
	i32 res = connect(sock->winsocket, addr->info->ai_addr, (i32)addr->info->ai_addrlen);

	if (res == SOCKET_ERROR)
	{
		closesocket(sock->winsocket);
		return;
	}
}

void ReceiveData(OSLIB_Socket *sock, u8 *buffer, u32 bufferSize)
{
	send(sock->winsocket, buffer, bufferSize, 0);
}

void SendData(OSLIB_Socket *sock, u8 *buffer, u32 bufferSize)
{
	recvfrom(sock->winsocket, buffer, bufferSize, 0, NULL, NULL);
}

void CloseSocket(OSLIB_Socket* sock)
{
	closesocket(sock->winsocket);
}