# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -Iinclude -g -std=c++20

# Source files and output
SRC = $(wildcard src/*.cpp)
OBJ = $(SRC:.cpp=.o)
TARGET = ss

# Build target
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Rule to create object files
src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Clean rule to remove generated files
clean:
	rm -f $(OBJ) $(TARGET)