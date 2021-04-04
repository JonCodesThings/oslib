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
	enum OSLIB_SocketProtocol protocol;
} OSLIB_Socket;

typedef struct OSLIB_NetworkAddress
{
	struct sockaddr_in *in;
	struct addrinfo *info;
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

enum HTTP_TOKENS
{
	HTTP_TOKEN_NONE,
	HTTP,
	HTTP_VERSION,
	HTTP_RESPONSE_CODE,
	HTTP_RESPONSE_STRING,
	HEADER,
	COLON,
	HEADER_VALUE,
	BODY,
	SEPARATOR,
	DOUBLE_SEPARATOR
};

static bool networkInit = false;

static void InitNetwork()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,0), &wsaData);
}

static void CloseNetwork()
{
	WSACleanup();
}

static u32 GetHeadersTotalLength(const OSLIB_HTTPHeader * h)
{
	u32 len = 0;
	while (h != NULL)
	{
		//const char *header = h->header + ": " + h->value;
		len += (u32)strlen(h->header) + 2 + (u32)strlen(h->value);
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
		stringLen += 4;
		break;
	case HTTP_POST:
		stringLen += 5;
		break;
	case HTTP_PATCH:
		stringLen += 6;
		break;
	case HTTP_DELETE:
		stringLen += 7;
	case HTTP_CONNECT:
		stringLen += 8;
		break;
	}

	// Increment stringLen with the length of the location string + any other request line stuff
	stringLen += (u32)strlen(request->location) + 11;

	// Increment stringLen with the headers total length + the empty line data
	stringLen += GetHeadersTotalLength(request->headers) + 4;

	// Increement stringLen with the request body data + the new line characters;
	stringLen += (u32)strlen(request->body) + 2;

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
		return "GET ";
	case HTTP_PUT:
		return "PUT ";
	case HTTP_POST:
		return "POST ";
	case HTTP_PATCH:
		return "PATCH ";
	case HTTP_CONNECT:
		return "CONNECT ";
	case HTTP_DELETE:
		return "DELETE ";
	}
}

