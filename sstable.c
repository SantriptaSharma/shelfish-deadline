#include "sstable.h"
#include "length.h"
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct _Stack
{
    int top;
    Element **array;
};

static void _InOrderStack(AVLNode *root, struct _Stack *stack)
{
    if (root == NULL) return;
    _InOrderStack(root->left, stack);
    stack->array[stack->top++] = root->element;
    _InOrderStack(root->right, stack);
}

MemorySSSegment* CreateSegmentFromTable(AVLNode *root, int count)
{
    MemorySSSegment *segment = malloc(sizeof(*segment));
    struct _Stack stack;
    stack.top = 0;
    stack.array = malloc(sizeof(*(stack.array)) * count);
    segment->keyCount = count;
    segment->elements = malloc(sizeof(*(segment->elements)) * count);
    segment->isOwner = false;
    _InOrderStack(root, &stack);

    memcpy(segment->elements, stack.array, sizeof(*(segment->elements)) * count);
    free(stack.array);
    
    return segment;
}

Shelf* WriteSegmentToFile(MemorySSSegment *segment, const char *directory, int index)
{
    char filename[256];
    sprintf(filename, "%s/%d-%ld.shelf", directory, index, time(0));
    FILE *file = fopen(filename, "wb");

    if (file == NULL)
    {
        remove(filename);
        return NULL;
    }

    fputc(segment->keyCount, file);
    
    for (int i = 0; i < segment->keyCount; i++)
    {
        Element *el = segment->elements[i];
        fputc(el->type, file);
        size_t len = strlen(el->key) + 1;
        fputc(el->isTombstone, file);
        fputc(len, file);
        fwrite(el->key, sizeof(*(el->key)), len, file);
        if (el->type == VT_INTEGER)
        {
            fwrite(&el->number, sizeof(el->number), 1, file);
        }
        else
        {
            len = strlen(el->string) + 1;
            fputc(len, file);
            fwrite(el->string, sizeof(*(el->string)), len, file);
        }
    }

    fclose(file);
    
    Shelf *shelf = malloc(sizeof(*shelf));
    strcpy(shelf->path, filename);
    shelf->index = index;
    shelf->size = segment->keyCount;
    return shelf;
}

int GetDiskTableLength(char *filename)
{
    FILE *file = fopen(filename, "rb");

    if (file == NULL)
    {
        return -1;
    }

    int count = fgetc(file);
    fclose(file);
    return count;
}

MemorySSSegment* ReadSegmentFromFile(char *filename)
{
    MemorySSSegment *segment = malloc(sizeof(*segment));
    segment->isOwner = true;
    FILE *file = fopen(filename, "rb");

    if (file == NULL)
    {
        free(segment);
        return NULL;
    }

    segment->keyCount = fgetc(file);
    segment->elements = malloc(sizeof(*(segment->elements)) * segment->keyCount);

    for (int i = 0; i < segment->keyCount; i++)
    {
        Element *el = malloc(sizeof(*el));
        el->type = fgetc(file);
        el->isTombstone = fgetc(file);

        size_t len = fgetc(file);
        el->key = malloc(sizeof(*(el->key)) * len);
        fread(el->key, sizeof(*(el->key)), len, file);

        if (el->type == VT_INTEGER)
        {
            fread(&el->number, sizeof(el->number), 1, file);
        }
        else
        {
            len = fgetc(file);
            el->string = malloc(sizeof(*(el->string)) * len);
            fread(el->string, sizeof(*(el->string)), len, file);
        }

        segment->elements[i] = el;
    }

    fclose(file);
    return segment;
}

void FreeSegment(MemorySSSegment *segment)
{
    if (segment->isOwner)
    {
        for (int i = 0; i < segment->keyCount; i++)
        {
            FreeElement(segment->elements[i]);
        }
    }

    free(segment->elements);
    free(segment);
}