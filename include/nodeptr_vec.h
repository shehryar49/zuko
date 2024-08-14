#ifndef NODEPTR_VECTOR_H_
#define NODEPTR_VECTOR_H_


#ifdef __cplusplus
extern "C"{
#endif
typedef struct Node Node;
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct nodeptr_vector
{
    Node** arr;
    size_t size;
    size_t capacity;
}nodeptr_vector;

void nodeptr_vector_init (nodeptr_vector* p);
void nodeptr_vector_push(nodeptr_vector* p,Node* val);
bool nodeptr_vector_pop(nodeptr_vector* p,Node** val);
void nodeptr_vector_assign(nodeptr_vector* p,nodeptr_vector* val); //makes deep copy
int nodeptr_vector_search(nodeptr_vector* p,Node* val);
void nodeptr_vector_destroy(nodeptr_vector* p);

#ifdef __cplusplus
}
#endif



#endif