# Task 07: Implement Bulk Operations

## Objective
Add efficient batch insertion capability for multiple key-value pairs.

## Current State
- Only single element insertion available
- Multiple insertions may trigger multiple resizes

## Implementation Steps
1. Define bulk operation structure:
   ```c
   typedef struct {
       const char *key;
       void *value;
   } HoneycombPair;
   ```

2. Add bulk insert function:
   ```c
   bool honeycomb_put_bulk(Honeycomb *comb, 
                          const HoneycombPair *pairs, 
                          size_t count);
   ```

3. Implement efficient bulk insertion:
   - Pre-calculate if resize is needed based on final size
   - Perform single resize if necessary
   - Insert all pairs
   - Return false if any insertion fails

## Implementation Strategy
```c
bool honeycomb_put_bulk(Honeycomb *comb, 
                       const HoneycombPair *pairs, 
                       size_t count) {
    // Check if resize needed
    size_t final_size = comb->size + count;
    if (final_size > comb->capacity * comb->load_factor) {
        // Calculate required capacity and resize once
        size_t required_capacity = final_size / comb->load_factor;
        // Resize to next power of 2 >= required_capacity
    }
    
    // Insert all pairs
    for (size_t i = 0; i < count; i++) {
        if (!honeycomb_put(comb, pairs[i].key, pairs[i].value)) {
            return false;
        }
    }
    return true;
}
```

## Verification
- Test bulk insert that triggers resize
- Test bulk insert within current capacity
- Verify single resize for large bulk operations