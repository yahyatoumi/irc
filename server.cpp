#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <iostream>

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
    if (socketFd < 0)
        std::cout << "failed to create socket" << std::endl;

    struct sockaddr_in *address = createIPv4Address("", 2000);

    if (bind(socketFd, (sockaddr *)address, sizeof *address) == 0)
        std::cout << "socket binded" << std::endl;

    if (listen(socketFd, 10) == 0)
        std::cout << "listenning on port 2000" << std::endl;

    struct sockaddr_in clientAddress;
    socklen_t clientAddressSize = sizeof(clientAddress);
    int clientSocketFd = accept(socketFd, (sockaddr *)&clientAddress, &clientAddressSize);
    char buff[1024];
    while (1)
    {
        if (recv(clientSocketFd, buff, 1024, 0) == 0)
        {
            printf("lost connection\n");
            break;
        }
        printf("received %s %i\n", buff, clientAddress.sin_port);
        bzero(buff, 1024);
    }
    delete address;
    return 0;
}
