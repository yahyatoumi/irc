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

class Channel{
private:
    std::vector<Client> chanel_clients;
    std::vector<Client> operators;
    std::string name;
    bool invite_only;

public:
    Channel();
    Channel(std::string &name);
    ~Channel();

    std::string get_name() const;
    std::vector<Client> getChannelClient() const;
};

#endif