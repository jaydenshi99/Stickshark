# Compiler and Flags
CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -O2

# Directories
SRCDIR := src
INCDIR := include
OBJDIR := obj
BINDIR := bin

# Target executable
TARGET := $(BINDIR)/main

# Find all source files recursively
SOURCES := $(shell find $(SRCDIR) -name "*.cpp")
OBJECTS := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SOURCES))

# Create necessary directories
$(shell mkdir -p $(OBJDIR) $(BINDIR))

# Default target
all: $(TARGET)

# Link all object files into the final executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile every source file into an object file
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

# Clean up build artifacts
clean:
	rm -rf $(OBJDIR) $(BINDIR)

# Always recompile every file
.PHONY: all clean
