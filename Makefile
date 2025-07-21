COMPILER = gcc
FILE = ./src/main.c
OBJECT_FILE = main
STANDARD = c99
FLAGS = -Wall -Werror


all:
	$(COMPILER) $(FILE) -o $(OBJECT_FILE) -std=$(STANDARD) $(FLAGS)


clear:
	rm $(OBJECT_FILE)


