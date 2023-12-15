#include "Client.hpp"

Client::Client()
{
    throw std::logic_error("cant use Client:defult constructor, use te one with parametrs");
}
Client::~Client()
{
}
Client::Client(int fd) : fd(fd)
{
    this->enteredPass = false;
    this->enteredNick = false;
    this->enteredUserName = false;
    this->password = "";
    this->userName = "";
    this->nickname = "";
    this->ipadress = "";
}

void Client::setip_address(std::string ip)
{
    this->ipadress = ip;
}
std::string Client::getip_address()
{
    return (this->ipadress);
}
void Client::setEnteredNick()
{
    this->enteredNick = true;
}
bool Client::getEnteredPass()
{
    return this->enteredPass;
}
bool Client::getEntredNick()
{
    return this->enteredNick;
}
bool Client::getEntredUserName()
{
    return enteredUserName;
}
void Client::setEnteredUserName()
{
    this->enteredUserName = true;
}
void Client::setEnteredPass()
{
    this->enteredPass = true;
}
void Client::setNickName(std::string nickname)
{
    this->nickname = nickname;
    setEnteredNick();
}
void Client::setUserName(std::string userName)
{
    this->userName = userName;
    setEnteredUserName();
}
void Client::setPassword(std::string &clientPass)
{
    this->password = clientPass;
    setEnteredPass();
}
bool Client::isAuthenticated()
{
    return (this->enteredPass && this->enteredNick && this->enteredUserName);
}
std::string Client::getnickname() const
{
    return this->nickname;
}
std::string Client::getUserName() const
{
    return this->userName;
}
int Client::getFd()
{
    return this->fd;
}
bool Client::operator==(const Client &right) const
{
    return (this->nickname == right.nickname || this->fd == right.fd) ? true : false;
}