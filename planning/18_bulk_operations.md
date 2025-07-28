# Task 18: Bulk Array Operations

## Objective
Add optimized functions for common array allocation patterns to improve efficiency and ergonomics.

## Current State
- Must calculate array sizes manually
- No guarantee of proper element alignment in arrays
- Repeated calls to `samrena_push` for array elements

## Implementation Steps
1. Add bulk allocation function to header:
   ```c
   void *samrena_push_array(Samrena *samrena, uint64_t count, uint64_t element_size);
   ```

2. Implement with overflow checks:
   ```c
   void *samrena_push_array(Samrena *samrena, uint64_t count, uint64_t element_size) {
       // Check for multiplication overflow
       if (count > 0 && element_size > UINT64_MAX / count) {
           samrena_set_error(SAMRENA_ERROR_OVERFLOW);
           return NULL;
       }
       uint64_t total_size = count * element_size;
       return samrena_push(samrena, total_size);
   }
   ```

3. Consider zero-initialized variant:
   ```c
   void *samrena_push_array_zero(Samrena *samrena, uint64_t count, uint64_t element_size);
   ```

4. Consider typed array macros for convenience:
   ```c
   #define SAMRENA_PUSH_ARRAY(arena, type, count) \
       ((type*)samrena_push_array(arena, count, sizeof(type)))
   ```

## Key Considerations
- Must check for multiplication overflow
- Should ensure proper alignment for element type
- More efficient than multiple individual allocations
- Document that count=0 is valid (returns valid pointer to 0-byte allocation)

## Use Cases
- Allocating arrays of structs
- Temporary buffers for algorithms
- Dynamic arrays that grow in chunks
- Zero-initialized arrays for safety

## Verification
- Test various count and element_size combinations
- Test overflow detection with large values
- Test zero count behavior
- Test alignment of returned arrays
- Compare performance vs repeated `samrena_push` calls