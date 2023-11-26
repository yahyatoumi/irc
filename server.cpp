#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <poll.h>

const int MAX_CLIENTS = 10;

int createTCPIPv4Socket()
{
	return socket(AF_INET, SOCK_STREAM, 0);
}

struct sockaddr_in *createIPv4Address(const char *ip, int port)
{
	struct sockaddr_in *address = new sockaddr_in();
	address->sin_port = htons(port);
	address->sin_family = AF_INET;
	if (std::strlen(ip) == 0)
		address->sin_addr.s_addr = INADDR_ANY;
	else
		inet_pton(AF_INET, ip, &address->sin_addr.s_addr);
	return address;
}


int main()
{
	int socketFd = createTCPIPv4Socket();
	int on = 1;
	int on2 = 1;
	if (socketFd < 0)
	{
		std::cerr << "Failed to create socket" << std::endl;
		return 1;
	}

	if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR,
				   (char *)&on, sizeof(on)) < 0)
	{
		perror("setsockopt() failed");
		close(socketFd);
		exit(-1);
	}

	struct sockaddr_in *address = createIPv4Address("127.0.0.1", 2000);

	if (bind(socketFd, (struct sockaddr *)address, sizeof *address) == 0)
	{
		std::cout << "Socket bound" << std::endl;
	}
	else
	{
		std::cerr << "Bind failed xx" << std::endl;
		close(socketFd);
		delete address;
		return 1;
	}

	if (listen(socketFd, 10) == 0)
	{
		std::cout << "Listening on port 2000" << std::endl;
	}
	else
	{
		std::cerr << "Listen failed" << std::endl;
		close(socketFd);
		delete address;
		return 1;
	}

	struct pollfd fds[MAX_CLIENTS + 1];
	memset(fds, 0, sizeof(fds));

	fds[0].fd = socketFd;
	fds[0].events = POLLIN;

	int clientCount = 0;

	while (1)
	{
		int ready = poll(fds, clientCount + 1, 300);
		if (ready < 0)
		{
			std::cerr << "Poll error" << std::endl;
			break;
		}

		if (fds[0].revents & POLLIN)
		{
			if (clientCount < MAX_CLIENTS)
			{
				struct sockaddr_in clientAddress;
				socklen_t clientAddressSize = sizeof(clientAddress);
				int clientSocketFd = accept(socketFd, (struct sockaddr *)&clientAddress, &clientAddressSize);

				if (clientSocketFd >= 0)
				{
					std::cout << "New connection accepted from client " << inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port) << std::endl;

					fds[clientCount + 1].fd = clientSocketFd;
					fds[clientCount + 1].events = POLLIN;
					clientCount++;
				}
			}
		}

		for (int i = 1; i <= clientCount; ++i)
		{
			if (fds[i].revents & POLLIN)
			{
				char buff[1024];
				ssize_t bytesRead;
				bytesRead = recv(fds[i].fd, buff, sizeof(buff) - 1, 0);
				if (bytesRead <= 0)
				{
					if (bytesRead == 0)
					{
						std::cout << "Client " << i << " disconnected" << std::endl;
					}
					else
					{
						std::cerr << "Error reading from client " << i << std::endl;
					}
					close(fds[i].fd);
					fds[i] = fds[clientCount];
					clientCount--;
					i--;
				}
				else
				{
					buff[bytesRead] = '\0';
					std::cout << "Received from client " << i << ": " << buff << std::endl;
					for (int x = 1; x <= clientCount; ++x)
					{
						if (x != i)
							send(fds[x].fd, buff, bytesRead, 0);
					}
				}
			}
		}
	}

	close(socketFd);
	delete address;
	return 0;
}
