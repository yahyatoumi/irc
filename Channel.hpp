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
    bool isChannelFull();
    void addInvited(Client &client);
    void removeInvitation(Client &client);
    bool isInvited(Client &client);
    std::string getModes();
    std::string getTopic();
    void setTopic(std::string newTopic);
    int getUserslimit();
    void setUserslimit(int limit);
    void setPassword(std::string pass);
    std::string getPassword();
    std::string get_name() const;
    std::vector<Client> getCHannelClients() const;
    int getChannelClient(const Client &client) const;
    void addClient(Client &client);
    int getNumberOfClients();
    void removeAClientFromChannel(int index);
    int addOperator(Client &client);
    bool isOperator(Client &client);
    bool getModeI();
    bool getModeK();
    bool getModeT();
    bool getModeL();
    void setModeI(bool state);
    void setModeL(bool state);
    void setModeT(bool state);
    void setModeK(bool state);
    void modifOp(int index, int state);
};

#endif