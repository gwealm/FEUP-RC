# Makefile to build the project

# Parameters
CC = gcc
CFLAGS = -Wall #-Werror

SRC = src/
INCLUDE = include/
BIN = bin/

# project name (generate executable with this name)
TARGET   = download


$(BIN)/$(TARGET): main.c $(SRC)/*.c
	$(CC) $(CFLAGS) -o $@ $^ -I $(INCLUDE)


.PHONY: clean
clean:
	rm $(BIN)/$(TARGET)