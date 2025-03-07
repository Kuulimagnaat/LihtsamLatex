CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iheaders

# kaustad
SRC_DIR = src
OBJ_DIR = obj
ASM_DIR = src/assembler

# Source ja obj failid
SRC_FILES = $(filter-out $(SRC_DIR)/Katse.c, $(wildcard $(SRC_DIR)/*.c))
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

# Assembli failid
ASM_FILE = $(ASM_DIR)/A_strlen.s
ASM_OBJ = $(OBJ_DIR)/A_strlen.o

# Output binary (now in the same directory as the Makefile)
TARGET = a.exe

# Phony targets
.PHONY: all clean run dirs

# Default target: compile the program
all: dirs $(TARGET)

# Kustutab ja teeb uue objektifailide kausta obj.
dirs: 
	if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)

# Link object files and the assembly object into the final executable
$(TARGET): $(OBJ_FILES) $(ASM_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Assemble the assembly file into an object file
$(ASM_OBJ): $(ASM_FILE)
	$(CC) -c -o $@ $<

# Run the program
run: $(TARGET)
	./$(TARGET)

# Clean build artifacts (removes the executable and object files)
clean:
	rmdir /s /q $(OBJ_DIR)
	del a.exe

debug:
	gcc $(filter-out $(SRC_DIR)/main.c, $(wildcard $(SRC_DIR)/*.c))
	a.exe