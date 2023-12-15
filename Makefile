# .-----------------------------------------------.
# |       ___                 _                   |
# |      / __) _             (_)                  |
# |     | |__ | |_            _   ____  ____      |
# |     |  __)|  _)          | | / ___)/ ___)     |
# |     | |   | |__  _____ __ | || |   ( (___      |
# |     |_|    \___)(_______)|_||_|    \____)     |
# '-----------------------------------------------'

NAME = ircserv
NAMB = irc_bot
src = ft_irc.o Channel.o Server.o Client.o
srcb =   Server.o  Bot.o Client.o Channel.o
CC = c++
CXXFLAGS =  -Wall -Wextra -Werror -std=c++98 
RM = rm -f

all: $(NAME)
		@$(CC) $(CXXFLAGS) $(src)  -o $(NAME)
$(NAME): $(src)  
	

$(src): Server.hpp Channel.hpp Client.hpp
$(srb): Server.hpp Bot.hpp Client.hpp Channel.hpp
bonus: $(NAMB)
		@$(CC) $(CXXFLAGS) $(srcb)  -o $(NAMB)
$(NAMB) : $(srcb)

clean:
	@$(RM) $(src) $(srcb)

fclean: clean
	@$(RM) $(NAME) $(NAMB)

re: fclean all