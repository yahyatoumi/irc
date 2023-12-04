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
    int channelUsersLimit;
    bool modeI;
    bool modeK;
    bool modeT;
    bool modeL;

public:
    Channel();
    Channel(std::string &name);
    ~Channel();
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
        for (int i = 0; i < this->channel_clients.size(); i++)
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
    int getNumberOfClients(){
        return this->channel_clients.size();
    }
    void removeAClientFromChannel(int index){
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
    bool isOperator(int i){
        if (i < 0)
            throw std::out_of_range("error: index[-1]");
        return this->channelOpArr[i];
    }
};

#endif