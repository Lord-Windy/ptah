# Task 16: Save/Restore Points

## Objective
Enable saving and restoring arena allocation state for temporary allocation patterns and speculative operations.

## Current State
- No way to temporarily allocate memory and then revert
- Common pattern in parsers and algorithms not supported
- Must manually track allocation positions

## Implementation Steps
1. Add savepoint type to header:
   ```c
   typedef struct {
       uint64_t allocated;
   } SamrenaSavepoint;
   ```

2. Add save/restore functions:
   ```c
   SamrenaSavepoint samrena_save(Samrena *samrena);
   void samrena_restore(Samrena *samrena, SamrenaSavepoint savepoint);
   ```

3. Implement save function:
   ```c
   SamrenaSavepoint samrena_save(Samrena *samrena) {
       SamrenaSavepoint savepoint = {0};
       if (samrena) {
           savepoint.allocated = samrena->allocated;
       }
       return savepoint;
   }
   ```

4. Implement restore function with validation:
   ```c
   void samrena_restore(Samrena *samrena, SamrenaSavepoint savepoint) {
       if (!samrena) return;
       if (savepoint.allocated <= samrena->capacity) {
           samrena->allocated = savepoint.allocated;
       }
   }
   ```

## Key Considerations
- Savepoint only stores allocation position, not memory contents
- Caller responsible for not using memory after restore
- Should validate savepoint is from same arena (future enhancement)
- Consider stack-like save/restore semantics

## Use Cases
- Parser backtracking
- Speculative compilation
- Temporary calculations
- Error recovery in complex operations

## Verification
- Test save captures current state
- Test restore rewinds to saved position
- Test multiple nested save/restore pairs
- Test restore with invalid savepoint
- Test allocations work correctly after restore