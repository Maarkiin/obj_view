# Compiler
CC := gcc

# Compiler flags for release mode
CFLAGS_RELEASE := -Wall -Wextra -Wl,-subsystem,windows -O3

# Compiler flags for debug mode
CFLAGS_DEBUG := -Wall -Wextra -g

# Library paths
LIBRARY_PATHS :=

# Linker flags
LDFLAGS :=

# Include directories
INCLUDES := -I./include

# Source files directory
SRC_DIR := src

# Source files
SRCS := $(wildcard $(SRC_DIR)/*.c)

# Object files directory
OBJ_DIR := obj

# Object files
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Binary output
TARGET := main

# Default target
all: release

# Release mode
release: CFLAGS := $(CFLAGS_RELEASE)
release: $(TARGET)

# Debug mode
debug: CFLAGS := $(CFLAGS_DEBUG)
debug: $(TARGET)

# Compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Link object files into the final binary
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LIBRARY_PATHS) $^ -o $@ $(LDFLAGS)

# Clean build artifacts
clean:
	del /Q $(OBJ_DIR)\*.o $(TARGET) 2>NUL

.PHONY: all release debug clean
