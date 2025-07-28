# Task 03: Implement Safe Key Copying

## Objective
Ensure all string keys are copied into arena-allocated memory owned by the hash map.

## Current State
- Keys might be stored as pointers to user-provided strings
- This is unsafe if the user modifies or frees the original string

## Implementation Steps
1. Modify the `honeycomb_put` function to:
   - Calculate the length of the input key string
   - Allocate memory from samrena for the key copy
   - Copy the key string into the allocated memory
   - Store the pointer to the copied key in the Cell structure

2. Update Cell structure if needed to ensure it uses the copied key

3. Ensure honeycomb_get and honeycomb_remove use the copied keys for comparison

## Code Example
```c
// In honeycomb_put:
size_t key_len = strlen(key) + 1;
char *key_copy = (char*)samrena_allocate(comb->arena, key_len);
if (key_copy == NULL) {
    return false;  // Arena exhausted
}
memcpy(key_copy, key, key_len);
// Use key_copy in the Cell instead of original key
```

## Verification
- Create a test that:
  - Creates a key in a local buffer
  - Inserts it into the hash map
  - Modifies the local buffer
  - Verifies the hash map still has the original key value