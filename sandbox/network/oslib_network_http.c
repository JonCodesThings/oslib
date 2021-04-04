#include <include/oslib/network.h>

#define PACKET_BUFFER_SIZE 1024

int main()
{
	struct OSLIB_Socket *sock = CreateSocket(TCP);

	struct OSLIB_NetworkAddress *addr = ConfigureNetworkAddress("www.httpbin.org", "80");

	struct OSLIB_HTTPRequest *rq = BuildHTTPRequest(HTTP_GET, "https://google.com", "");

	//SendHTTPRequest(sock, addr, rq);

	u8 buffer[PACKET_BUFFER_SIZE];

	const char *s = "GET /get HTTP/1.1\r\nUser-Agent: OSLIB\nHost: httpbin.org\r\nAccept: application/json\r\n\r\n";

	memcpy(buffer, s, sizeof(char) * strlen(s));
	buffer[strlen(s)] = '\0';

	SendDataTo(sock, buffer, PACKET_BUFFER_SIZE, addr);

	i32 ret = 0;

	while ((ret = ReceiveData(sock, buffer, PACKET_BUFFER_SIZE)) == 0)
		;

	buffer[ret] = '\0';

	printf("%s\n", buffer);

	struct OSLIB_HTTPResponse *res = AlocateHTTPResponseStructure();

	ParseHTTPResponse(buffer, res);

	PrintHTTPResponseData(res);

	return 0;
}