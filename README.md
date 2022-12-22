## Shelfish Prototype
Implemented as a 2-layer LSM tree, with one in-memory layer and one on disk layer. Compaction has not been implemented.

## Building
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

# Testing
Mostly manual, assertions, a few input sets (similar to insert.txt), modifying parameters like max mem table size and the like. Persistence was tested by adding keys, stopping the program, and trying to query/delete/update those keys after a restart of the program.

# Improvements
Performance, bloom filters, levels, compaction