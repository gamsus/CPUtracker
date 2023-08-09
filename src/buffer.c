#include <pthread.h>  
#include <stdlib.h>     
#include <string.h>     
#include <time.h>    
#include <sys/time.h>   

#include "../include/buffer.h"

typedef struct Buffer {
    pthread_cond_t canProduce;
    pthread_cond_t canConsume;
    pthread_mutex_t mutex;
    size_t tail;
    size_t head;
    size_t elNum;
    size_t elSize;
    size_t capacity;
    uint8_t buffer[];
} Buffer;

Buffer* bufferNew(size_t dataSize) {
    Buffer* buffer;

    if(dataSize == 0) {
        return NULL;
    }
        
    buffer = malloc(sizeof(*buffer) + (dataSize));

    if(buffer == NULL) {
        return NULL;
    }

    *buffer = (Buffer){.mutex = PTHREAD_MUTEX_INITIALIZER,
      .canConsume = PTHREAD_COND_INITIALIZER,
      .canProduce = PTHREAD_COND_INITIALIZER,
      .tail = 0,
      .head = 0,
      .elNum = 0,
      .capacity = 1,
      .elSize = dataSize,
    };
    return buffer;
}

bool bufferEmpty(Buffer* buffer) {
    if(buffer == NULL) {
        return false;
    }
    if(buffer->elNum == 0) {
        return true;
    }
    return false;
}

bool bufferFull(Buffer* buffer) {
    if(buffer == NULL) {
        return false;
    }
    if(buffer->elNum == buffer->capacity) {
        return true;
    }
    return false;
}

int bufferPush(Buffer* buffer, void* element, uint8_t time) {
    struct timeval now;
    struct timespec timeout;
    uint8_t* ptr;

    if(buffer == NULL) {
        return 0;
    }
        
    if(element == NULL) {
        return 0;
    }
        
    gettimeofday(&now, NULL);
    timeout.tv_sec = now.tv_sec + time;
    timeout.tv_nsec = now.tv_usec * 1000;
    pthread_mutex_lock(&buffer->mutex);

    while(bufferFull(buffer)) {
        if (pthread_cond_timedwait(&buffer->canProduce, &buffer->mutex, &timeout) != 0) {
            pthread_mutex_unlock(&buffer->mutex);
            return 0;
        }
    }

    ptr = &buffer->buffer[buffer->head * buffer->elSize];
    memcpy(ptr, element, buffer->elSize);
    buffer->elNum++;
    buffer->head = (buffer->head + 1) % buffer->capacity;
    pthread_cond_signal(&buffer->canConsume);
    pthread_mutex_unlock(&buffer->mutex);
    return 1;
}

int bufferPop(Buffer* buffer, void* element, uint8_t time) {
    struct timeval now;
    struct timespec timeout;
    uint8_t* ptr;

    if(buffer == NULL)
        return 0;

    if(element == NULL)
        return 0;

    gettimeofday(&now, NULL);
    timeout.tv_sec = now.tv_sec + time;
    timeout.tv_nsec = now.tv_usec * 1000;

    pthread_mutex_lock(&buffer->mutex);
    while (bufferEmpty(buffer)) {
        if(pthread_cond_timedwait(&buffer->canConsume, &buffer->mutex, &timeout) != 0){
            pthread_mutex_unlock(&buffer->mutex);

            return 0;
        }
    }

    ptr = &buffer->buffer[buffer->tail * buffer->elSize];
    memcpy(element, ptr, buffer->elSize);
    buffer->elNum--;
    buffer->tail = (buffer->tail + 1) % buffer->capacity;
    pthread_cond_signal(&buffer->canProduce);
    pthread_mutex_unlock(&buffer->mutex);
    return 1;
}

void bufferDelete(Buffer* buffer) {
    if(buffer == NULL) {
        return;
    }
    pthread_mutex_destroy(&buffer->mutex);
    pthread_cond_destroy(&buffer->canProduce);
    pthread_cond_destroy(&buffer->canConsume);
    free(buffer);
}
