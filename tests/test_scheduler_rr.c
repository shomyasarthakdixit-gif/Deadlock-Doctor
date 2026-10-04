#include <assert.h>
#include <stdio.h>

#include "process_manager.h"
#include "scheduler.h"

int main(void)
{
    ProcessManager pm;
    Scheduler scheduler;

    printf("========================================\n");
    printf("       ROUND ROBIN SCHEDULER TEST\n");
    printf("========================================\n");

    /*
     * ------------------------------------------------
     * TEST 1: Basic Round Robin
     * ------------------------------------------------
     *
     * Time quantum = 2
     *
     * P0: Burst = 5
     * P1: Burst = 3
     *
     * Expected execution:
     *
     * P0: 0 -> 2
     * P1: 2 -> 4
     * P0: 4 -> 6
     * P1: 6 -> 7
     * P0: 7 -> 8
     */

    pm_init(&pm);

    int p0 = pm_create_process(&pm,
                               "Process-A",
                               0,
                               5,
                               1);

    int p1 = pm_create_process(&pm,
                               "Process-B",
                               0,
                               3,
                               1);

    assert(p0 >= 0);
    assert(p1 >= 0);

    assert(pm_set_state(&pm,
                        p0,
                        PROCESS_READY) == 0);

    assert(pm_set_state(&pm,
                        p1,
                        PROCESS_READY) == 0);

    scheduler_init(&scheduler,
                   SCHEDULER_ROUND_ROBIN,
                   2);

    int completed =
        scheduler_run(&scheduler, &pm);

    assert(completed == 2);

    Process *process0 =
        pm_get_process(&pm, p0);

    Process *process1 =
        pm_get_process(&pm, p1);

    assert(process0 != NULL);
    assert(process1 != NULL);

    /*
     * P0 completes at time 8.
     * P1 completes at time 7.
     */
    assert(process0->completion_time == 8);
    assert(process1->completion_time == 7);

    /*
     * Turnaround:
     *
     * P0 = 8 - 0 = 8
     * P1 = 7 - 0 = 7
     */
    assert(process0->turnaround_time == 8);
    assert(process1->turnaround_time == 7);

    /*
     * Waiting:
     *
     * P0 = 8 - 5 = 3
     * P1 = 7 - 3 = 4
     */
    assert(process0->waiting_time == 3);
    assert(process1->waiting_time == 4);

    assert(process0->remaining_time == 0);
    assert(process1->remaining_time == 0);

    assert(process0->state == PROCESS_TERMINATED);
    assert(process1->state == PROCESS_TERMINATED);

    printf("[PASS] Basic Round Robin scheduling\n");
    printf("[PASS] Preemption and remaining time\n");
    printf("[PASS] Completion times correct\n");
    printf("[PASS] Turnaround times correct\n");
    printf("[PASS] Waiting times correct\n");

    /*
     * ------------------------------------------------
     * TEST 2: Invalid time quantum
     * ------------------------------------------------
     */

    pm_init(&pm);

    int p2 = pm_create_process(&pm,
                               "InvalidQuantum",
                               0,
                               5,
                               1);

    assert(p2 >= 0);

    assert(pm_set_state(&pm,
                        p2,
                        PROCESS_READY) == 0);

    scheduler_init(&scheduler,
                   SCHEDULER_ROUND_ROBIN,
                   0);

    completed =
        scheduler_run(&scheduler, &pm);

    assert(completed == -1);

    Process *invalid_quantum_process =
        pm_get_process(&pm, p2);

    assert(invalid_quantum_process != NULL);

    assert(invalid_quantum_process->state ==
           PROCESS_READY);

    assert(invalid_quantum_process->remaining_time == 5);

    printf("[PASS] Invalid time quantum rejected\n");

    /*
     * ------------------------------------------------
     * TEST 3: Different arrival times
     * ------------------------------------------------
     *
     * P0 arrives at 0, burst 4
     * P1 arrives at 3, burst 2
     *
     * Quantum = 2
     */

    pm_init(&pm);

    int p3 = pm_create_process(&pm,
                               "Early",
                               0,
                               4,
                               1);

    int p4 = pm_create_process(&pm,
                               "Late",
                               3,
                               2,
                               1);

    assert(p3 >= 0);
    assert(p4 >= 0);

    assert(pm_set_state(&pm,
                        p3,
                        PROCESS_READY) == 0);

    assert(pm_set_state(&pm,
                        p4,
                        PROCESS_READY) == 0);

    scheduler_init(&scheduler,
                   SCHEDULER_ROUND_ROBIN,
                   2);

    completed =
        scheduler_run(&scheduler, &pm);

    assert(completed == 2);

    Process *early =
        pm_get_process(&pm, p3);

    Process *late =
        pm_get_process(&pm, p4);

    assert(early != NULL);
    assert(late != NULL);

    assert(early->remaining_time == 0);
    assert(late->remaining_time == 0);

    assert(early->state == PROCESS_TERMINATED);
    assert(late->state == PROCESS_TERMINATED);

    printf("[PASS] Round Robin with different arrivals\n");

    /*
     * ------------------------------------------------
     * TEST 4: No READY processes
     * ------------------------------------------------
     */

    pm_init(&pm);

    int p5 = pm_create_process(&pm,
                               "WaitingOnly",
                               0,
                               5,
                               1);

    assert(p5 >= 0);

    /*
     * NEW -> READY -> RUNNING -> WAITING
     */
    assert(pm_set_state(&pm,
                        p5,
                        PROCESS_READY) == 0);

    assert(pm_set_state(&pm,
                        p5,
                        PROCESS_RUNNING) == 0);

    assert(pm_set_state(&pm,
                        p5,
                        PROCESS_WAITING) == 0);

    scheduler_init(&scheduler,
                   SCHEDULER_ROUND_ROBIN,
                   2);

    completed =
        scheduler_run(&scheduler, &pm);

    assert(completed == 0);

    Process *waiting_process =
        pm_get_process(&pm, p5);

    assert(waiting_process != NULL);

    assert(waiting_process->state ==
           PROCESS_WAITING);

    assert(waiting_process->remaining_time == 5);

    printf("[PASS] No READY processes handled\n");

    /*
     * ------------------------------------------------
     * FINAL RESULT
     * ------------------------------------------------
     */

    printf("\n========================================\n");
    printf("    ALL ROUND ROBIN TESTS PASSED\n");
    printf("========================================\n");

    return 0;
}
