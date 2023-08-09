#include <stdlib.h>
#include <stdio.h>

#include "../include/watchdog.h"

// Function to free the signal resources
void freeSignal(Signal* signal) {
    pthread_mutex_destroy(&signal->mutex);
    pthread_cond_destroy(&signal->signal_cv);
    free(signal);
}

int watchdogCreate(pthread_t* thread, void* (*threadFunction)(void*), pthread_t* watchdogThread, void* (*watchdogFunction)(void*)) {
    Signal* signal = (Signal*)malloc(sizeof(Signal));
    if (signal == NULL) {
        return -1;
    }

    // Initialize mutex and condition variable
    if (pthread_mutex_init(&signal->mutex, NULL) != 0) {
        free(signal);
        return -1;
    }
    if (pthread_cond_init(&signal->signal_cv, NULL) != 0) {
        pthread_mutex_destroy(&signal->mutex);
        free(signal);
        return -1;
    }

    // Store the main thread ID
    signal->thread = pthread_self();

    if (pthread_create(thread, NULL, threadFunction, signal) != 0) {
        freeSignal(signal);
        return -1;
    }

    if (pthread_create(watchdogThread, NULL, watchdogFunction, signal) != 0) {
        freeSignal(signal);
        return -1;
    }

    return 0;
}

void watchdogSendSignal(Signal* signal) {
    pthread_mutex_lock(&signal->mutex);
    pthread_cond_signal(&signal->signal_cv);
    pthread_mutex_unlock(&signal->mutex);
}