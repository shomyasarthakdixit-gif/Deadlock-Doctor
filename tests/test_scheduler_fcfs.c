#include <assert.h>
#include <stdio.h>

#include "process_manager.h"
#include "scheduler.h"

int main(void)
{
    ProcessManager pm;
    Scheduler scheduler;

    printf("========================================\n");
    printf("          FCFS SCHEDULER TEST\n");
    printf("========================================\n");

    pm_init(&pm);

    /*
     * Create three processes:
     *
     * P0: Arrival = 0, Burst = 5
     * P1: Arrival = 1, Burst = 3
     * P2: Arrival = 2, Burst = 2
     */
    int p0 = pm_create_process(&pm,
                               "Process-A",
                               0,
                               5,
                               1);

    int p1 = pm_create_process(&pm,
                               "Process-B",
                               1,
                               3,
                               1);

    int p2 = pm_create_process(&pm,
                               "Process-C",
                               2,
                               2,
                               1);

    assert(p0 >= 0);
    assert(p1 >= 0);
    assert(p2 >= 0);

    /*
     * Processes must enter READY state
     * before the scheduler can execute them.
     */
    assert(pm_set_state(&pm,
                        p0,
                        PROCESS_READY) == 0);

    assert(pm_set_state(&pm,
                        p1,
                        PROCESS_READY) == 0);

    assert(pm_set_state(&pm,
                        p2,
                        PROCESS_READY) == 0);

    printf("\n[PASS] Processes created and moved to READY\n");

    /*
     * Initialize FCFS scheduler.
     */
    scheduler_init(&scheduler,
                   SCHEDULER_FCFS,
                   0);

    /*
     * Run FCFS.
     */
    int completed =
        scheduler_run(&scheduler, &pm);

    assert(completed == 3);

    printf("\n[PASS] FCFS completed all processes\n");

    /*
     * Expected FCFS execution:
     *
     * P0: 0 -> 5
     * P1: 5 -> 8
     * P2: 8 -> 10
     */

    Process *process0 = pm_get_process(&pm, p0);
    Process *process1 = pm_get_process(&pm, p1);
    Process *process2 = pm_get_process(&pm, p2);

    assert(process0 != NULL);
    assert(process1 != NULL);
    assert(process2 != NULL);

    /*
     * Check completion times.
     */
    assert(process0->completion_time == 5);
    assert(process1->completion_time == 8);
    assert(process2->completion_time == 10);

    printf("[PASS] Completion times correct\n");

    /*
     * Turnaround Time = Completion - Arrival
     *
     * P0 = 5 - 0 = 5
     * P1 = 8 - 1 = 7
     * P2 = 10 - 2 = 8
     */
    assert(process0->turnaround_time == 5);
    assert(process1->turnaround_time == 7);
    assert(process2->turnaround_time == 8);

    printf("[PASS] Turnaround times correct\n");

    /*
     * Waiting Time = Turnaround - Burst
     *
     * P0 = 5 - 5 = 0
     * P1 = 7 - 3 = 4
     * P2 = 8 - 2 = 6
     */
    assert(process0->waiting_time == 0);
    assert(process1->waiting_time == 4);
    assert(process2->waiting_time == 6);

    printf("[PASS] Waiting times correct\n");

    /*
     * All processes must be TERMINATED.
     */
    assert(process0->state == PROCESS_TERMINATED);
    assert(process1->state == PROCESS_TERMINATED);
    assert(process2->state == PROCESS_TERMINATED);

    printf("[PASS] All processes terminated\n");

    /*
     * Print final scheduling results.
     */
    scheduler_print_results(&pm);

    printf("\n========================================\n");
    printf("       ALL FCFS TESTS PASSED\n");
    printf("========================================\n");

    return 0;
}
