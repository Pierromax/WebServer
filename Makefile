CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

#color
BLUE = \033[34m
WHITE = \033[37m

#Directory
OBJS_DIR = .objs
SRCS_DIR = ./srcs
INCLUDES_DIR = ./includes
INC = -I $(INCLUDES_DIR)

#cpp files
SRCS_FILE = Request.cpp Response.cpp Webserv.cpp main.cpp Serveur.cpp Client.cpp
SRCS = $(addprefix $(SRCS_DIR)/,$(SRCS_FILE))

#.o files
OBJS_SRCS = $(patsubst $(SRCS_DIR)/%.cpp, $(OBJS_DIR)/%.o, $(SRCS))
OBJS = $(OBJS_SRCS)

NAME = webserv

all: $(NAME)

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp
	@mkdir -p $(OBJS_DIR) 
	@$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

$(NAME): $(OBJS)
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@make --silent banner

clean:
	@rm -rf $(OBJS_DIR)

fclean: clean
	@rm -f $(NAME)

re : fclean all

banner:
	@echo "$(WHITE)"
	@echo "              ███████████████████████████████████████"
	@echo "              ███████████████████████████████████████"
	@echo "              ██  $(BLUE)        _  _   __  _  _      $(WHITE)   ███"
	@echo "              ██  $(BLUE)\    / |_ |_) (_  |_ |_) \  /$(WHITE)   ███"
	@echo "              ██  $(BLUE) \/\/  |_ |_) __) |_ | \  \/ $(WHITE)   ███"
	@echo "              ██                                  ███"
	@echo "              ███████████████████████████████████████"
	@echo "              ███████████████████████████████████████"
	@echo "              ██                                  ███"
	@echo "              ██                                  ███"
	@echo "              ██  ████████████████████████████"
	@echo "              ██  █████████████████████████$(BLUE)   ██  █████  █$(WHITE)"
	@echo "              ██  ██████████████████████$(BLUE)  ████  █████████ █████$(WHITE)"
	@echo "              ██  ███████████████████$(BLUE)  ██████  ███████████ ███████$(WHITE)"
	@echo "              ██                     $(BLUE)████████ ████████████  ████████$(WHITE)"
	@echo "              ██                   $(BLUE)█████████  █████████████  ████████$(WHITE)"
	@echo "              ██  ███████████████"
	@echo "              ██  ██████████████ $(BLUE)██████████  ███████████████  ██████████$(WHITE)"
	@echo "              ██  █████████████ $(BLUE)███████████  ███████████████  ███████████$(WHITE)"
	@echo "              ██  █████████████ $(BLUE)███████████ ████████████████  ███████████$(WHITE)"
	@echo "              ██               $(BLUE)████████████ █████████████████ ████████████$(WHITE)"
	@echo "              ██               $(BLUE)███████████  █████████████████ ████████████$(WHITE)"
	@echo "              ██  ████████████"
	@echo "              ██  ████████████ $(BLUE)███████████  █████████████████ ████████████$(WHITE)"
	@echo "              ██  ████████████ $(BLUE)███████████  █████████████████ ████████████$(WHITE)"
	@echo "              ██  ████████████ $(BLUE)████████████ █████████████████ ███████████$(WHITE)"
	@echo "              ██                $(BLUE)███████████ ████████████████  ███████████$(WHITE)"
	@echo "              ██                 $(BLUE)██████████  ███████████████  ██████████$(WHITE)"
	@echo "              ██  ██████████████ $(BLUE)██████████  ███████████████  █████████$(WHITE)"
	@echo "              ██  ███████████████  $(BLUE)█████████  █████████████  █████████$(WHITE)"
	@echo "              ██  █████████████████ $(BLUE)████████  █████████████  ████████$(WHITE)"
	@echo "              ██  ██████████████████  $(BLUE)███████  ███████████  ███████$(WHITE)"
	@echo "              ██                        $(BLUE)██████  █████████  ██████$(WHITE)"
	@echo "              ███████████████████████████  $(BLUE)████  ███████  ████$(WHITE)"
	@echo "              ███████████████████████████████      $(BLUE)███$(WHITE)"
	@echo "              ███████████████████████████████████████"
	@echo ""

.PHONY: all fclean clean re banner