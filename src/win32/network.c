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

typedef struct OSLIB_HTTPHeader
{
	const char *header;
	const char *value;
	struct OSLIB_HTTPHeader *next;
} OSLIB_HTTPHeader;

typedef struct OSLIB_HTTPRequest
{
	const char *location;
	const char *body;
	enum OSLIB_HTTPRequestMethod method;
	OSLIB_HTTPHeader *headers;
} OSLIB_HTTPRequest;

typedef struct OSLIB_HTTPResponse
{
	i32 responseCode;
	const char *responseString;
	OSLIB_HTTPHeader *headers;
	const char *messageBody;
} OSLIB_HTTPResponse;

static enum HTTP_TOKENS
{
	HTTP,
	HTTP_VERSION,
	HTTP_RESPONSE_CODE,
	HTTP_RESPONSE_STRING,
	HEADER,
	COLON,
	HEADER_VALUE,
	BODY,
	SEPERATOR
};

void InitNetwork()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,0), &wsaData);
}

void CloseNetwork()
{
	WSACleanup();
}

static u32 GetHeadersTotalLength(const OSLIB_HTTPHeader * h)
{
	u32 len = 0;
	while (h != NULL)
	{
		//const char *header = h->header + ": " + h->value;
		len += strlen(h->header) + 2 + strlen(h->value);
		h = h->next;
	}
	return len;
}

static u32 GetHTTPRequestStringLength(const OSLIB_HTTPRequest *const request)
{
	u32 stringLen = 0;

	// Increment stringLen depending on which request method is being used
	switch (request->method)
	{
	default:
		break;
	case HTTP_GET:
	case HTTP_PUT:
		stringLen += 3;
		break;
	case HTTP_POST:
		stringLen += 4;
		break;
	case HTTP_PATCH:
		stringLen += 5;
		break;
	case HTTP_DELETE:
		stringLen += 6;
	case HTTP_CONNECT:
		stringLen += 7;
		break;
	}

	// Increment stringLen with the length of the location string + any other request line stuff
	stringLen += strlen(request->location) + 11;

	// Increment stringLen with the headers total length + the empty line data
	stringLen += GetHeadersTotalLength(request->headers) + 4;

	// Increement stringLen with the request body data + the new line characters;
	stringLen += strlen(request->body) + 2;

	return stringLen;
}

static const char* GetHTTPRequestMethod(const OSLIB_HTTPRequest* const request)
{
	switch (request->method)
	{
	default:
		return "";
		break;
	case HTTP_GET:
		return "GET";
	case HTTP_PUT:
		return "PUT";
	case HTTP_POST:
		return "POST";
	case HTTP_PATCH:
		return "PATCH";
	case HTTP_CONNECT:
		return "CONNECT";
	case HTTP_DELETE:
		return "DELETE";
	}
}

static void AppendHeaders(char* string, u32 *currentIndex, const OSLIB_HTTPHeader* h)
{
	const char * colon = ": ";
	while (h != NULL)
	{
		u32 headerLen = strlen(h->header);
		u32 valueLen = strlen(h->value);
		memcpy(&string[*currentIndex], h->header, sizeof(char) * headerLen);
		*currentIndex += headerLen;
		memcpy(&string[*currentIndex], colon, sizeof(char) * 2);
		*currentIndex += 2;
		memcpy(&string[*currentIndex], h->value, sizeof(char) * valueLen);
		*currentIndex += valueLen;
		h = h->next;
	}
}

static void CopyToString(char* buffer, u32 *current, const char* copy, const u32 size)
{
	memcpy(&buffer[*current], copy, sizeof(char) * size);
	*current += size;
}

