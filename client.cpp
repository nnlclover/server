#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <sys/ioctl.h>
#include <vector>
#include <poll.h>

const int timeout = 20;


class connection {
private:
    int sock = -1;
public:
    connection(const char* ip, int port)
    {
        sockaddr_in main_addr;
	    memset(&main_addr, 0, sizeof(main_addr));
	    main_addr.sin_addr.s_addr = inet_addr(ip);
	    main_addr.sin_family = AF_INET;
	    main_addr.sin_port = htons(port);

	    sock = socket(AF_INET, SOCK_STREAM, 0);
	    if (sock < 0)
	    {
	    	fprintf(stderr, "Error establishing the server socket\n");
	    	return;
	    }

        if(connect(sock, (sockaddr*)&main_addr, sizeof(main_addr)))
        {
            perror("failed connect");
            return;
        }

    }
    int execute(const char* request)
    {
        char* buffer = (char*)malloc(8192);
        buffer[0] = 0x06;
        buffer[1] = 0x08;
        uint16_t size = strlen(request);
        memcpy(&buffer[2], &size, 2);
        memcpy(&buffer[4], request, size);
        int len = size + 4;
        send(sock, buffer, len, 0);
        memset(buffer, 0, 8192);
        
        recv(sock, buffer, 8192, 0);
        std::cout << buffer << std::endl;
        return 0;
    }

};


int main()
{
	connection conn("127.0.0.1", 3425);

    conn.execute("SELECT * FROM chats;");

	return 0;
}
