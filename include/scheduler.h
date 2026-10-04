#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process_manager.h"

#define SCHEDULER_QUEUE_SIZE MAX_PROCESSES

typedef enum {
    SCHEDULER_FCFS = 0,
    SCHEDULER_ROUND_ROBIN
} SchedulingAlgorithm;

typedef struct {
    int pid[SCHEDULER_QUEUE_SIZE];
    int front;
    int rear;
    int count;
} ReadyQueue;

typedef struct {
    SchedulingAlgorithm algorithm;
    int time_quantum;
    int current_time;

    ReadyQueue ready_queue;
} Scheduler;

/* Initialize scheduler */
void scheduler_init(Scheduler *scheduler,
                    SchedulingAlgorithm algorithm,
                    int time_quantum);

/* Add a READY process to the scheduler */
int scheduler_enqueue(Scheduler *scheduler,
                      const ProcessManager *pm,
                      int pid);

/* Remove a process from the ready queue */
int scheduler_dequeue(Scheduler *scheduler);

/* Clear the ready queue */
void scheduler_clear_queue(Scheduler *scheduler);

/* Run FCFS scheduling */
int scheduler_run_fcfs(Scheduler *scheduler,
                        ProcessManager *pm);

/* Run Round Robin scheduling */
int scheduler_run_round_robin(Scheduler *scheduler,
                               ProcessManager *pm);

/* Run the selected scheduling algorithm */
int scheduler_run(Scheduler *scheduler,
                  ProcessManager *pm);

/* Display ready queue */
void scheduler_print_queue(const Scheduler *scheduler);

/* Display scheduling results */
void scheduler_print_results(const ProcessManager *pm);

#endif /* SCHEDULER_H */
