NAME = MattDaemon

CXX = c++

CXXFLAGS = -Wall -Wextra -Werror -std=c++11

OBJ = ./cpp/Client.o ./cpp/Daemon.o ./cpp/DaemonServer.o ./cpp/Tintin_reporter.o ./main.o

all : $(NAME)

$(NAME) : $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

clean :
	rm -f $(OBJ)

fclean : clean
	rm -f $(NAME)

re : fclean all