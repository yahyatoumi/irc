#include "Bot.hpp"
#include "message.hpp"
#include <sstream>
Bot::Bot(int port) : Client(0)
{
    std::string rpl_msg;
    char buffer[1000];
    struct sockaddr_in serv_addr;
    this->nickname = "bot";
    this->userName = "Bot";
    this->ipadress = "127.0.0.1";
    this->fd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    std::cout << "<<<<" << port << this->fd << std::endl;
    if (!connect(this->fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)))
        std::cout << "conected" << std::endl;
    const char *message = "bot\r\n";
    if (send(this->fd, message, strlen(message), 0) == -1)
        std::cerr << "Error sending message" << std::endl;
    std::time_t currentTime = std::time(nullptr);
    std::tm *localTime = std::localtime(&currentTime);
    int year = localTime->tm_year + 1900;
    int month = localTime->tm_mon + 1;
    int day = localTime->tm_mday;
    int hour = localTime->tm_hour;
    int minute = localTime->tm_min;
    std::ostringstream oss;
    while (1)
    {
        std::memset(buffer, 0, sizeof(buffer));
        ssize_t size = recv(this->fd, buffer, sizeof(buffer), 0);
        if (size == 0)
        {
            std::cout << "server disconnected" << std::endl;
            std::exit(0);
        }
        std::cout << buffer << std::endl;
        std::string comand = buffer;
        size_t pos = comand.find('!');
        std::string name = comand.substr(1, pos - 1);
        pos = comand.find("bot");
        std::string msg = comand.substr(pos + 5);
        std::cout << "msg : " << msg << "from : " << name << std::endl;
        if (!strncmp(msg.c_str(), "hi bot", 6) || !strncmp(msg.c_str(), "hi", 2))
        {

            rpl_msg = "PRIVMSG " + name + " :hi " + name + " how can i help you 🙂 \r\n";
            send(this->fd, rpl_msg.c_str(), rpl_msg.size(), 0);
        }
        else if (!strncmp(msg.c_str(), "time", 4) || !strncmp(msg.c_str(), "date", 4))
        {
            oss << year << "-";
            if (month < 10)
                oss << "0";
            oss << month << "-";
            if (day < 10)
                oss << "0";
            oss << day << " ";
            if (hour < 10)
                oss << "0";
            oss << hour << ":";
            if (minute < 10)
                oss << "0";
            oss << minute;
            std::string dateTimeString = oss.str();
            rpl_msg = "PRIVMSG " + name + " :hi " + name + " the localtime is " + dateTimeString + "📅 \r\n";
            send(this->fd, rpl_msg.c_str(), rpl_msg.size(), 0);
        }
        else if (!strncmp(msg.c_str(), "joke", 4))
        {
            rpl_msg = "PRIVMSG " + name + " :Did you hear about the mathematician who’s afraid of negative numbers? 🃏 \r\n";
            send(this->fd, rpl_msg.c_str(), rpl_msg.size(), 0);
        }
    }
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cout << "[./bot] [port]" << std::endl;
        return (10);
    }
    Bot bot(std::atoi(argv[1]));
}