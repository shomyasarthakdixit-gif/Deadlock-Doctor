#include "scheduler.h"

#include <stdio.h>
#include <string.h>

static void ready_queue_init(ReadyQueue *queue)
{
    if (queue == NULL)
        return;

    memset(queue, 0, sizeof(*queue));
}

static int valid_pid(int pid)
{
    return pid >= 0 && pid < MAX_PROCESSES;
}

static int queue_is_full(const ReadyQueue *queue)
{
    return queue->count >= SCHEDULER_QUEUE_SIZE;
}

static int queue_is_empty(const ReadyQueue *queue)
{
    return queue->count == 0;
}
static int queue_push(ReadyQueue *queue, int pid)
{
    if (queue == NULL ||
        !valid_pid(pid) ||
        queue_is_full(queue)) {
        return -1;
    }

    queue->pid[queue->rear] = pid;

    queue->rear =
        (queue->rear + 1) % SCHEDULER_QUEUE_SIZE;

    queue->count++;

    return 0;
}

static int queue_pop(ReadyQueue *queue)
{
    if (queue == NULL || queue_is_empty(queue))
        return -1;

    int pid = queue->pid[queue->front];

    queue->front =
        (queue->front + 1) % SCHEDULER_QUEUE_SIZE;

    queue->count--;

    return pid;
}

static void add_arrived_processes(Scheduler *scheduler,
                                  ProcessManager *pm)
{
    if (scheduler == NULL || pm == NULL)
        return;

    for (int pid = 0; pid < MAX_PROCESSES; pid++) {

        Process *process = pm_get_process(pm, pid);

        if (process == NULL)
            continue;

        if (process->state != PROCESS_READY)
            continue;

        if (process->arrival_time > scheduler->current_time)
            continue;

        int already_queued = 0;

        for (int i = 0;
             i < scheduler->ready_queue.count;
             i++) {

            int index =
                (scheduler->ready_queue.front + i)
                % SCHEDULER_QUEUE_SIZE;

            if (scheduler->ready_queue.pid[index] == pid) {
                already_queued = 1;
                break;
            }
        }

        if (!already_queued)
            queue_push(&scheduler->ready_queue, pid);
    }
}

static int find_next_fcfs_process(ProcessManager *pm,
                                  int current_time)
{
    int selected_pid = -1;
    int selected_arrival = 0;

    for (int pid = 0; pid < MAX_PROCESSES; pid++) {

        Process *process = pm_get_process(pm, pid);

        if (process == NULL)
            continue;

        if (process->state != PROCESS_READY)
            continue;

        if (process->remaining_time <= 0)
            continue;

        if (process->arrival_time > current_time)
            continue;

        if (selected_pid == -1 ||
            process->arrival_time < selected_arrival ||
            (process->arrival_time == selected_arrival &&
             process->pid < selected_pid)) {

            selected_pid = process->pid;
            selected_arrival = process->arrival_time;
        }
    }

    return selected_pid;
}

static int find_next_arrival(const ProcessManager *pm,
                             int current_time)
{
    int next_arrival = -1;

    for (int pid = 0; pid < MAX_PROCESSES; pid++) {

        const Process *process =
            pm_get_process_const(pm, pid);

        if (process == NULL)
            continue;

        if (process->state != PROCESS_READY)
            continue;

        if (process->remaining_time <= 0)
            continue;

        if (process->arrival_time <= current_time)
            continue;

        if (next_arrival == -1 ||
            process->arrival_time < next_arrival) {

            next_arrival = process->arrival_time;
        }
    }

    return next_arrival;
}

static void print_gantt_entry(int pid,
                              int start_time,
                              int end_time)
{
    printf("| P%d [%d-%d] ",
           pid,
           start_time,
           end_time);
}

void scheduler_init(Scheduler *scheduler,
                    SchedulingAlgorithm algorithm,
                    int time_quantum)
{
    if (scheduler == NULL)
        return;

    memset(scheduler, 0, sizeof(*scheduler));

    scheduler->algorithm = algorithm;
    scheduler->time_quantum = time_quantum;
    scheduler->current_time = 0;

    ready_queue_init(&scheduler->ready_queue);
}

int scheduler_enqueue(Scheduler *scheduler,
                      const ProcessManager *pm,
                      int pid)
{
    if (scheduler == NULL ||
        pm == NULL ||
        !valid_pid(pid)) {
        return -1;
    }

    const Process *process =
        pm_get_process_const(pm, pid);

    if (process == NULL)
        return -1;

    if (process->state != PROCESS_READY)
        return -1;

    if (queue_is_full(&scheduler->ready_queue))
        return -1;

    for (int i = 0;
         i < scheduler->ready_queue.count;
         i++) {

        int index =
            (scheduler->ready_queue.front + i)
            % SCHEDULER_QUEUE_SIZE;

        if (scheduler->ready_queue.pid[index] == pid)
            return -1;
    }

    return queue_push(&scheduler->ready_queue, pid);
}

int scheduler_dequeue(Scheduler *scheduler)
{
    if (scheduler == NULL)
        return -1;

    return queue_pop(&scheduler->ready_queue);
}

void scheduler_clear_queue(Scheduler *scheduler)
{
    if (scheduler == NULL)
        return;

    ready_queue_init(&scheduler->ready_queue);
}

