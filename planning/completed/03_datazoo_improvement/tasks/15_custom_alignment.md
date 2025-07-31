# Task 15: Custom Alignment Support

## Objective
Add explicit alignment control for allocations to support SIMD operations and hardware requirements.

## Current State
- `samrena_push` automatically calculates alignment based on size
- No way to request specific alignment (e.g., 16-byte for SSE, 32-byte for AVX)
- Current max alignment is `alignof(max_align_t)`

## Implementation Steps
1. Add aligned allocation function to header:
   ```c
   void *samrena_push_aligned(Samrena *samrena, uint64_t size, uint64_t alignment);
   ```

2. Implement validation and alignment logic:
   - Verify alignment is power of 2
   - Verify alignment is reasonable (e.g., <= PAGE_SIZE)
   - Calculate aligned offset from current position
   - Ensure space for both alignment padding and allocation

3. Consider updating existing `samrena_push` to use the new function:
   ```c
   void *samrena_push(Samrena *samrena, uint64_t size) {
       size_t alignment = /* current auto-calculation */;
       return samrena_push_aligned(samrena, size, alignment);
   }
   ```

## Key Considerations
- Alignment must be power of 2
- May waste more memory due to padding
- Document that alignment > size may be inefficient
- Consider capping maximum alignment

## Code Structure
```c
void *samrena_push_aligned(Samrena *samrena, uint64_t size, uint64_t alignment) {
    // Validate alignment is power of 2
    if (alignment & (alignment - 1)) {
        samrena_set_error(SAMRENA_ERROR_INVALID_SIZE);
        return NULL;
    }
    
    // Calculate aligned offset
    uint64_t aligned_offset = (samrena->allocated + alignment - 1) 
                              & ~(alignment - 1);
    // ... rest of allocation
}
```

## Verification
- Test various alignment values (1, 2, 4, 8, 16, 32, 64)
- Test non-power-of-2 alignment fails
- Test alignment larger than size
- Verify allocated memory is properly aligned
- Test interaction with automatic expansion (if implemented)