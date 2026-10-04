#include "cli.h"

#include "deadlock_detector.h"
#include "process_manager.h"
#include "recovery.h"
#include "resource_manager.h"
#include "scheduler.h"
#include "synchronization.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_BUFFER_SIZE 128

static int read_integer(const char *prompt,
                        int *value)
{
    char buffer[INPUT_BUFFER_SIZE];

    if (prompt == NULL || value == NULL) {
        return -1;
    }

    for (;;) {
        printf("%s", prompt);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            return -1;
        }

        char *end = NULL;

        errno = 0;

        long parsed =
            strtol(buffer, &end, 10);

        while (end != NULL && isspace((unsigned char) *end)) {
            end++;
        }

        if (errno == ERANGE ||
            end == buffer ||
            (end != NULL && *end != '\0')) {

            printf("Invalid integer. Please try again.\n");
            continue;
        }

        if (parsed < INT_MIN ||
            parsed > INT_MAX) {

            printf("Integer is out of range.\n");
            continue;
        }

        *value = (int) parsed;
        return 0;
    }
}

static int read_positive_integer(const char *prompt,
                                 int *value)
{
    for (;;) {
        if (read_integer(prompt, value) != 0) {
            return -1;
        }

        if (*value > 0) {
            return 0;
        }

        printf("Value must be greater than zero.\n");
    }
}

static int read_non_negative_integer(const char *prompt,
                                     int *value)
{
    for (;;) {
        if (read_integer(prompt, value) != 0) {
            return -1;
        }

        if (*value >= 0) {
            return 0;
        }

        printf("Value cannot be negative.\n");
    }
}

static int read_text(const char *prompt,
                     char *buffer,
                     size_t size)
{
    if (prompt == NULL ||
        buffer == NULL ||
        size < 2) {
        return -1;
    }

    for (;;) {
        printf("%s", prompt);

        if (fgets(buffer, size, stdin) == NULL) {
            return -1;
        }

        buffer[strcspn(buffer, "\n")] = '\0';

        if (buffer[0] == '\0') {
            printf("Input cannot be empty.\n");
            continue;
        }

        return 0;
    }
}

static void print_banner(void)
{
    printf("\n");
    printf("========================================\n");
    printf("           DEADLOCK DOCTOR\n");
    printf("      A Resource & Deadlock Manager\n");
    printf("========================================\n");
}

static void print_menu(void)
{
    printf("\n----------------------------------------\n");
    printf("                MAIN MENU\n");
    printf("----------------------------------------\n");
    printf("1.  Create Process\n");
    printf("2.  List Processes\n");
    printf("3.  Create Resource\n");
    printf("4.  Request Resource\n");
    printf("5.  Release Resource\n");
    printf("6.  Run FCFS Scheduling\n");
    printf("7.  Run Round Robin Scheduling\n");
    printf("8.  Show System State\n");
    printf("9.  Detect Deadlock\n");
    printf("10. Recover Deadlock\n");
    printf("11. Run Synchronization Demo\n");
    printf("0.  Exit\n");
    printf("----------------------------------------\n");
}

static void handle_create_process(ProcessManager *pm,
                                   ResourceManager *rm)
{
    char name[PROCESS_NAME_LEN];
    int arrival_time;
    int burst_time;
    int priority;

    if (read_text("Process name: ",
                  name,
                  sizeof(name)) != 0) {
        return;
    }

    if (read_non_negative_integer(
            "Arrival time: ",
            &arrival_time) != 0) {
        return;
    }

    if (read_positive_integer(
            "Burst time: ",
            &burst_time) != 0) {
        return;
    }

    if (read_integer("Priority: ", &priority) != 0) {
        return;
    }

    int pid = pm_create_process(pm,
                                name,
                                arrival_time,
                                burst_time,
                                priority);

    if (pid < 0) {
        printf("Failed to create process.\n");
        return;
    }

    if (rm_register_process(rm, pid) != 0) {
        printf("Warning: failed to register P%d with Resource Manager.\n",
               pid);

        pm_terminate_process(pm, pid);
        return;
    }

    if (pm_set_state(pm,
                     pid,
                     PROCESS_READY) != 0) {
        printf("Warning: P%d could not enter READY state.\n",
               pid);
        return;
    }

    printf("P%d is ready for execution.\n", pid);
}

static void handle_list_processes(const ProcessManager *pm)
{
    pm_list_processes(pm);
}

