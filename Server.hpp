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
            // std::cout << this->clients[i].getnickname() << this->clients[i].getnickname().length() << std::endl;
            // std::cout << nickname << nickname.length() << std::endl;
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

    void channel_send_message(Channel &channel, Client &sender, const char *message)
    {
        std::vector<Client> clients = channel.getCHannelClients();
        std::cout << clients.size() << " sizeeeeeezezezezeze  " << message << std::endl;
        std::cout << "sizeeeeee" << clients.size() << std::endl;
        std::string stringMessage(message);
        for (int i = 0; i < clients.size(); i++)
        {
            std::cout << i << 'x' << std::endl;
            if (clients[i].getFd() != sender.getFd())
            {
                std::cout << "sent to " << clients[i].getnickname() << std::endl;
                send(clients[i].getFd(), message, std::strlen(message), 0);
            }
        }
    }

    bool isThereEnoughParams(int paramsSize, std::deque<char> &modes, int plus)
    {
        int nOfModesNeedsParam = 0;
        int i = 0;
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

    void addModes(std::deque<std::string> &params, std::deque<char> &modes, int index, std::string &channelName)
    {
        Channel &channel = this->channels[find_channel(channelName)];

        if (!modes.size())
        {
            // display modes;
            std::string rpl = RPL_CHANNELMODES(this->hostname, channelName, this->clients[index].getnickname(), channel.getModes());
            std::cout << "sent :" << rpl;
            if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                std::runtime_error("send failed");
            return;
        }
        if (modes.size())
            std::cout << modes.size() << modes[0] << std::endl;
        if (params.size())
            std::cout << params.size() << params[0] << std::endl;
        while (modes.size())
        {
            if (modes[0] == 'i' && !channel.getModeI())
            {
                channel.setModeI(true);
                std::cout << "entred\n";
                std::string rpl = RPL_MODEIS(channelName, this->clients[index].getnickname(), "+i");
                std::cout << channelName << " " << this->clients[index].getnickname() << std::endl;
                std::cout << "sent :" << rpl;
                if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    std::runtime_error("send failed");
                channel_send_message(channel, clients[index], rpl.c_str());
            }
            else if (modes[0] == 'k')
            {
                channel.setModeK(true);
                channel.setPassword(params[0]);
                std::cout << "entred\n";
                std::string rpl = RPL_MODEIS(channelName, this->clients[index].getnickname(), "+k");
                std::cout << channelName << " " << this->clients[index].getnickname() << std::endl;
                std::cout << "sent :" << rpl;
                if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    std::runtime_error("send failed");
                channel_send_message(channel, clients[index], rpl.c_str());
                params.pop_front();
            }
            else if (modes[0] == 't' && !channel.getModeT())
            {
                channel.setModeT(true);
                std::cout << "entred\n";
                std::string rpl = RPL_MODEIS(channelName, this->clients[index].getnickname(), "+t");
                std::cout << channelName << " " << this->clients[index].getnickname() << std::endl;
                std::cout << "sent :" << rpl;
                if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    std::runtime_error("send failed");
                channel_send_message(channel, clients[index], rpl.c_str());
            }
            else if (modes[0] == 'l')
            {
                if (std::atoi(params[0].c_str()) > 0 && (!channel.getModeL() || channel.getUserslimit() != std::atoi(params[0].c_str())))
                {
                    std::cout << "dududududud\n";
                    channel.setModeL(true);
                    channel.setUserslimit(std::atoi(params[0].c_str()));
                    std::cout << "entred\n";
                    std::string rpl = RPL_MODEISLIMIT(channelName, this->clients[index].getnickname(), "+l", std::to_string(channel.getUserslimit()));
                    std::cout << channelName << " " << this->clients[index].getnickname() << std::endl;
                    std::cout << "sent :" << rpl;
                    if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                        std::runtime_error("send failed");
                    channel_send_message(channel, clients[index], rpl.c_str());
                }
                params.pop_front();
            }
            else if (modes[0] == 'o')
            {
                std::cout << "fofo\n";
                int clientIndexInClients = getClientIndexByNickname(params[0]);
                int clientIndex = clientIndexInClients >= 0 ? channel.getChannelClient(this->clients[clientIndexInClients]) : 0;
                if (clientIndexInClients < 0 || clientIndex < 0)
                {
                    std::cout << "xxxxxxxxxxxxxxxpppppppp\n";
                    std::string rpl = ERR_NOSUCHNICK(this->hostname, channelName, params[0]);
                    if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                        throw std::runtime_error("send failed");
                }
                else if (!channel.isOperator(this->clients[clientIndexInClients]))
                {
                    std::cout << "before endddddddd" << clientIndex << std::endl;
                    channel.modifOp(clientIndex, true);
                    std::string rpl = RPL_MODEISOP(channelName, this->clients[index].getnickname(), "+o", params[0]);
                    std::cout << "sent :" << rpl;
                    if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                        std::runtime_error("send failed");
                    channel_send_message(channel, clients[index], rpl.c_str());
                }
                params.pop_front();
            }
            modes.pop_front();
        }
    }
    void removeModes(std::deque<std::string> &params, std::deque<char> &modes, int index, std::string &channelName)
    {
        Channel &channel = this->channels[find_channel(channelName)];

        if (modes.size())
            std::cout << modes.size() << modes[0] << std::endl;
        if (params.size())
            std::cout << params.size() << params[0] << std::endl;
        std::cout << modes.size() << "7777777777arb\n";
        while (modes.size())
        {
            if (modes[0] == 'i' && channel.getModeI())
            {
                channel.setModeI(false);
                std::cout << "entred\n";
                std::string rpl = RPL_MODEIS(channelName, this->clients[index].getnickname(), "-i");
                std::cout << channelName << " " << this->clients[index].getnickname() << std::endl;
                std::cout << "sent :" << rpl;
                if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    std::runtime_error("send failed");
                channel_send_message(channel, clients[index], rpl.c_str());
            }
            else if (modes[0] == 'k' && channel.getModeK())
            {
                channel.setModeK(false);
                std::string rpl = RPL_MODEIS(channelName, this->clients[index].getnickname(), "-k");
                std::cout << channelName << " " << this->clients[index].getnickname() << std::endl;
                std::cout << "sent :" << rpl;
                if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    std::runtime_error("send failed");
                channel_send_message(channel, clients[index], rpl.c_str());
            }
            else if (modes[0] == 't' && channel.getModeT())
            {
                channel.setModeT(false);
                std::cout << "entred\n";
                std::string rpl = RPL_MODEIS(channelName, this->clients[index].getnickname(), "-t");
                std::cout << channelName << " " << this->clients[index].getnickname() << std::endl;
                std::cout << "sent :" << rpl;
                if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    std::runtime_error("send failed");
                channel_send_message(channel, clients[index], rpl.c_str());
            }
            else if (modes[0] == 'l' && channel.getModeL())
            {
                std::cout << "dududududud\n";
                channel.setModeL(false);
                std::cout << "entred\n";
                std::string rpl = RPL_MODEIS(channelName, this->clients[index].getnickname(), "-l");
                std::cout << channelName << " " << this->clients[index].getnickname() << std::endl;
                std::cout << "sent :" << rpl;
                if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    std::runtime_error("send failed");
                channel_send_message(channel, clients[index], rpl.c_str());
            }
            else if (modes[0] == 'o')
            {
                std::cout << "fofo\n";
                int clientIndexInClients = getClientIndexByNickname(params[0]);
                int clientIndex = clientIndexInClients >= 0 ? channel.getChannelClient(this->clients[clientIndexInClients]) : 0;
                if (clientIndexInClients < 0 || clientIndex < 0)
                {
                    std::cout << "xxxxxxxxxxxxxxxpppppppp\n";
                    std::string rpl = ERR_NOSUCHNICK(this->hostname, channelName, params[0]);
                    if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                        throw std::runtime_error("send failed");
                }
                else if (channel.isOperator(this->clients[clientIndexInClients]))
                {
                    std::cout << "before endddddddd" << clientIndex << std::endl;
                    channel.modifOp(clientIndex, false);
                    std::string rpl = RPL_MODEISOP(channelName, this->clients[index].getnickname(), "-o", params[0]);
                    std::cout << "sent :" << rpl;
                    if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                        std::runtime_error("send failed");
                    channel_send_message(channel, clients[index], rpl.c_str());
                }
                params.pop_front();
            }
            modes.pop_front();
        }
    }

    void parseMode(std::string &command, int index)
    {
        bool plus = true;
        int i = 0;
        bool modeI = false;
        bool modeK = false;
        bool modeT = false;
        bool modeL = false;
        bool modeO = false;
        std::string target = "";
        std::deque<std::string> params;
        std::deque<char> modes;
        std::cout << "comd :" << command << std::endl;
        while (command[i] == ' ')
        {
            i++;
        }
        target += command[i];
        i++;
        while (std::isalpha(command[i]))
        {
            target += command[i];
            i++;
        }
        std::cout << "channel name :" << target << std::endl;
        if (target[0] != '#' || this->find_channel(target) == -1)
        {
            std::string rpl = ERR_NOSUCHNICK(this->hostname, target, target);
            if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                std::runtime_error("send failed");
            return;
        }
        else if (this->channels[this->find_channel(target)].getChannelClient(this->clients[index]) == -1)
        {
            std::string rpl = ERR_NOTONCHANNEL(this->hostname, target);
            if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                std::runtime_error("send failed");
            return;
        }
        else if (!this->channels[this->find_channel(target)].isOperator(this->clients[index]))
        {
            std::string rpl = ERR_NOTOP(this->hostname, target);
            if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                std::runtime_error("send failed");
            return;
        }
        while (command[i] && !std::isalpha(command[i]))
        {
            i++;
        }

        if (command[i - 1] == '-' && command[i])
        {
            plus = false;
        }
        std::cout << "ii2 : " << i << std::endl;
        while (std::isalpha(command[i]) && command[i])
        {
            if (command[i] != 'i' && command[i] != 'l' && command[i] != 't' && command[i] != 'o' && command[i] != 'k')
            {
                std::cout << "hereooooo\n";
                std::string rpl = ERR_UNKNOWNMODE(this->hostname, this->clients[index].getnickname(), target, command[i]);
                if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    std::runtime_error("send failed");
                std::cout << command[i] << "is not a recognised channel mode." << std::endl;
            }
            else
            {
                if (command[i] == 'i')
                {
                    modes.push_back('i');
                }
                else if (command[i] == 'o')
                {
                    modes.push_back('o');
                }
                else if (command[i] == 'k')
                {
                    modes.push_back('k');
                }
                else if (command[i] == 't')
                {
                    modes.push_back('t');
                }
                else if (command[i] == 'l')
                {
                    modes.push_back('l');
                }
            }
            i++;
        }
        std::cout << "ii2 : " << i << std::endl;

        while (command[i])
        {
            while (command[i] && command[i] == ' ')
                i++;
            if (command[i])
            {
                std::cout << "YY1" << (int)command[i] << "YY1" << std::endl;
                params.push_back("");
                while (command[i] != ' ' && command[i])
                    params[params.size() - 1] += command[i++];
            }
        }
        std::cout << "modeI : " << modeI << std::endl;
        std::cout << "modeO : " << modeO << std::endl;
        std::cout << "modeK : " << modeK << std::endl;
        std::cout << "modeT : " << modeT << std::endl;
        std::cout << "modeL : " << modeL << std::endl;
        std::cout << params.size() << std::endl;
        std::cout << modes.size() << std::endl;
        int stacki = 0;
        while (stacki < params.size())
        {
            std::cout << "params[" << stacki << "] :" << params[stacki] << std::endl;
            stacki++;
        }
        stacki = 0;
        while (stacki < modes.size())
        {
            std::cout << "stack[" << stacki << "] :" << modes[stacki] << std::endl;
            stacki++;
        }
        std::cout << "fififififififi\n";
        if (!isThereEnoughParams(params.size(), modes, plus))
        {
            std::string rpl = ERR_NEEDMOREPARAMS(this->hostname, this->clients[index].getnickname());
            if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                std::runtime_error("send failed");
            return;
        }
        if (plus)
            addModes(params, modes, index, target);
        else
            removeModes(params, modes, index, target);
        // Channel &channel = this->channels[find_channel(target)];
        // std::string username;
        // std::string number;
        // if (params.size() == 2)
        // {
        //     username = params[0];
        //     number = params[1];
        // }
        // else if (params.size() && modeL)
        // {
        //     username = "";
        //     number = params[0];
        // }
        // else if (params.size() && modeO)
        // {
        //     username = params[0];
        //     number = "";
        // }
        // int clientIndex = channel.getChannelClient(this->clients[getClientIndexByNickname(username)]);
        // std::cout << clientIndex << channel.isOperator(this->clients[index]) << std::endl;
        // if (plus)
        // {
        //     if (modeI && !channel.getModeI())
        //     {
        //         channel.setModeI(true);
        //         std::cout << "entred\n";
        //         std::string rpl = RPL_MODEIS(target, this->clients[index].getnickname(), "+i");
        //         std::cout << target << " " << this->clients[index].getnickname() << std::endl;
        //         std::cout << "sent :" << rpl;
        //         if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
        //             std::runtime_error("send failed");
        //         channel_send_message(channel, clients[index], rpl.c_str());
        //     }
        //     if (modeO && clientIndex != -1 && !channel.isOperator(this->clients[getClientIndexByNickname(username)]))
        //     {
        //         std::cout << "before endddddddd" << clientIndex << std::endl;
        //         channel.modifOp(clientIndex, true);
        //         std::string rpl = RPL_MODEISOP(target, this->clients[index].getnickname(), "+o", username);
        //         std::cout << "sent :" << rpl;
        //         if (send(this->clients[index].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
        //             std::runtime_error("send failed");
        //         channel_send_message(channel, clients[index], rpl.c_str());
        //     }
        // }
        // std::cout << "endddddddd\n";
    }
    void extractChannelsName(std::vector<std::string> &channelsNames, std::vector<std::string> &keys, std::string &params)
    {
        int i = 0;
        while (params[i] && params[i] == ' ')
            i++;
        std::string channels = "";
        while (params[i] && params[i] != ' ')
        {
            channels += params[i];
            i++;
        }
        while (params[i] && params[i] == ' ')
            i++;
        while (params[i])
        {
            std::string tmpKey = "";
            while (params[i] && params[i] != ' ')
            {
                tmpKey += params[i];
                i++;
            }
            if (tmpKey.length())
            {
                keys.push_back(tmpKey);
                std::cout << "key : " << tmpKey << std::endl;
            }
            while (params[i] && params[i] == ' ')
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
                std::cout << "channel : " << tmpChannels << std::endl;
            }
            if (channels[i] == ',')
                i++;
        }
    }
    void parse(const char *buff, int i)
    {
        std::cout << "xxxxxxxxxxxxxxx       x xxxxxxxx x x x x\n";
        if (!std::strncmp(buff, "PASS ", 5))
        {
            std::string password(buff + 5);
            if (this->clients[i].isAuthenticated())
            {
                std::string rpl = ERR_ALREADYREGISTERED("hostname", this->clients[i].getnickname());
                if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    std::runtime_error("send failed");
            }
            else if (!password.length())
            {
                std::string rpl = ERR_NEEDMOREPARAMS("hostname", this->clients[i].getnickname());
                if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    std::runtime_error("send failed");
            }
            else if (password.length() == 0 || password != this->password)
            {
                std::string rpl = ERR_PASSWDMISMATCH("hostname", this->clients[i].getnickname());
                if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    std::runtime_error("send failed");
            }
            else
                this->clients[i].setPassword(password);
        }
        else if (!std::strncmp(buff, "NICK ", 5))
        {
            std::string nickname(buff + 5);
            std::cout << "nick : " << nickname << std::endl;
            if (!this->clients[i].getEnteredPass())
            {
                std::string rpl = ERR_ALREADYREGISTERED("127.0.0.1", this->clients[i].getnickname());
                if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    std::runtime_error("send failed");
            }
            else if (!nickname.length())
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
        else if (!std::strncmp(buff, "USER ", 5))
        {
            std::string username(buff + 5);
            int len = 0;
            while (std::isalpha(username[len]))
            {
                len++;
            }
            username = username.substr(0, len);
            std::cout << "now username " << username << std::endl;
            if (!this->clients[i].getEnteredPass())
            {
                std::string rpl = ERR_ALREADYREGISTERED("127.0.0.1", this->clients[i].getnickname());
                if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    std::runtime_error("send failed");
            }
            else if (username.length() == 0)
            {
                std::cout << "nickname is now : " << username;
                std::string rpl = ERR_NEEDMOREPARAMS("127.0.0.1", this->clients[i].getnickname());
                if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    std::runtime_error("send failed");
            }
            else if (this->clients[i].isAuthenticated())
            {
                std::string rpl = ERR_ALREADYREGISTERED("127.0.0.1", this->clients[i].getnickname());
                if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    std::runtime_error("send failed");
            }
            else
            {
                this->clients[i].setUserName(username);
            }
        }
        else if (!clients[i].isAuthenticated())
        {
            std::string rpl = ERR_NOTREGISTERED("127.0.0.1", this->clients[i].getnickname());
            if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                std::runtime_error("send failed");
        }
        else if (!std::strncmp(buff, "JOIN #", 6))
        {
            if (this->clients[i].getEnteredPass() && this->clients[i].getEntredNick())
            {
                std::vector<std::string> channelsNames;
                std::vector<std::string> keys;
                std::string channelsParams(buff + 5);
                extractChannelsName(channelsNames, keys, channelsParams);
                for (int j = 0; j < channelsNames.size(); j++)
                {
                    int channel_index = find_channel(channelsNames[j]);
                    if (channel_index >= 0)
                    {
                        if (this->channels[channel_index].getChannelClient(this->clients[i]) == -1)
                        {
                            std::cout << "foooo" << std::endl;
                            this->channels[channel_index].addClient(this->clients[i]);
                            std::cout << this->clients[i].getnickname() << " is now a member in 1 " << this->channels[channel_index].get_name() << std::endl;
                            std::string rpl = RPL_JOIN(this->clients[i].getnickname(), this->clients[i].getUserName(), channelsNames[j], "127.0.0.1");
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
                        Channel channel(channelsNames[j]);
                        this->channels.push_back(channel);
                        std::cout << this->clients.size() << std::endl;
                        this->channels[this->channels.size() - 1].addOperator(this->clients[i]);
                        this->channels[this->channels.size() - 1].addClient(this->clients[i]);
                        std::cout << this->clients[i].getnickname() << " is now a member in 2 " << channel.get_name() << std::endl;
                        std::string rpl = RPL_JOIN(this->clients[i].getnickname(), this->clients[i].getUserName(), channelsNames[j], "127.0.0.1");
                        std::cout << rpl;
                        std::cout << "i : " << i << std::endl;
                        send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0);
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
            std::cout << "glockkkkk " << channel_index << channel_name << std::endl;
            if (channel_index >= 0 && this->channels[channel_index].getChannelClient(clients[i]) >= 0)
            {
                std::cout << "IS OPERATOOOOOR : " << this->channels[channel_index].isOperator(this->clients[i]) << std::endl;
                channel_send_message(this->channels[channel_index], clients[i], (":" + clients[i].getnickname() + " " + buff + "\r\n").c_str());
            }
            else if (channel_index == -1)
            {
                std::string rpl = ERR_NOSUCHCHANNEL(channel_name, channel_name, clients[i].getnickname());
                if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    std::runtime_error("send failed");
            }
            else if (channel_index >= 0 && this->channels[channel_index].getChannelClient(clients[i]) == -1)
            {
                std::string rpl = ERR_NOTONCHANNEL(channel_name, channel_name);
                if (send(this->clients[i].getFd(), rpl.c_str(), std::strlen(rpl.c_str()), 0) < 0)
                    std::runtime_error("send failed");
            }
        }
        else if (!std::strncmp(buff, "MODE", 4))
        {
            if (std::strlen(buff) == 6)
            {
                std::cout << "target is not entered" << std::endl;
            }
            else if (buff[4] == ' ')
            {
                std::string restOfCommand(buff + 5);
                std::cout << "restOfCommand : " << restOfCommand << " len : " << restOfCommand.length() << std::endl;
                parseMode(restOfCommand, i);
            }
            else
            {
                std::cout << "error :: command not correct!!" << std::endl;
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
                    std::cout << "Received from client " << i << ": " << lastBuff << " " << lastBuff.length() << std::endl;
                    if (lastBuff[lastBuff.length() - 1] == '\n' || lastBuff[lastBuff.length() - 1] == '\r')
                        lastBuff = lastBuff.substr(0, lastBuff.length() - 1);
                    if (lastBuff[lastBuff.length() - 1] == '\n' || lastBuff[lastBuff.length() - 1] == '\r')
                        lastBuff = lastBuff.substr(0, lastBuff.length() - 1);
                    parse(lastBuff.c_str(), i - 1);
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
        this->hostname = "hostname";
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