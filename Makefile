EXP_BITS ?= 8
MANT_BITS ?= 23

# Compiler and basic flags
CXX      := g++
CXXFLAGS := -O2 -std=c++17 -Wall -Wextra -pedantic -frounding-math \
            -DEXP_BITS=$(EXP_BITS) \
            -DMANT_BITS=$(MANT_BITS)
INCLUDES := -I src/

# Build directories
BUILD_DIR := build
BIN_DIR   := bin

# Project configuration
TARGET   := $(BIN_DIR)/hub_float_demo
SRCS     := test/main.cpp \
            src/hub_float.cpp
OBJS     := $(SRCS:%.cpp=$(BUILD_DIR)/%.o)
DEPS     := $(OBJS:.o=.d)

# Create necessary directories
$(shell mkdir -p $(BUILD_DIR)/src $(BUILD_DIR)/test $(BIN_DIR))

# Main targets
.PHONY: all clean

all: $(TARGET)

# Link the target
$(TARGET): $(OBJS)
	@echo "Linking $@..."
	@$(CXX) $(OBJS) -o $@ $(CXXFLAGS)

# Compile source files
$(BUILD_DIR)/%.o: %.cpp
	@echo "Compiling $<..."
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

# Include dependency files
-include $(DEPS)

# Clean build artifacts
clean:
	@echo "Cleaning build files..."
	@rm -rf $(BUILD_DIR)
	@rm -rf $(BIN_DIR)
	@echo "Clean complete"
