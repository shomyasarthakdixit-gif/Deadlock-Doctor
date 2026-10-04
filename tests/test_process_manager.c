#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "process_manager.h"

int main(void)
{
    ProcessManager pm;

    printf("========================================\n");
    printf("       PROCESS MANAGER TESTS\n");
    printf("========================================\n");

    pm_init(&pm);

    /* Test empty process manager */
    assert(pm_get_process_count(&pm) == 0);

    /* Test process creation */
    int p1 = pm_create_process(&pm,
                               "Process-A",
                               0,
                               5,
                               1);

    assert(p1 >= 0);
    assert(pm_process_exists(&pm, p1));

    Process *process = pm_get_process(&pm, p1);

    assert(process != NULL);
    assert(strcmp(process->name, "Process-A") == 0);
    assert(process->arrival_time == 0);
    assert(process->burst_time == 5);
    assert(process->remaining_time == 5);
    assert(process->priority == 1);
    assert(process->state == PROCESS_NEW);

    printf("[PASS] Process creation\n");

    /* Test duplicate process name */
    int duplicate =
        pm_create_process(&pm,
                          "Process-A",
                          1,
                          4,
                          2);

    assert(duplicate == -1);

    printf("[PASS] Duplicate process name rejected\n");

    /* Create another process */
    int p2 = pm_create_process(&pm,
                               "Process-B",
                               2,
                               3,
                               2);

    assert(p2 >= 0);
    assert(p2 != p1);

    printf("[PASS] Unique PID allocation\n");

    /* Test lifecycle: NEW -> READY */
    assert(pm_set_state(&pm,
                        p1,
                        PROCESS_READY) == 0);

    assert(pm_get_process(&pm, p1)->state ==
           PROCESS_READY);

    printf("[PASS] NEW -> READY\n");

    /* Test lifecycle: READY -> RUNNING */
    assert(pm_set_state(&pm,
                        p1,
                        PROCESS_RUNNING) == 0);

    assert(pm_get_process(&pm, p1)->state ==
           PROCESS_RUNNING);

    printf("[PASS] READY -> RUNNING\n");

    /* Test lifecycle: RUNNING -> WAITING */
    assert(pm_set_state(&pm,
                        p1,
                        PROCESS_WAITING) == 0);

    assert(pm_get_process(&pm, p1)->state ==
           PROCESS_WAITING);

    printf("[PASS] RUNNING -> WAITING\n");

    /* Test lifecycle: WAITING -> READY */
    assert(pm_set_state(&pm,
                        p1,
                        PROCESS_READY) == 0);

    assert(pm_get_process(&pm, p1)->state ==
           PROCESS_READY);

    printf("[PASS] WAITING -> READY\n");

    /* Test invalid transition: READY -> WAITING */
    assert(pm_set_state(&pm,
                        p1,
                        PROCESS_WAITING) == -1);

    assert(pm_get_process(&pm, p1)->state ==
           PROCESS_READY);

    printf("[PASS] Invalid state transition rejected\n");

    /* Test termination */
    assert(pm_set_state(&pm,
                        p1,
                        PROCESS_RUNNING) == 0);

    assert(pm_terminate_process(&pm, p1) == 0);

    assert(pm_process_exists(&pm, p1));
    assert(pm_get_process(&pm, p1)->state == PROCESS_TERMINATED);

    printf("[PASS] Process termination\n");

    /* Test invalid PID */
    assert(pm_get_process(&pm, -1) == NULL);
    assert(pm_get_process(&pm, MAX_PROCESSES) == NULL);

    printf("[PASS] Invalid PID rejected\n");

    /* Display remaining process */
    pm_list_processes(&pm);

    printf("\n========================================\n");
    printf("       ALL PROCESS TESTS PASSED\n");
    printf("========================================\n");

    return 0;
}
