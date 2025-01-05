CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iheaders

SRC_DIR = src
OBJ_DIR = obj

SRC_FILES = $(filter-out $(SRC_DIR)/Katse.c, $(wildcard $(SRC_DIR)/*.c))
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

TARGET = a.exe

.PHONY: all clean run dirs

all: dirs $(TARGET)

dirs:
	mkdir -p $(OBJ_DIR)

$(TARGET): $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
	rm -rf $(OBJ_DIR)