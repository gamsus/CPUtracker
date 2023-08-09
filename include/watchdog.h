#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <pthread.h>

typedef struct Signal{
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t signal_cv;
} Signal;

int watchdogCreate(pthread_t* thread, void* (*threadFunction)(void*),
                   pthread_t* watchdogThread, void* (*watchdogFunction)(void*));
void watchdogSendSignal(Signal* signal);

#endif
