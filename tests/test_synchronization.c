#include "synchronization.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    printf("Running synchronization tests...\n");

    SynchronizationManager sync;

    assert(sync_init(&sync, 2) == 0);

    assert(sync_lock(&sync) == 0);
    assert(sync_unlock(&sync) == 0);

    assert(sync_signal(&sync) == 0);
    assert(sync_broadcast(&sync) == 0);

    assert(sync_semaphore_wait(&sync) == 0);
    assert(sync_semaphore_post(&sync) == 0);

    sync_destroy(&sync);

    printf("[PASS] Mutex operations\n");
    printf("[PASS] Condition variable operations\n");
    printf("[PASS] Counting semaphore operations\n");

    printf("\nRunning synchronization demonstration...\n");

    assert(sync_run_demo() == 0);

    printf("\nAll synchronization tests passed.\n");

    return 0;
}
