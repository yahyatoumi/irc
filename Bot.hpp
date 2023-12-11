#include "Client.hpp"
#include <unistd.h>
#include <poll.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "message.hpp"

class Bot : public Client
{
public:
    Bot(int port);
};