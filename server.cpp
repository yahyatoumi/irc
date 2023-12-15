#include "Server.hpp"

int Server::getClientIndexByFD(int fd)
{
    for (size_t i = 0; i < clients.size(); i++)
    {
        if (this->clients[i].getFd() == fd)
        {
            return i;
        }
    }
    return -1;
}

bool Server::isNickNameValid(std::string &nickname)
{
    if (nickname.empty() || nickname.size() > 9 || std::isdigit(nickname[0]) || nickname[0] == '#' || nickname[0] == ':')
        return false;

    for (size_t i = 0; i < nickname.length(); i++)
    {
        if (!std::isalnum(nickname[i]) && nickname[i] != '_' && nickname[i] != '|' && nickname[i] != '-' && nickname[i] != '^' &&
            nickname[i] != '[' && nickname[i] != ']' && nickname[i] != '{' && nickname[i] != '}')
            return false;
        if (nickname[i] <= 13 && nickname[i] >= 9)
            return false;
    }
    return true;
}

int Server::getClientIndexByNickname(std::string nickname)
{
    for (size_t i = 0; i < clients.size(); i++)
    {
        if (this->clients[i].getnickname() == nickname)
        {
            return i;
        }
    }
    return -1;
}

int Server::find_channel(std::string &name)
{
    for (size_t i = 0; i < this->channels.size(); i++)
    {
        if (this->channels[i].get_name().length() && this->channels[i].get_name() == name)
            return i;
    }
    return -1;
}

void Server::channel_send_message(Channel &channel, Client &sender, const char *message)
{
    std::vector<Client> clients = channel.getCHannelClients();
    std::string stringMessage(message);
    for (size_t i = 0; i < clients.size(); i++)
    {
        if (clients[i].getFd() != sender.getFd())
        {
            send(clients[i].getFd(), message, std::strlen(message), 0);
        }
    }
}

bool Server::isThereEnoughParams(int paramsSize, std::deque<char> &modes, int plus)
{
    int nOfModesNeedsParam = 0;
    size_t i = 0;
    while (i < modes.size())
    {
        if ((modes[i] == 'l' && plus) || (modes[i] == 'k' && plus) || modes[i] == 'o')
            nOfModesNeedsParam++;
        i++;
    }
    if (paramsSize < nOfModesNeedsParam)
        return 0;
    return 1;
}

void Server::addModes(std::deque<std::string> &params, std::deque<char> &modes, int index, std::string &channelName)
{
    Channel &channel = this->channels[find_channel(channelName)];

    if (!modes.size())
    {
        std::string rpl = RPL_CHANNELMODES(this->clients[index].getip_address(), channelName, this->clients[index].getnickname(), channel.getModes());
        if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
            throw std::runtime_error("send failed");
        return;
    }

    while (modes.size())
    {
        if (modes[0] == 'i' && !channel.getModeI())
        {
            channel.setModeI(true);
            std::string rpl = RPL_MODEIS(channelName, this->clients[index].getnickname(), "+i");
            if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                throw std::runtime_error("send failed");
            channel_send_message(channel, clients[index], rpl.c_str());
        }
        else if (modes[0] == 'k')
        {
            channel.setModeK(true);
            channel.setPassword(params[0]);
            std::string rpl = RPL_MODEIS(channelName, this->clients[index].getnickname(), "+k");
            if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                throw std::runtime_error("send failed");
            channel_send_message(channel, clients[index], rpl.c_str());
            params.pop_front();
        }
        else if (modes[0] == 't' && !channel.getModeT())
        {
            channel.setModeT(true);
            std::string rpl = RPL_MODEIS(channelName, this->clients[index].getnickname(), "+t");
            if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                throw std::runtime_error("send failed");
            channel_send_message(channel, clients[index], rpl.c_str());
        }
        else if (modes[0] == 'l')
        {
            if (std::atoi(params[0].c_str()) > 0 && (!channel.getModeL() || channel.getUserslimit() != std::atoi(params[0].c_str())))
            {
                channel.setModeL(true);
                channel.setUserslimit(std::atoi(params[0].c_str()));
                std::string rpl = RPL_MODEISLIMIT(channelName, this->clients[index].getnickname(), "+l", std::to_string(channel.getUserslimit()));
                if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    throw std::runtime_error("send failed");
                channel_send_message(channel, clients[index], rpl.c_str());
            }
            params.pop_front();
        }
        else if (modes[0] == 'o')
        {
            int clientIndexInClients = getClientIndexByNickname(params[0]);
            int clientIndex = clientIndexInClients >= 0 ? channel.getChannelClient(this->clients[clientIndexInClients]) : 0;
            if (clientIndexInClients < 0 || clientIndex < 0)
            {
                std::string rpl = ERR_NOSUCHNICK(this->clients[index].getip_address(), channelName, params[0]);
                if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    throw std::runtime_error("send failed");
            }
            else if (!channel.isOperator(this->clients[clientIndexInClients]))
            {
                channel.modifOp(clientIndex, true);
                std::string rpl = RPL_MODEISOP(channelName, this->clients[index].getnickname(), "+o", params[0]);
                if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    throw std::runtime_error("send failed");
                channel_send_message(channel, clients[index], rpl.c_str());
            }
            params.pop_front();
        }
        modes.pop_front();
    }
}

