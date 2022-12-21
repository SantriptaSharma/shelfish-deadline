#include "sstable.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

MemorySSSegment* CreateSegmentFromTable(AVLNode *root, int count)
{
    MemorySSSegment *segment = malloc(sizeof(*segment));
    segment->keyCount = count;
    segment->elements = malloc(sizeof(*(segment->elements)) * count);
    
    AVLNode *min = FindMinimum(root), *max = FindMaximum(root);
    int n = strlen(min->element->key);
    segment->keyRange[0] = malloc(sizeof(*(segment->keyRange[0])) * (n + 1));
    strcpy(segment->keyRange[0], min->element->key);

    n = strlen(max->element->key);
    segment->keyRange[1] = malloc(sizeof(*(segment->keyRange[1])) * (n + 1));
    strcpy(segment->keyRange[1], max->element->key);
}

MemorySSSegment* ReadSegmentFromFile(char *filename);
MemorySSSegment* WriteSegmentToFile(MemorySSSegment *segment, char *filename);
MemorySSSegment* MergeSegments(MemorySSSegment *segment1, MemorySSSegment *segment2);

void FreeSegment(MemorySSSegment *segment)
{
    for (int i = 0; i < segment->keyCount; i++)
    {
        FreeElement(segment->elements[i]);
    }
    
    free(segment->elements);
    free(segment->keyRange[0]);
    free(segment->keyRange[1]);
    free(segment);
}