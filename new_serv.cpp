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
#include "Channel.hpp"
#include <vector>
#include "message.hpp"

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

int find_channel(std::vector<Channel> &channels, std::string &name)
{
	for (int i = 0; i < channels.size(); i++)
	{
		if (channels[i].get_name() == name)
			return i;
	}
	return -1;
}

void channel_send_message(Channel &channel, Client &sender, char *message)
{
	std::vector<Client> clients = channel.getCHannelClients();
	std::cout << 2 << "   " << message << std::endl;
	std::cout << "sizeeeeee" << clients.size() << std::endl;
	std::string xxx(message);
	for (int i = 0; i < clients.size(); i++)
	{
		std::cout << i << 'x' << std::endl;
		if (clients[i].getFd() != sender.getFd())
		{
			std::cout << "sent to " << clients[i].getname() << std::endl;
			send(clients[i].getFd(), (":" + sender.getname() + " " + xxx).c_str(), (":" + sender.getname() + " " + xxx).length(), 0);
		}
	}
}

void parse(char *buff, std::vector<Client> &clients, std::vector<Channel> &channels, int i)
{
	if (!std::strncmp(buff, "JOIN #", 6))
	{

		std::string channel_name_holder(buff + 5);
		std::string channel_name = channel_name_holder.substr(0, channel_name_holder.size() - 2);
		int channel_index = find_channel(channels, channel_name);
		if (channel_index >= 0)
		{
			if (channels[channel_index].getChannelClient(clients[i]) == -1)
			{
				std::cout << "foooo" << std::endl;
				channels[channel_index].addClient(clients[i]);
				std::cout << clients[i].getname() << " is now a member in 1 " << channels[channel_index].get_name() << std::endl;
				std::string rpl = RPL_JOIN(clients[i].getname(), "username", channel_name, "127.0.0.1");
				if (send(clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
				{
					std::cout << "failed\n";
					exit(1);
				}
			}
			else
				std::cout << "didi not" << std::endl;
		}
		else
		{
			std::cout << "frankkkkkkkkkkkkk---*-*-*-*-" << std::endl;
			Channel channel(channel_name);
			channels.push_back(channel);
			std::cout << clients.size() << std::endl;
			channels[channels.size() - 1].addClient(clients[i]);
			std::cout << clients[i].getname() << " is now a member in 2 " << channel.get_name() << std::endl;
			std::string rpl = RPL_JOIN(clients[i].getname(), "username", channel_name, "127.0.0.1");
			// std::cout << "sent: " << rpl;
			std::cout << "i : " << i << std::endl;
			send(clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0);
		}
	}
	else if (!std::strncmp(buff, "PRIVMSG #", 9))
	{
		char channel_holder[std::strlen(buff + 9)];
		int x = 0;
		while (buff[x + 8] && buff[x + 8] != ' ')
		{
			channel_holder[x] = buff[x + 8];
			x++;
		}
		channel_holder[x] = '\0';
		std::string channel_name(channel_holder);
		int channel_index = find_channel(channels, channel_name);
		std::cout << "glockkkkk " << channel_index << channel_name << std::endl;
		if (channel_index >= 0 && channels[channel_index].getChannelClient(clients[i]) >= 0)
		{
			channel_send_message(channels[channel_index], clients[i], buff);
		}
	}
	// std::cout << "channels: ";
	// for (int x = 0; x < channels.size(); x++)
	// {
	// 	std::cout << channels[x].get_name() << std::endl;
	// }
}

int main(int ac, char **av)
{
	if (ac != 3)
		return 0;
	try
	{

		int socketFd = createTCPIPv4Socket();
		int on = 1;
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

		struct sockaddr_in *address = createIPv4Address("", 2000);

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

		// struct pollfd fds[MAX_CLIENTS + 1];
		std::vector<struct pollfd> fds(1);
		fds[0].fd = socketFd;
		std::cout << "vvvvvvvvvv\n";
		fds[0].events = POLLIN;

		std::vector<Client> clients;
		std::vector<Channel> channels;

		while (1)
		{
			std::cout << "fd : " << fds.size() << std::endl;
			int ready = poll(fds.data(), fds.size(), 300);
			if (ready < 0)
			{
				std::cerr << "Poll error" << std::endl;
				break;
			}

			if (fds[0].revents & POLLIN)
			{
				struct sockaddr_in clientAddress;
				socklen_t clientAddressSize = sizeof(clientAddress);
				int clientSocketFd = accept(socketFd, (struct sockaddr *)&clientAddress, &clientAddressSize);

				if (clientSocketFd >= 0)
				{
					std::cout << "New connection accepted from client " << inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port) << std::endl;
					struct pollfd new_struct;
					new_struct.events = POLLIN;
					new_struct.fd = clientSocketFd;
					fds.push_back(new_struct);
				}
			}
			std::cout << "clients : " << clients.size() << std::endl;
			int fds_size = fds.size();
			for (int i = 1; i < fds_size; i++)
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
							std::cout << i << std::endl;
							int size = channels.size();
							for (int x = 0; x < size; i++)
							{
								int index = channels[x].getChannelClient(clients[i - 1]);
								if (index >= 0)
								{
									channels.erase(channels.begin() + index);
									size--;
								}
							}
						}
						else
						{
							std::cerr << "Error reading from client " << i << std::endl;
						}
						std::cout << "fi\n";
						std::cout << i << clients.size() << std::endl;
						clients.erase(clients.begin() + i - 1);
						std::cout << "fo\n";
						close(fds[i].fd);
						fds.erase(fds.begin() + i);
						fds_size--;
					}
					else
					{
						buff[bytesRead] = '\0';
						std::cout << "Received from client " << i << ": " << buff;
						if (!std::strncmp(buff, "NICK ", 5))
						{
							if (std::strlen(buff) > 5 + 9 + 2)
							{
								std::cout << "max nickname len is 9" << std::endl;
								exit(1);
							}
							for (int i = 0; i < clients.size(); i++)
							{
								std::string x(buff + 5);
								std::string holder = x.substr(0, x.size() - 2);
								std::cout << holder << " " << clients[i].getname() << std::endl;
								if (!std::strncmp(buff + 5, clients[i].getname().c_str(), std::strlen(buff + 5) - 2))
								{
									std::cout << "nickname already exist" << std::endl;
									exit(1);
								}
							}
							std::string x(buff + 5);
							std::string holder = x.substr(0, x.size() - 2);
							Client new_client(fds[fds.size() - 1].fd);
							new_client.setNickName(holder);
							std::cout << "fffffff " << new_client.getname() << std::endl;
							clients.push_back(new_client);
						}
						else
						{
							parse(buff, clients, channels, i - 1);
						}
					}
				}
			}
		}

		close(socketFd);
		delete address;
		return 0;
	}
	catch (std::exception &e)
	{
		std::cout << "exception: " << e.what() << std::endl;
	}
}