void Server::removeModes(std::deque<std::string> &params, std::deque<char> &modes, int index, std::string &channelName)
{
    Channel &channel = this->channels[find_channel(channelName)];

    while (modes.size())
    {
        if (modes[0] == 'i' && channel.getModeI())
        {
            channel.setModeI(false);
            std::string rpl = RPL_MODEIS(channelName, this->clients[index].getnickname(), "-i");
            if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                throw std::runtime_error("send failed");
            channel_send_message(channel, clients[index], rpl.c_str());
        }
        else if (modes[0] == 'k' && channel.getModeK())
        {
            channel.setModeK(false);
            std::string rpl = RPL_MODEIS(channelName, this->clients[index].getnickname(), "-k");
            if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                throw std::runtime_error("send failed");
            channel_send_message(channel, clients[index], rpl.c_str());
        }
        else if (modes[0] == 't' && channel.getModeT())
        {
            channel.setModeT(false);
            std::string rpl = RPL_MODEIS(channelName, this->clients[index].getnickname(), "-t");
            if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                throw std::runtime_error("send failed");
            channel_send_message(channel, clients[index], rpl.c_str());
        }
        else if (modes[0] == 'l' && channel.getModeL())
        {
            channel.setModeL(false);
            std::string rpl = RPL_MODEIS(channelName, this->clients[index].getnickname(), "-l");
            if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                throw std::runtime_error("send failed");
            channel_send_message(channel, clients[index], rpl.c_str());
        }
        else if (modes[0] == 'o')
        {
            int clientIndexInClients = getClientIndexByNickname(params[0]);
            int clientIndex = clientIndexInClients >= 0 ? channel.getChannelClient(this->clients[clientIndexInClients]) : 0;
            if (clientIndexInClients < 0 || clientIndex < 0)
            {
                std::string rpl = ERR_NOSUCHNICK(this->clients[index].getip_address(), channelName, params[0]);
                if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    throw std::runtime_error("send failed");
            }
            else if (channel.isOperator(this->clients[clientIndexInClients]))
            {
                channel.modifOp(clientIndex, false);
                std::string rpl = RPL_MODEISOP(channelName, this->clients[index].getnickname(), "-o", params[0]);
                if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    throw std::runtime_error("send failed");
                channel_send_message(channel, clients[index], rpl.c_str());
            }
            params.pop_front();
        }
        modes.pop_front();
    }
}

std::deque<std::string> extractModeParams(std::string &command)
{
    std::deque<std::string> prms;
    char *holder = const_cast<char *>(command.c_str());
    char *token = strtok(holder, " ");
    while (token)
    {
        std::cout << "token : " << token << std::endl;
        prms.push_back(token);
        token = strtok(nullptr, " ");
    }
    return prms;
}

