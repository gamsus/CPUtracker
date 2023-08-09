#ifndef BUFFER_H
#define BUFFER_H

#include <stdbool.h>    
#include <stdint.h>     
#include <stddef.h>    

typedef struct Buffer Buffer;

Buffer* bufferNew(size_t data_size);

int bufferPush(Buffer* buffer, void* element, uint8_t maxTime);

int bufferPop(Buffer* buffer, void* element, uint8_t maxTime);

bool bufferEmpty(Buffer* buffer);

bool bufferFull(Buffer* buffer);

void bufferDelete(Buffer* buffer);

#endif
