/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ytoumi <ytoumi@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/15 19:21:45 by ytoumi            #+#    #+#             */
/*   Updated: 2023/11/15 20:11:33 by ytoumi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL
#define CHANNEL

#include <iostream>
#include "Client.hpp"
#include <vector>

class Client;

class Channel
{
private:
    std::vector<Client> channel_clients;
    std::vector<Client> invitedClients;
    std::vector<bool> channelOpArr;
    std::vector<Client> operators;
    std::string name;
    std::string password;
    std::string topic;
    size_t channelUsersLimit;
    bool modeI;
    bool modeK;
    bool modeT;
    bool modeL;
    std::vector<char> modes;

public:
    Channel();
    Channel(std::string &name);
    ~Channel();
    bool isChannelFull()
    {
        if (this->modeL && channel_clients.size() >= channelUsersLimit)
            return true;
        return false;
    }
    void addInvited(Client &client)
    {
        for (size_t i = 0; i < invitedClients.size(); i++)
        {
            if (client == invitedClients[i])
                return;
        }
        this->invitedClients.push_back(client);
    }
    void removeInvitation(Client &client)
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
    bool isInvited(Client &client)
    {
        for (size_t i = 0; i < invitedClients.size(); i++)
        {
            if (client == invitedClients[i])
                return true;
        }
        return false;
    }
    std::string getModes()
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
    std::string getTopic()
    {
        return this->topic;
    }
    void setTopic(std::string newTopic)
    {
        modeT = true;
        this->topic = newTopic;
    }
    int getUserslimit()
    {
        return this->channelUsersLimit;
    }
    void setUserslimit(int limit)
    {
        this->modeL = true;
        this->channelUsersLimit = limit;
    }
    void setPassword(std::string pass)
    {
        this->modeK = true;
        this->password = pass;
    }
    std::string getPassword()
    {
        return this->password;
    }
    std::string get_name() const
    {
        return this->name;
    }
    std::vector<Client> getCHannelClients() const
    {
        return this->channel_clients;
    }
    int getChannelClient(const Client &client) const
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
    void addClient(Client &client)
    {
        this->channel_clients.push_back(client);
    }
    int getNumberOfClients()
    {
        return this->channel_clients.size();
    }
    void removeAClientFromChannel(int index)
    {
        this->channel_clients.erase(this->channel_clients.begin() + index);
    }
    int addOperator(Client &client)
    {
        int clientIndex = getChannelClient(client);
        std::cout << clientIndex << std::endl;
        if (clientIndex >= 0 && this->channelOpArr[clientIndex] == true)
            return -1;
        this->channelOpArr.push_back(1);
        return clientIndex;
    }
    bool isOperator(Client &client)
    {
        int i = getChannelClient(client);
        std::cout << channelOpArr[i] << "babe\n";
        if (i >= 0)
            return this->channelOpArr[i];
        return false;
    }
    bool getModeI()
    {
        return this->modeI;
    }
    bool getModeK()
    {
        return this->modeK;
    }
    bool getModeT()
    {
        return this->modeT;
    }
    bool getModeL()
    {
        return this->modeL;
    }
    void setModeI(bool state)
    {
        this->modeI = state;
        if (std::find(modes.begin(), modes.end(), 'i') == modes.end() && state)
            modes.push_back('i');
        else if (!state)
            modes.erase(std::find(modes.begin(), modes.end(), 'i'));
    }
    void setModeL(bool state)
    {
        this->modeL = state;
        if (std::find(modes.begin(), modes.end(), 'l') == modes.end() && state)
            modes.push_back('l');
        else if (!state)
            modes.erase(std::find(modes.begin(), modes.end(), 'l'));
    }
    void setModeT(bool state)
    {
        this->modeT = state;
        if (std::find(modes.begin(), modes.end(), 't') == modes.end() && state)
            modes.push_back('t');
        else if (!state)
            modes.erase(std::find(modes.begin(), modes.end(), 't'));
    }
    void setModeK(bool state)
    {
        this->modeK = state;
        if (std::find(modes.begin(), modes.end(), 'k') == modes.end() && state)
            modes.push_back('k');
        else if (!state)
            modes.erase(std::find(modes.begin(), modes.end(), 'k'));
    }
    void modifOp(int index, int state)
    {
        this->channelOpArr[index] = state;
    }
};

#endif