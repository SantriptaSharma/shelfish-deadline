## Shelfish Prototype
Implemented as a 2-layer LSM tree, with one in-memory layer and one on disk layer.

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
    