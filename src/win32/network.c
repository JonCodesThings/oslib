#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#include <stdbool.h>

#include <stdio.h>

#include <include/oslib/network.h>

typedef struct OSLIB_Socket
{
	SOCKET winsocket;
	bool blocking;
} OSLIB_Socket;

typedef struct OSLIB_NetworkAddress
{
	struct sockaddr_in *in;
} OSLIB_NetworkAddress;

typedef struct OSLIB_HTTPRequestHeader
{
	const char *header;
	const char *value;
	struct OSLIB_HTTPRequestHeader *next;
} OSLIB_HTTPRequestHeader;

typedef struct OSLIB_HTTPRequest
{
	const char *location;
	const char *body;
	enum OSLIB_HTTPRequestMethod method;
	OSLIB_HTTPRequestHeader *headers;
} OSLIB_HTTPRequest;

void InitNetwork()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,0), &wsaData);
}

void CloseNetwork()
{
	WSACleanup();
}

static u32 GetHeadersTotalLength(OSLIB_HTTPRequestHeader* h)
{
	u32 len = 0;
	while (h != NULL)
	{
		//const char *header = h->header + ": " + h->value;
		len += strlen(h->header) + 2 + strlen(h->value);
		h++;
	}
	return len;
}

static void BuildHTTPRequest(OSLIB_HTTPRequest *request)
{
	u32 stringLen = 0;

	// Increment stringLen depending on which request method is being used
	switch (request->method)
	{
	default:
		break;
	case GET:
	case PUT:
		stringLen += 3;
		break;
	case POST:
		stringLen += 4;
		break;
	case PATCH:
		stringLen += 5;
		break;
	case CONNECT:
		stringLen += 7;
		break;
	}
	
	// Increment stringLen with the length of the location string + any other request line stuff
	stringLen += strlen(request->location) + 11;

	// Increment stringLen with the headers total length + the empty line data
	stringLen += GetHeadersTotalLength(request->headers) + 4;

	// Increement stringLen with the request body data + the new line characters;
	stringLen += strlen(request->body) + 2;

	//const char *requestLine = "GET" + request->location + "HTTP/1.1\r\n";

	if (request->headers == NULL)
		return;
	OSLIB_HTTPRequestHeader *h = request->headers;
	while (h != NULL)
	{
		//const char *header = h->header + ": " + h->value;
		h++;
	}
	//const char *newline = "\r\n";
	//const char *emptyline = "\r\n";
	//const char *body = request->body + "\r\n";
}

OSLIB_NetworkAddress *ConfigureNetworkAddress(const char *location, const char *port)
{
	OSLIB_NetworkAddress *addr = Allocate(sizeof(OSLIB_NetworkAddress));

	struct sockaddr_in *hints = Allocate(sizeof(struct sockaddr_in));

	hints->sin_family = AF_INET;
	hints->sin_port = htons(port);
	hints->sin_addr.s_addr = inet_addr(location);

	addr->in = hints;

	return addr;
}

OSLIB_Socket *CreateSocket()
{
	OSLIB_Socket *sock = Allocate(sizeof(OSLIB_Socket));

	struct addrinfo hints;

	ZeroMemory(&hints, sizeof(struct addrinfo));
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	sock->winsocket = INVALID_SOCKET;
	sock->winsocket = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);
	sock->blocking = true;

	if (sock->winsocket == INVALID_SOCKET)
	{
		return NULL;
	}

	return sock;
}

void BindSocket(OSLIB_Socket* socket, OSLIB_NetworkAddress* addr)
{
	i32 res = bind(socket->winsocket, addr->in, sizeof(struct sockaddr_in));

	if (res == SOCKET_ERROR)
	{
		printf("%d\n", WSAGetLastError());
	}

	printf("Server: Receiving IP(s) used: %s\n", inet_ntoa(addr->in->sin_addr));

	printf("Server: Receiving port used: %d\n", htons(addr->in->sin_port));
}

i32 ReceiveData(OSLIB_Socket *sock, u8 *buffer, u32 bufferSize)
{
	i32 res = recvfrom(sock->winsocket, buffer, bufferSize, 0, NULL, NULL);
	if (res == SOCKET_ERROR)
		return 0;
	return res;
}

i32 ReceiveDataFrom(OSLIB_Socket *sock, u8 *buffer, u32 bufferSize)
{
	struct sockaddr_in from;
	i32 siz = sizeof(struct sockaddr_in);

	i32 res = recvfrom(sock->winsocket, buffer, bufferSize, 0, &from, &siz);
	if (res == SOCKET_ERROR)
	{
		printf("%d\n", WSAGetLastError());
		return 0;
	}

	getpeername(sock->winsocket, &from, &siz);

	printf("Receiving from IP used: %s\n", inet_ntoa(from.sin_addr));

	printf("Receiving from port used: %d\n", htons(from.sin_port));
	return res;
}

i32 SendDataTo(OSLIB_Socket *sock, u8 *buffer, u32 bufferSize, struct OSLIB_NetworkAddress *addr)
{
	i32 res = sendto(sock->winsocket, buffer, 1024, 0, addr->in, sizeof(struct sockaddr_in));

	printf("Client: Sending IP(s) used: %s\n", inet_ntoa(addr->in->sin_addr));

	printf("Client: Sending port used: %d\n", htons(addr->in->sin_port));

	if (res == SOCKET_ERROR)
	{
		i32 i = WSAGetLastError();
		switch (i)
		{
		case WSAETIMEDOUT:
			return -1;
		case WSAEHOSTUNREACH:
			return -2;
		case WSAEAFNOSUPPORT:
			return -3;
		case WSAEADDRNOTAVAIL:
			return -4;
		case WSAEDESTADDRREQ:
			return -5;
		case WSAENOBUFS:
			return -6;
		case WSAENETRESET:
			return -7;
		case WSAEINVAL:
			return -8;
		default:
			break;
		}
	}

	return res;
}

i32 SendDataToAll(OSLIB_Socket *sock, u8 *buffer, u32 bufferSize)
{
	return send(sock->winsocket, buffer, bufferSize, bufferSize);
}

void CloseSocket(OSLIB_Socket* sock)
{
	closesocket(sock->winsocket);
}