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

# Find all .cpp files recursively in SRCDIR
SOURCES := $(shell find $(SRCDIR) -name "*.cpp")
OBJECTS := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SOURCES))

# Create necessary directories
$(shell mkdir -p $(OBJDIR) $(BINDIR))

# Default target
all: $(TARGET)

# Link all object files into the final executable
$(TARGET): $(OBJECTS)
	@echo "Linking target: $@"
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files into object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@) # Ensure obj directory structure matches src
	@echo "Compiling: $<"
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

# Clean up build artifacts
clean:
	@echo "Cleaning up..."
	rm -rf $(OBJDIR) $(BINDIR)

# Phony targets
.PHONY: all clean
