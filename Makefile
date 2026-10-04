CC = gcc

CFLAGS = -Wall -Wextra -std=c11 -g
CPPFLAGS = -Iinclude

TARGET = deadlockdoctor

SRC = src/main.c \
      src/resource_manager.c \
      src/deadlock_detector.c

OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
