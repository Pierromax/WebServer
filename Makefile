CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I$(INCLUDES_DIR)

#color
BLUE = \033[34m
WHITE = \033[37m

#Directory
OBJS_DIR = ./objs
SRCS_DIR = ./srcs
INCLUDES_DIR = ./includes

#cpp files
SRCS = $(SRCS_DIR)/Webserv.cpp \
       $(SRCS_DIR)/Server.cpp \
       $(SRCS_DIR)/Request.cpp \
       $(SRCS_DIR)/Client.cpp \
       $(SRCS_DIR)/config/Tokenizer.cpp \
       $(SRCS_DIR)/config/Parser.cpp \
       $(SRCS_DIR)/config/ConfigValidator.cpp \
       $(SRCS_DIR)/main.cpp

#.o files
OBJS = $(SRCS:$(SRCS_DIR)/%.cpp=$(OBJS_DIR)/%.o)

NAME = webserver

all: $(NAME)

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(NAME): $(OBJS)
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@make banner

clean:
	rm -rf $(OBJS_DIR) logs

fclean: clean
	rm -f $(NAME)

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