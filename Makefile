CXX ?= g++
CXXFLAGS = -g -Wall -std=c++11 -I.
LDFLAGS = 

ifeq ($(OS),Windows_NT)
    CXXFLAGS += -DWINDOWS
    LDFLAGS += -lpdcurses -lws2_32
    TARGET := battleship.exe
else
    CXXFLAGS += -DUNIX
    LDFLAGS += -lncurses
    TARGET := battleship
    
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
    endif
endif

SOURCES = ai/ai_player.cpp \
          game/board_size_menu.cpp \
          game/game_board.cpp \
          game/game_piece.cpp \
          network/client.cpp \
          network/host.cpp \
          ui/animation.cpp \
          ui/config.cpp \
          ui/ui_helper.cpp \
          user/user.cpp \
          utils/game_logic_utils.cpp \
          utils/system_utils.cpp \
          utils/text_utils.cpp \
          main.cpp 

OBJS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo Linking $@...
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS)
	@echo Compilation successful!

%.o: %.cpp
	@echo Compiling $<...
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo Cleaning up object files and target...
	@rm -f $(OBJS) $(TARGET)
	@rm -f user/*.o game/*.o ai/*.o ui/*.o network/*.o utils/*.o
	@echo Clean complete

rebuild: clean all

info:
	@echo "--- Build Information ---"
	@echo "OS Type: $(if $(filter Windows_NT, $(OS)),Windows,UNIX-like ($(UNAME_S)))"
	@echo "Compiler: $(CXX)"
	@echo "Target: $(TARGET)"
	@echo "CXXFLAGS: $(CXXFLAGS)"
	@echo "LDFLAGS: $(LDFLAGS)"
	@echo "Source files: $(SOURCES)"
	@echo "Object files: $(OBJS)"
	@echo "-------------------------"

debug: CXXFLAGS += -DDEBUG -O0
debug: rebuild

release: CXXFLAGS += -O2 -DNDEBUG
release: rebuild

.PHONY: all clean rebuild info debug release