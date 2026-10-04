#include "deadlock_detector.h"

#include <stdio.h>
#include <string.h>

static int request_can_be_satisfied(
    const ResourceManager *rm,
    int pid,
    const int *work)
{
    for (int r = 0; r < rm->resource_count; r++) {
        if (rm->request[pid][r] > work[r])
            return 0;
    }

    return 1;
}

void deadlock_detect(const ResourceManager *rm,
                     DeadlockReport *report)
{
    if (rm == NULL || report == NULL)
        return;

    memset(report, 0, sizeof(*report));

    int work[MAX_RESOURCES];
    int finish[MAX_PROCESSES];

    /* Initially, work = currently available resources */
    for (int r = 0; r < rm->resource_count; r++) {
        work[r] = rm->resources[r].available;
    }

    /*
     * A process with no resources allocated cannot be
     * part of a deadlock.
     */
    for (int p = 0; p < MAX_PROCESSES; p++) {

        if (!rm->registered_process[p]) {
            finish[p] = 1;
            continue;
        }

        int has_allocation = 0;

        for (int r = 0; r < rm->resource_count; r++) {
            if (rm->allocation[p][r] > 0) {
                has_allocation = 1;
                break;
            }
        }

        finish[p] = !has_allocation;
    }

    /*
     * Multiple-instance deadlock detection algorithm.
     *
     * If a process's outstanding requests can be satisfied,
     * assume it finishes and releases its allocated resources.
     */
    int changed;

    do {
        changed = 0;

        for (int p = 0; p < MAX_PROCESSES; p++) {

            if (finish[p])
                continue;

            if (!request_can_be_satisfied(rm, p, work))
                continue;

            for (int r = 0; r < rm->resource_count; r++) {
                work[r] += rm->allocation[p][r];
            }

            finish[p] = 1;
            changed = 1;
        }

    } while (changed);

    /*
     * Any process that could not finish is deadlocked.
     */
    for (int p = 0; p < MAX_PROCESSES; p++) {

        if (finish[p])
            continue;

        report->deadlock_exists = 1;

        report->deadlocked_processes[
            report->deadlocked_process_count++
        ] = p;
    }

    /*
     * Identify resources held or requested by
     * the deadlocked processes.
     */
    if (report->deadlock_exists) {

        for (int r = 0; r < rm->resource_count; r++) {

            int involved = 0;

            for (int i = 0;
                 i < report->deadlocked_process_count;
                 i++) {

                int p = report->deadlocked_processes[i];

                if (rm->allocation[p][r] > 0 ||
                    rm->request[p][r] > 0) {

                    involved = 1;
                    break;
                }
            }

            if (involved) {
                report->involved_resources[
                    report->involved_resource_count++
                ] = r;
            }
        }
    }
}

void deadlock_print_report(const ResourceManager *rm,
                           const DeadlockReport *report)
{
    if (rm == NULL || report == NULL)
        return;

    printf("\n========================================\n");
    printf("          DEADLOCK ANALYSIS\n");
    printf("========================================\n");

    if (!report->deadlock_exists) {
        printf("STATUS: NO DEADLOCK DETECTED\n");
        printf("The current resource state is recoverable.\n");
        printf("========================================\n");
        return;
    }

    printf("STATUS: DEADLOCK DETECTED\n\n");

    printf("Deadlocked processes:\n");

    for (int i = 0;
         i < report->deadlocked_process_count;
         i++) {

        int p = report->deadlocked_processes[i];

        printf("\nP%d:\n", p);

        printf("  Holding:\n");

        int holding = 0;

        for (int r = 0; r < rm->resource_count; r++) {

            if (rm->allocation[p][r] > 0) {

                printf("    R%d (%s): %d instance(s)\n",
                       r,
                       rm->resources[r].name,
                       rm->allocation[p][r]);

                holding = 1;
            }
        }

        if (!holding)
            printf("    None\n");

        printf("  Waiting for:\n");

        int waiting = 0;

        for (int r = 0; r < rm->resource_count; r++) {

            if (rm->request[p][r] > 0) {

                printf("    R%d (%s): %d instance(s)\n",
                       r,
                       rm->resources[r].name,
                       rm->request[p][r]);

                waiting = 1;
            }
        }

        if (!waiting)
            printf("    None\n");
    }

    printf("\nResources involved:\n");

    for (int i = 0;
         i < report->involved_resource_count;
         i++) {

        int r = report->involved_resources[i];

        printf("  R%d (%s)\n",
               r,
               rm->resources[r].name);
    }

    printf("\nDiagnosis:\n");
    printf("The listed processes cannot complete because\n");
    printf("their outstanding resource requests cannot be\n");
    printf("satisfied with the resources currently available.\n");

    printf("========================================\n");
}

