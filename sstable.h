#ifndef _SSTABLE_H_
#define _SSTABLE_H_

#include "element.h"
#include "avl.h"
#include "shelf.h"

typedef struct _SSTableSegment 
{
    int keyCount;
    Element **elements;
    bool isOwner;
} MemorySSSegment;

MemorySSSegment* CreateSegmentFromTable(AVLNode *root, int count);
MemorySSSegment* ReadSegmentFromFile(char *filename);
int GetDiskTableLength(char *filename);
Shelf* WriteSegmentToFile(MemorySSSegment *segment, const char *directory, int index);
void FreeSegment(MemorySSSegment *segment);
#endif