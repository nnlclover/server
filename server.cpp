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
#include <sstream>
#include <thread>
#include <deque>
#include <mutex>
#include <condition_variable>


const int timeout = 20;

void sbs(int sock, bool state)
{
	long u = state ? 1 : 0;
	ioctl(sock, FIONBIO, &u);
}

std::deque<int> myQueue; // Очередь
std::mutex mtx;          // Мьютекс для доступа к очереди
std::condition_variable cv; // Условная переменная для ожидания новых элементов


void worker()
{
	while(true)
	{

	}
}

int main()
{
	std::thread pipe_thread(worker); 

	sockaddr_in main_addr;
	memset(&main_addr, 0, sizeof(main_addr));
	main_addr.sin_addr.s_addr = INADDR_ANY;
	main_addr.sin_family = AF_INET;
	main_addr.sin_port = htons(3425);
	
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		fprintf(stderr, "Error establishing the server socket\n");
		exit(0);
	}

	int optval = 1;
 	if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(int)) == -1) 
	{ 
        perror("setsockopt") ;
        exit(-1) ;
    }

	if (bind(sock, (sockaddr *)&main_addr, sizeof(main_addr)))
	{
		perror("failed bind\n");
	}

	if (listen(sock, 5))
	{
		perror("failed bind\n");
	}

	char *read_buffer = (char *)malloc(1024);

	struct sock_st
	{
		unsigned long last_time = 0;
		sockaddr remote_addr{};
		socklen_t addr_remote_len = 0;
		int sock = -1, state = -1;
		uint16_t waited_size = -1;
		std::string stream;
	};

	sbs(sock, true);
	std::vector<sock_st*> socket_list;

	int i, _sock, res;
	
	pollfd polling;

	while (true)
	{
		memset(&polling, 0, sizeof(polling));
		polling.fd = sock;
		polling.events = POLLIN | POLLRDNORM;

		if (socket_list.size() == 0)
			sbs(sock, false);

		if (poll(&polling, 1, 0) > 0)
		{
			if (polling.revents & POLLIN || polling.revents & POLLRDNORM)
			{
				sock_st* client = new sock_st;
				_sock = accept(sock, (sockaddr *)&client->remote_addr, &client->addr_remote_len);
				if (_sock < 0)
				{
					close(_sock);
					delete client;
					continue;
				}
				client->sock = _sock;
				client->last_time = time(NULL);
				sbs(_sock, true);
				socket_list.push_back(client);
				memset(&client, 0, sizeof(client));
				printf("new connection!\n");
				if (socket_list.size() > 1)
					sbs(sock, true);
			}
		}

		const u_long now_time_with = time(NULL) - timeout;

		for (i = 0; i < socket_list.size(); ++i)
		{
			sock_st &st = *socket_list[i];
			if (st.last_time < now_time_with)
			{
				printf("timeout erase %d\n", i);
				close(socket_list[i]->sock);
				socket_list.erase(socket_list.begin() + i);
			}
			memset(&polling, 0, sizeof(polling));
			polling.fd = st.sock;
			polling.events = POLLIN | POLLRDNORM;
			if (poll(&polling, 1, 0) > 0)
			{
				if (polling.events & POLLIN || polling.events & POLLRDNORM)
				{
					std::cout << "has data" << std::endl;
					memset(read_buffer, 0, 1024);
					while ((res = recv(st.sock, read_buffer, 1024, 0)) > 0)
					{
						char header[20];
						memset(&header, 0, 20);
						if (read_buffer[0] != 0x06) // command for server if it is broken
						{
							header[0] = 0x20; // Status code
							header[1] = 0x01; // fail request
							send(st.sock, header, 20, 0);
							st.stream = "";
							st.last_time = time(NULL);
							break;
						}
						int offset;
						switch (read_buffer[1])
						{
						case 0x08: 
							offset = 4;
							memcpy(&st.waited_size, &read_buffer[2], 2);
							st.stream = "";
						case 0x09: // add to last packet
							if(offset != 4) offset = 2;

							if (st.waited_size > res - offset)
							{
								st.stream.append(&read_buffer[offset], res - offset);
								st.waited_size -= res - offset;
								st.state = 0x03;
								std::cout << "append" << std::endl;
							}
							else
							{
								st.stream.append(&read_buffer[offset], st.waited_size);
								st.waited_size = 0;
								st.state = 0x01;
								std::cout << "success" << std::endl;
							}
							break;
						}

						st.last_time = time(NULL); // Update last time
					}
				}
			}
		}

		for(auto& obj : socket_list)
		{
			if(obj->state == 0x01)
			{
				obj->state = 0x22;
				std::cout << obj->stream << std::endl;
				//send(obj->sock, "ok", 2, 0);
			}
		}
	}

	free(read_buffer);
	return 0;
}
