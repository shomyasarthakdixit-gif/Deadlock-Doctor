#include "process_manager.h"

#include <stdio.h>
#include <string.h>

static int valid_pid(int pid)
{
    return pid >= 0 && pid < MAX_PROCESSES;
}

static int valid_process_name(const char *name)
{
    return name != NULL && name[0] != '\0';
}

static int process_has_name(const ProcessManager *pm,
                            const char *name)
{
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (pm->processes[i].active &&
            strcmp(pm->processes[i].name, name) == 0) {
            return 1;
        }
    }

    return 0;
}

static int valid_state_transition(ProcessState current,
                                  ProcessState next)
{
    if (current == PROCESS_NEW &&
        next == PROCESS_READY)
        return 1;

    if (current == PROCESS_READY &&
        next == PROCESS_RUNNING)
        return 1;

    if (current == PROCESS_RUNNING &&
        (next == PROCESS_READY ||
         next == PROCESS_WAITING ||
         next == PROCESS_TERMINATED))
        return 1;

    if (current == PROCESS_WAITING &&
        next == PROCESS_READY)
        return 1;

    if (current == PROCESS_READY &&
        next == PROCESS_TERMINATED)
        return 1;

    if (current == PROCESS_WAITING &&
        next == PROCESS_TERMINATED)
        return 1;

    return 0;
}

void pm_init(ProcessManager *pm)
{
    if (pm == NULL)
        return;

    memset(pm, 0, sizeof(*pm));
}

int pm_create_process(ProcessManager *pm,
                      const char *name,
                      int arrival_time,
                      int burst_time,
                      int priority)
{
    if (pm == NULL || !valid_process_name(name))
        return -1;

    if (arrival_time < 0 || burst_time <= 0)
        return -1;

    if (process_has_name(pm, name))
        return -1;

    if (pm->process_count >= MAX_PROCESSES)
        return -1;

    int pid = -1;

    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (!pm->processes[i].active) {
            pid = i;
            break;
        }
    }

    if (pid < 0)
        return -1;

    Process *process = &pm->processes[pid];

    memset(process, 0, sizeof(*process));

    process->pid = pid;

    strncpy(process->name,
            name,
            PROCESS_NAME_LEN - 1);

    process->name[PROCESS_NAME_LEN - 1] = '\0';

    process->state = PROCESS_NEW;
    process->arrival_time = arrival_time;
    process->burst_time = burst_time;
    process->remaining_time = burst_time;
    process->priority = priority;

    process->waiting_time = 0;
    process->turnaround_time = 0;
    process->completion_time = 0;

    process->active = 1;

    pm->process_count++;

    printf("Process P%d (%s) created.\n",
           process->pid,
           process->name);

    return pid;
}

int pm_terminate_process(ProcessManager *pm, int pid)
{
    if (pm == NULL || !valid_pid(pid))
        return -1;

    Process *process = &pm->processes[pid];

    if (!process->active)
        return -1;

    if (process->state == PROCESS_TERMINATED)
        return -1;

    process->state = PROCESS_TERMINATED;

    printf("Process P%d (%s) terminated.\n",
           process->pid,
           process->name);

    return 0;
}

int pm_set_state(ProcessManager *pm,
                 int pid,
                 ProcessState new_state)
{
    if (pm == NULL || !valid_pid(pid))
        return -1;

    Process *process = &pm->processes[pid];

    if (!process->active)
        return -1;

    if (process->state == new_state)
        return 0;

    if (!valid_state_transition(process->state,
                                 new_state)) {
        return -1;
    }

    process->state = new_state;

    return 0;
}

int pm_process_exists(const ProcessManager *pm, int pid)
{
    if (pm == NULL || !valid_pid(pid))
        return 0;

    return pm->processes[pid].active;
}

Process *pm_get_process(ProcessManager *pm, int pid)
{
    if (pm == NULL || !valid_pid(pid))
        return NULL;

    if (!pm->processes[pid].active)
        return NULL;

    return &pm->processes[pid];
}

const Process *pm_get_process_const(const ProcessManager *pm,
                                    int pid)
{
    if (pm == NULL || !valid_pid(pid))
        return NULL;

    if (!pm->processes[pid].active)
        return NULL;

    return &pm->processes[pid];
}

void pm_list_processes(const ProcessManager *pm)
{
    if (pm == NULL)
        return;

    printf("\n============================================================\n");
    printf("                    PROCESS TABLE\n");
    printf("============================================================\n");

    printf("%-5s %-15s %-12s %-8s %-8s %-8s\n",
           "PID",
           "NAME",
           "STATE",
           "ARRIVAL",
           "BURST",
           "REMAIN");

    printf("------------------------------------------------------------\n");

    int found = 0;

    for (int i = 0; i < MAX_PROCESSES; i++) {
        const Process *process = &pm->processes[i];

        if (!process->active)
            continue;

        found = 1;

        printf("P%-4d %-15s %-12s %-8d %-8d %-8d\n",
               process->pid,
               process->name,
               pm_state_to_string(process->state),
               process->arrival_time,
               process->burst_time,
               process->remaining_time);
    }

    if (!found)
        printf("No active processes.\n");

    printf("------------------------------------------------------------\n");
}

void pm_show_process(const ProcessManager *pm, int pid)
{
    const Process *process = pm_get_process_const(pm, pid);

    if (process == NULL) {
        printf("Process P%d does not exist.\n", pid);
        return;
    }

    printf("\n========== PROCESS P%d ==========\n", process->pid);
    printf("Name            : %s\n", process->name);
    printf("State           : %s\n",
           pm_state_to_string(process->state));
    printf("Arrival Time    : %d\n", process->arrival_time);
    printf("Burst Time      : %d\n", process->burst_time);
    printf("Remaining Time  : %d\n", process->remaining_time);
    printf("Priority        : %d\n", process->priority);
    printf("Waiting Time    : %d\n", process->waiting_time);
    printf("Turnaround Time : %d\n", process->turnaround_time);
    printf("Completion Time : %d\n", process->completion_time);
}

int pm_get_process_count(const ProcessManager *pm)
{
    if (pm == NULL)
        return 0;

    return pm->process_count;
}

const char *pm_state_to_string(ProcessState state)
{
    switch (state) {
    case PROCESS_NEW:
        return "NEW";

    case PROCESS_READY:
        return "READY";

    case PROCESS_RUNNING:
        return "RUNNING";

    case PROCESS_WAITING:
        return "WAITING";

    case PROCESS_TERMINATED:
        return "TERMINATED";

    default:
        return "UNKNOWN";
    }
}
