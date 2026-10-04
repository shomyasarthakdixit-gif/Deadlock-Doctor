#include "deadlock_detector.h"
#include "process_manager.h"
#include "recovery.h"
#include "resource_manager.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    ProcessManager pm;
    ResourceManager rm;
    DeadlockReport report;
    RecoveryReport recovery;

    pm_init(&pm);
    rm_init(&rm);

    int p0 = pm_create_process(&pm,
                               "Process-A",
                               0,
                               5,
                               1);

    int p1 = pm_create_process(&pm,
                               "Process-B",
                               0,
                               4,
                               1);

    assert(p0 == 0);
    assert(p1 == 1);

    assert(pm_set_state(&pm,
                        p0,
                        PROCESS_READY) == 0);

    assert(pm_set_state(&pm,
                        p1,
                        PROCESS_READY) == 0);

    assert(rm_register_process(&rm, p0) == 0);
    assert(rm_register_process(&rm, p1) == 0);

    int r0 = rm_create_resource(&rm,
                                "Resource-A",
                                1);

    int r1 = rm_create_resource(&rm,
                                "Resource-B",
                                1);

    assert(r0 == 0);
    assert(r1 == 1);

    /*
     * P0 holds R0.
     * P1 holds R1.
     */
    assert(rm_request_resource(&rm,
                               p0,
                               r0,
                               1) == RESOURCE_ALLOCATED);

    assert(rm_request_resource(&rm,
                               p1,
                               r1,
                               1) == RESOURCE_ALLOCATED);

    /*
     * Both processes become RUNNING before waiting,
     * matching the Process Manager lifecycle rules.
     */
    assert(pm_set_state(&pm,
                        p0,
                        PROCESS_RUNNING) == 0);

    assert(pm_set_state(&pm,
                        p1,
                        PROCESS_RUNNING) == 0);

    /*
     * P0 waits for R1.
     * P1 waits for R0.
     */
    assert(rm_request_resource(&rm,
                               p0,
                               r1,
                               1) == RESOURCE_WAITING);

    assert(pm_set_state(&pm,
                        p0,
                        PROCESS_WAITING) == 0);

    assert(rm_request_resource(&rm,
                               p1,
                               r0,
                               1) == RESOURCE_WAITING);

    assert(pm_set_state(&pm,
                        p1,
                        PROCESS_WAITING) == 0);

    deadlock_detect(&rm, &report);

    assert(report.deadlock_exists == 1);
    assert(report.deadlocked_process_count == 2);

    printf("[PASS] Deadlock detected before recovery\n");

    assert(recovery_execute(&pm,
                            &rm,
                            &report,
                            &recovery) == 0);

    assert(recovery.victim_pid == 0);
    assert(recovery.released_instances == 1);
    assert(recovery.recovered == 1);

    assert(pm_get_process_const(&pm, p0)->state ==
           PROCESS_TERMINATED);

    assert(pm_get_process_const(&pm, p1)->state ==
           PROCESS_READY);

    assert(rm_get_process_allocation(&rm, p1, r0) == 1);
    assert(rm_get_process_allocation(&rm, p1, r1) == 1);

    DeadlockReport after;

    deadlock_detect(&rm, &after);

    assert(after.deadlock_exists == 0);

    printf("[PASS] Recovery released victim resources\n");
    printf("[PASS] Waiting process returned to READY\n");
    printf("[PASS] Deadlock disappeared after recovery\n");

    printf("\nAll recovery tests passed.\n");

    return 0;
}
