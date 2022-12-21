#include "log.h"
#include "avl.h"

#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "element.h"
#include "sstable.h"

// Defines the maximum size which the AVL tree can reach before it is flushed to disk
#define MAX_MEM_TABLE_SIZE 128

#include "length.h"

const char *directory = "./.shelf";

int main()
{
    DIR *dir = opendir(directory);

    if (dir)
    {
        // TODO: Restore state
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

                if (node != NULL && !node->element->isTombstone) 
                {
                    PrintElement(node->element);
                    LOG("%s", "");
                }
                else 
                {
                    LOG("Key %s not found", key);
                }

                //TODO: Go through disk entries
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

        if (memTableSize > MAX_MEM_TABLE_SIZE)
        {
            MemorySSSegment *seg = CreateSegmentFromTable(memTable, memTableSize);
        }

        free(line);
    }

    // TODO: Final flush to disk here

    FreeAVL(memTable);
}