#include "element.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "length.h"

Element* CreateElement(const char *key, ValueType vt, bool tombstone)
{
    Element *new = malloc(sizeof(*new));
    new->key = malloc(sizeof(*(new->key)) * MAX_KEY_LENGTH);
    strncpy(new->key, key, MAX_KEY_LENGTH);
    new->type = vt;
    new->isTombstone = tombstone;

    if (new->type == VT_INTEGER) new->number = 0; else new->string = malloc(sizeof(*(new->string) * MAX_STRING_LENGTH));

    return new;
}

void FreeElement(Element *a)
{
    free(a->key);
    if (a->type == VT_STRING) free(a->string);
    free(a);
}

void PrintElement(Element *a)
{
    printf("%s: ", a->key);
    if (a->type == VT_STRING) printf("%s", a->string); else printf("%d", a->number);
}

int CompareElements(Element *a, Element *b)
{
    return strcmp(a->key, b->key);
}