static const char * BuildHTTPRequest(const OSLIB_HTTPRequest *const request)
{
	u32 stringLen = GetHTTPRequestStringLength(request);

	char *requestString = Allocate(sizeof(char) * stringLen + 1);
	requestString[stringLen] = '\0';

	u32 current = 0;

	const char *method = GetHTTPRequestMethod(request);
	u32 methodLen = strlen(method);

	CopyToString(requestString, &current, method, methodLen);

	methodLen = strlen(request->location);
	CopyToString(requestString, &current, request->location, methodLen);

	const char *http = "HTTP/1.1";
	methodLen = strlen(http);
	CopyToString(requestString, &current, http, methodLen);

	const char *newlineChars = "\r\n";

	CopyToString(requestString, &current, newlineChars, 2);

	AppendHeaders(requestString, current, request->headers);

	CopyToString(requestString, &current, newlineChars, 2);

	CopyToString(requestString, &current, newlineChars, 2);

	methodLen = strlen(request->body);
	CopyToString(requestString, &current, request->body, methodLen);

	CopyToString(requestString, &current, newlineChars, 2);

	return requestString;
}

void ParseHTTPResponse(const char* responseData, OSLIB_HTTPResponse* response)
{
	if (responseData[0] != 'H' || responseData[1] != 'T' || responseData[2] != 'T' || responseData[3] != 'P')
		return;

	response->messageBody = NULL;
	response->responseCode = 0;
	response->responseString = NULL;
	response->headers = NULL;

	u32 responseDataLen = strlen(responseData);
	char accum[32];
	u32 accumIndex = 0;
	enum HTTP_TOKENS prev;
	OSLIB_HTTPHeader *currentHeader = NULL;
	for (u32 i = 0; i < responseDataLen; ++i)
	{
		accum[accumIndex++] = responseData[i];
		accum[accumIndex] = '\0';

		if (!strcmp(accum, "\r\n"))
		{
			accumIndex = 0;
			prev = SEPERATOR;
		}

		if (!strcmp(accum, "HTTP"))
		{
			accumIndex = 0;
			prev = HTTP;
			continue;
		}

		if (accumIndex == 1 && accum[accumIndex - 1] == '/' && prev == HTTP)
		{
			accumIndex = 0;
			continue;
		}

		if (accumIndex > 0)
		{
			if (accum[accumIndex - 1] == ' ' || accum[accumIndex - 1] == '\r\n')
			{
				u32 strl = strlen(accum);
				char *buff = NULL;
				switch (prev)
				{
				default:
					accumIndex = 0;
					break;
				case HTTP:
					prev = HTTP_VERSION;
					break;
				case HTTP_VERSION:
					response->responseCode = atoi(accum);
					prev = HTTP_RESPONSE_CODE;
					accumIndex = 0;
					break;
				case HTTP_RESPONSE_CODE:
					u32 responseStrLen = strlen(accum);
					buff = Allocate(sizeof(char) * responseStrLen);
					buff[responseStrLen] = '\0';
					memcpy(buff, responseStrLen, sizeof(char) * responseStrLen);
					response->responseString = buff;
					prev = HTTP_RESPONSE_STRING;
					break;
				case HEADER:
					if (!strcmp(accum, ":"))
						prev = COLON;
					break;
				case COLON:
					buff = Allocate(sizeof(char) * strl);
					memcpy(buff, accum, sizeof(char) * strl);
					currentHeader->value = buff;
					break;
					break;
				case SEPERATOR:
					if (response->headers != NULL)
					{
						buff = Allocate(sizeof(char) * strl);
						memcpy(buff, accum, sizeof(char) * strl);
						response->messageBody = buff;
						break;
					}
				case HEADER_VALUE:
					if (response->responseCode == 0)
						break;
					OSLIB_HTTPHeader *header = Allocate(sizeof(OSLIB_HTTPHeader));
					header->next = NULL;
					buff = Allocate(sizeof(char) * strl);
					memcpy(buff, accum, sizeof(char) * strl);
					header->header = buff;
					prev = HEADER;
					currentHeader->next = header;
					currentHeader = header;
					if (response->headers == NULL)
						response->headers = header;
					break;					
				}
			}
		}
	}
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