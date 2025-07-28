# Task 06: Implement Hash Map Iterator

## Objective
Create an iterator system for traversing all key-value pairs in the hash map.

## Current State
- No way to iterate through all elements
- Users cannot enumerate contents

## Implementation Steps
1. Define iterator structure in datazoo.h:
   ```c
   typedef struct HoneycombIterator HoneycombIterator;
   ```

2. Add iterator API functions:
   ```c
   HoneycombIterator* honeycomb_iterator_create(Honeycomb *comb);
   bool honeycomb_iterator_next(HoneycombIterator *iter, 
                                const char **key, void **value);
   ```

3. Implement iterator structure in honeycomb.c:
   - Track current bucket index
   - Track current cell in bucket chain
   - Reference to honeycomb being iterated

4. Implement iterator logic:
   - Start at first bucket
   - Skip empty buckets
   - Traverse chains within buckets
   - Return false when no more elements

## Memory Management
- Iterator allocated from honeycomb's samrena
- No explicit destroy needed (arena cleanup handles it)

## Example Usage
```c
HoneycombIterator *iter = honeycomb_iterator_create(comb);
const char *key;
void *value;
while (honeycomb_iterator_next(iter, &key, &value)) {
    printf("Key: %s\n", key);
}
```

## Verification
- Test iterating empty hash map
- Test iterating hash map with various sizes
- Verify all inserted elements are visited exactly once