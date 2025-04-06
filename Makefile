EXP_BITS ?= 8
MANT_BITS ?= 23

# Compiler and basic flags
CXX      := g++
CXXFLAGS := -g -std=c++17 -Wall -Wextra -pedantic -frounding-math \
            -DEXP_BITS=$(EXP_BITS) \
            -DMANT_BITS=$(MANT_BITS)
INCLUDES := -I src/

# Build directories
BUILD_DIR := build
BIN_DIR   := bin

# Source files
TEST_DIR   := test
SRC_DIR    := src
HUB_SRC    := $(SRC_DIR)/hub_float.cpp

# Find all test directories with main.cpp
TEST_MAINS := $(shell find $(TEST_DIR) -name main.cpp)
TEST_NAMES := $(patsubst $(TEST_DIR)/%/main.cpp,%,$(TEST_MAINS))
BIN_TARGETS := $(addprefix $(BIN_DIR)/, $(TEST_NAMES))

# Hub float object
HUB_OBJ    := $(BUILD_DIR)/$(SRC_DIR)/hub_float.o

# Create necessary directories
$(shell mkdir -p $(BUILD_DIR)/$(SRC_DIR) $(BIN_DIR))

# Main targets
.PHONY: all tests clean

all: tests

tests: $(BIN_TARGETS)

# Template for test targets
define TEST_TEMPLATE
TEST_SOURCES_$(1) := $$(wildcard $$(TEST_DIR)/$(1)/*.cpp)
TEST_OBJECTS_$(1) := $$(patsubst $$(TEST_DIR)/%.cpp, $$(BUILD_DIR)/$$(TEST_DIR)/%.o, $$(TEST_SOURCES_$(1)))

$$(BIN_DIR)/$(1): $$(TEST_OBJECTS_$(1)) $$(HUB_OBJ)
	@echo "Linking $$@..."
	@mkdir -p $$(@D)
	@$$(CXX) $$(CXXFLAGS) $$^ -o $$@

$$(TEST_OBJECTS_$(1)): $$(BUILD_DIR)/$$(TEST_DIR)/$(1)/%.o: $$(TEST_DIR)/$(1)/%.cpp
	@echo "Compiling $$<..."
	@mkdir -p $$(@D)
	@$$(CXX) $$(CXXFLAGS) $$(INCLUDES) -MMD -MP -c $$< -o $$@
endef

# Generate rules for each test
$(foreach test,$(TEST_NAMES),$(eval $(call TEST_TEMPLATE,$(test))))

# Compile hub_float.cpp
$(HUB_OBJ): $(HUB_SRC)
	@echo "Compiling hub_float..."
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

# Include dependencies
-include $(wildcard $(BUILD_DIR)/$(SRC_DIR)/*.d)
-include $(wildcard $(BUILD_DIR)/$(TEST_DIR)/*/*.d)

# Clean build artifacts
clean:
	@echo "Cleaning build files..."
	@rm -rf $(BUILD_DIR)
	@rm -rf $(BIN_DIR)
	@echo "Clean complete"
