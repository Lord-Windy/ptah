# Task 09: Implement Type-Safe Macro Layer

## Objective
Create optional type-safe macro wrappers for compile-time type checking.

## Current State
- Only void* interface available
- No compile-time type safety

## Implementation Steps
1. Create macro definition system in datazoo.h:
   ```c
   #define HONEYCOMB_DECLARE(name, value_type)
   #define HONEYCOMB_DEFINE(name, value_type)
   ```

2. Generate type-safe wrapper functions:
   - Typed create function
   - Typed put with value_type parameter
   - Typed get returning value_type*
   - Typed remove function
   - Typed iterator functions

3. Implementation approach:
   - Use token pasting to create unique function names
   - Wrap underlying void* implementation
   - Provide type casting in wrapper functions

## Example Macro Usage
```c
// In header file
HONEYCOMB_DECLARE(IntMap, int)

// In source file  
HONEYCOMB_DEFINE(IntMap, int)

// Usage
Samrena *arena = samrena_allocate(1000);
IntMap *map = intmap_create(16, 0.75, arena);
intmap_put(map, "count", 42);
int *value = intmap_get(map, "count");
```

## Fallback Plan
- If macro approach proves too complex or unmaintainable
- Document clear void* API usage patterns
- Provide example code for safe casting

## Verification
- Test macro with various types (int, struct, pointers)
- Verify compile-time type checking works
- Ensure no runtime overhead from wrappers