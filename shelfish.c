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

#define MAX_MEM_TABLE_HEIGHT 8

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

                if (node != NULL && !node->isTombstone) 
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
                    node->isTombstone = true;
                }
                else 
                {
                    Insert(&memTable, CreateElement(key, VT_INTEGER), true);
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

                    if (negative) cursor++;

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

                    char val[MAX_STRING_LENGTH];

                    strcpy(val, cursor);

                    Element *el = NULL;
                    node = Find(memTable, key);

                    if (isNum)
                    {
                        el = CreateElement(key, VT_INTEGER);
                        el->number = num;
                    }
                    else
                    {
                        el = CreateElement(key, VT_STRING);
                        strcpy(el->string, start);
                    }

                    if (node)
                    {
                        node->isTombstone = false;
                        node->element = el;
                    }
                    else
                    {
                        Insert(&memTable, el, false);
                    }
                }
            break;
        }

        PrintInOrder(memTable);

        free(line);
    }

    FreeAVL(memTable);
}