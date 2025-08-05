CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -g
LDFLAGS = -lpthread


SRC_DIR = src
TEST_DIR = test
OBJ_DIR = build
TARGET = main


SRC_SOURCES := $(wildcard $(SRC_DIR)/*.c)
TEST_SOURCES := $(wildcard $(TEST_DIR)/*.c)


SRC_OBJECTS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_SOURCES))
TEST_OBJECTS := $(patsubst $(TEST_DIR)/%.c, $(OBJ_DIR)/%.o, $(TEST_SOURCES))


OBJECTS := $(SRC_OBJECTS) $(TEST_OBJECTS)


all: $(OBJ_DIR) $(TARGET)


$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@


$(OBJ_DIR)/%.o: $(TEST_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@


$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)


clean:
	rm -rf $(OBJ_DIR) $(TARGET)
