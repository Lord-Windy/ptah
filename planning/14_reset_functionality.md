# Task 14: Reset/Clear Functionality

## Objective
Add ability to reset an arena's allocation pointer without deallocating memory, allowing efficient reuse.

## Current State
- Only way to reuse arena memory is to deallocate and reallocate
- No support for frame-based or temporary allocation patterns
- Common pattern in game engines and rendering systems is not supported

## Implementation Steps
1. Add reset function to header:
   ```c
   void samrena_reset(Samrena *samrena);
   ```

2. Implement reset function in samrena.c:
   ```c
   void samrena_reset(Samrena *samrena) {
       if (!samrena) {
           return;
       }
       samrena->allocated = 0;
   }
   ```

3. Consider adding partial reset to a specific position:
   ```c
   void samrena_reset_to(Samrena *samrena, uint64_t position);
   ```

## Key Considerations
- Very simple implementation with high utility
- No memory is freed, just reused
- Caller responsible for ensuring no dangling pointers
- Document that all previous allocations become invalid

## Use Cases
- Per-frame allocations in game loops
- Temporary scratch space for algorithms
- Request-scoped allocations in servers
- Clearing between test cases

## Verification
- Test reset sets allocated to 0
- Test allocations work correctly after reset
- Test multiple reset cycles
- Verify capacity remains unchanged
- Test reset with NULL arena (should be no-op)