int scheduler_run_fcfs(Scheduler *scheduler,
                       ProcessManager *pm)
{
    if (scheduler == NULL || pm == NULL)
        return -1;

    scheduler->current_time = 0;

    scheduler_clear_queue(scheduler);

    printf("\n========================================\n");
    printf("              FCFS SCHEDULING\n");
    printf("========================================\n");

    int completed = 0;

    for (;;) {

        int pid =
            find_next_fcfs_process(pm,
                                   scheduler->current_time);

        if (pid == -1) {

            int next_arrival =
                find_next_arrival(pm,
                                  scheduler->current_time);

            if (next_arrival == -1)
                break;

            scheduler->current_time = next_arrival;
            continue;
        }

        Process *process = pm_get_process(pm, pid);

        if (process == NULL)
            continue;

        if (pm_set_state(pm,
                         pid,
                         PROCESS_RUNNING) != 0) {
            return -1;
        }

        int start_time = scheduler->current_time;

        if (scheduler->current_time <
            process->arrival_time) {

            scheduler->current_time =
                process->arrival_time;

            start_time =
                scheduler->current_time;
        }

        scheduler->current_time +=
            process->remaining_time;

        process->remaining_time = 0;

        process->completion_time =
            scheduler->current_time;

        process->turnaround_time =
            process->completion_time -
            process->arrival_time;

        process->waiting_time =
            process->turnaround_time -
            process->burst_time;

        print_gantt_entry(pid,
                          start_time,
                          scheduler->current_time);

        if (pm_terminate_process(pm, pid) != 0)
            return -1;

        completed++;
    }

    printf("|\n");
    printf("Completed processes: %d\n",
           completed);

    return completed;
}

int scheduler_run_round_robin(Scheduler *scheduler,
                              ProcessManager *pm)
{
    if (scheduler == NULL || pm == NULL)
        return -1;

    if (scheduler->time_quantum <= 0)
        return -1;

    scheduler->current_time = 0;

    scheduler_clear_queue(scheduler);

    printf("\n========================================\n");
    printf("          ROUND ROBIN SCHEDULING\n");
    printf("========================================\n");

    int completed = 0;

    for (;;) {

        add_arrived_processes(scheduler, pm);

        if (queue_is_empty(&scheduler->ready_queue)) {

            int next_arrival =
                find_next_arrival(pm,
                                  scheduler->current_time);

            if (next_arrival == -1)
                break;

            scheduler->current_time = next_arrival;

            add_arrived_processes(scheduler, pm);

            if (queue_is_empty(&scheduler->ready_queue))
                continue;
        }

        int pid = queue_pop(&scheduler->ready_queue);

        if (!valid_pid(pid))
            continue;

        Process *process =
            pm_get_process(pm, pid);

        if (process == NULL)
            continue;

        if (process->state != PROCESS_READY)
            continue;

        if (pm_set_state(pm,
                         pid,
                         PROCESS_RUNNING) != 0) {
            return -1;
        }

        int start_time = scheduler->current_time;

        int run_time = process->remaining_time;

        if (run_time > scheduler->time_quantum)
            run_time = scheduler->time_quantum;

        scheduler->current_time += run_time;

        process->remaining_time -= run_time;

        print_gantt_entry(pid,
                          start_time,
                          scheduler->current_time);

        /*
         * A process that arrived during this time slice
         * becomes eligible before the current process is
         * requeued.
         */
        add_arrived_processes(scheduler, pm);

        if (process->remaining_time == 0) {

            process->completion_time =
                scheduler->current_time;

            process->turnaround_time =
                process->completion_time -
                process->arrival_time;

            process->waiting_time =
                process->turnaround_time -
                process->burst_time;

            if (pm_terminate_process(pm, pid) != 0)
                return -1;

            completed++;

        } else {

            if (pm_set_state(pm,
                             pid,
                             PROCESS_READY) != 0) {
                return -1;
            }

            if (scheduler_enqueue(scheduler,
                                  pm,
                                  pid) != 0) {
                return -1;
            }
        }
    }

    printf("|\n");
    printf("Completed processes: %d\n",
           completed);

    return completed;
}

int scheduler_run(Scheduler *scheduler,
                  ProcessManager *pm)
{
    if (scheduler == NULL || pm == NULL)
        return -1;

    switch (scheduler->algorithm) {

    case SCHEDULER_FCFS:
        return scheduler_run_fcfs(scheduler, pm);

    case SCHEDULER_ROUND_ROBIN:
        return scheduler_run_round_robin(scheduler, pm);

    default:
        return -1;
    }
}

void scheduler_print_queue(const Scheduler *scheduler)
{
    if (scheduler == NULL)
        return;

    printf("\n========== READY QUEUE ==========\n");

    if (queue_is_empty(&scheduler->ready_queue)) {
        printf("Queue is empty.\n");
        return;
    }

    for (int i = 0;
         i < scheduler->ready_queue.count;
         i++) {

        int index =
            (scheduler->ready_queue.front + i)
            % SCHEDULER_QUEUE_SIZE;

        printf("P%d",
               scheduler->ready_queue.pid[index]);

        if (i + 1 < scheduler->ready_queue.count)
            printf(" -> ");
    }

    printf("\n");
}

void scheduler_print_results(const ProcessManager *pm)
{
    if (pm == NULL)
        return;

    printf("\n============================================================\n");
    printf("                  SCHEDULING RESULTS\n");
    printf("============================================================\n");

    printf("%-5s %-12s %-10s %-10s %-12s %-12s\n",
           "PID",
           "STATE",
           "ARRIVAL",
           "BURST",
           "WAITING",
           "TURNAROUND");

    printf("------------------------------------------------------------\n");

    for (int pid = 0; pid < MAX_PROCESSES; pid++) {

        const Process *process =
            pm_get_process_const(pm, pid);

        if (process == NULL)
            continue;

        printf("P%-4d %-12s %-10d %-10d %-12d %-12d\n",
               process->pid,
               pm_state_to_string(process->state),
               process->arrival_time,
               process->burst_time,
               process->waiting_time,
               process->turnaround_time);
    }

    printf("------------------------------------------------------------\n");
}