void Server::parseMode(std::string &command, int index)
{
    bool plus = true;
    int i = 0;
    std::deque<std::string> allParams = extractModeParams(command);
    while ((unsigned long)i < allParams.size())
    {
        std::cout << "token [" << i << "] : " << allParams[i] << std::endl;
        i++;
    }
    std::string target = allParams[0];
    allParams.pop_front();
    if (target[0] != '#' || this->find_channel(target) == -1)
    {
        std::string rpl = ERR_NOSUCHNICK(this->clients[index].getip_address(), target, target);
        if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
            throw std::runtime_error("send failed");
        return;
    }
    else if (this->channels[this->find_channel(target)].getChannelClient(this->clients[index]) == -1)
    {
        std::string rpl = ERR_NOTONCHANNEL(this->clients[index].getip_address(), target);
        if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
            throw std::runtime_error("send failed");
        return;
    }

    if (!allParams.size())
    {
        std::string rpl = RPL_CHANNELMODES(this->clients[index].getip_address(), target, this->clients[index].getnickname(), this->channels[this->find_channel(target)].getModes());
        if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
            throw std::runtime_error("send failed");
        return;
    }
    std::string modesString = allParams[0];
    std::deque<char> modes;
    allParams.pop_front();
    i = 0;
    while ((unsigned long)i < modesString.length() && !std::isalpha(modesString[i]))
    {
        if (modesString[i] == '-')
            plus = false;
        if (modesString[i] == '+')
            plus = true;
        i++;
    }
    if (!this->channels[this->find_channel(target)].isOperator(this->clients[index]))
    {
        std::string rpl = ERR_NOTOP(this->clients[index].getip_address(), target);
        if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
            throw std::runtime_error("send failed");
        return;
    }
    while (std::isalpha(modesString[i]) && modesString[i])
    {
        if (modesString[i] != 'i' && modesString[i] != 'l' && modesString[i] != 't' && modesString[i] != 'o' && modesString[i] != 'k')
        {
            std::string rpl = ERR_UNKNOWNMODE(this->clients[index].getip_address(), this->clients[index].getnickname(), target, modesString[i]);
            if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                throw std::runtime_error("send failed");
        }
        else
        {
            if (modesString[i] == 'i')
            {
                modes.push_back('i');
            }
            else if (modesString[i] == 'o')
            {
                modes.push_back('o');
            }
            else if (modesString[i] == 'k')
            {
                modes.push_back('k');
            }
            else if (modesString[i] == 't')
            {
                modes.push_back('t');
            }
            else if (modesString[i] == 'l')
            {
                modes.push_back('l');
            }
        }
        i++;
    }
    if (!isThereEnoughParams(allParams.size(), modes, plus))
    {
        std::string rpl = ERR_NEEDMOREPARAMS(this->clients[index].getnickname(), this->clients[index].getip_address());
        std::cout << "sent : " << rpl << std::endl;
        if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
            throw std::runtime_error("send failed");
        return;
    }
    if (plus)
        addModes(allParams, modes, index, target);
    else
        removeModes(allParams, modes, index, target);
}

void Server::extractChannelsName(std::vector<std::string> &channelsNames, std::vector<std::string> &keys, std::string &params)
{
    int i = 0;
    while (params[i] && params[i] == ' ')
        i++;
    std::string channels = "";
    std::string keysString = "";
    while (params[i] && params[i] != ' ')
    {
        channels += params[i];
        i++;
    }
    while (params[i] && params[i] == ' ')
        i++;
    while (params[i])
    {
        keysString += params[i];
        i++;
    }
    i = 0;
    while (channels[i])
    {
        std::string tmpChannels = "";
        while (channels[i] && channels[i] != ',')
        {
            tmpChannels += channels[i];
            i++;
        }
        if (tmpChannels.length())
        {
            channelsNames.push_back(tmpChannels);
        }
        if (channels[i] == ',')
            i++;
    }
    if (keysString[0] == ':')
    {
        keys.push_back(keysString.substr(1, keysString.length()));
    }
    else
    {
        i = 0;
        while (keysString[i])
        {
            std::string tmpKey = "";
            while (keysString[i] && keysString[i] != ',')
            {
                tmpKey += keysString[i];
                i++;
            }
            if (tmpKey.length())
            {
                keys.push_back(tmpKey);
            }
            if (keysString[i] == ',')
                i++;
        }
    }
}

void Server::extractInviteParams(std::vector<std::string> &params, std::string paramsString)
{
    int i = 0;
    while (paramsString[i] == ' ' && paramsString[i])
        i++;
    while (paramsString[i])
    {
        std::string tmpParam = "";
        while (paramsString[i] && paramsString[i] != ' ')
        {
            tmpParam += paramsString[i];
            i++;
        }
        if (tmpParam.length())
        {
            params.push_back(tmpParam);
        }
        while (paramsString[i] && paramsString[i] == ' ')
        {
            if (paramsString[i] == ':')
            {
                break;
            }
            i++;
        }
    }
    if (paramsString[i] == ':')
    {

        std::string tmpParam = "";
        while (paramsString[i])
        {
            tmpParam += paramsString[i];
            i++;
        }
        params.push_back(tmpParam);
    }
}

void Server::extractKickParams(std::vector<std::string> &kickParams, const char *paramsStr)
{
    int i = 0;

    while (paramsStr[i])
    {
        while (paramsStr[i] && paramsStr[i] == ' ')
            i++;
        if (!paramsStr[i] || paramsStr[i] == ':')
            break;
        std::string tmpParam = "";
        while (paramsStr[i] && paramsStr[i] != ' ')
        {
            tmpParam += paramsStr[i];
            i++;
        }
        if (tmpParam.length())
        {
            kickParams.push_back(tmpParam);
        }
    }
    if (paramsStr[i] == ':')
    {
        i++;
        std::string tmpParam = "";
        while (paramsStr[i])
        {
            tmpParam += paramsStr[i];
            i++;
        }
        if (tmpParam.length())
        {
            kickParams.push_back(tmpParam);
        }
    }
}

