#ifndef ZUKO_MEM_MAP_H_
#define ZUKO_MEM_MAP_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "zobject.h"

typedef enum memmap_slot_status
{
    MM_EMPTY,
    MM_OCCUPIED,
    MM_DELETED
}memmap_slot_status;

typedef struct mem_info
{
    char type;
    bool ismarked;
}mem_info;

typedef struct memmap_slot
{
    void* key;
    mem_info val;
    memmap_slot_status stat;
}memmap_slot;

typedef struct mem_map
{
    memmap_slot* table;
    size_t size;
    size_t capacity;
}mem_map;


void mem_map_init(mem_map* h);
void mem_map_set(mem_map* h,void* key,mem_info val);
void mem_map_emplace(mem_map* h,void* key,mem_info val);
bool mem_map_get(mem_map* h,void* key,mem_info* val);
mem_info* mem_map_getref(mem_map* h,void* key);
bool mem_map_erase(mem_map* h,void* key);
void mem_map_clear(mem_map* h);
void mem_map_destroy(mem_map* h);




#ifdef __cplusplus
}
#endif

#endif
