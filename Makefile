CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

#Directory
OBJS_DIR = ./objs
SRCS_DIR = ./srcs
INCLUDES_DIR = ./includes

#cpp files
SRCS_FILE = main.cpp Request.cpp Server.cpp Webserver.cpp

#.o files
OBJS_SRCS = $(patsubst $(SRCS_DIR)/%.cpp, $(OBJS_DIR)/%.o, $(SRCS_FILE))
OBJS = OBJS_SRCS

NAME = webserver

all: $(NAME)

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp
	@mkdir -p $(OBJS_DIR) 
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

clean:
	rm -rf $(OBJS_DIR)

fclean: clean
	rm -f $(NAME)

re : fclean all

.PHONY: all fclean clean re