#include<stdlib.h>
#include<stdbool.h>

#define ALIGNMENT 0x10
#define COMPUTE_PADDING(n) (ALIGNMENT - (n % ALIGNMENT))

typedef enum ObjectFlags{
    UNMARKED = 0,
    MARKED = 1
}ObjectFlags;

typedef struct Metadata{
    size_t size;

    int flag : 8;
    
    int padding : 8;

    char reserved[COMPUTE_PADDING(sizeof(size_t) + 1)];
}Metadata;

typedef struct Object{
    Metadata metadata;
    unsigned char data[];

}Object;

typedef struct StackFrame{
    StackFrame* parent;
    void* data;
}StackFrame;


void gcInit(size_t heapSize);
void gcMalloc(size_t size);
void gcStackPush(void*);
void gcStackPop();
void gcMark();
void gcSweep();
void gcCollect();   