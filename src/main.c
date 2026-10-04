#include <stdio.h>

#include "resource_manager.h"
#include "deadlock_detector.h"

int main(void)
{
    printf("========================================\n");
    printf("           DEADLOCK DOCTOR\n");
    printf("     A Resource & Deadlock Manager\n");
    printf("========================================\n\n");

    ResourceManager rm;
    DeadlockReport report;

    /* Initialize Resource Manager */
    rm_init(&rm);

    /* Register processes */
    rm_register_process(&rm, 1);
    rm_register_process(&rm, 2);

    /* Create two resources with one instance each */
    int r1 = rm_create_resource(&rm, "Resource-A", 1);
    int r2 = rm_create_resource(&rm, "Resource-B", 1);

    if (r1 < 0 || r2 < 0) {
        printf("Error creating resources.\n");
        return 1;
    }

    printf("\n--- Resource Allocation ---\n");

    /* P1 gets Resource-A */
    rm_request_resource(&rm, 1, r1, 1);

    /* P2 gets Resource-B */
    rm_request_resource(&rm, 2, r2, 1);

    printf("\n--- Resource Requests ---\n");

    /* P1 waits for Resource-B */
    rm_request_resource(&rm, 1, r2, 1);

    /* P2 waits for Resource-A */
    rm_request_resource(&rm, 2, r1, 1);

    /* Display current resource state */
    rm_list_resources(&rm);

    /* Show process resource information */
    rm_show_process_resources(&rm, 1);
    rm_show_process_resources(&rm, 2);

    /* Detect deadlock */
    printf("\n--- Deadlock Detection ---\n");

    deadlock_detect(&rm, &report);
    deadlock_print_report(&rm, &report);

    return 0;
}
