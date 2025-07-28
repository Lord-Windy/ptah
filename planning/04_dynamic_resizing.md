# Task 04: Implement Dynamic Resizing

## Objective
Add automatic resizing when the hash map reaches a configurable load factor threshold.

## Current State
- Hash map likely has fixed size
- Performance degrades as it fills up

## Implementation Steps
1. Add load factor field to Honeycomb structure (default 0.75)
2. Add current element count tracking
3. Implement resize logic:
   - Calculate when resize is needed (elements > capacity * load_factor)
   - Allocate new larger bucket array from samrena
   - Rehash all existing entries into new buckets
   - Update honeycomb to use new bucket array

4. Trigger resize check in honeycomb_put after successful insertion

## Key Considerations
- All memory allocation must use samrena
- Old bucket array doesn't need explicit deallocation (arena handles it)
- Choose good growth factor (typically 2x)
- Handle case where samrena allocation fails during resize

## Code Structure
```c
static bool honeycomb_resize(Honeycomb *comb) {
    size_t new_capacity = comb->capacity * 2;
    Cell **new_buckets = samrena_allocate(comb->arena, 
                                          sizeof(Cell*) * new_capacity);
    if (!new_buckets) return false;
    
    // Rehash all entries...
    // Update comb->buckets and comb->capacity
    return true;
}
```

## Verification
- Test inserting elements past the load factor threshold
- Verify the hash map resizes and all elements are still accessible
- Test resize failure when arena is exhausted