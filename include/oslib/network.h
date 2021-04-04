#ifndef OSLIB_NETWORK_H
#define OSLIB_NETWORK_H

#include <include/oslib/platform.h>

struct OSLIB_Socket;

enum OSLIB_SocketProtocol
{
	TCP,
	UDP
};

struct OSLIB_NetworkAddress;

struct OSLIB_HTTPRequest;

struct OSLIB_HTTPResponse;

enum OSLIB_HTTPRequestMethod
{
	NONE = 0,
	HTTP_GET,
	HTTP_POST,
	HTTP_PUT,
	HTTP_DELETE,
	HTTP_CONNECT,
	HTTP_PATCH
};

struct OSLIB_NetworkAddress *ConfigureNetworkAddress(const char *location, const char *port);

struct OSLIB_Socket *CreateSocket(enum OSLIB_SocketProtocol protocol);

void BindSocket(struct OSLIB_Socket* sock, struct OSLIB_NetworkAddress *addr);

void ConnectSocket(struct OSLIB_Socket *sock, struct OSLIB_NetworkAddress *addr);

i32 SendDataTo(struct OSLIB_Socket *sock, u8 *buffer, u32 bufferSize, struct OSLIB_NetworkAddress *addr);

i32 SendDataToAll(struct OSLIB_Socket *sock, u8 *buffer, u32 bufferSize);

i32 ReceiveData(struct OSLIB_Socket *sock, u8 *buffer, u32 bufferSize);

i32 ReceiveDataFrom(struct OSLIB_Socket *sock, u8 *buffer, u32 bufferSize);

void CloseSocket(struct OSLIB_Socket* sock);

struct OSLIB_HTTPRequest *BuildHTTPRequest(enum OSLIB_HTTPRequestMethod method, const char *location, const char *body);

void AddHTTPHeader(struct OSLIB_HTTPRequest * const request, const char* header, const char* value);

void SendHTTPRequest(struct OSLIB_Socket* socket, struct OSLIB_NetworkAddress* addr, struct OSLIB_HTTPRequest* request);

struct OSLIB_HTTPResponse *AlocateHTTPResponseStructure();

void ParseHTTPResponse(const char* responseData, struct OSLIB_HTTPResponse *response);

void PrintHTTPResponseData(const struct OSLIB_HTTPResponse *response);

#endif