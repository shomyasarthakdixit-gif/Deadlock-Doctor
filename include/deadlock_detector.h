#ifndef DEADLOCK_DETECTOR_H
#define DEADLOCK_DETECTOR_H

#include "resource_manager.h"

typedef struct {
    int deadlock_exists;

    int deadlocked_processes[MAX_PROCESSES];
    int deadlocked_process_count;

    int involved_resources[MAX_RESOURCES];
    int involved_resource_count;
} DeadlockReport;

void deadlock_detect(const ResourceManager *rm,
                     DeadlockReport *report);

void deadlock_print_report(const ResourceManager *rm,
                           const DeadlockReport *report);

#endif

