# Task: Define Operations Interface

## Overview
Create the SamrenaOps structure that defines the operations each adapter must implement. This is the core of the hexagonal architecture pattern.

## Requirements
- Complete set of arena operations
- Consistent function signatures
- Support for optional operations
- Clear lifecycle management

## Implementation Details

### 1. Define Operations Structure
```c
typedef struct {
    const char* name;  // Adapter name for debugging
    
    // Lifecycle operations
    SamrenaError (*create)(void** context, const void* config);
    void (*destroy)(void* context);
    
    // Core memory operations
    void* (*push)(void* context, uint64_t size);
    void* (*push_zero)(void* context, uint64_t size);
    
    // Query operations
    uint64_t (*allocated)(void* context);
    uint64_t (*capacity)(void* context);
    
    // Optional operations (can be NULL)
    SamrenaError (*reserve)(void* context, uint64_t min_capacity);
    void (*reset)(void* context);
    
    // Debug operations (optional)
    void (*dump_stats)(void* context, FILE* out);
} SamrenaOps;
```

### 2. Define Operation Helpers
```c
// Check if optional operation is available
#define SAMRENA_HAS_OP(arena, op) \
    ((arena)->impl->ops->op != NULL)

// Safe operation call with NULL check
#define SAMRENA_CALL_OP(arena, op, ...) \
    (SAMRENA_HAS_OP(arena, op) ? \
     (arena)->impl->ops->op((arena)->context, ##__VA_ARGS__) : \
     SAMRENA_ERROR_UNSUPPORTED_OPERATION)
```

## Location
- `libs/samrena/src/samrena_internal.h` - Operations structure
- `libs/samrena/include/samrena.h` - Public operation helpers

## Dependencies
- Task 01: Core interface types defined

## Verification
- [ ] All required operations included
- [ ] Optional operations clearly marked
- [ ] Consistent parameter ordering
- [ ] Return types appropriate for each operation

## Notes
- Keep required operations minimal
- Optional operations enable adapter-specific features
- Consider adding capability flags in the future