#include <stdio.h>

#include "process_manager.h"
#include "scheduler.h"
#include "resource_manager.h"
#include "deadlock_detector.h"

int main(void)
{
    printf("========================================\n");
    printf("           DEADLOCK DOCTOR\n");
    printf("     Process & Resource Management\n");
    printf("========================================\n");

    ProcessManager pm;
    ResourceManager rm;
    DeadlockReport report;
    Scheduler scheduler;

    /* Initialize managers */
    pm_init(&pm);
    rm_init(&rm);

    /*
     * --------------------------------------------------
     * PROCESS MANAGEMENT
     * --------------------------------------------------
     */

    printf("\n--- Process Creation ---\n");

    int p1 = pm_create_process(&pm,
                               "Process-A",
                               0,
                               5,
                               1);

    int p2 = pm_create_process(&pm,
                               "Process-B",
                               0,
                               4,
                               2);

    if (p1 < 0 || p2 < 0) {
        printf("Error creating processes.\n");
        return 1;
    }

    /* Register the same PIDs with Resource Manager */
    rm_register_process(&rm, p1);
    rm_register_process(&rm, p2);

    /* NEW -> READY */
    pm_set_state(&pm, p1, PROCESS_READY);
    pm_set_state(&pm, p2, PROCESS_READY);

    pm_list_processes(&pm);

    /*
     * --------------------------------------------------
     * RESOURCE CREATION
     * --------------------------------------------------
     */

    printf("\n--- Resource Creation ---\n");

    int r1 = rm_create_resource(&rm,
                                "Resource-A",
                                1);

    int r2 = rm_create_resource(&rm,
                                "Resource-B",
                                1);

    if (r1 < 0 || r2 < 0) {
        printf("Error creating resources.\n");
        return 1;
    }

    /*
     * --------------------------------------------------
     * PROCESS EXECUTION
     * --------------------------------------------------
     */

    printf("\n--- Process State Transition ---\n");

    pm_set_state(&pm, p1, PROCESS_RUNNING);

    printf("Process P%d is now RUNNING.\n", p1);

    /*
     * --------------------------------------------------
     * RESOURCE ALLOCATION
     * --------------------------------------------------
     */

    printf("\n--- Resource Allocation ---\n");

    /* P1 gets Resource-A */
    rm_request_resource(&rm, p1, r1, 1);

    /* P2 gets Resource-B */
    rm_request_resource(&rm, p2, r2, 1);

    /*
     * --------------------------------------------------
     * DEADLOCK SCENARIO
     * --------------------------------------------------
     */

    printf("\n--- Resource Requests ---\n");

    /*
     * P1 is waiting for Resource-B.
     * Move P1 to WAITING to reflect the resource state.
     */
    if (pm_set_state(&pm,
                     p1,
                     PROCESS_WAITING) == 0) {

        rm_request_resource(&rm, p1, r2, 1);
    }

    /*
     * P2 is waiting for Resource-A.
     */
        if (pm_set_state(&pm,
                     p2,
                     PROCESS_RUNNING) == 0) {

        if (pm_set_state(&pm,
                         p2,
                         PROCESS_WAITING) == 0) {

            rm_request_resource(&rm, p2, r1, 1);
        }
    }

    /*
     * --------------------------------------------------
     * CURRENT SYSTEM STATE
     * --------------------------------------------------
     */

    printf("\n--- Current Process State ---\n");

    pm_list_processes(&pm);

    printf("\n--- Current Resource State ---\n");

    rm_list_resources(&rm);

    rm_show_process_resources(&rm, p1);
    rm_show_process_resources(&rm, p2);

    /*
     * --------------------------------------------------
     * DEADLOCK DETECTION
     * --------------------------------------------------
     */

    printf("\n--- Deadlock Detection ---\n");

    deadlock_detect(&rm, &report);
    deadlock_print_report(&rm, &report);

    /*
     * --------------------------------------------------
     * SCHEDULER DEMONSTRATION
     * --------------------------------------------------
     *
     * The current processes are WAITING because of the
     * deliberate deadlock scenario, so the scheduler
     * will correctly skip them.
     */

    printf("\n--- Scheduler Check ---\n");

    scheduler_init(&scheduler,
                   SCHEDULER_FCFS,
                   0);

    int scheduled =
        scheduler_run(&scheduler, &pm);

    printf("Scheduler completed %d process(es).\n",
           scheduled);

    return 0;
}
