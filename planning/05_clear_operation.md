# Task 05: Implement Clear Operation

## Objective
Add a honeycomb_clear function that empties the hash map while retaining its capacity.

## Current State
- No clear operation exists
- Users must recreate the hash map to empty it

## Implementation Steps
1. Add function declaration in datazoo.h:
   ```c
   void honeycomb_clear(Honeycomb *comb);
   ```

2. Implement in honeycomb.c:
   - Set all bucket pointers to NULL
   - Reset element count to 0
   - Keep capacity and buckets array unchanged
   - No need to free individual cells (arena handles memory)

## Implementation
```c
void honeycomb_clear(Honeycomb *comb) {
    if (comb == NULL) return;
    
    // Clear all buckets
    for (size_t i = 0; i < comb->capacity; i++) {
        comb->buckets[i] = NULL;
    }
    
    comb->size = 0;
    // Note: No memory deallocation needed - arena handles it
}
```

## Verification
- Test clearing an empty hash map
- Test clearing a hash map with elements
- Verify capacity remains the same after clear
- Verify elements can be added after clearing
