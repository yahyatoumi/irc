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
src = neww.o Channel.o Server.o
srcb =  Bot.o 
CC = c++
CXXFLAGS =  -Wall -Wextra -Werror -std=c++98 -fsanitize=address
RM = rm -f

all: $(NAME)
		@$(CC) $(CXXFLAGS) $(src)  -o $(NAME)
$(NAME): $(src)  
	

$(src): Server.hpp
$(srb): Server.hpp Bot.hpp
bonus: all $(NAMB)
		@$(CC) $(CXXFLAGS) $(srcb)  -o $(NAMB)
$(NAMB) : $(srcb)
		
clean:
	@$(RM) $(src) $(srcb)

fclean: clean
	@$(RM) $(NAME) $(NAMB)

re: fclean all