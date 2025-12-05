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

# Source files by directory
DATA_SOURCES = data/game_state.cpp \
               data/board_data.cpp \
               data/ship_data.cpp

LOGIC_SOURCES = logic/game_logic.cpp \
                logic/ai_logic.cpp \
                logic/network_logic.cpp

UI_SOURCES = ui/ui_renderer.cpp \
             ui/ui_config.cpp \
             ui/ui_animation.cpp \
             ui/ui_helpers.cpp

GAME_SOURCES = game/game_loop.cpp \
               game/ai_game_loop.cpp \
               game/multiplayer_game_loop.cpp \
               game/game_controller.cpp

TEST_SOURCES = tests/SeaBattle_1_test.cpp

SOURCES = $(DATA_SOURCES) $(LOGIC_SOURCES) $(UI_SOURCES) $(GAME_SOURCES) $(TEST_SOURCES) main.cpp

OBJS = $(SOURCES:.cpp=.o)

# Default target
all: $(TARGET) check_test_file

# Build executable
$(TARGET): $(OBJS)
	@echo Linking $@...
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS)
	@echo Compilation successful!
	@echo "Run with: ./$(TARGET)"

# Compile object files
%.o: %.cpp
	@echo Compiling $<...
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# Check if test data file exists
check_test_file:
	@mkdir -p tests
	@if [ ! -f tests/SeaBattle_1_test.dat ]; then \
		echo ""; \
		echo "========================================"; \
		echo "WARNING: tests/SeaBattle_1_test.dat not found!"; \
		echo "Please create this file with your test data."; \
		echo "========================================"; \
		echo ""; \
	fi

# Clean build files
clean:
	@echo Cleaning up object files and targets...
	@rm -f $(OBJS) $(TARGET)
	@rm -f data/*.o logic/*.o game/*.o ui/*.o tests/*.o
	@rm -f test_results.txt
	@echo Clean complete

# Clean and rebuild
rebuild: clean all

# Show build information
info:
	@echo "--- Build Information ---"
	@echo "OS Type: $(if $(filter Windows_NT, $(OS)),Windows,UNIX-like ($(UNAME_S)))"
	@echo "Compiler: $(CXX)"
	@echo "Target: $(TARGET)"
	@echo "CXXFLAGS: $(CXXFLAGS)"
	@echo "LDFLAGS: $(LDFLAGS)"
	@echo "Source files: $(SOURCES)"
	@echo "Object files: $(OBJS)"
	@echo ""
	@echo "Test files location: tests/"
	@echo "Test data file: tests/SeaBattle_1_test.dat"
	@echo "Test results output: test_results.txt"
	@echo "-------------------------"

# Debug build
debug: CXXFLAGS += -DDEBUG -O0
debug: rebuild

# Release build
release: CXXFLAGS += -O2 -DNDEBUG
release: rebuild

# Run tests directly
test: $(TARGET)
	@echo "Tests are integrated into the game."
	@echo "Run the game and select '6) Debug Tests' from menu."
	@echo ""
	@echo "Test modes available:"
	@echo "  0) Return to Main Menu"
	@echo "  1) Automatic Tests"
	@echo "  2) Manual Console Input"
	@echo "  3) File-based Tests (tests/SeaBattle_1_test.dat)"
	@echo "  4) All Tests"

# Help
help:
	@echo "SeaBattle Makefile Help"
	@echo "======================="
	@echo ""
	@echo "Main Targets:"
	@echo "  make              - Build main game executable"
	@echo "  make all          - Same as 'make' + check for test file"
	@echo "  make clean        - Remove all build files"
	@echo "  make rebuild      - Clean and rebuild"
	@echo "  make info         - Show build configuration"
	@echo ""
	@echo "Special Targets:"
	@echo "  make debug        - Build with debug symbols (-O0)"
	@echo "  make release      - Build with optimizations (-O2)"
	@echo "  make test         - Show test instructions"
	@echo "  make help         - Show this help message"
	@echo ""
	@echo "Testing:"
	@echo "  Tests are integrated into the game menu."
	@echo "  Run the game and select 'Debug Tests' option."
	@echo "  Test modes:"
	@echo "    0) Return to Main Menu - exit test menu"
	@echo "    1) Automatic Tests - pre-programmed test suite"
	@echo "    2) Manual Console - input test data manually"
	@echo "    3) File-based - read from tests/SeaBattle_1_test.dat"
	@echo "    4) All Tests - run all three modes"
	@echo ""
	@echo "  Test results are saved to test_results.txt"
	@echo "  Create tests/SeaBattle_1_test.dat for file-based tests"
	@echo ""

.PHONY: all clean rebuild info debug release test help check_test_file