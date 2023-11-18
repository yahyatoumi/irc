#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

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
        std::cout << "failed to creat socket" << std::endl;

    const char *ip = "127.0.0.1";
    struct sockaddr_in *address = createIPv4Address(ip, 2000);

    bind(socketFd, (sockaddr *)address, sizeof(address));
    int result = connect(socketFd, (sockaddr *)(address), sizeof *address);
    if (result == 0)
        std::cout << "connected..." << std::endl;

    std::string buff;
    while (1)
    {
        std::getline(std::cin, buff);
        if (buff == "exit")
        {
            break;
        }
        std::cout << "sent " << buff << std::endl;
        send(socketFd, buff.c_str(), buff.length(), 0);
    }
    delete address;
    return 0;
}
