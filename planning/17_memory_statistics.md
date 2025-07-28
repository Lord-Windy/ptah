# Task 17: Memory Statistics and Debugging

## Objective
Add optional instrumentation to track memory usage patterns for debugging and optimization.

## Current State
- No visibility into allocation patterns
- No way to track peak usage or fragmentation
- Difficult to debug memory-related issues

## Implementation Steps
1. Add statistics structure to header:
   ```c
   typedef struct {
       uint64_t total_allocations;
       uint64_t peak_usage;
       uint64_t current_usage;
       uint64_t wasted_bytes;    // Due to alignment
   } SamrenaStats;
   ```

2. Add compile-time flag for statistics:
   ```c
   #ifdef SAMRENA_ENABLE_STATS
   // Statistics tracking code
   #endif
   ```

3. Add statistics field to Samrena structure (conditionally):
   ```c
   #ifdef SAMRENA_ENABLE_STATS
       SamrenaStats stats;
   #endif
   ```

4. Add function to retrieve statistics:
   ```c
   SamrenaStats samrena_get_stats(Samrena *samrena);
   ```

5. Update allocation functions to track statistics when enabled

## Key Considerations
- Should be compile-time optional to avoid overhead
- Track both successful and failed allocations
- Consider thread safety if statistics are shared
- Minimal impact on performance when disabled

## Statistics to Track
- Total number of allocations
- Peak memory usage
- Current usage vs capacity
- Bytes wasted due to alignment
- Number of failed allocations

## Verification
- Test statistics are updated correctly
- Test compilation with and without statistics enabled
- Verify minimal performance impact when disabled
- Test statistics accuracy with various allocation patterns