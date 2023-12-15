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

class Channel;

class Client
{
protected:
    std::string nickname;
    std::string userName;
    std::string password;
    std::string ipadress;
    int fd;
    bool enteredPass;
    bool enteredNick;
    bool enteredUserName;

public:
    Client();
    Client(int fd);
    ~Client();
    void setip_address(std::string ip);
    std::string getip_address();
    void setEnteredNick();
    bool getEnteredPass();
    bool getEntredNick();
    bool getEntredUserName();
    void setEnteredUserName();
    void setEnteredPass();
    void setNickName(std::string nickname);
    void setUserName(std::string userName);
    void setPassword(std::string &clientPass);
    bool isAuthenticated();
    std::string getnickname() const;
    std::string getUserName() const;
    int getFd();
    bool operator==(const Client &right) const;
};

#endif