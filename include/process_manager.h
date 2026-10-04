#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include "resource_manager.h"

#define PROCESS_NAME_LEN 32

typedef enum {
    PROCESS_NEW = 0,
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_WAITING,
    PROCESS_TERMINATED
} ProcessState;

typedef struct {
    int pid;
    char name[PROCESS_NAME_LEN];

    ProcessState state;

    int arrival_time;
    int burst_time;
    int remaining_time;
    int priority;

    int waiting_time;
    int turnaround_time;
    int completion_time;

    int active;
} Process;

typedef struct {
    Process processes[MAX_PROCESSES];
    int process_count;
} ProcessManager;

/* Initialize the Process Manager */
void pm_init(ProcessManager *pm);

/* Create a new process and return its PID */
int pm_create_process(ProcessManager *pm,
                      const char *name,
                      int arrival_time,
                      int burst_time,
                      int priority);

/* Terminate a process */
int pm_terminate_process(ProcessManager *pm, int pid);

/* Change process state */
int pm_set_state(ProcessManager *pm,
                 int pid,
                 ProcessState new_state);

/* Check whether a PID belongs to an active process */
int pm_process_exists(const ProcessManager *pm, int pid);

/* Get a process */
Process *pm_get_process(ProcessManager *pm, int pid);

/* Get a process without modifying it */
const Process *pm_get_process_const(const ProcessManager *pm, int pid);

/* Display all processes */
void pm_list_processes(const ProcessManager *pm);

/* Display one process */
void pm_show_process(const ProcessManager *pm, int pid);

/* Get process count */
int pm_get_process_count(const ProcessManager *pm);

/* Convert process state to readable text */
const char *pm_state_to_string(ProcessState state);

#endif /* PROCESS_MANAGER_H */

