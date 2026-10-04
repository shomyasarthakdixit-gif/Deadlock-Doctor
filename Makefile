CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g

TARGET = deadlockdoctor

SRC = src/main.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
