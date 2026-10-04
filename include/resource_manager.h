#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#define MAX_RESOURCES 32
#define MAX_PROCESSES 32
#define RESOURCE_NAME_LEN 32

typedef enum {
    RESOURCE_ALLOCATED = 0,
    RESOURCE_WAITING = 1,
    RESOURCE_ERROR = -1
} ResourceResult;

/* Represents one resource type */
typedef struct {
    int id;
    char name[RESOURCE_NAME_LEN];

    int total;
    int available;
} Resource;

/*
 * Resource Manager
 *
 * allocation[pid][rid]
 * = number of instances of resource rid
 *   currently held by process pid.
 *
 * request[pid][rid]
 * = number of instances of resource rid
 *   currently requested by process pid.
 */
typedef struct {
    Resource resources[MAX_RESOURCES];

    int resource_count;

    int allocation[MAX_PROCESSES][MAX_RESOURCES];

    int request[MAX_PROCESSES][MAX_RESOURCES];

    /*
     * Indicates whether a process is registered
     * with the resource manager.
     *
     * Process lifecycle/state management itself
     * belongs to the Process Manager.
     */
    int registered_process[MAX_PROCESSES];

} ResourceManager;


/* Initialize the resource manager */
void rm_init(ResourceManager *rm);


/* Process registration */
int rm_register_process(ResourceManager *rm, int pid);

int rm_unregister_process(ResourceManager *rm, int pid);


/* Resource creation */
int rm_create_resource(ResourceManager *rm,
                       const char *name,
                       int instances);


/* Request a resource */
int rm_request_resource(ResourceManager *rm,
                        int pid,
                        int resource_id,
                        int instances);


/* Release an allocated resource */
int rm_release_resource(ResourceManager *rm,
                        int pid,
                        int resource_id,
                        int instances);


/* Display all resources */
void rm_list_resources(const ResourceManager *rm);


/* Display resources held/requested by a process */
void rm_show_process_resources(const ResourceManager *rm,
                               int pid);


/* Check whether a resource exists */
int rm_resource_exists(const ResourceManager *rm,
                       int resource_id);


/* Get number of resources */
int rm_get_resource_count(const ResourceManager *rm);


/* Get number of resource instances held by a process */
int rm_get_process_allocation(const ResourceManager *rm,
                              int pid,
                              int resource_id);


/* Get number of resource instances requested by a process */
int rm_get_process_request(const ResourceManager *rm,
                           int pid,
                           int resource_id);

#endif /* RESOURCE_MANAGER_H */

