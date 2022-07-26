NAME = webserv

RM = rm -rf
MKDIR = mkdir -p

SRC_DIR = src
OUT_DIR = out
DEBUG_DIR = debug

CXX = clang++
WARNING = -Wall -Wextra -Werror
CXXFLAGS = -O0

SRCS = main.cpp \
	Server.cpp \
	PollElement.cpp \
	Config.cpp \
	Location.cpp \
	ServerConfig.cpp \
	Utils.cpp \
	HttpRequest.cpp \
	HttpResponse.cpp \
	HttpResponseStatus.cpp \
	Uri.cpp
SRCS_PATH = $(addprefix $(SRC_DIR)/,$(SRCS))
SRCS_PATH_BONUS = $(addprefix $(SRC_DIR)/,$(SRCS_BONUS))
OBJS = $(addprefix $(OUT_DIR)/,$(SRCS_PATH:%.cpp=%.o))
OBJS_DEBUG = $(addprefix $(DEBUG_DIR)/,$(SRCS_PATH:%.cpp=%.o))

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) -o $(NAME)

$(OUT_DIR)/%.o: %.cpp
	$(MKDIR) $(@D)
	$(COMPILE.cc) $< $(WARNING) -MMD -MP -o $@

$(DEBUG_DIR)/%.o: %.cpp
	$(MKDIR) $(@D)
	$(COMPILE.cc) $< $(WARNING) -MMD -MP -o $@

.PHONY: debug debug_clean debug_fclean debug_re
debug: CXXFLAGS = -gdwarf-4 -fsanitize=address -fno-omit-frame-pointer \
	-fno-optimize-sibling-calls
debug: WARNING = -Wall -Wextra
debug: $(OBJS_DEBUG)
	$(CXX) $(CXXFLAGS) $(OBJS_DEBUG) $(LDFLAGS) -o $(NAME)_debug
debug_clean:
	$(RM) $(DEBUG_DIR)
debug_fclean: debug_clean
	$(RM) $(NAME)_debug
debug_re: debug_fclean debug

.PHONY: clean fclean re
clean: 
	$(RM) $(OUT_DIR)
fclean: clean
	$(RM) $(NAME)
re: fclean all

.PHONY: cc check_cc check_infer
cc:
	bear -- make re
check_cc: cc
	codechecker analyze compile_commands.json -o .cc
	codechecker parse .cc
check_infer: re
	infer run -- make re