static void AppendHeaders(char* string, u32 *currentIndex, const OSLIB_HTTPHeader* h)
{
	const char * colon = ": ";
	while (h != NULL)
	{
		u32 headerLen = (u32)strlen(h->header);
		u32 valueLen = (u32)strlen(h->value);
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

static const char * BuildHTTPRequestString(const OSLIB_HTTPRequest *const request)
{
	u32 stringLen = GetHTTPRequestStringLength(request);

	char *requestString = Allocate(sizeof(char) * stringLen + 1);
	requestString[stringLen] = '\0';

	u32 current = 0;

	const char *method = GetHTTPRequestMethod(request);
	u32 methodLen = (u32)strlen(method);

	CopyToString(requestString, &current, method, methodLen);

	methodLen = (u32)strlen(request->location);
	CopyToString(requestString, &current, request->location, methodLen);

	const char *http = " HTTP/1.1";
	methodLen = (u32)strlen(http);
	CopyToString(requestString, &current, http, methodLen);

	const char *newlineChars = "\r\n";

	CopyToString(requestString, &current, newlineChars, 2);

	AppendHeaders(requestString, &current, request->headers);

	CopyToString(requestString, &current, newlineChars, 2);

	CopyToString(requestString, &current, newlineChars, 2);

	methodLen = (u32)strlen(request->body);
	CopyToString(requestString, &current, request->body, methodLen);

	CopyToString(requestString, &current, newlineChars, 2);

	return requestString;
}

OSLIB_HTTPRequest *BuildHTTPRequest(enum OSLIB_HTTPRequestMethod method, const char *location, const char *body)
{
	OSLIB_HTTPRequest *request = Allocate(sizeof(OSLIB_HTTPRequest));
	request->method = method;
	request->location = location;
	request->body = body;
	request->headers = NULL;
}

void AddHTTPHeader(OSLIB_HTTPRequest * const request, const char* header, const char* value)
{
	OSLIB_HTTPHeader *h = Allocate(sizeof(OSLIB_HTTPHeader));
	h->header = header;
	h->value = value;
	h->next = NULL;
	if (request->headers == NULL)
	{
		request->headers = h;
		return;
	}
	OSLIB_HTTPHeader *existing = request->headers;
	while (existing->next != NULL)
		existing = existing->next;
	existing->next = h;
}

void SendHTTPRequest(OSLIB_Socket* socket, OSLIB_NetworkAddress* addr, OSLIB_HTTPRequest* request)
{
	const char *req = BuildHTTPRequestString(request);
	u32 len = (u32)strlen(req);
	SendDataTo(socket, req, sizeof(char) * len, addr);
}

OSLIB_HTTPResponse *AlocateHTTPResponseStructure()
{
	return Allocate(sizeof(OSLIB_HTTPResponse));
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
	char accum[1024];
	u32 accumIndex = 0;
	enum HTTP_TOKENS prev = HTTP_TOKEN_NONE;
	OSLIB_HTTPHeader *currentHeader = NULL;
	for (u32 i = 0; i < responseDataLen; ++i)
	{
		accum[accumIndex++] = responseData[i];
		accum[accumIndex] = '\0';

		if (!strcmp(accum, "\r\n"))
		{
			accumIndex = 0;
			if (prev == SEPARATOR)
			{
				prev = DOUBLE_SEPARATOR;
				i++;
				char *buff = Allocate(sizeof(char) * (responseDataLen - i + 1));
				memcpy(buff, &responseData[i], sizeof(char) * (responseDataLen - i));
				buff[responseDataLen - i] = '\0';
				response->messageBody = buff;
				break;
			}
			else
				prev = SEPARATOR;
			continue;
		}

		if (!strcmp(accum, "HTTP"))
		{
			accumIndex = 0;
			prev = HTTP;
			continue;
		}

		if (prev == DOUBLE_SEPARATOR)
		{
			
		}

		if (accumIndex == 1 && accum[accumIndex - 1] == '/' && prev == HTTP)
		{
			accumIndex = 0;
			continue;
		}

		if (accumIndex > 0)
		{
			if (accum[accumIndex - 1] == ':' || accum[accumIndex - 1] == ' ' || accum[accumIndex - 1] == '\r'|| accum[accumIndex - 1] == '\n')
			{
				u32 strl = accumIndex - 1;
				char *buff = NULL;
				switch (prev)
				{
				default:
					break;
				case HTTP:
					prev = HTTP_VERSION;
					accumIndex = 0;
					break;
				case HTTP_VERSION:
					response->responseCode = atoi(accum);
					prev = HTTP_RESPONSE_CODE;
					accumIndex = 0;
					break;
				case HTTP_RESPONSE_CODE:
					u32 responseStrLen = (u32)strlen(accum) - 1;
					buff = Allocate(sizeof(char) * responseStrLen + 1);
					buff[responseStrLen] = '\0';
					memcpy(buff, accum, sizeof(char) * responseStrLen);
					response->responseString = buff;
					prev = HTTP_RESPONSE_STRING;
					accumIndex = 1;
					accum[0] = '\r';
					break;
				case HEADER:
					if (!strcmp(accum, ":"))
					{
						prev = COLON;
						accumIndex = 0;
					}
					break;
				case COLON:
					if (accum[accumIndex - 1] == ' ' || accum[accumIndex - 1] == ':')
						break;
					buff = Allocate(sizeof(char) * strl);
					memcpy(buff, accum, sizeof(char) * strl);
					buff[strl] = '\0';
					currentHeader->value = buff;
					accum[0] = '\r';
					accumIndex = 1;
					prev = HEADER_VALUE;
					break;
				case DOUBLE_SEPARATOR:
					buff = Allocate(sizeof(char) * strl);
					memcpy(buff, accum, sizeof(char) * strl);
					response->messageBody = buff;
					break;
				case SEPARATOR:
					if (accumIndex > 1)
					{
						OSLIB_HTTPHeader *header = Allocate(sizeof(OSLIB_HTTPHeader));
						header->next = NULL;
						prev = HEADER;
						if (accum[accumIndex - 1] == ':')
						{
							buff = Allocate(sizeof(char) * (accumIndex));
							memcpy(buff, accum, sizeof(char) * (accumIndex - 1));
							buff[accumIndex - 1] = '\0';
							i--;
						}
						
						header->header = buff;
						if (currentHeader != NULL)
							currentHeader->next = header;
						currentHeader = header;
						if (response->headers == NULL)
							response->headers = header;
						accumIndex = 0;
					}
					break;
				case HEADER_VALUE:
					if (response->responseCode == 0 || accum[accumIndex - 1] == '  ')
						break;
					OSLIB_HTTPHeader *header = Allocate(sizeof(OSLIB_HTTPHeader));
					header->next = NULL;
					buff = Allocate(sizeof(char) * strl);
					memcpy(buff, accum, sizeof(char) * strl);
					header->header = buff;
					prev = HEADER;
					if (currentHeader != NULL)
						currentHeader->next = header;
					currentHeader = header;
					accumIndex = 1;
					accum[0] = '\r';
					break;					
				}
			}
		}
	}
}

OSLIB_NetworkAddress *ConfigureNetworkAddress(const char *location, const char *port)
{
	OSLIB_NetworkAddress *addr = Allocate(sizeof(OSLIB_NetworkAddress));

	struct addrinfo *hints = Allocate(sizeof(struct addrinfo));
	struct addrinfo *i = NULL;

	ZeroMemory(hints, sizeof(struct addrinfo));

	hints->ai_family = AF_INET;
	hints->ai_socktype = SOCK_STREAM;
	hints->ai_protocol = IPPROTO_TCP;

	getaddrinfo(location, port, hints, &i);

	printf("%d\n", WSAGetLastError());

	//hints->sin_port = htons(port);
	//hints->sin_addr.s_addr = inet_addr(location);

	addr->in = hints;
	addr->info = i;

	return addr;
}

OSLIB_Socket *CreateSocket(enum OSLIB_SocketProtocol protocol)
{
	if (!networkInit)
		InitNetwork();

	OSLIB_Socket *sock = Allocate(sizeof(OSLIB_Socket));

	struct addrinfo hints;

	ZeroMemory(&hints, sizeof(struct addrinfo));
	switch (protocol)
	{
	case TCP:
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_socktype = SOCK_STREAM;
		break;
	case UDP:
		hints.ai_protocol = IPPROTO_UDP;
		hints.ai_socktype = SOCK_DGRAM;
		break;
	}
	hints.ai_family = AF_INET;

	sock->winsocket = INVALID_SOCKET;
	sock->winsocket = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);
	sock->blocking = true;
	sock->protocol = protocol;

	if (sock->winsocket == INVALID_SOCKET)
	{
		printf("%d\n", WSAGetLastError());
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
	if (sock->protocol == TCP)
	{
		for (struct addrinfo *addrInfo = addr->info; addrInfo != NULL; addrInfo = addrInfo->ai_next)
		{
			if (connect(sock->winsocket, addrInfo->ai_addr, addrInfo->ai_addrlen) == 0)
				break;
		}

		printf("%d\n", WSAGetLastError());
	}

	i32 res = sendto(sock->winsocket, buffer, strlen(buffer), 0, addr->in, sizeof(struct sockaddr_in));

	printf("Client: Sending IP(s) used: %s\n", inet_ntoa(addr->in->sin_addr));

	printf("Client: Sending port used: %d\n", htons(addr->in->sin_port));

	if (res == SOCKET_ERROR)
	{
		printf("%d\n", WSAGetLastError());

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

void PrintHTTPResponseData(const struct OSLIB_HTTPResponse* response)
{
	printf("Response Code: %d\nResponse String: %s\n", response->responseCode, response->responseString);

	OSLIB_HTTPHeader *header = response->headers;
	while (header->next != NULL)
	{
		printf("%s:%s\n", header->header, header->value);
		header = header->next;
	}

	printf("Response Body:%s\n", response->messageBody);
}