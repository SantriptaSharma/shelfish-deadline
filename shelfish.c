#include "log.h"
#include "avl.h"

#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <math.h>

#include "element.h"
#include "sstable.h"
#include "shelf.h"

// Defines the maximum size which the AVL tree can reach before it is flushed to disk
#define MAX_MEM_TABLE_SIZE 8

#include "length.h"

const char *directory = "./.shelves";

MemorySSSegment* SegmentFromShelf(Shelf *shelf)
{
    char path[256];
    strcpy(path, directory);
    strcat(path, "/");
    strcat(path, shelf->path);

    MemorySSSegment *segment = ReadSegmentFromFile(path);

    return segment;
}

int main()
{
    DIR *dir = opendir(directory);
    int shelfTop = 0, shelfCapacity = 4;
    Shelf **shelves = malloc(sizeof(*shelves) * shelfCapacity);

    if (dir)
    {
        LOG("Directory %s already exists, restoring state...", directory);
    }
    else if (errno == ENOENT)
    {
        LOG("Directory %s does not exist, creating...", directory);
        mkdir(directory, 0777);
        dir = opendir(directory);
    }
    else
    {
        ERROR("File System Error");
    }

    ASSERT("Target directory exists", dir);

    struct dirent *ent;

    while ((ent = readdir(dir)) != NULL)
    {
        if (ent->d_type == DT_REG)
        {
            char *filename = ent->d_name;
            char fullPath[256];
            strcpy(fullPath, directory);
            strcat(fullPath, "/");
            strcat(fullPath, filename);
            int shelfIndex;
            sscanf(filename, "%d", &shelfIndex);
            int length = GetDiskTableLength(fullPath);
            Shelf *shelf = malloc(sizeof(*shelf));
            strcpy(shelf->path, filename);
            shelf->index = shelfTop;
            shelf->size = length;
            shelves[shelfTop++] = shelf;

            if (shelfTop == shelfCapacity)
            {
                shelfCapacity *= 2;
                shelves = realloc(shelves, sizeof(*shelves) * shelfCapacity);
            }
        }
    }

    closedir(dir);

    AVLNode *memTable = NULL;
    int memTableSize = 0;

    LOG("Query engine started. The readme file contains a list of commands.");
    LOG("Force exits WILL cause data loss.");

    while (true)
    {
        LOG("Enter a query (empty to exit): ");
        char *line = NULL;
        size_t lineSize = 0;

        int length = getline(&line, &lineSize, stdin);

        if (line[length - 1] == '\n')
        {
            line[--length] = '\0';
        }

        if (length <= 0)
        {
            LOG("Empty input, exiting...");
            free(line);
            break;
        }

        char op = line[0];
        char *cursor = line;

        if (op == '-' || op == '?') 
        {
            cursor++;
            length--;
        }
        
        char *buffer = malloc(sizeof(*buffer) * (length + 1));

        int found = sscanf(cursor, "%s", buffer);

        if (!found)
        {
            LOG("Key not found");
            free(buffer);
            free(line);
            continue;
        }

        size_t keyLength = strlen(buffer);
        if (keyLength == 0 || keyLength >= MAX_KEY_LENGTH)
        {
            LOG("Invalid key size");
            free(buffer);
            free(line);
            continue;
        }

        char key[MAX_KEY_LENGTH];
        strcpy(key, buffer);

        free(buffer);

        cursor += keyLength;
        while (*cursor == ' ') cursor++;

        AVLNode *node = NULL;

        switch (op)
        {
            case '?':
                node = Find(memTable, key);

                if (node != NULL) 
                {
                    if (node->element->isTombstone)
                    {
                        LOG("Key %s not found", key);
                        break;
                    }

                    PrintElement(node->element);
                    LOG("%s", "");
                    break;
                }
                else 
                {
                    LOG("Key %s not found in memory, searching disk", key);
                    for (int i = shelfTop - 1; i >= 0; i--)
                    {
                        Shelf *shelf = shelves[i];
                        MemorySSSegment *segment = SegmentFromShelf(shelf);

                        if (segment == NULL)
                        {
                            LOG("Failed to read segment index %d", shelf->index);
                            continue;
                        }

                        int a = 0;
                        int b = shelf->size - 1;

                        bool found = false;

                        while (a <= b)
                        {
                            int m = (a + b) / 2;
                            Element *element = segment->elements[m];

                            int cmp = strcmp(key, element->key);

                            if (cmp == 0)
                            {
                                if (!element->isTombstone)
                                {
                                    PrintElement(element);
                                    LOG("%s", "");
                                }

                                found = true;
                                break;
                            }
                            else if (cmp < 0)
                            {
                                b = m - 1;
                            }
                            else
                            {
                                a = m + 1;
                            }
                        }

                        FreeSegment(segment);
                        if (found) break;
                    }
                }
            break;

            case '-':
                node = Find(memTable, key);

                if (node != NULL) 
                {
                    node->element->isTombstone = true;
                }
                else 
                {
                    Insert(&memTable, CreateElement(key, VT_INTEGER, true));
                    memTableSize++;
                }
            break;

            default:
                if (*cursor == '\0')
                {
                    LOG("No value provided for key %s", key);
                }
                else
                {
                    bool isNum = true;
                    int n = strlen(cursor);

                    if (n >= MAX_STRING_LENGTH)
                    {
                        LOG("Value too long (Longer than %d characters)", MAX_STRING_LENGTH - 1);
                        break;
                    }

                    char * const start = cursor;
                    int num = 0;
                    bool negative = *cursor == '-';

                    if (negative) 
                    {
                        cursor++;
                        n--;
                    }

                    while (*cursor != '\0')
                    {
                        if (!isdigit(*cursor))
                        {
                            isNum = false;
                            break;
                        }

                        num += (*cursor - '0') * pow(10, --n);
                        cursor++;
                    }

                    num *= negative ? -1 : 1;

                    char val[MAX_STRING_LENGTH];

                    strcpy(val, cursor);

                    Element *el = NULL;
                    node = Find(memTable, key);

                    if (isNum)
                    {
                        el = CreateElement(key, VT_INTEGER, false);
                        el->number = num;
                    }
                    else
                    {
                        el = CreateElement(key, VT_STRING, false);
                        strcpy(el->string, start);
                    }

                    if (node)
                    {
                        node->element = el;
                    }
                    else
                    {
                        Insert(&memTable, el);
                        memTableSize++;
                    }
                }
            break;
        }

        if (memTableSize >= MAX_MEM_TABLE_SIZE)
        {
            LOG("Table max size exceeded, flushing to disk...");
            MemorySSSegment *seg = CreateSegmentFromTable(memTable, memTableSize);
            Shelf *s = WriteSegmentToFile(seg, directory, shelfTop);

            if (s != NULL) 
            {
                shelves[shelfTop++] = s;
                if (shelfTop == shelfCapacity)
                {
                    shelfCapacity *= 2;
                    shelves = realloc(shelves, sizeof(*shelves) * shelfCapacity);
                }
            }

            FreeSegment(seg);
            FreeAVL(memTable);
            memTable = NULL;
            memTableSize = 0;
        }

        free(line);
    }

    LOG("Closing, flushing memory table to disk...");
    if (memTable != NULL)
    {
        MemorySSSegment *seg = CreateSegmentFromTable(memTable, memTableSize);
        Shelf *s = NULL;
        s = WriteSegmentToFile(seg, directory, shelfTop);
        if (s != NULL) free(s);
        FreeSegment(seg);
        FreeAVL(memTable);
        memTable = NULL;
    }

    for (int i = 0; i < shelfTop; i++)
    {
        free(shelves[i]);
    }

    free(shelves);
    FreeAVL(memTable);
}