static void handle_create_resource(ResourceManager *rm)
{
    char name[RESOURCE_NAME_LEN];
    int instances;

    if (read_text("Resource name: ",
                  name,
                  sizeof(name)) != 0) {
        return;
    }

    if (read_positive_integer(
            "Number of instances: ",
            &instances) != 0) {
        return;
    }

    int resource_id =
        rm_create_resource(rm,
                           name,
                           instances);

    if (resource_id < 0) {
        printf("Failed to create resource.\n");
        return;
    }

    printf("R%d is now available.\n",
           resource_id);
}

static int process_has_pending_requests(
    const ResourceManager *rm,
    int pid)
{
    for (int r = 0;
         r < rm->resource_count;
         ++r) {

        if (rm_get_process_request(rm,
                                   pid,
                                   r) > 0) {
            return 1;
        }
    }

    return 0;
}

static void handle_request_resource(
    ProcessManager *pm,
    ResourceManager *rm)
{
    int pid;
    int resource_id;
    int instances;

    if (read_non_negative_integer(
            "Process PID: ",
            &pid) != 0) {
        return;
    }

    if (!pm_process_exists(pm, pid)) {
        printf("Process P%d does not exist.\n", pid);
        return;
    }

    Process *process =
        pm_get_process(pm, pid);

    if (process == NULL ||
        process->state == PROCESS_TERMINATED) {
        printf("P%d is terminated and cannot request resources.\n",
               pid);
        return;
    }

    if (read_non_negative_integer(
            "Resource ID: ",
            &resource_id) != 0) {
        return;
    }

    if (!rm_resource_exists(rm, resource_id)) {
        printf("Resource R%d does not exist.\n",
               resource_id);
        return;
    }

    if (read_positive_integer(
            "Number of instances: ",
            &instances) != 0) {
        return;
    }

    ProcessState original_state =
        process->state;

    /*
     * If the process is READY, it must enter RUNNING before
     * it can become WAITING for a resource.
     */
    if (original_state == PROCESS_READY) {
        if (pm_set_state(pm,
                         pid,
                         PROCESS_RUNNING) != 0) {
            printf("Unable to move P%d to RUNNING.\n",
                   pid);
            return;
        }
    }

    ResourceResult result =
        rm_request_resource(rm,
                            pid,
                            resource_id,
                            instances);

    if (result == RESOURCE_ALLOCATED) {

        /*
         * If this request completes all pending requests and the
         * process was waiting, it can become READY again.
         */
        if (process->state == PROCESS_WAITING &&
            !process_has_pending_requests(rm, pid)) {

            if (pm_set_state(pm,
                             pid,
                             PROCESS_READY) != 0) {
                printf("Warning: P%d could not return to READY.\n",
                       pid);
            }
        } else if (original_state == PROCESS_READY) {

            /*
             * This was a normal READY process that immediately
             * received its resource. Put it back into READY so
             * the scheduler can select it later.
             */
            pm_set_state(pm,
                         pid,
                         PROCESS_READY);
        }

    } else if (result == RESOURCE_WAITING) {

        if (process->state == PROCESS_RUNNING ||
            process->state == PROCESS_READY) {

            if (process->state == PROCESS_READY) {
                if (pm_set_state(pm,
                                 pid,
                                 PROCESS_RUNNING) != 0) {
                    printf("Failed to move P%d to RUNNING.\n",
                           pid);
                    return;
                }
            }

            if (pm_set_state(pm,
                             pid,
                             PROCESS_WAITING) != 0) {
                printf("Failed to move P%d to WAITING.\n",
                       pid);
            }
        }
    } else {
        printf("Resource request failed.\n");
    }
}

static void handle_release_resource(
    ProcessManager *pm,
    ResourceManager *rm)
{
    int pid;
    int resource_id;
    int instances;

    if (read_non_negative_integer(
            "Process PID: ",
            &pid) != 0) {
        return;
    }

    if (!pm_process_exists(pm, pid)) {
        printf("Process P%d does not exist.\n",
               pid);
        return;
    }

    if (read_non_negative_integer(
            "Resource ID: ",
            &resource_id) != 0) {
        return;
    }

    if (!rm_resource_exists(rm, resource_id)) {
        printf("Resource R%d does not exist.\n",
               resource_id);
        return;
    }

    if (read_positive_integer(
            "Number of instances: ",
            &instances) != 0) {
        return;
    }

    if (rm_release_resource(rm,
                            pid,
                            resource_id,
                            instances) != 0) {
        printf("Resource release failed.\n");
        return;
    }

    /*
     * A release may make a waiting request satisfiable.
     */
    int awakened =
        recovery_wake_waiting_processes(pm, rm);

    if (awakened > 0) {
        printf("%d waiting process(es) moved to READY.\n",
               awakened);
    }
}