bool isChannelNameValide(std::string &Channelname)
{
    int i = 0;
    if (Channelname[0] != '#')
        return false;
    while (Channelname[i])
    {
        if (Channelname[i] == ' ' || Channelname[i] == 7)
            return false;
        i++;
    }
    return true;
}

std::string extractCommand(const char *buff)
{
    int i = 0;
    std::string cmd = "";
    while (buff[i] && buff[i] != ' ')
    {
        cmd += buff[i];
        i++;
    }
    return (cmd);
}

void Server::parse(const char *buff, int i)
{
    std::string cmd = extractCommand(buff);
    std::cout << "command == " << cmd << std::endl;
    if (cmd == "bot")
    {
        this->clients[i].setNickName("bot");
        this->clients[i].setUserName("bot");
        this->clients[i].setip_address("127.0.0.1");
        this->clients[i].setEnteredUserName();
        this->clients[i].setEnteredPass();
    }
    if (cmd == "PASS")
    {
        std::string password(buff + 5);
        if (this->clients[i].isAuthenticated())
        {
            std::string rpl = ERR_ALREADYREGISTERED(this->clients[i].getip_address(), this->clients[i].getnickname());
            if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                throw std::runtime_error("send failed");
        }
        else if (!password.length())
        {
            std::string rpl = ERR_NEEDMOREPARAMS(this->clients[i].getip_address(), this->clients[i].getnickname());
            if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                throw std::runtime_error("send failed");
        }
        else if (password.length() == 0 || password != this->password)
        {
            std::string rpl = ERR_PASSWDMISMATCH(this->clients[i].getip_address(), this->clients[i].getnickname());
            if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                throw std::runtime_error("send failed");
        }
        else
            this->clients[i].setPassword(password);
    }
    else if (cmd == "NICK")
    {
        std::string nickname(buff + 5);
        if (!this->clients[i].getEnteredPass())
        {
            std::string rpl = ERR_ALREADYREGISTERED(this->clients[i].getip_address(), this->clients[i].getnickname());
            if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                throw std::runtime_error("send failed");
        }
        else if (!nickname.length())
        {
            std::string rpl = ERR_NONICKNAMEGIVEN(this->clients[i].getip_address(), this->clients[i].getnickname());
            if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                throw std::runtime_error("send failed");
        }
        else if (!isNickNameValid(nickname) || nickname == "bot")
        {
            std::string rpl = ERR_ERRONEUSNICKNAME(this->clients[i].getip_address(), this->clients[i].getnickname());
            if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                throw std::runtime_error("send failed");
        }
        else if (getClientIndexByNickname(nickname) >= 0)
        {
            std::string rpl = ERR_NICKNAMEINUSE(this->clients[i].getip_address(), this->clients[i].getnickname());
            if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                throw std::runtime_error("send failed");
        }
        else
        {
            if (this->clients[i].getEntredNick())
            {
                std::string rpl = RPL_NICKCHANGE(this->clients[i].getnickname(), nickname, this->clients[i].getip_address());
                if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    throw std::runtime_error("send failed");
            }
            this->clients[i].setNickName(nickname);
        }
    }
    else if (cmd == "USER")
    {
        std::string username(buff + 5);
        int len = 0;
        while (std::isalpha(username[len]))
        {
            len++;
        }
        username = username.substr(0, len);
        if (!this->clients[i].getEnteredPass())
        {
            std::string rpl = ERR_ALREADYREGISTERED(this->clients[i].getip_address(), this->clients[i].getnickname());
            if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                throw std::runtime_error("send failed");
        }
        else if (username.length() == 0)
        {
            std::string rpl = ERR_NEEDMOREPARAMS(this->clients[i].getip_address(), this->clients[i].getnickname());
            if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                throw std::runtime_error("send failed");
        }
        else if (this->clients[i].isAuthenticated())
        {
            std::string rpl = ERR_ALREADYREGISTERED(this->clients[i].getip_address(), this->clients[i].getnickname());
            if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                throw std::runtime_error("send failed");
        }
        else
        {
            this->clients[i].setUserName(username);
            this->clients[i].setip_address(inet_ntoa(clientAddress.sin_addr));
            std::string name = RPL_WELCOME(this->clients[i].getnickname(), this->clients[i].getip_address());
            send(this->clients[i].getFd(), name.c_str(), name.size(), 0);
            name = RPL_YOURHOST(this->clients[i].getnickname(), this->clients[i].getip_address());
            send(this->clients[i].getFd(), name.c_str(), name.size(), 0);
            name = RPL_CREATED(this->clients[i].getnickname(), this->clients[i].getip_address());
            send(this->clients[i].getFd(), name.c_str(), name.size(), 0);
            name = RPL_MYINFO(this->clients[i].getnickname(), this->clients[i].getip_address());
            send(this->clients[i].getFd(), name.c_str(), name.size(), 0);
        }
    }
    else if (!clients[i].isAuthenticated())
    {
        std::string rpl = ERR_NOTREGISTERED(this->clients[i].getip_address(), this->clients[i].getnickname());
        if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
            throw std::runtime_error("send failed");
    }
    else if (cmd == "INVITE")
    {
        if (this->clients[i].getEnteredPass() && this->clients[i].getEntredNick())
        {
            std::vector<std::string> channelsNames;
            std::vector<std::string> params;
            std::string paramsString(buff + 6);
            extractInviteParams(params, paramsString);
            if (params.size() < 2)
            {
                std::string rpl = ERR_NEEDMOREPARAMS(this->clients[i].getip_address(), this->clients[i].getnickname());
                if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    throw std::runtime_error("send failed");
            }
            else
            {
                if (this->getClientIndexByNickname(params[0]) < 0)
                {
                    std::string rpl = ERR_NOSUCHNICK(this->clients[i].getip_address(), this->clients[i].getnickname(), params[0]);
                    if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                        throw std::runtime_error("send failed");
                }
                else if (this->find_channel(params[1]) < 0)
                {
                    std::string rpl = ERR_NOSUCHNICK(this->clients[i].getip_address(), this->clients[i].getnickname(), params[1]);
                    if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                        throw std::runtime_error("send failed");
                }
                else if (this->channels[this->find_channel(params[1])].getChannelClient(this->clients[i]) < 0)
                {
                    std::string rpl = ERR_NOTONCHANNEL(this->clients[i].getnickname(), params[0]);
                    if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                        throw std::runtime_error("send failed");
                }
                else if (!this->channels[find_channel(params[1])].isOperator(this->clients[i]))
                {
                    std::string rpl = ERR_NOTOP(this->clients[i].getnickname(), params[1]);
                    if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                        throw std::runtime_error("send failed");
                }
                else if (this->channels[this->find_channel(params[1])].getChannelClient(this->clients[this->getClientIndexByNickname(params[0])]) >= 0)
                {
                    std::string rpl = ERR_USERONCHANNEL(this->clients[i].getip_address(), params[0], params[1]);
                    if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                        throw std::runtime_error("send failed");
                }
                else
                {
                    this->channels[find_channel(params[1])].addInvited(this->clients[this->getClientIndexByNickname(params[0])]);
                    std::string rpl = RPL_INVITING(this->clients[i].getip_address(), this->clients[i].getnickname(), params[0], params[1]);
                    send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0);
                    rpl = RPL_INVITE(this->clients[i].getnickname(), this->clients[i].getUserName(), this->clients[i].getip_address(), params[0], params[1]);
                    for (std::vector<Client>::iterator it = this->clients.begin(); it != this->clients.end(); it++)
                    {
                        if (it->getnickname() == params[0])
                            send(it->getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0);
                    }
                }
            }
        }
    }
    else if (cmd == "JOIN")
    {
        if (this->clients[i].getEnteredPass() && this->clients[i].getEntredNick())
        {
            std::vector<std::string> channelsNames;
            std::vector<std::string> keys;
            std::string channelsParams(buff + 5);
            extractChannelsName(channelsNames, keys, channelsParams);
            for (size_t j = 0; j < channelsNames.size(); j++)
            {
                int channel_index = find_channel(channelsNames[j]);
                if (channel_index >= 0)
                {
                    if (this->channels[channel_index].getChannelClient(clients[i]) >= 0)
                    {
                    }
                    else if (this->channels[channel_index].getModeI() && !this->channels[channel_index].isInvited(this->clients[i]))
                    {
                        std::string rpl = ERR_INVITEONLY(this->clients[i].getnickname(), this->clients[i].getip_address());
                        if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                        {
                            throw std::runtime_error("send failed");
                        }
                    }
                    else if (this->channels[channel_index].isChannelFull())
                    {
                        std::string rpl = ERR_CHANNELISFULL(this->clients[i].getnickname(), this->clients[i].getUserName());
                        if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                        {
                            throw std::runtime_error("send failed");
                        }
                    }
                    else if (this->channels[channel_index].getModeK() && (!keys.size() || this->channels[channel_index].getPassword() != keys[0]))
                    {
                        std::string rpl = ERR_BADCHANNELKEY(this->clients[i].getnickname(), this->clients[i].getip_address(), channelsNames[j]);
                        if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                        {
                            throw std::runtime_error("send failed");
                        }
                    }
                    else if (this->channels[channel_index].getChannelClient(this->clients[i]) == -1)
                    {
                        this->channels[channel_index].addClient(this->clients[i]);
                        std::string rpl;
                        this->channels[channel_index].removeInvitation(this->clients[i]);
                        this->channels[channel_index].pushBackToOppArr();

                        std::string names = "";
                        std::vector<Client> channelClients = this->channels[this->find_channel(channelsNames[j])].getCHannelClients();

                        for (size_t o = 0; o < channelClients.size(); o++)
                        {
                            if (channels[channel_index].isOperator(channelClients[o]))
                                names += "@";
                            names += channelClients[o].getnickname() + " ";
                        }
                        for (size_t o = 0; o < channelClients.size(); o++)
                        {
                            rpl = RPL_JOIN(this->clients[i].getnickname(), this->clients[i].getUserName(), channelsNames[j], this->clients[i].getip_address());
                            send(channelClients[o].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0);
                        }
                        rpl = RPL_NAMREPLY(this->clients[i].getip_address(), names, channelsNames[j], this->clients[i].getnickname());
                        send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0);
                        rpl = RPL_ENDOFNAMES(this->clients[i].getip_address(), this->clients[i].getnickname(), channelsNames[j]);
                        send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0);
                        this->channels[channel_index].getModes();
                        time_t timeNow = time(&timeNow);
                        rpl = RPL_CREATIONTIME(this->clients[i].getip_address(), channelsNames[j], this->clients[i].getnickname(), std::to_string(timeNow));
                        send(this->clients[i].getFd(), rpl.c_str(), rpl.size(), 0);
                    }
                    else
                    {
                    }
                    if (this->channels[channel_index].getModeK() && keys.size())
                        keys.erase(keys.begin());
                }
                else
                {
                    if (!isChannelNameValide(channelsNames[j]))
                    {
                        std::string rpl = ERR_BADCHANNELNAME(this->clients[i].getip_address(), this->clients[i].getnickname(), channelsNames[j]);
                        send(this->clients[i].getFd(), rpl.c_str(), rpl.size(), 0);
                    }
                    else
                    {
                        Channel channel(channelsNames[j]);
                        this->channels.push_back(channel);
                        this->channels[this->channels.size() - 1].addOperator(this->clients[i]);
                        this->channels[this->channels.size() - 1].addClient(this->clients[i]);
                        std::string rpl = RPL_JOIN(this->clients[i].getnickname(), this->clients[i].getUserName(), channelsNames[j], this->clients[i].getip_address());
                        send(this->clients[i].getFd(), rpl.c_str(), rpl.size(), 0);
                        rpl = ":" + this->clients[i].getnickname() + " MODE " + channelsNames[j] + " +ns \r\n";
                        send(this->clients[i].getFd(), rpl.c_str(), rpl.size(), 0);
                        rpl = ":" + clients[i].getip_address() + " 353 " + clients[i].getnickname() + " = " + channelsNames[j] + " :@" + clients[i].getnickname() + " \r\n";
                        send(this->clients[i].getFd(), rpl.c_str(), rpl.size(), 0);
                        rpl = RPL_ENDOFNAMES(this->clients[i].getip_address(), this->clients[i].getnickname(), channelsNames[j]);
                    }
                }
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
        if (channel_index >= 0 && this->channels[channel_index].getChannelClient(clients[i]) >= 0)
        {
            channel_send_message(this->channels[channel_index], clients[i], (":" + clients[i].getnickname() + " " + buff + "\r\n").c_str());
        }
        else if (channel_index == -1)
        {
            std::string rpl = ERR_NOSUCHCHANNEL(channel_name, channel_name, clients[i].getnickname());
            if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                throw std::runtime_error("send failed");
        }
        else if (channel_index >= 0 && this->channels[channel_index].getChannelClient(clients[i]) == -1)
        {
            std::string rpl = ERR_NOTONCHANNEL(channel_name, channel_name);
            if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                throw std::runtime_error("send failed");
        }
    }

    else if (cmd == "PRIVMSG")
    {
        std::string comand = buff + 8;
        size_t pos = comand.find(' ');
        std::string name = comand.substr(0, pos);
        std::string msg = comand.substr(pos + 1, comand.size() - name.size());
        if (msg[0] == ':')
            msg = comand.substr(pos + 2, comand.size() - name.size());
        for (std::vector<Client>::iterator sender = clients.begin(); sender != clients.end(); sender++)
        {
            if (sender->getFd() == this->clients[i].getFd())
            {
                for (std::vector<Client>::iterator resever = clients.begin(); resever != clients.end(); resever++)
                {
                    if (resever->getnickname() == name)
                    {
                        std::string privmsg = PRIVMSG_FORMAT(sender->getnickname(), sender->getUserName(), sender->getip_address(), resever->getnickname(), msg.c_str());
                        send(resever->getFd(), privmsg.c_str(), privmsg.size(), 0);
                    }
                }
            }
        }
    }
    else if (cmd == "MODE")
    {
        if (std::strlen(buff) == 4)
        {
            std::cout << "target is not entered" << std::endl;
            std::string rpl = ERR_NEEDMOREPARAMS(this->clients[i].getnickname(), this->clients[i].getip_address());
            send(this->clients[i].getFd(), rpl.c_str(), rpl.length(), 0);
        }
        else
        {
            std::string restOfCommand(buff + 5);
            std::cout << "restOfCommand : " << restOfCommand << " len : " << restOfCommand.length() << std::endl;
            parseMode(restOfCommand, i);
        }
    }
    else if (cmd == "KICK")
    {
        std::vector<std::string> kickParams;
        extractKickParams(kickParams, buff + 5);
        for (size_t j = 0; j < kickParams.size(); j++)
        {
            std::cout << "param[" << j << "] : " << kickParams[j] << std::endl;
        }
        if (kickParams.size() < 2)
        {
            std::string rpl = ERR_NEEDMOREPARAMS(this->clients[i].getip_address(), this->clients[i].getnickname());
            if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                throw std::runtime_error("send failed");
        }
        else if (this->find_channel(kickParams[0]) < 0 || this->channels[find_channel(kickParams[0])].getChannelClient(clients[i]) < 0)
        {
            std::string rpl = ERR_NOSUCHCHANNEL(this->clients[i].getip_address(), kickParams[0], this->clients[i].getnickname());
            if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                throw std::runtime_error("send failed");
        }
        else if (!this->channels[find_channel(kickParams[0])].isOperator(this->clients[i]))
        {
            std::string rpl = ERR_NOTOP(this->clients[i].getip_address(), kickParams[0]);
            if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                throw std::runtime_error("send failed");
        }
        else if (this->getClientIndexByNickname(kickParams[1]) < 0 || this->channels[find_channel(kickParams[0])].getChannelClient(this->clients[this->getClientIndexByNickname(kickParams[1])]) < 0)
        {
            std::cout << "nick : " << kickParams[1] << std::endl;
            std::cout << "nick : " << clients[1].getnickname() << std::endl;
            std::string rpl = ERR_USERNOTINCHANNEL(this->clients[i].getip_address(), kickParams[0]);
            if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                throw std::runtime_error("send failed");
        }
        else
        {
            std::string rpl = RPL_KICK(this->clients[i].getnickname(), this->clients[i].getUserName(), this->clients[i].getip_address(), kickParams[0], kickParams[1], (kickParams.size() == 3 ? kickParams[2] : "for no reason"));
            channel_send_message(this->channels[find_channel(kickParams[0])], this->clients[i], rpl.c_str());
            if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                throw std::runtime_error("send failed");
            std::cout << "herrororororo\n";
            this->channels[find_channel(kickParams[0])].removeAClientFromChannel(this->channels[find_channel(kickParams[0])].getChannelClient(this->clients[this->getClientIndexByNickname(kickParams[1])]));
            if (this->channels[find_channel(kickParams[0])].getCHannelClients().size() == 0)
            {
                this->channels.erase(channels.begin() + find_channel(kickParams[0]));
            }
        }
    }
    else if (cmd == "TOPIC")
    {
        std::vector<std::string> topicParams;
        extractKickParams(topicParams, buff + 5);
        for (size_t j = 0; j < topicParams.size(); j++)
        {
            std::cout << "param[" << j << "] : " << topicParams[j] << std::endl;
        }
        if (topicParams.size() < 1)
        {
            std::string rpl = ERR_NEEDMOREPARAMS(this->clients[i].getip_address(), this->clients[i].getnickname());
            if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                throw std::runtime_error("send failed");
        }
        else if (this->find_channel(topicParams[0]) < 0 || this->channels[find_channel(topicParams[0])].getChannelClient(clients[i]) < 0)
        {
            std::string rpl = ERR_NOSUCHCHANNEL(this->clients[i].getip_address(), topicParams[0], this->clients[i].getnickname());
            if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                throw std::runtime_error("send failed");
        }
        else if (!this->channels[find_channel(topicParams[0])].isOperator(this->clients[i]) && topicParams.size() == 2 && !this->channels[find_channel(topicParams[0])].getModeT())
        {
            std::string rpl = ERR_NOTOP(this->clients[i].getip_address(), topicParams[0]);
            if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                throw std::runtime_error("send failed");
        }
        else if (topicParams.size() == 2)
        {
            this->channels[find_channel(topicParams[0])].setTopic(topicParams[1]);
            std::string rpl = RPL_SETTOPIC(this->clients[i].getnickname(), this->clients[i].getip_address(), topicParams[0], topicParams[1]);
            channel_send_message(this->channels[find_channel(topicParams[0])], this->clients[i], rpl.c_str());
            if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                throw std::runtime_error("send failed");
            std::cout << "herrororororo\n";
        }
        else
        {
            std::string rpl = RPL_TOPICDISPLAY(this->clients[i].getip_address(), this->clients[i].getnickname(), topicParams[0], this->channels[find_channel(topicParams[0])].getTopic());
            if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                throw std::runtime_error("send failed");
            std::cout << "herrororororo\n";
        }
    }
    else if (cmd != "PONG")
    {
        std::string rpl = ERR_UNKNOWNCOMMAND(this->clients[i].getip_address(), this->clients[i].getnickname(), cmd);
        if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
            throw std::runtime_error("send failed");
        std::cout << "herrororororo\n";
    }
}

