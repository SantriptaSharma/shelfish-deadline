#ifndef _ELEMENT_H_
#define _ELEMENT_H_

#include <stdbool.h>

typedef enum _VT_Canonical {
    VT_STRING, VT_INTEGER
} ValueType;

typedef struct _ElementCanonical {
    char *key;
    ValueType type;
    union {
        char *string;
        int number;
    };
    bool isTombstone;
} Element;

Element* CreateElement(const char *key, ValueType vt, bool tombstone);
int CompareElements(Element *a, Element *b);
void PrintElement(Element *a);
void FreeElement(Element *a);
#endif