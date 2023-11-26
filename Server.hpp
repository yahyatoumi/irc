#include <iostream>
#include "Client.hpp"
#include <vector>
#include <unistd.h>
#include <poll.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "message.hpp"

class Client;
class Channel;

class Server
{
private:
    int port;
    int sockFD;
    std::string password;
    std::vector<struct pollfd> fds;
    std::vector<Client> clients;
    std::vector<Channel> channels;

public:
    Server();

    int getClientIndexByFD(int fd)
    {
        for (int i = 0; i < clients.size(); i++)
        {
            if (this->clients[i].getFd() == fd)
            {
                return i;
            }
        }
        return -1;
    }
    bool isNickNameValid(std::string &nickname)
    {
        if (nickname.empty() || nickname.size() > 9 || std::isdigit(nickname[0]) || nickname[0] == '#' || nickname[0] == ':')
            return false;

        for (int i = 0; i < nickname.length(); i++)
        {
            if (!std::isalnum(nickname[i]) && nickname[i] != '_' && nickname[i] != '|' && nickname[i] != '-' && nickname[i] != '^' &&
                nickname[i] != '[' && nickname[i] != ']' && nickname[i] != '{' && nickname[i] != '}')
                return false;
            if (nickname[i] <= 13 && nickname[i] >= 9)
                return false;
        }
        return true;
    }
    int getClientIndexByNickname(std::string nickname)
    {
        for (int i = 0; i < clients.size(); i++)
        {
            if (this->clients[i].getnickname() == nickname)
            {
                return i;
            }
        }
        return -1;
    }
    int find_channel(std::string &name)
    {
        for (int i = 0; i < this->channels.size(); i++)
        {
            if (this->channels[i].get_name().length() && this->channels[i].get_name() == name)
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
                std::cout << "sent to " << clients[i].getnickname() << std::endl;
                send(clients[i].getFd(), (":" + sender.getnickname() + " " + xxx).c_str(), (":" + sender.getnickname() + " " + xxx).length(), 0);
            }
        }
    }
    void parse(char *buff, int i)
    {
        if (!std::strncmp(buff, "PASS ", 5))
        {
            std::string password(buff + 5);
            if (this->clients[i].isAuthenticated())
            {
                std::string rpl = ERR_ALREADYREGISTERED("127.0.0.1", this->clients[i].getnickname());
                if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    std::runtime_error("send failed");
            }
            else if (password.length() == 0 || password.substr(0, password.size() - 2) != this->password)
            {
                std::string rpl = ERR_PASSWDMISMATCH("127.0.0.1", this->clients[i].getnickname());
                if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    std::runtime_error("send failed");
            }
            else
                this->clients[i].setPassword(password);
        }
        else if (!std::strncmp(buff, "NICK ", 5))
        {
            std::string nickname(buff + 5);
            nickname = nickname.substr(0, nickname.size() - 2);
            std::cout << "nick : " << nickname << std::endl;
            if (!this->clients[i].getEnteredPass())
            {
                std::string rpl = ERR_ALREADYREGISTERED("127.0.0.1", this->clients[i].getnickname());
                if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    std::runtime_error("send failed");
            }
            else if (std::strlen(buff) <= 5)
            {
                std::string rpl = ERR_NONICKNAMEGIVEN("127.0.0.1", this->clients[i].getnickname());
                if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    std::runtime_error("send failed");
            }
            else if (!isNickNameValid(nickname))
            {
                std::string rpl = ERR_ERRONEUSNICKNAME("127.0.0.1", this->clients[i].getnickname());
                if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    std::runtime_error("send failed");
            }
            else if (getClientIndexByNickname(nickname) >= 0)
            {
                std::string rpl = ERR_NICKNAMEINUSE("127.0.0.1", this->clients[i].getnickname());
                if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    std::runtime_error("send failed");
            }
            else
            {
                std::cout << "nickname is now : " << nickname;
                std::string rpl = RPL_NICKCHANGE(this->clients[i].getnickname(), nickname, "127.0.0.1");
                this->clients[i].setNickName(nickname);
                if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    std::runtime_error("send failed");
            }
        }
        else if (!std::strncmp(buff, "JOIN #", 6))
        {
            if (this->clients[i].getEnteredPass() && this->clients[i].getEntredNick())
            {
                std::string channel_name_holder(buff + 5);
                std::string channel_name = channel_name_holder.substr(0, channel_name_holder.size() - 2);
                int channel_index = find_channel(channel_name);
                if (channel_index >= 0)
                {
                    if (this->channels[channel_index].getChannelClient(this->clients[i]) == -1)
                    {
                        std::cout << "foooo" << std::endl;
                        this->channels[channel_index].addClient(this->clients[i]);
                        std::cout << this->clients[i].getnickname() << " is now a member in 1 " << this->channels[channel_index].get_name() << std::endl;
                        std::string rpl = RPL_JOIN(this->clients[i].getnickname(), this->clients[i].getnickname(), channel_name, "127.0.0.1");
                        if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                        {
                            exit(1);
                        }
                    }
                    else
                        std::cout << "didi not" << std::endl;
                }
                else
                {
                    std::cout << this->clients.size() << "pcpcpcpcpcpcpcp\n";
                    Channel channel(channel_name);
                    this->channels.push_back(channel);
                    std::cout << this->clients.size() << std::endl;
                    this->channels[this->channels.size() - 1].addClient(this->clients[i]);
                    std::cout << this->clients[i].getnickname() << " is now a member in 2 " << channel.get_name() << std::endl;
                    std::string rpl = RPL_JOIN(this->clients[i].getnickname(), this->clients[i].getnickname(), channel_name, "127.0.0.1");
                    std::cout << rpl;
                    std::cout << "i : " << i << std::endl;
                    send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0);
                }
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
            int channel_index = find_channel(channel_name);
            std::cout << "glockkkkk " << channel_index << channel_name << std::endl;
            if (channel_index >= 0 && this->channels[channel_index].getChannelClient(clients[i]) >= 0)
            {
                channel_send_message(this->channels[channel_index], clients[i], buff);
            }
        }
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
    void createNewClienFD()
    {
        struct sockaddr_in clientAddress;
        socklen_t clientAddressSize = sizeof(clientAddress);
        int clientSocketFd = accept(this->sockFD, (struct sockaddr *)&clientAddress, &clientAddressSize);

        if (clientSocketFd >= 0)
        {
            std::cout << "New connection accepted from client " << inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port) << std::endl;
            struct pollfd new_struct;
            new_struct.events = POLLIN;
            new_struct.fd = clientSocketFd;
            fds.push_back(new_struct);
            Client newClient(clientSocketFd);
            this->clients.push_back(newClient);
        }
    }
    void pollinFDs()
    {
        int fds_size = fds.size();
        for (int i = 1; i < fds_size; i++)
        {
            if (fds[i].revents & POLLIN)
            {
                char buff[1024];
                ssize_t bytesRead;
                bytesRead = recv(fds[i].fd, buff, sizeof(buff) - 1, 0);
                std::cout << "bytes read " << bytesRead << " fd: " << i << std::endl;
                if (bytesRead <= 0)
                {
                    if (bytesRead == 0)
                    {
                        std::cout << "Client " << i << " disconnected" << std::endl;
                        std::cout << i << std::endl;
                        int size = this->channels.size();
                        for (int x = 0; x < size; x++)
                        {
                            int index = this->channels[x].getChannelClient(this->clients[i - 1]);
                            if (index >= 0)
                            {
                                this->channels[x].removeAClientFromChannel(index);
                                if (!this->channels[x].getNumberOfClients())
                                {
                                    this->channels.erase(this->channels.begin() + x);
                                    size--;
                                }
                            }
                        }
                    }
                    else
                    {
                        std::cerr << "Error reading from client " << i << std::endl;
                    }
                    std::cout << "fi\n";
                    std::cout << i << "--" << this->clients.size() << std::endl;
                    this->clients.erase(this->clients.begin() + i - 1);
                    std::cout << "fo\n";
                    close(this->fds[i].fd);
                    this->fds.erase(this->fds.begin() + i);
                    fds_size--;
                }
                else
                {
                    buff[bytesRead] = '\0';
                    std::cout << "Received from client " << i << ": " << buff;
                    parse(buff, i - 1);
                }
            }
        }
    }
    void serverPoll()
    {
        while (true)
        {
            int ready = poll(this->fds.data(), this->fds.size(), 300);
            if (ready < 0)
            {
                std::cerr << "Poll error" << std::endl;
                break;
            }
            if (this->fds[0].revents & POLLIN)
                createNewClienFD();
            pollinFDs();
        }
    }
    Server(char *port, char *password)
    {
        std::cout << "fofofofof" << std::endl;
        this->port = std::atoi(port);
        if (this->port < 0 || this->port > 65535 || std::strlen(port) > 5)
            std::out_of_range("error: port out of range");
        this->password = password;
        this->sockFD = socket(AF_INET, SOCK_STREAM, 0);
        int on = 1;
        if (this->sockFD < 0)
            std::runtime_error("Failed to create socket");

        if (setsockopt(this->sockFD, SOL_SOCKET, SO_REUSEADDR,
                       (char *)&on, sizeof(on)) < 0)
        {
            close(this->sockFD);
            std::runtime_error("setsockopt() failed");
        }

        struct sockaddr_in *address = createIPv4Address("", this->port);

        if (bind(this->sockFD, (struct sockaddr *)address, sizeof *address) == 0)
        {
            std::cout << "Socket bound" << std::endl;
        }
        else
        {
            close(this->sockFD);
            delete address;
            std::runtime_error("Bind failed xx");
        }

        if (listen(this->sockFD, 10) == 0)
        {
            std::cout << "Listening on port " << this->port << std::endl;
        }
        else
        {
            delete address;
            close(this->sockFD);
            std::runtime_error("Listen failed");
        }
        struct pollfd fd0;
        fd0.fd = this->sockFD;
        fd0.events = POLLIN;
        this->fds.push_back(fd0);
        serverPoll();
    }
};