struct sockaddr_in *Server::createIPv4Address(int port)
{
    struct sockaddr_in *address = new sockaddr_in();
    address->sin_port = htons(port);
    address->sin_family = AF_INET;
    address->sin_addr.s_addr = INADDR_ANY;

    return address;
}

void Server::createNewClienFD()
{
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

void Server::pollinFDs()
{
    int fds_size = fds.size();
    for (int i = 1; i < fds_size; i++)
    {
        if (fds[i].revents & POLLIN)
        {
            char buff[1024];
            memset(buff, 0, 1024);
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
                    std::cout << "channels size :" << size << std::endl;
                    for (int x = 0; x < size; x++)
                    {
                        int index = this->channels[x].getChannelClient(this->clients[i - 1]);
                        std::cout << "index : " << index << std::endl;
                        if (index >= 0)
                        {
                            std::cout << "removed from channel : " << this->channels[x].get_name() << std::endl;
                            this->channels[x].removeAClientFromChannel(index);
                            if (!this->channels[x].getNumberOfClients())
                            {
                                this->channels.erase(this->channels.begin() + x);
                                size--;
                                x--;
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
                std::string lastBuff = buff;
                memset(buff, 0, 1024);
                std::cout << "lastbuff1 " << lastBuff << "XX" << static_cast<int>(buff[bytesRead - 1]) << "XX";
                while (lastBuff[lastBuff.length() - 1] != '\n')
                {
                    std::cout << "here\n";
                    bytesRead = recv(fds[i].fd, buff, sizeof(buff) - 1, 0);
                    std::string holder(buff);
                    lastBuff += holder;
                    memset(buff, 0, 1024);
                    std::cout << "lastbuff2 " << lastBuff;
                }
                if (lastBuff[lastBuff.length() - 1] == '\n' || lastBuff[lastBuff.length() - 1] == '\r')
                    lastBuff = lastBuff.substr(0, lastBuff.length() - 1);
                if (lastBuff[lastBuff.length() - 1] == '\n' || lastBuff[lastBuff.length() - 1] == '\r')
                    lastBuff = lastBuff.substr(0, lastBuff.length() - 1);
                std::cout << "Received from client " << i << ": " << lastBuff << " X " << lastBuff.length() << std::endl;
                parse(lastBuff.c_str(), i - 1);
            }
        }
    }
}

void Server::serverPoll()
{
    while (true)
    {
        int ready = poll(this->fds.data(), this->fds.size(), 100);
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

Server::Server(char *port, char *password)
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

    if (setsockopt(this->sockFD, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        close(this->sockFD);
        std::runtime_error("setsockopt() failed");
    }

    struct sockaddr_in *address = createIPv4Address(this->port);

    if (bind(this->sockFD, (struct sockaddr *)address, sizeof *address) == 0)
    {
        std::cout << "Socket bound" << std::endl;
    }
    else
    {
        close(this->sockFD);
        delete address;
        throw std::runtime_error("Bind failed xx");
    }

    if (listen(this->sockFD, SOMAXCONN) == 0)
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