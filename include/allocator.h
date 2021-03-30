#ifndef _ALLOCATOR_H
#define _ALLOCATOR_H

#include "stdint.h"
#define FREE_SLOT -1
#define UNFREE_SLOT -2
#define ALLOCATED_SLOT_SHIFT 100
#define MAX_ELEMENT_NUM 10000

#define MAX_CHUNK_SIZE 160

struct FrameListNum {
    uint32_t index;
    struct FrameListNum *next;
    struct FrameListNum *prev;
};

struct FrameChunk {
    uint32_t index;
    uint8_t chunk16[32];
    uint8_t chunk32[32]; 
    uint8_t chunk64[16];
    uint8_t chunk128[6];
    uint8_t chunk256[3];
};

typedef struct _RawFrameArray{
    uint64_t base_addr, end_addr;
    int32_t val[0x10000 + 100];
    struct FrameListNum *freeList[20];
} FrameArray;

FrameArray* NewFrameArray();
uint64_t new_memory(struct _RawFrameArray*, uint64_t);
void free_memory(struct _RawFrameArray*, uint64_t);

#endif