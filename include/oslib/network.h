#ifndef OSLIB_NETWORK_H
#define OSLIB_NETWORK_H

#include <include/oslib/platform.h>

struct OSLIB_Socket;

struct OSLIB_NetworkAddress;

struct OSLIB_HTTPRequest;

struct OSLIB_HTTPResponse;

enum OSLIB_HTTPRequestMethod
{
	NONE = 0,
	GET,
	POST,
	PUT,
	CONNECT,
	PATCH
};

void InitNetwork();

void CloseNetwork();

struct OSLIB_NetworkAddress *ConfigureNetworkAddress(const char *location, const char *port);

struct OSLIB_Socket *CreateSocket();

void BindSocket(struct OSLIB_Socket* sock, struct OSLIB_NetworkAddress *addr);

void ConnectSocket(struct OSLIB_Socket *sock, struct OSLIB_NetworkAddress *addr);

i32 SendDataTo(struct OSLIB_Socket *sock, u8 *buffer, u32 bufferSize, struct OSLIB_NetworkAddress *addr);

i32 SendDataToAll(struct OSLIB_Socket *sock, u8 *buffer, u32 bufferSize);

i32 ReceiveData(struct OSLIB_Socket *sock, u8 *buffer, u32 bufferSize);

i32 ReceiveDataFrom(struct OSLIB_Socket *sock, u8 *buffer, u32 bufferSize);

void CloseSocket(struct OSLIB_Socket* sock);

#endif