static void handle_fcfs(ProcessManager *pm)
{
    Scheduler scheduler;

    scheduler_init(&scheduler,
                   SCHEDULER_FCFS,
                   0);

    int completed =
        scheduler_run(&scheduler, pm);

    if (completed < 0) {
        printf("FCFS scheduling failed.\n");
        return;
    }

    scheduler_print_results(pm);

    printf("\nFCFS completed %d process(es).\n",
           completed);
}

static void handle_round_robin(ProcessManager *pm)
{
    int quantum;

    if (read_positive_integer(
            "Time quantum: ",
            &quantum) != 0) {
        return;
    }

    Scheduler scheduler;

    scheduler_init(&scheduler,
                   SCHEDULER_ROUND_ROBIN,
                   quantum);

    int completed =
        scheduler_run(&scheduler, pm);

    if (completed < 0) {
        printf("Round Robin scheduling failed.\n");
        return;
    }

    scheduler_print_results(pm);

    printf("\nRound Robin completed %d process(es).\n",
           completed);
}

static void handle_show_state(
    const ProcessManager *pm,
    const ResourceManager *rm)
{
    printf("\n========================================\n");
    printf("             SYSTEM STATE\n");
    printf("========================================\n");

    pm_list_processes(pm);

    rm_list_resources(rm);

    for (int pid = 0;
         pid < MAX_PROCESSES;
         ++pid) {

        if (rm->registered_process[pid]) {
            rm_show_process_resources(rm, pid);
        }
    }

    printf("========================================\n");
}

static void handle_detect_deadlock(
    const ResourceManager *rm)
{
    DeadlockReport report;

    deadlock_detect(rm, &report);
    deadlock_print_report(rm, &report);
}

static void handle_recover_deadlock(
    ProcessManager *pm,
    ResourceManager *rm)
{
    DeadlockReport report;

    deadlock_detect(rm, &report);

    if (!report.deadlock_exists) {
        printf("\nNo deadlock exists. Recovery is not required.\n");
        return;
    }

    deadlock_print_report(rm, &report);

    RecoveryReport recovery;

    if (recovery_execute(pm,
                         rm,
                         &report,
                         &recovery) != 0) {
        printf("Deadlock recovery failed.\n");
        return;
    }

    recovery_print_report(rm, &recovery);

    printf("\nSystem state after recovery:\n");

    pm_list_processes(pm);
    rm_list_resources(rm);

    DeadlockReport after;

    deadlock_detect(rm, &after);

    if (!after.deadlock_exists) {
        printf("\nFinal status: NO DEADLOCK DETECTED\n");
    } else {
        printf("\nFinal status: DEADLOCK STILL EXISTS\n");
        deadlock_print_report(rm, &after);
    }
}

static void handle_synchronization_demo(void)
{
    if (sync_run_demo() != 0) {
        printf("Synchronization demonstration failed.\n");
        return;
    }

    printf("Synchronization demonstration completed successfully.\n");
}

int cli_run(void)
{
    ProcessManager pm;
    ResourceManager rm;

    pm_init(&pm);
    rm_init(&rm);

    print_banner();

    for (;;) {
        print_menu();

        int choice;

        if (read_integer("Enter choice: ",
                         &choice) != 0) {

            printf("\nInput stream closed. Exiting.\n");
            break;
        }

        switch (choice) {
            case 1:
                handle_create_process(&pm, &rm);
                break;

            case 2:
                handle_list_processes(&pm);
                break;

            case 3:
                handle_create_resource(&rm);
                break;

            case 4:
                handle_request_resource(&pm, &rm);
                break;

            case 5:
                handle_release_resource(&pm, &rm);
                break;

            case 6:
                handle_fcfs(&pm);
                break;

            case 7:
                handle_round_robin(&pm);
                break;

            case 8:
                handle_show_state(&pm, &rm);
                break;

            case 9:
                handle_detect_deadlock(&rm);
                break;

            case 10:
                handle_recover_deadlock(&pm, &rm);
                break;

            case 11:
                handle_synchronization_demo();
                break;

            case 0:
                printf("\nExiting DeadlockDoctor.\n");
                return 0;

            default:
                printf("Invalid menu choice.\n");
                break;
        }
    }

    return 0;
}
