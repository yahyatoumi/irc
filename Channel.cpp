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
    this->topic = "";
}
bool Channel::isChannelFull()
{
    if (this->modeL && channel_clients.size() >= channelUsersLimit)
        return true;
    return false;
}
void Channel::addInvited(Client &client)
{
    for (size_t i = 0; i < invitedClients.size(); i++)
    {
        if (client == invitedClients[i])
            return;
    }
    this->invitedClients.push_back(client);
}
void Channel::removeInvitation(Client &client)
{
    std::vector<Client>::iterator itr;
    for (itr = invitedClients.begin(); itr < invitedClients.end(); itr++)
    {
        if (client == *itr)
        {
            invitedClients.erase(itr);
            return;
        }
    }
}
bool Channel::isInvited(Client &client)
{
    for (size_t i = 0; i < invitedClients.size(); i++)
    {
        if (client == invitedClients[i])
            return true;
    }
    return false;
}
std::string Channel::getModes()
{
    std::cout << "size == " << modes.size() << std::endl;
    if (!modes.size())
    {
        std::cout << "returned here\n";
        return "+ns";
    }
    std::string mds = "+ns";
    for (size_t i = 0; i < this->modes.size(); i++)
    {
        mds += this->modes[i];
    }
    return mds;
}
std::string Channel::getTopic()
{
    return this->topic;
}
void Channel::setTopic(std::string newTopic)
{
    modeT = true;
    this->topic = newTopic;
}
int Channel::getUserslimit()
{
    return this->channelUsersLimit;
}
void Channel::setUserslimit(int limit)
{
    this->modeL = true;
    this->channelUsersLimit = limit;
}
void Channel::setPassword(std::string pass)
{
    this->modeK = true;
    this->password = pass;
}
std::string Channel::getPassword()
{
    return this->password;
}
std::string Channel::get_name() const
{
    return this->name;
}
std::vector<Client> Channel::getCHannelClients() const
{
    return this->channel_clients;
}
int Channel::getChannelClient(const Client &client) const
{
    for (size_t i = 0; i < this->channel_clients.size(); i++)
    {
        if (this->channel_clients[i] == client)
        {
            return i;
        }
    }
    return -1;
}
void Channel::addClient(Client &client)
{
    this->channel_clients.push_back(client);
}
int Channel::getNumberOfClients()
{
    return this->channel_clients.size();
}
void Channel::removeAClientFromChannel(int index)
{
    this->channel_clients.erase(this->channel_clients.begin() + index);
    this->channelOpArr.erase(this->channelOpArr.begin() + index);
}
int Channel::addOperator(Client &client)
{
    int clientIndex = getChannelClient(client);
    std::cout << clientIndex << std::endl;
    if (clientIndex >= 0 && this->channelOpArr[clientIndex] == true)
        return -1;
    this->channelOpArr.push_back(1);
    return clientIndex;
}
bool Channel::isOperator(Client &client)
{
    int i = getChannelClient(client);
    std::cout << channelOpArr[i] << "babe\n";
    if (i >= 0)
        return this->channelOpArr[i];
    return false;
}
bool Channel::getModeI()
{
    return this->modeI;
}
bool Channel::getModeK()
{
    return this->modeK;
}
bool Channel::getModeT()
{
    return this->modeT;
}
bool Channel::getModeL()
{
    return this->modeL;
}
void Channel::setModeI(bool state)
{
    this->modeI = state;
    if (std::find(modes.begin(), modes.end(), 'i') == modes.end() && state)
        modes.push_back('i');
    else if (!state)
    {
        std::vector<char>::iterator tmp = std::find(modes.begin(), modes.end(), 'i');
        if (tmp != modes.end())
        {
            modes.erase(tmp);
        }
    }
}
void Channel::setModeL(bool state)
{
    this->modeL = state;
    if (std::find(modes.begin(), modes.end(), 'l') == modes.end() && state)
        modes.push_back('l');
    else if (!state)
    {
        std::vector<char>::iterator tmp = std::find(modes.begin(), modes.end(), 'l');
        if (tmp != modes.end())
        {
            modes.erase(tmp);
        }
    }
}
void Channel::setModeT(bool state)
{
    this->modeT = state;
    if (std::find(modes.begin(), modes.end(), 't') == modes.end() && state)
        modes.push_back('t');
    else if (!state)
    {
        std::vector<char>::iterator tmp = std::find(modes.begin(), modes.end(), 't');
        if (tmp != modes.end())
        {
            modes.erase(tmp);
        }
    }
}
void Channel::setModeK(bool state)
{
    this->modeK = state;
    if (std::find(modes.begin(), modes.end(), 'k') == modes.end() && state)
        modes.push_back('k');
    else if (!state)
    {
        std::vector<char>::iterator tmp = std::find(modes.begin(), modes.end(), 'k');
        if (tmp != modes.end())
        {
            modes.erase(tmp);
        }
    }
}
void Channel::modifOp(int index, int state)
{
    this->channelOpArr[index] = state;
}
void Channel::pushBackToOppArr()
{
    this->channelOpArr.push_back(0);
}