CC = gcc

CFLAGS = -Wall -Wextra -std=c11 -g -pthread
CPPFLAGS = -Iinclude -D_POSIX_C_SOURCE=200809L

TARGET = deadlockdoctor

SRC = src/main.c \
      src/process_manager.c \
      src/scheduler.c \
      src/resource_manager.c \
      src/deadlock_detector.c \
      src/synchronization.c \
      src/recovery.c

OBJ = $(SRC:.c=.o)

SYNC_TEST = tests/test_synchronization
RECOVERY_TEST = tests/test_recovery

SYNC_TEST_OBJ = tests/test_synchronization.o
RECOVERY_TEST_OBJ = tests/test_recovery.o

.PHONY: all test test-sync test-recovery clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

test: test-sync test-recovery

test-sync: $(SYNC_TEST)
	./$(SYNC_TEST)

$(SYNC_TEST): $(SYNC_TEST_OBJ) src/synchronization.o
	$(CC) $(CFLAGS) $^ -o $@

test-recovery: $(RECOVERY_TEST)
	./$(RECOVERY_TEST)

$(RECOVERY_TEST): $(RECOVERY_TEST_OBJ) \
                  src/process_manager.o \
                  src/resource_manager.o \
                  src/deadlock_detector.o \
                  src/recovery.o
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(OBJ) \
	      $(SYNC_TEST_OBJ) \
	      $(RECOVERY_TEST_OBJ) \
	      $(TARGET) \
	      $(SYNC_TEST) \
	      $(RECOVERY_TEST)

.PHONY: all test test-sync test-recovery clean
