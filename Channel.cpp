#include "Channel.hpp"

class Channel;

Channel::Channel()
{
    // this->channel_clients = std::vector<Client>();
}

Channel::~Channel()
{
}

Channel::Channel(std::string &name)
{
    if (!name.length())
        throw std::logic_error("error: Channel name");

    int i = 0;
    while (name[i])
    {
        if (name[i] == 7 || name[i] == ' ' || name[i] == ',')
            throw std::logic_error("error: Channel name");
        i++;
    }
    this->name = name;
    this->channel_clients = std::vector<Client>();
    this->channelUsersLimit = 10;
    this->modeI = false;
    this->modeK = false;
    this->modeT = false;
    this->modeL = false;
}