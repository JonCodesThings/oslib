#include <include/oslib/network.h>

#include <stdio.h>

#define PACKET_BUFFER_SIZE 1024

int main()
{
	InitNetwork();

	struct OSLIB_Socket *sock = CreateSocket();

	struct OSLIB_NetworkAddress *addr = ConfigureNetworkAddress("127.0.0.1", "25565");

	BindSocket(sock, addr);

	u8 buffer[PACKET_BUFFER_SIZE];

	while (ReceiveDataFrom(sock, buffer, 1024) == 0)
		;

	printf("%s\n", buffer);

	CloseSocket(sock);

	CloseNetwork();
}