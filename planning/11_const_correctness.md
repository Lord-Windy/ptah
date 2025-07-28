# Task 11: Ensure Const Correctness

## Objective
Add proper const qualifiers throughout the API for safety and clarity.

## Current State
- May lack const qualifiers on read-only operations
- Unclear which operations modify the hash map

## Implementation Steps
1. Review all function signatures and add const where appropriate:
   - honeycomb_get: Should take `const Honeycomb *comb`
   - honeycomb_contains: Should take `const Honeycomb *comb`  
   - Iterator next: Should return `const char *key`
   - Any other read-only operations

2. Update function parameters:
   - Key parameters should be `const char *key` everywhere
   - Read-only struct parameters should be const

3. Internal functions:
   - Hash function should take `const char *key`
   - Comparison functions should use const parameters

## Example Changes
```c
// Before
void* honeycomb_get(Honeycomb *comb, char *key);

// After  
void* honeycomb_get(const Honeycomb *comb, const char *key);

// Iterator should return const key
bool honeycomb_iterator_next(HoneycombIterator *iter,
                            const char **key, void **value);
```

## Benefits
- Prevents accidental modifications
- Clearer API contracts
- Better compiler optimizations
- Allows passing const honeycomb to read operations

## Verification
- Ensure all tests still compile and pass
- Try passing const Honeycomb* to appropriate functions
- Verify no const-cast warnings from compiler