#ifndef HEAP_H
#define HEAP_H

#include"GarbageCollector.h"

#include<pthread.h>

typedef struct Heap{
    void* data;
    size_t offset;
    size_t size;
    size_t numberObject;

    pthread_rwlock_t lock;
}Heap;

Heap* heapInit(size_t size);
void heapDestroy(Heap*);
void* heapAllocate(Heap*, size_t size);
void heapReduceMemory(Heap*);

bool heapContains(Heap*,void*);
bool heapScan(Heap*, Object**);

#define AVAILABLE_HEAP_SIZE(heap) ((heap)->size - (heap)->offset)
#define ALLOCATED_HEAP_SIZE(heap) (heap->offset)

#endif

