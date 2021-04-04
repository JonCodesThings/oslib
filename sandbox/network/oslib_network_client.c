#include <include/oslib/network.h>

#include <stdio.h>
#include <string.h>

#define PACKET_BUFFER_SIZE 1024

int main()
{
	struct OSLIB_Socket *sock = CreateSocket(UDP);

	struct OSLIB_NetworkAddress *addr = ConfigureNetworkAddress("127.0.0.1", "25565");

	//ConnectSocket(sock, addr);

	u8 buffer[PACKET_BUFFER_SIZE];

	const char *string = "Hello!";

	memcpy(buffer, string, sizeof(char) * strlen(string) + 1);

	SendDataTo(sock, buffer, sizeof(char) * strlen(string), addr);

	while (ReceiveData(sock, buffer, 1024) == 0)
		;

	printf("%s\n", buffer);

	CloseSocket(sock);

}