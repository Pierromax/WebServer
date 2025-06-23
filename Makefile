CXX = c++
CXXFLAGS = -Wall -Wextra -std=c++98 -gdwarf-2

#color
BLUE = \033[34m
WHITE = \033[37m

#Directory
OBJS_DIR = .objs
SRCS_DIR = ./srcs
INCLUDES_DIR = ./includes
INC = -I $(INCLUDES_DIR)

#cpp files
SRCS = $(SRCS_DIR)/Webserv.cpp \
       $(SRCS_DIR)/utils/location.cpp \
       $(SRCS_DIR)/Server.cpp \
       $(SRCS_DIR)/Request.cpp \
       $(SRCS_DIR)/Client.cpp \
       $(SRCS_DIR)/CGIsHandling.cpp \
       $(SRCS_DIR)/ParsingError.cpp \
       $(SRCS_DIR)/config/Tokenizer.cpp \
       $(SRCS_DIR)/config/Parser.cpp \
       $(SRCS_DIR)/config/ValidateConfig.cpp \
	   $(SRCS_DIR)/config/validateDirectives.cpp \
       $(SRCS_DIR)/response/Response.cpp \
	   $(SRCS_DIR)/response/login.cpp \
       $(SRCS_DIR)/response/directoryListing.cpp \
       $(SRCS_DIR)/response/errorPage.cpp \
       $(SRCS_DIR)/response/handleCGI.cpp \
       $(SRCS_DIR)/response/handleDelete.cpp \
       $(SRCS_DIR)/response/handleGet.cpp \
       $(SRCS_DIR)/response/handlePost.cpp \
       $(SRCS_DIR)/response/pathHandling.cpp \
       $(SRCS_DIR)/response/redirect.cpp \
       $(SRCS_DIR)/main.cpp


#.o files
OBJS = $(SRCS:$(SRCS_DIR)/%.cpp=$(OBJS_DIR)/%.o)

NAME = webserv

# Marqueurs pour suivre le mode de compilation
TEST_MARKER = .test_mode
DEBUG_MARKER = .debug_mode
NORMAL_MARKER = .normal_mode

all: $(NORMAL_MARKER) $(NAME)

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

$(NAME): $(OBJS)
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@make --silent banner

# Cible pour définir le mode normal (CONFIG_TESTER=0, DEBUG=0)
$(NORMAL_MARKER):
	@if [ -f $(TEST_MARKER) ] || [ -f $(DEBUG_MARKER) ]; then \
		rm -rf $(OBJS_DIR); \
		rm -f $(TEST_MARKER) $(DEBUG_MARKER); \
	fi
	@touch $(NORMAL_MARKER)
	@$(eval CXXFLAGS += -DCONFIG_TESTER=0 -DDEBUG=0)

# Cible pour le testeur de configuration
tester: $(TEST_MARKER) $(NAME)
	@echo "$(BLUE)Webserv built in tester mode (CONFIG_TESTER=1, DEBUG=0)$(WHITE)"

# Marqueur pour le mode test
$(TEST_MARKER):
	@if [ -f $(NORMAL_MARKER) ] || [ -f $(DEBUG_MARKER) ]; then \
		rm -rf $(OBJS_DIR); \
		rm -f $(NORMAL_MARKER) $(DEBUG_MARKER); \
	fi
	@touch $(TEST_MARKER)
	@$(eval CXXFLAGS += -DCONFIG_TESTER=1 -DDEBUG=0)

# Cible pour le mode debug
debug: $(DEBUG_MARKER) $(NAME)
	@echo "$(BLUE)Webserv built in debug mode (CONFIG_TESTER=0, DEBUG=1)$(WHITE)"

# Marqueur pour le mode debug
$(DEBUG_MARKER):
	@if [ -f $(NORMAL_MARKER) ] || [ -f $(TEST_MARKER) ]; then \
		rm -rf $(OBJS_DIR); \
		rm -f $(NORMAL_MARKER) $(TEST_MARKER); \
	fi
	@touch $(DEBUG_MARKER)
	@$(eval CXXFLAGS += -DCONFIG_TESTER=0 -DDEBUG=1)

test: tester
	@./test.sh

clean:
	rm -rf $(OBJS_DIR) logs
	rm -f $(TEST_MARKER) $(DEBUG_MARKER) $(NORMAL_MARKER)

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

.PHONY: all fclean clean re banner tester test debug