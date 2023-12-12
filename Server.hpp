#include <iostream>
#include "Client.hpp"
#include <vector>
#include <unistd.h>
#include <poll.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "message.hpp"
#include <deque>
#include "Channel.hpp"

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
    std::string hostname;

public:
    struct sockaddr_in clientAddress;
    int getClientIndexByFD(int fd);
    bool isNickNameValid(std::string &nickname);
    int getClientIndexByNickname(std::string nickname);
    int find_channel(std::string &name);
    void channel_send_message(Channel &channel, Client &sender, const char *message);
    bool isThereEnoughParams(int paramsSize, std::deque<char> &modes, int plus);
    void addModes(std::deque<std::string> &params, std::deque<char> &modes, int index, std::string &channelName);
    void removeModes(std::deque<std::string> &params, std::deque<char> &modes, int index, std::string &channelName);
    void parseMode(std::string &command, int index);
    void extractChannelsName(std::vector<std::string> &channelsNames, std::vector<std::string> &keys, std::string &params);
    void extractInviteParams(std::vector<std::string> &params, std::string paramsString);
    void extractKickParams(std::vector<std::string> &kickParams, const char *paramsStr);
    void parse(const char *buff, int i);
    struct sockaddr_in *createIPv4Address(int port);
    void createNewClienFD();
    void pollinFDs();
    void serverPoll();
    Server(char *port, char *password);
};