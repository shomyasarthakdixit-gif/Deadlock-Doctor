#include <assert.h>
#include <stdio.h>

#include "process_manager.h"
#include "scheduler.h"

int main(void)
{
    ProcessManager pm;
    Scheduler scheduler;

    printf("========================================\n");
    printf("       FCFS EDGE CASE TESTS\n");
    printf("========================================\n");

    /*
     * ------------------------------------------------
     * TEST 1: Different arrival times
     * ------------------------------------------------
     */

    pm_init(&pm);

    int p0 = pm_create_process(&pm,
                               "Early",
                               0,
                               2,
                               1);

    int p1 = pm_create_process(&pm,
                               "Late",
                               5,
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
                   SCHEDULER_FCFS,
                   0);

    int completed =
        scheduler_run(&scheduler, &pm);

    assert(completed == 2);

    Process *early =
        pm_get_process(&pm, p0);

    Process *late =
        pm_get_process(&pm, p1);

    assert(early != NULL);
    assert(late != NULL);

    /*
     * P0: 0 -> 2
     * CPU idle: 2 -> 5
     * P1: 5 -> 8
     */
    assert(early->completion_time == 2);
    assert(early->waiting_time == 0);
    assert(early->turnaround_time == 2);

    assert(late->completion_time == 8);
    assert(late->waiting_time == 0);
    assert(late->turnaround_time == 3);

    printf("[PASS] Different arrival times\n");

    /*
     * ------------------------------------------------
     * TEST 2: Same arrival time
     * ------------------------------------------------
     */

    pm_init(&pm);

    int p2 = pm_create_process(&pm,
                               "First",
                               0,
                               4,
                               1);

    int p3 = pm_create_process(&pm,
                               "Second",
                               0,
                               2,
                               1);

    assert(p2 >= 0);
    assert(p3 >= 0);

    assert(pm_set_state(&pm,
                        p2,
                        PROCESS_READY) == 0);

    assert(pm_set_state(&pm,
                        p3,
                        PROCESS_READY) == 0);

    scheduler_init(&scheduler,
                   SCHEDULER_FCFS,
                   0);

    completed =
        scheduler_run(&scheduler, &pm);

    assert(completed == 2);

    Process *first =
        pm_get_process(&pm, p2);

    Process *second =
        pm_get_process(&pm, p3);

    assert(first != NULL);
    assert(second != NULL);

    /*
     * Same arrival time.
     * Lower PID runs first.
     */
    assert(first->completion_time == 4);
    assert(second->completion_time == 6);

    printf("[PASS] Same arrival time uses PID order\n");

    /*
     * ------------------------------------------------
     * TEST 3: WAITING process must not be scheduled
     * ------------------------------------------------
     */

    pm_init(&pm);

    int p4 = pm_create_process(&pm,
                               "ReadyProcess",
                               0,
                               3,
                               1);

    int p5 = pm_create_process(&pm,
                               "WaitingProcess",
                               0,
                               4,
                               1);

    assert(p4 >= 0);
    assert(p5 >= 0);

    /*
     * ReadyProcess:
     * NEW -> READY
     */
    assert(pm_set_state(&pm,
                        p4,
                        PROCESS_READY) == 0);

    /*
     * WaitingProcess:
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
                   SCHEDULER_FCFS,
                   0);

    completed =
        scheduler_run(&scheduler, &pm);

    assert(completed == 1);

    Process *ready_process =
        pm_get_process(&pm, p4);

    Process *waiting_process =
        pm_get_process(&pm, p5);

    assert(ready_process != NULL);
    assert(waiting_process != NULL);

    assert(ready_process->state ==
           PROCESS_TERMINATED);

    assert(waiting_process->state ==
           PROCESS_WAITING);

    assert(waiting_process->remaining_time == 4);

    printf("[PASS] WAITING process skipped\n");

    /*
     * ------------------------------------------------
     * TEST 4: TERMINATED process must not be scheduled
     * ------------------------------------------------
     */

    pm_init(&pm);

    int p6 = pm_create_process(&pm,
                               "Terminated",
                               0,
                               5,
                               1);

    int p7 = pm_create_process(&pm,
                               "Ready",
                               0,
                               2,
                               1);

    assert(p6 >= 0);
    assert(p7 >= 0);

    assert(pm_set_state(&pm,
                        p6,
                        PROCESS_READY) == 0);

    assert(pm_terminate_process(&pm, p6) == 0);

    assert(pm_set_state(&pm,
                        p7,
                        PROCESS_READY) == 0);

    scheduler_init(&scheduler,
                   SCHEDULER_FCFS,
                   0);

    completed =
        scheduler_run(&scheduler, &pm);

    assert(completed == 1);

    Process *terminated_process =
        pm_get_process(&pm, p6);

    Process *ready_process2 =
        pm_get_process(&pm, p7);

    assert(terminated_process != NULL);
    assert(ready_process2 != NULL);

    assert(terminated_process->state ==
           PROCESS_TERMINATED);

    assert(terminated_process->remaining_time == 5);

    assert(ready_process2->state ==
           PROCESS_TERMINATED);

    printf("[PASS] TERMINATED process skipped\n");

    /*
     * ------------------------------------------------
     * TEST 5: No READY processes
     * ------------------------------------------------
     */

    pm_init(&pm);

    int p8 = pm_create_process(&pm,
                               "WaitingOnly",
                               0,
                               5,
                               1);

    assert(p8 >= 0);

    /*
     * NEW -> READY -> RUNNING -> WAITING
     */
    assert(pm_set_state(&pm,
                        p8,
                        PROCESS_READY) == 0);

    assert(pm_set_state(&pm,
                        p8,
                        PROCESS_RUNNING) == 0);

    assert(pm_set_state(&pm,
                        p8,
                        PROCESS_WAITING) == 0);

    scheduler_init(&scheduler,
                   SCHEDULER_FCFS,
                   0);

    completed =
        scheduler_run(&scheduler, &pm);

    assert(completed == 0);

    Process *waiting_only =
        pm_get_process(&pm, p8);

    assert(waiting_only != NULL);

    assert(waiting_only->state ==
           PROCESS_WAITING);

    printf("[PASS] No READY processes handled\n");

    /*
     * ------------------------------------------------
     * FINAL RESULT
     * ------------------------------------------------
     */

    printf("\n========================================\n");
    printf("    ALL FCFS EDGE TESTS PASSED\n");
    printf("========================================\n");

    return 0;
}
