#include "recovery.h"

#include <stdio.h>
#include <string.h>

static int process_allocation_total(const ResourceManager *rm,
                                    int pid)
{
    int total = 0;

    for (int r = 0; r < rm->resource_count; ++r) {
        total += rm->allocation[pid][r];
    }

    return total;
}

static int process_has_pending_requests(const ResourceManager *rm,
                                        int pid)
{
    for (int r = 0; r < rm->resource_count; ++r) {
        if (rm->request[pid][r] > 0) {
            return 1;
        }
    }

    return 0;
}

static int wake_waiting_processes(ProcessManager *pm,
                                  ResourceManager *rm,
                                  int victim_pid)
{
    int awakened = 0;

    for (int pid = 0; pid < MAX_PROCESSES; ++pid) {
        Process *process = pm_get_process(pm, pid);

        if (process == NULL ||
            pid == victim_pid ||
            process->state != PROCESS_WAITING) {
            continue;
        }

        /*
         * Only submit a recorded request when enough instances
         * are currently available. This avoids adding the same
         * waiting request twice inside rm_request_resource().
         */
        for (int r = 0; r < rm->resource_count; ++r) {
            int requested = rm->request[pid][r];

            if (requested <= 0) {
                continue;
            }

            if (rm->resources[r].available < requested) {
                continue;
            }

            rm_request_resource(rm,
                                pid,
                                r,
                                requested);
        }

        if (!process_has_pending_requests(rm, pid)) {
            if (pm_set_state(pm,
                             pid,
                             PROCESS_READY) == 0) {
                awakened++;
            }
        }
    }

    return awakened;
}

int recovery_select_victim(const ProcessManager *pm,
                           const ResourceManager *rm,
                           const DeadlockReport *report)
{
    if (pm == NULL || rm == NULL || report == NULL ||
        !report->deadlock_exists ||
        report->deadlocked_process_count <= 0) {
        return -1;
    }

    int best_pid = -1;
    int best_allocation = -1;

    for (int i = 0;
         i < report->deadlocked_process_count;
         ++i) {

        int pid = report->deadlocked_processes[i];

        if (!pm_process_exists(pm, pid)) {
            continue;
        }

        int allocation =
            process_allocation_total(rm, pid);

        /*
         * Policy:
         * 1. Select the process holding the largest number
         *    of resource instances.
         * 2. If tied, select the lowest PID.
         */
        if (allocation > best_allocation ||
            (allocation == best_allocation &&
             (best_pid < 0 || pid < best_pid))) {

            best_pid = pid;
            best_allocation = allocation;
        }
    }

    return best_pid;
}

int recovery_execute(ProcessManager *pm,
                     ResourceManager *rm,
                     const DeadlockReport *report,
                     RecoveryReport *recovery_report)
{
    if (pm == NULL || rm == NULL ||
        report == NULL ||
        recovery_report == NULL) {
        return -1;
    }

    memset(recovery_report,
           0,
           sizeof(*recovery_report));

    recovery_report->victim_pid = -1;

    if (!report->deadlock_exists) {
        return -1;
    }

    int victim =
        recovery_select_victim(pm, rm, report);

    if (victim < 0) {
        return -1;
    }

    recovery_report->victim_pid = victim;

    /*
     * Cancel the victim's outstanding requests first.
     * The process is going to be terminated.
     */
    for (int r = 0; r < rm->resource_count; ++r) {
        rm->request[victim][r] = 0;
    }

    /*
     * Release everything currently held by the victim.
     */
    for (int r = 0; r < rm->resource_count; ++r) {
        int held = rm->allocation[victim][r];

        if (held <= 0) {
            continue;
        }

        if (rm_release_resource(rm,
                                victim,
                                r,
                                held) == 0) {

            recovery_report->released_instances += held;
            recovery_report->released_resource_types++;
        }
    }

    /*
     * Terminate the selected victim.
     */
    Process *victim_process =
        pm_get_process(pm, victim);

    if (victim_process != NULL) {
        victim_process->remaining_time = 0;
    }

    if (pm_terminate_process(pm, victim) != 0) {
        return -1;
    }

    /*
     * Resources released by the victim may satisfy waiting
     * processes.
     */
    recovery_report->awakened_processes =
        wake_waiting_processes(pm, rm, victim);

    /*
     * Verify whether the deadlock has actually disappeared.
     */
    DeadlockReport after_recovery;

    deadlock_detect(rm, &after_recovery);

    recovery_report->recovered =
        !after_recovery.deadlock_exists;

    return 0;
}

void recovery_print_report(const ResourceManager *rm,
                           const RecoveryReport *report)
{
    (void) rm;

    if (report == NULL) {
        return;
    }

    printf("\n========================================\n");
    printf("          DEADLOCK RECOVERY\n");
    printf("========================================\n");

    if (report->victim_pid < 0) {
        printf("No recovery action was performed.\n");
        printf("========================================\n");
        return;
    }

    printf("Selected victim       : P%d\n",
           report->victim_pid);

    printf("Resource types freed   : %d\n",
           report->released_resource_types);

    printf("Resource instances freed: %d\n",
           report->released_instances);

    printf("Processes awakened     : %d\n",
           report->awakened_processes);

    printf("\nRecovery result        : %s\n",
           report->recovered
               ? "SYSTEM RECOVERED"
               : "DEADLOCK STILL EXISTS");

    printf("========================================\n");
}
