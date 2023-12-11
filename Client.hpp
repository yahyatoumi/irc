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
    Client()
    {
    }
    // Client(Client &toCopy){
    //     this->fd = toCopy.fd;
    //     this->enteredPass = toCopy.enteredPass;
    //     this->enteredNick = toCopy.enteredNick;
    //     this->enteredUserName = toCopy.enteredUserName;
    //     this->password = toCopy.password;
    //     this->userName = toCopy.userName;
    //     this->nickname = toCopy.nickname;
    // }
    Client(int fd) : fd(fd)
    {
        this->enteredPass = false;
        this->enteredNick = false;
        this->enteredUserName = false;
        this->password = "";
        this->userName = "";
        this->nickname = "";
        this->ipadress = "";
    }
    // Client &operator=(const Client &toCopy)
    // {
    //     this->fd = toCopy.fd;
    //     this->enteredPass = toCopy.enteredPass;
    //     this->enteredNick = toCopy.enteredNick;
    //     this->enteredUserName = toCopy.enteredUserName;
    //     this->password = toCopy.password;
    //     this->userName = toCopy.userName;
    //     this->nickname = toCopy.nickname;
    //     return *this;
    // }

    void setip_address(std::string ip)
    {
        this->ipadress = ip;
    }
    std::string getip_address()
    {
        return (this->ipadress);
    }
    void setEnteredNick()
    {
        this->enteredNick = true;
    }
    bool getEnteredPass()
    {
        return this->enteredPass;
    }
    bool getEntredNick()
    {
        return this->enteredNick;
    }
    bool getEntredUserName()
    {
        return enteredUserName;
    }
    void setEnteredUserName()
    {
        this->enteredUserName = true;
    }
    void setEnteredPass()
    {
        this->enteredPass = true;
    }
    void setNickName(std::string nickname)
    {
        this->nickname = nickname;
        setEnteredNick();
    }
    void setUserName(std::string userName)
    {
        this->userName = userName;
        setEnteredUserName();
    }
    void setPassword(std::string &clientPass)
    {
        this->password = clientPass;
        setEnteredPass();
    }
    bool isAuthenticated()
    {
        return (this->enteredPass && this->enteredNick && this->enteredUserName);
    }
    std::string getnickname() const
    {
        return this->nickname;
    }
    std::string getUserName() const
    {
        return this->userName;
    }
    int getFd()
    {
        return this->fd;
    }
    bool operator==(const Client &right) const
    {
        return (this->nickname == right.nickname || this->fd == right.fd) ? true : false;
    }
};

#endif