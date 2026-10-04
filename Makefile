CC = gcc

CFLAGS = -Wall -Wextra -std=c11 -g -pthread
CPPFLAGS = -Iinclude -D_POSIX_C_SOURCE=200809L

TARGET = deadlockdoctor

SRC = src/main.c \
      src/cli.c \
      src/process_manager.c \
      src/scheduler.c \
      src/resource_manager.c \
      src/deadlock_detector.c \
      src/synchronization.c \
      src/recovery.c

OBJ = $(SRC:.c=.o)

TEST_TARGETS = \
	tests/test_process_manager \
	tests/test_scheduler_fcfs \
	tests/test_scheduler_fcfs_edge \
	tests/test_scheduler_rr \
	tests/test_synchronization \
	tests/test_recovery

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

test: $(TEST_TARGETS)
	@for test in $(TEST_TARGETS); do \
		echo ""; \
		echo "========================================"; \
		echo "Running $$test"; \
		echo "========================================"; \
		./$$test || exit 1; \
	done

tests/test_process_manager: tests/test_process_manager.o \
                            src/process_manager.o
	$(CC) $(CFLAGS) $^ -o $@

tests/test_scheduler_fcfs: tests/test_scheduler_fcfs.o \
                           src/process_manager.o \
                           src/scheduler.o
	$(CC) $(CFLAGS) $^ -o $@

tests/test_scheduler_fcfs_edge: tests/test_scheduler_fcfs_edge.o \
                                src/process_manager.o \
                                src/scheduler.o
	$(CC) $(CFLAGS) $^ -o $@

tests/test_scheduler_rr: tests/test_scheduler_rr.o \
                         src/process_manager.o \
                         src/scheduler.o
	$(CC) $(CFLAGS) $^ -o $@

tests/test_synchronization: tests/test_synchronization.o \
                            src/synchronization.o
	$(CC) $(CFLAGS) $^ -o $@

tests/test_recovery: tests/test_recovery.o \
                     src/process_manager.o \
                     src/resource_manager.o \
                     src/deadlock_detector.o \
                     src/recovery.o
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(OBJ) \
	      tests/*.o \
	      $(TARGET) \
	      $(TEST_TARGETS)
