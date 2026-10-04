#include "resource_manager.h"

#include <stdio.h>
#include <string.h>

static int valid_pid(int pid)
{
    return pid >= 0 && pid < MAX_PROCESSES;
}

static int valid_resource_index(const ResourceManager *rm, int resource_id)
{
    return resource_id >= 0 &&
           resource_id < rm->resource_count;
}

void rm_init(ResourceManager *rm)
{
    if (rm == NULL)
        return;

    memset(rm, 0, sizeof(*rm));
}

int rm_register_process(ResourceManager *rm, int pid)
{
    if (rm == NULL || !valid_pid(pid))
        return -1;

    if (rm->registered_process[pid])
        return -1;

    rm->registered_process[pid] = 1;

    printf("Process P%d registered.\n", pid);

    return 0;
}

int rm_unregister_process(ResourceManager *rm, int pid)
{
    if (rm == NULL || !valid_pid(pid))
        return -1;

    if (!rm->registered_process[pid])
        return -1;

    for (int r = 0; r < rm->resource_count; r++) {
        if (rm->allocation[pid][r] != 0 ||
            rm->request[pid][r] != 0) {
            return -1;
        }
    }

    rm->registered_process[pid] = 0;

    printf("Process P%d unregistered.\n", pid);

    return 0;
}

int rm_create_resource(ResourceManager *rm,
                       const char *name,
                       int instances)
{
    if (rm == NULL || name == NULL)
        return -1;

    if (instances <= 0)
        return -1;

    if (rm->resource_count >= MAX_RESOURCES)
        return -1;

    for (int i = 0; i < rm->resource_count; i++) {
        if (strcmp(rm->resources[i].name, name) == 0)
            return -1;
    }

    int id = rm->resource_count;

    rm->resources[id].id = id;

    strncpy(rm->resources[id].name,
            name,
            RESOURCE_NAME_LEN - 1);

    rm->resources[id].name[RESOURCE_NAME_LEN - 1] = '\0';

    rm->resources[id].total = instances;
    rm->resources[id].available = instances;

    rm->resource_count++;

    printf("Resource R%d (%s) created: %d instance(s).\n",
           id,
           rm->resources[id].name,
           instances);

    return id;
}

int rm_request_resource(ResourceManager *rm,
                        int pid,
                        int resource_id,
                        int instances)
{
    if (rm == NULL)
        return RESOURCE_ERROR;

    if (!valid_pid(pid))
        return RESOURCE_ERROR;

    if (!valid_resource_index(rm, resource_id))
        return RESOURCE_ERROR;

    if (!rm->registered_process[pid])
        return RESOURCE_ERROR;

    if (instances <= 0)
        return RESOURCE_ERROR;

    Resource *resource = &rm->resources[resource_id];

    if (resource->available >= instances) {

        resource->available -= instances;

        rm->allocation[pid][resource_id] += instances;

        rm->request[pid][resource_id] = 0;

        printf("P%d allocated %d instance(s) of R%d (%s).\n",
               pid,
               instances,
               resource_id,
               resource->name);

        return RESOURCE_ALLOCATED;
    }

    rm->request[pid][resource_id] += instances;

    printf("P%d must WAIT for %d instance(s) of R%d (%s).\n",
           pid,
           instances,
           resource_id,
           resource->name);

    return RESOURCE_WAITING;
}

int rm_release_resource(ResourceManager *rm,
                        int pid,
                        int resource_id,
                        int instances)
{
    if (rm == NULL)
        return RESOURCE_ERROR;

    if (!valid_pid(pid))
        return RESOURCE_ERROR;

    if (!valid_resource_index(rm, resource_id))
        return RESOURCE_ERROR;

    if (!rm->registered_process[pid])
        return RESOURCE_ERROR;

    if (instances <= 0)
        return RESOURCE_ERROR;

    if (rm->allocation[pid][resource_id] < instances)
        return RESOURCE_ERROR;

    rm->allocation[pid][resource_id] -= instances;

    rm->resources[resource_id].available += instances;

    printf("P%d released %d instance(s) of R%d (%s).\n",
           pid,
           instances,
           resource_id,
           rm->resources[resource_id].name);

    return 0;
}

void rm_list_resources(const ResourceManager *rm)
{
    if (rm == NULL)
        return;

    printf("\n========== RESOURCE TABLE ==========\n");

    if (rm->resource_count == 0) {
        printf("No resources created.\n");
        return;
    }

    printf("%-5s %-20s %-10s %-10s\n",
           "ID",
           "NAME",
           "TOTAL",
           "AVAILABLE");

    printf("-----------------------------------------------\n");

    for (int i = 0; i < rm->resource_count; i++) {
        printf("R%-4d %-20s %-10d %-10d\n",
               rm->resources[i].id,
               rm->resources[i].name,
               rm->resources[i].total,
               rm->resources[i].available);
    }

    printf("-----------------------------------------------\n");
}

void rm_show_process_resources(const ResourceManager *rm, int pid)
{
    if (rm == NULL || !valid_pid(pid))
        return;

    if (!rm->registered_process[pid]) {
        printf("P%d is not registered.\n", pid);
        return;
    }

    printf("\n========== PROCESS P%d ==========\n", pid);

    printf("Held resources:\n");

    int holding_any = 0;

    for (int r = 0; r < rm->resource_count; r++) {
        if (rm->allocation[pid][r] > 0) {
            printf("  R%d (%s): %d instance(s)\n",
                   r,
                   rm->resources[r].name,
                   rm->allocation[pid][r]);

            holding_any = 1;
        }
    }

    if (!holding_any)
        printf("  None\n");

    printf("Waiting requests:\n");

    int waiting_any = 0;

    for (int r = 0; r < rm->resource_count; r++) {
        if (rm->request[pid][r] > 0) {
            printf("  R%d (%s): %d instance(s)\n",
                   r,
                   rm->resources[r].name,
                   rm->request[pid][r]);

            waiting_any = 1;
        }
    }

    if (!waiting_any)
        printf("  None\n");
}

int rm_resource_exists(const ResourceManager *rm, int resource_id)
{
    if (rm == NULL)
        return 0;

    return valid_resource_index(rm, resource_id);
}

int rm_get_resource_count(const ResourceManager *rm)
{
    if (rm == NULL)
        return 0;

    return rm->resource_count;
}

int rm_get_process_allocation(const ResourceManager *rm,
                              int pid,
                              int resource_id)
{
    if (rm == NULL)
        return -1;

    if (!valid_pid(pid))
        return -1;

    if (!valid_resource_index(rm, resource_id))
        return -1;

    return rm->allocation[pid][resource_id];
}

int rm_get_process_request(const ResourceManager *rm,
                           int pid,
                           int resource_id)
{
    if (rm == NULL)
        return -1;

    if (!valid_pid(pid))
        return -1;

    if (!valid_resource_index(rm, resource_id))
        return -1;

    return rm->request[pid][resource_id];
}
