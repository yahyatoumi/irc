#include "Channel.hpp"

Channel::Channel(){
    throw std::logic_error("cant use Channel:defult constructor, use te one with parametrs");
}

Channel::~Channel(){
}

Channel::Channel(std::string &name){
    if (!name.length())
        throw std::logic_error("error: Channel name");

    if (name[0] != '#' && name[0] != '&')
        throw std::logic_error("error: Channel name");

    
    int i = 0;
    while (name[i]){
        if (name[i] == 7 || name[i] == ' ' || name[i] == ',')
            throw std::logic_error("error: Channel name");
        i++;
    }
}