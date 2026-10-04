#include "synchronization.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#define DEMO_THREAD_COUNT 4
#define DEMO_COUNTER_ITERATIONS 10000

typedef struct {
    SynchronizationManager *sync;
    long counter;
} CounterContext;

typedef struct {
    SynchronizationManager *sync;
    int started;
    int ready;
    int awakened;
} ConditionContext;

typedef struct {
    SynchronizationManager *sync;
    int active;
    int max_active;
} SemaphoreContext;

static void sleep_milliseconds(long milliseconds)
{
    struct timespec request;

    request.tv_sec = milliseconds / 1000;
    request.tv_nsec = (milliseconds % 1000) * 1000000L;

    nanosleep(&request, NULL);
}

static void *counter_worker(void *arg)
{
    CounterContext *context = arg;

    for (int i = 0; i < DEMO_COUNTER_ITERATIONS; ++i) {
        if (sync_lock(context->sync) != 0) {
            return NULL;
        }

        context->counter++;

        sync_unlock(context->sync);
    }

    return NULL;
}

static void *condition_worker(void *arg)
{
    ConditionContext *context = arg;

    if (sync_lock(context->sync) != 0) {
        return NULL;
    }

    context->started = 1;
    sync_broadcast(context->sync);

    while (!context->ready) {
        if (sync_wait(context->sync) != 0) {
            sync_unlock(context->sync);
            return NULL;
        }
    }

    context->awakened = 1;

    sync_unlock(context->sync);

    return NULL;
}

static void *semaphore_worker(void *arg)
{
    SemaphoreContext *context = arg;

    if (sync_semaphore_wait(context->sync) != 0) {
        return NULL;
    }

    if (sync_lock(context->sync) != 0) {
        sync_semaphore_post(context->sync);
        return NULL;
    }

    context->active++;

    if (context->active > context->max_active) {
        context->max_active = context->active;
    }

    sync_unlock(context->sync);

    sleep_milliseconds(20);

    if (sync_lock(context->sync) == 0) {
        context->active--;
        sync_unlock(context->sync);
    }

    sync_semaphore_post(context->sync);

    return NULL;
}

int sync_init(SynchronizationManager *sync,
              unsigned int semaphore_count)
{
    if (sync == NULL || semaphore_count == 0) {
        return -1;
    }

    memset(sync, 0, sizeof(*sync));

    if (pthread_mutex_init(&sync->mutex, NULL) != 0) {
        return -1;
    }

    if (pthread_cond_init(&sync->condition, NULL) != 0) {
        pthread_mutex_destroy(&sync->mutex);
        return -1;
    }

    if (sem_init(&sync->semaphore, 0, semaphore_count) != 0) {
        pthread_cond_destroy(&sync->condition);
        pthread_mutex_destroy(&sync->mutex);
        return -1;
    }

    sync->initialized = 1;
    sync->semaphore_initialized = 1;

    return 0;
}

void sync_destroy(SynchronizationManager *sync)
{
    if (sync == NULL || !sync->initialized) {
        return;
    }

    if (sync->semaphore_initialized) {
        sem_destroy(&sync->semaphore);
        sync->semaphore_initialized = 0;
    }

    pthread_cond_destroy(&sync->condition);
    pthread_mutex_destroy(&sync->mutex);

    sync->initialized = 0;
}

int sync_lock(SynchronizationManager *sync)
{
    if (sync == NULL || !sync->initialized) {
        return -1;
    }

    return pthread_mutex_lock(&sync->mutex);
}

int sync_unlock(SynchronizationManager *sync)
{
    if (sync == NULL || !sync->initialized) {
        return -1;
    }

    return pthread_mutex_unlock(&sync->mutex);
}

int sync_wait(SynchronizationManager *sync)
{
    if (sync == NULL || !sync->initialized) {
        return -1;
    }

    return pthread_cond_wait(&sync->condition,
                             &sync->mutex);
}

int sync_signal(SynchronizationManager *sync)
{
    if (sync == NULL || !sync->initialized) {
        return -1;
    }

    return pthread_cond_signal(&sync->condition);
}

int sync_broadcast(SynchronizationManager *sync)
{
    if (sync == NULL || !sync->initialized) {
        return -1;
    }

    return pthread_cond_broadcast(&sync->condition);
}

int sync_semaphore_wait(SynchronizationManager *sync)
{
    if (sync == NULL ||
        !sync->initialized ||
        !sync->semaphore_initialized) {
        return -1;
    }

    return sem_wait(&sync->semaphore);
}

