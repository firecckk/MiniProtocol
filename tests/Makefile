# Root directory
ROOT = ..

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -I$(ROOT)/mprotocol

# Header files
HEADERS = $(ROOT)/mprotocol/mprotocol.h

# Output directory
BIN_DIR = $(ROOT)/tests/bin

# Output executable
TARGET ?= main
#TARGET = serial_test

# Include files
INC = $(ROOT)/mprotocol/user_fields.c $(ROOT)/mprotocol/host.c

# Source files
SRC = $(TARGET).c $(INC)

# Build target
all: $(BIN_DIR)/$(TARGET)
	@echo "Build All complete"

tests:
	@echo "Building test suits"
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/packet_generator packet_generator.c $(INC)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/packet_parser packet_parser.c $(INC)

$(BIN_DIR)/$(TARGET): $(SRC) $(HEADERS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$(TARGET) $(SRC)

# Run target
run: $(BIN_DIR)/$(TARGET)
	@echo "Running program $(BIN_DIR)/$(TARGET)"
	$(BIN_DIR)/$(TARGET)

# Clean target
clean:
	rm -f $(BIN_DIR)/$(TARGET)

# Phony targets
.PHONY: all clean