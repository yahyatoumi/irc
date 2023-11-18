/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ytoumi <ytoumi@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/15 19:14:56 by ytoumi            #+#    #+#             */
/*   Updated: 2023/11/15 20:53:05 by ytoumi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT
#define CLIENT

#include <iostream>
#include "Channel.hpp"

class Channel;

class Client{
private:
    std::string nickname;
    std::string host_realname;
    std::string username_onhost;
    std::string server_name;
    Channel chanels[10];

public:
    kick(Chanel &chanel, Client &toBeKicked);
    changeMode(Chanel &chanel);
    invite(Chanel &chanel, Client &toBeInvited);
    changeTopic(Channel &chanel, std::string &newTopic);
    seeTopic(Channel &chanel);
};

#endif