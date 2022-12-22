# Project Description
An implementation of a persistent data store loosely following the structure of a two-level LSM tree, with one in-memory level and one disk level.

### **Main Data Structures Implemented**
- Element Structure (element.h), essentially a key:value entry which allows values of multiple types (string and int in this case)
- Modified the AVL from the main course content to work with the aforementioned Element structure, using a custom compare function
- Sorted String Tables (sstable.h), An intermediary memory representation allowing transferring of sorted sequences of Elements to and from the disk, custom binary format for the files
- Shelf Descriptor (shelf.h), an file descriptor used for pointing to .shelf files

### **Possible Improvements**
- I've not implemented compaction in here, since I was unsure about when to compact (since it is quite an expensive procedure), and was not able to decide on a policy. This is bad, since it leads to the accumulation of various shelves, adversely impacting read throughput for larger datasets.
- Implementing more on-disk levels can be useful when paired with a bloom filter on each level for optimising reads, since we can simply skip levels which certainly don't have the keys we are looking for.
- Each SSTable on disk could store a sparse index (skip list implementation possible), which allows us to skip through unnecessary elements, and start the binary search process further within the sequence, giving marginal read performance benefits.
- Each table on disk could store its "range", which refers to the "smallest" (lexically) key within it and the largest key within it. This also increases read throughput by allowing us to skip reading through tables which definitely do not hold the key we are looking for.

# Testing
Most of the testing for this program was done manually, that is, I did not generate any random sets for testing. I used assertions, a few input sets (like insert.txt), manual queries, parameter modification (like the max mem table size before it is flushed etc). I used valgrind to find & plug memory leaks and gdb to debug the program.

Persistence was test by simply performing operations, restarting the program, performing more operations, and checking the soundness of their results.

Verdict: seems good to me, probably works 👍

# Usage
The query engine can be built using `make` and the output (and the stored shelves) can be cleaned using `make clean`

## Caution
Modifying the shelves directory directly will most likely result in errors

### Command List
The query engine can perform the following operations:
- Store key value pairs:\
    key value\
    Where key is an unquoted string of alphanumerics + underscores and value is either a string (unquoted) or integer\
    Examples:
    - length 30
    - width 50
    - name bouillabaisse
    - extension_granted 2 days
- Search for the value associated with a key:\
    ?key\
    Where key is an unquoted string of alphanumerics + underscores\
    Examples:
    - ?length
    - ?name
- Delete a particular entry by key:\
    -key\
    Where key is an unquoted string of alphanumerics + underscores\
    Examples:
    - -length
    - -extension_granted