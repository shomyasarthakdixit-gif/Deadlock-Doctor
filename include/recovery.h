#ifndef RECOVERY_H
#define RECOVERY_H

#include "deadlock_detector.h"
#include "process_manager.h"
#include "resource_manager.h"

typedef struct {
    int victim_pid;

    int released_resource_types;
    int released_instances;

    int awakened_processes;

    int recovered;
} RecoveryReport;

/* Select a deterministic victim from the deadlocked processes. */
int recovery_select_victim(const ProcessManager *pm,
                           const ResourceManager *rm,
                           const DeadlockReport *report);

/*
 * Try to satisfy currently pending requests of WAITING processes.
 * Processes whose outstanding requests are completely satisfied
 * are moved from WAITING to READY.
 */
int recovery_wake_waiting_processes(ProcessManager *pm,
                                     ResourceManager *rm);

/* Perform deadlock recovery and verify the resulting state. */
int recovery_execute(ProcessManager *pm,
                     ResourceManager *rm,
                     const DeadlockReport *report,
                     RecoveryReport *recovery_report);

/* Display recovery results. */
void recovery_print_report(const ResourceManager *rm,
                           const RecoveryReport *report);

#endif /* RECOVERY_H */
