#ifndef _AVL_H_
#define _AVL_H_

#include "element.h"
#include <stdbool.h>

typedef struct _AVLCanonical
{
    Element *element;
    int height;
    bool isTombstone;
    struct _AVLCanonical *left, *right, *parent;
} AVLNode;

void Insert(AVLNode **root, Element* element);
AVLNode* Find(AVLNode *root, const char* element);
AVLNode* FindMinimum(AVLNode *root);
AVLNode* FindSuccessor(AVLNode *target);
void RotateLeft(AVLNode **root, AVLNode *target);
void RotateRight(AVLNode **root, AVLNode *target);
AVLNode* Delete(AVLNode *target);
void PrintInOrder(AVLNode *root);
void FreeAVL(AVLNode *root);

#endif