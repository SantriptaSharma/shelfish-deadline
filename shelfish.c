#include "log.h"
#include "avl.h"

#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "element.h"

#define MAX_MEM_TABLE_HEIGHT 9

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

    AVLNode *mem_table = NULL;

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

        char *buffer = malloc(sizeof(*buffer) * (length + 1));

        sscanf(line, "%s", buffer);
        size_t keyLength = strlen(buffer);
        if (keyLength == 0 || keyLength >= MAX_KEY_LENGTH)
        {
            LOG("Invalid key size");
            free(buffer);
            free(line);
            continue;
        }

        char *cursor = line + keyLength;
        char key[MAX_KEY_LENGTH];
        strcpy(key, buffer);

        sscanf(cursor, "%s", buffer);
        size_t valLength = strlen(buffer);
        if (valLength == 0 || valLength >= MAX_STRING_LENGTH)
        {
            LOG("Invalid value size");
            free(buffer);
            free(line);
            continue;
        }

        bool isNumber = true;
        for (int i = 0; i < valLength; i++)
        {
            if (!isdigit(buffer[i]))
            {
                isNumber = false;
                break;
            }
        }

        if (isNumber)
        {
            int value = strtol(buffer, NULL, 10);
            Element *e = CreateElement(key, VT_INTEGER);
            e->number = value;
            Insert(&mem_table, e);
            LOG("Inserted %s:%d", key, value);
        }
        else
        {
            char value[MAX_STRING_LENGTH];
            strcpy(value, buffer);
            Element *e = CreateElement(key, VT_STRING);
            strcpy(e->string, value);
            Insert(&mem_table, e);
            LOG("Inserted %s:%s", key, value);
        }

        PrintInOrder(mem_table);

        free(buffer);
        free(line);
    }

    FreeAVL(mem_table);
}