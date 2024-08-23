# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++20 -Iinclude -Wall -Wextra -O2

# Source files
SRCS = $(wildcard src/*.cpp) $(wildcard src/util/*.cpp) $(wildcard src/log/*.cpp)

# Object files
OBJS = $(SRCS:src/%.cpp=build/%.o)

# Executable name
TARGET = build/main

# Build directory
BUILD_DIR = build

# Default target
all: $(TARGET)

# Build the target
$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) -lpthread -lboost_system -lboost_filesystem -lboost_thread -lssl -lcrypto

# Compile source files into object files
build/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Run the server (example: make run ARGS="0.0.0.0 8080 ./ 4")
run: $(TARGET)
	./$(TARGET) $(ARGS)

.PHONY: all clean run