int sync_semaphore_post(SynchronizationManager *sync)
{
    if (sync == NULL ||
        !sync->initialized ||
        !sync->semaphore_initialized) {
        return -1;
    }

    return sem_post(&sync->semaphore);
}

int sync_run_demo(void)
{
    SynchronizationManager sync;

    if (sync_init(&sync, 2) != 0) {
        fprintf(stderr,
                "Failed to initialize synchronization primitives.\n");
        return -1;
    }

    printf("\n========================================\n");
    printf("       SYNCHRONIZATION DEMONSTRATION\n");
    printf("========================================\n");

    /*
     * ------------------------------------------------------
     * Mutex demonstration
     * ------------------------------------------------------
     */
    CounterContext counter_context = {
        .sync = &sync,
        .counter = 0
    };

    pthread_t counter_threads[DEMO_THREAD_COUNT];
    int counter_created = 0;

    for (int i = 0; i < DEMO_THREAD_COUNT; ++i) {
        if (pthread_create(&counter_threads[i],
                           NULL,
                           counter_worker,
                           &counter_context) != 0) {
            break;
        }

        counter_created++;
    }

    for (int i = 0; i < counter_created; ++i) {
        pthread_join(counter_threads[i], NULL);
    }

    long expected_counter =
        (long) counter_created * DEMO_COUNTER_ITERATIONS;

    printf("\n[Mutex]\n");
    printf("Threads completed       : %d\n", counter_created);
    printf("Expected counter value  : %ld\n",
           expected_counter);
    printf("Actual counter value    : %ld\n",
           counter_context.counter);

    if (counter_created != DEMO_THREAD_COUNT ||
        counter_context.counter != expected_counter) {
        sync_destroy(&sync);
        return -1;
    }

    printf("Result                  : PASS\n");

    /*
     * ------------------------------------------------------
     * Condition-variable demonstration
     * ------------------------------------------------------
     */
    ConditionContext condition_context = {
        .sync = &sync,
        .started = 0,
        .ready = 0,
        .awakened = 0
    };

    pthread_t condition_thread;

    if (pthread_create(&condition_thread,
                       NULL,
                       condition_worker,
                       &condition_context) != 0) {
        sync_destroy(&sync);
        return -1;
    }

    if (sync_lock(&sync) != 0) {
        pthread_join(condition_thread, NULL);
        sync_destroy(&sync);
        return -1;
    }

    while (!condition_context.started) {
        if (sync_wait(&sync) != 0) {
            sync_unlock(&sync);
            pthread_join(condition_thread, NULL);
            sync_destroy(&sync);
            return -1;
        }
    }

    condition_context.ready = 1;
    sync_broadcast(&sync);

    sync_unlock(&sync);

    pthread_join(condition_thread, NULL);

    printf("\n[Condition Variable]\n");
    printf("Waiting thread awakened : %s\n",
           condition_context.awakened ? "YES" : "NO");
    printf("Result                  : %s\n",
           condition_context.awakened ? "PASS" : "FAIL");

    if (!condition_context.awakened) {
        sync_destroy(&sync);
        return -1;
    }

    /*
     * ------------------------------------------------------
     * Counting semaphore demonstration
     * ------------------------------------------------------
     */
    SemaphoreContext semaphore_context = {
        .sync = &sync,
        .active = 0,
        .max_active = 0
    };

    pthread_t semaphore_threads[DEMO_THREAD_COUNT];
    int semaphore_created = 0;

    for (int i = 0; i < DEMO_THREAD_COUNT; ++i) {
        if (pthread_create(&semaphore_threads[i],
                           NULL,
                           semaphore_worker,
                           &semaphore_context) != 0) {
            break;
        }

        semaphore_created++;
    }

    for (int i = 0; i < semaphore_created; ++i) {
        pthread_join(semaphore_threads[i], NULL);
    }

    printf("\n[Counting Semaphore]\n");
    printf("Semaphore capacity     : 2\n");
    printf("Threads completed      : %d\n",
           semaphore_created);
    printf("Maximum concurrent     : %d\n",
           semaphore_context.max_active);
    printf("Result                 : %s\n",
           (semaphore_created == DEMO_THREAD_COUNT &&
            semaphore_context.max_active <= 2 &&
            semaphore_context.max_active > 0)
               ? "PASS"
               : "FAIL");

    int success =
        semaphore_created == DEMO_THREAD_COUNT &&
        semaphore_context.max_active <= 2 &&
        semaphore_context.max_active > 0;

    sync_destroy(&sync);

    printf("\n========================================\n");

    return success ? 0 : -1;
}
