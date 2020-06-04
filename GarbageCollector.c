#ifndef GARBAGECOLLECTOR_H
#define GARBAGECOLLECTOR_H

#include"Heap.h"
#include<stdio.h>

static pthread_mutex_t gcMutex;
static pthread_key_t gcStackKey;

static Heap* heap = NULL;

static inline void gcLock(){ pthread_mutex_lock(&gcMutex); }
static inline void gcUnlock(){ pthread_mutex_unlock(&gcMutex); }

void gcInit(size_t heapSize)
{
    heap = heapInit(heapSize);

    pthread_mutex_init(&gcMutex, NULL);
    pthread_key_create(&gcStackKey, NULL);

    pthread_setspecific(gcStackKey, NULL);

}

void* gcMalloc(size_t size)
{
    gcLock();
    void *ret = heapAllocate(heap, size);

    gcUnlock();
    return ret;
}

void gcStackPush(void* data)
{
    StackFrame* headFrame = (StackFrame*)pthread_getspecific(gcStackKey);
    StackFrame* newFrame = (StackFrame*)malloc(sizeof(StackFrame));

    newFrame->parent = headFrame;
    newFrame->data = data;

    pthread_setspecific(gcStackKey, newFrame);

}

void gcStackPop()
{
    StackFrame* currFrame = (StackFrame*)pthread_getspecific(gcStackKey);
    if(currFrame)
    {
        pthread_setspecific(gcStackKey, currFrame->parent);
    }

    free(currFrame);
}

void gcMark()
{
    StackFrame* currFrame = (StackFrame*)pthread_getspecific(gcStackKey);

    Object* object = NULL;

    while (heapScan(heap, &object))
    {
        if(object->metadata.flag == UNMARKED) continue;

        StackFrame* frame = currFrame;
        bool reachable = false;

        while (frame)
        {
            if(frame->data == object->data)
                reachable = true;
            
            frame = frame->parent;
        }

        if(!reachable)
            object->metadata.flag = UNMARKED;
        else
            object->metadata.flag = MARKED;
        
    }
    
}

void gcSweep()
{
    heapReduceMemory(heap);
}

void gcCollect()
{
    gcLock();

    gcMark();
    gcSweep();

    gcUnlock();
}

#endif

