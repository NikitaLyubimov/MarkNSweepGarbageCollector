#include"Heap.h"

#include<stdio.h>
#include<string.h>

#define POINTER_OFFSET(data, offset) ((char*)data + offset) 
#define OBJECT_DATA_WIDTH(object) ((object)->metadata.size + (object)->metadata.padding)
#define OBJECT_WIDTH(object) (sizeof(Metadata) + OBJECT_DATA_WIDTH(object))

#define WRLOCK(lock) pthread_rwlock_wrlock(lock)
#define UNLOCK(lock) pthread_rwlock_unlock(lock)

Heap* heapInit(size_t size)
{
    Heap* heap = (Heap*)malloc(sizeof(Heap));
    if(heap == NULL)
    {
        fprintf(stderr, "Couldn't create heap\n");
        return NULL;
    }

    heap->offset = 0;
    heap->size = size;
    heap->data = malloc(size);

    if(heap->data == NULL)
    {
        fprintf(stderr, "Couldn't allocate memory for heap\n");
        return NULL;
    }
    heap->numberObject = 0;

    pthread_rwlock_init(&heap->lock, NULL);

    printf("Heap created successfuly\n");
    return heap;
}

void heapDestroy(Heap* heap)
{
    pthread_rwlock_destroy(&heap->lock);
    free(heap->data);
    free(heap);
    printf("Heap destroyed successfuly");
}

void* heapAllocate(Heap* heap, size_t size)
{
    WRLOCK(&heap->lock);

    size_t padding = COMPUTE_PADDING(size);

    size_t objSize = sizeof(Metadata) + size + padding;
    void* ret = NULL;

    if(objSize > AVAILABLE_HEAP_SIZE(heap))
    {
        fprintf(stderr, "Heap is full\n");
        return ret;
    }

    Object* newObject = (Object*)POINTER_OFFSET(heap->data, heap->offset);
    newObject->metadata.flag = UNMARKED;
    newObject->metadata.padding = padding;
    newObject->metadata.size = size;

    memset(newObject->data, 0, size + padding);

    heap->offset += objSize;
    heap->numberObject++;

    ret = (void*)newObject->data;
    UNLOCK(&heap->lock);
    return ret;
}

void heapReduceMemory(Heap* heap)
{
    WRLOCK(&heap->lock);

    void* limitObj = (void*)POINTER_OFFSET(heap->data, heap->offset);
    void* voidObj = (void*)POINTER_OFFSET(heap->data, 0);

    size_t newOffset = 0;
    if(heap->numberObject == 1)
    {
        Object* object = (Object*)voidObj;
        if(object->metadata.flag == UNMARKED)
        {
            memset(heap->data, 0, heap->offset);
            heap->numberObject = 0;
        }
            
        return;
    }

    while (voidObj < limitObj)
    {
        Object* object = (Object*)voidObj;
        size_t objWidth = OBJECT_WIDTH(object);


        if(object->metadata.flag == MARKED)
        {

            if((void*)POINTER_OFFSET(heap->data, newOffset) != voidObj)
            {
                Object* replaceObj = (Object*)POINTER_OFFSET(heap->data, heap->offset);

                for(int i = 0; i < objWidth;++i)
                {
                    ((unsigned char*)replaceObj)[i] = ((unsigned char*)object)[i];
                }

                heap->numberObject--;
            }

            newOffset += objWidth;
        }

        voidObj = (void*)POINTER_OFFSET(voidObj, objWidth);
    }

    heap->offset = newOffset;
    UNLOCK(&heap->lock);
}



bool heapContains(Heap* heap, void* data)
{
    return (data > heap->data && data < (void*)POINTER_OFFSET(heap->data, heap->offset));
}

bool heapScan(Heap* heap, Object** object)
{
    if(heap->offset == 0)
        return false;

    if(*object == NULL)
    {
        *object = (Object*)POINTER_OFFSET(heap->data, 0);
        return true;
    }

    void* limitObj = (void*)POINTER_OFFSET(heap->data, heap->offset);
    void* voidObj = (void*)POINTER_OFFSET(*object, OBJECT_WIDTH(*object));

    if(voidObj < limitObj)
    {
        *object = voidObj;
        return true;
    }

    *object = NULL;
    return false;


}