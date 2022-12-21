#ifndef _SSTABLE_H_
#define _SSTABLE_H_

#include "element.h"
#include "bloom.h"
#include "avl.h"

typedef struct _SSTableSegment 
{
    int keyCount;
    Element **elements;
    char *keyRange[2];
} MemorySSSegment;

MemorySSSegment* CreateSegmentFromTable(AVLNode *root, int count);
MemorySSSegment* ReadSegmentFromFile(char *filename);
MemorySSSegment* WriteSegmentToFile(MemorySSSegment *segment, char *filename);
MemorySSSegment* MergeSegments(MemorySSSegment *segment1, MemorySSSegment *segment2);
void FreeSegment(MemorySSSegment *segment);
#endif