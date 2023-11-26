#include "Channel.hpp"
#include "Client.hpp"
#include "Server.hpp"
class Client;
class Channel;

int main(int ac, char **av){
    if (ac != 3)
		return 0;
    try{
    Server server(av[1], av[2]);
    }catch(std::exception &e){
      std::cout << e.what() << std::endl;
    }
}