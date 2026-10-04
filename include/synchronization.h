#ifndef SYNCHRONIZATION_H
#define SYNCHRONIZATION_H

#include <pthread.h>
#include <semaphore.h>

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    sem_t semaphore;

    int initialized;
    int semaphore_initialized;
} SynchronizationManager;

/* Initialize synchronization primitives. */
int sync_init(SynchronizationManager *sync,
              unsigned int semaphore_count);

/* Destroy synchronization primitives. */
void sync_destroy(SynchronizationManager *sync);

/* Mutex operations. */
int sync_lock(SynchronizationManager *sync);
int sync_unlock(SynchronizationManager *sync);

/*
 * Condition-variable operations.
 *
 * sync_wait() must be called while the mutex is locked.
 * The caller should normally use it inside a while-loop
 * that checks its own condition/predicate.
 */
int sync_wait(SynchronizationManager *sync);
int sync_signal(SynchronizationManager *sync);
int sync_broadcast(SynchronizationManager *sync);

/* Counting semaphore operations. */
int sync_semaphore_wait(SynchronizationManager *sync);
int sync_semaphore_post(SynchronizationManager *sync);

/* Demonstrate mutex, condition variable and semaphore usage. */
int sync_run_demo(void);

#endif /* SYNCHRONIZATION_H */
