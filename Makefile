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

# Default target: build the executable
all: force_rebuild $(TARGET)

# Always recompile everything
force_rebuild:
	@echo "Forcing recompilation of all files."

# Link all object files into the executable
$(TARGET): $(OBJECTS)
	@echo "Linking target: $@"
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile every source file into an object file
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling: $<"
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

# Clean up build artifacts
clean:
	@echo "Cleaning up..."
	rm -rf $(OBJDIR) $(BINDIR)

# Phony targets to ensure rules always run
.PHONY: all clean force_rebuild
