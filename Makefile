# Makefile
CXX = g++
CXXFLAGS = -Wall -Wextra -Werror

SRC = main.cpp Debugger.cpp Breakpoint.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = nullDBG

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(TARGET)

re: fclean all

.PHONY: all clean fclean re
