# Task 01: Define Element Access Interface

## Objective
Design a comprehensive API for safe and efficient element access in Samvector, providing both bounds-checked and unchecked variants for different use cases.

## Requirements
- Bounds-checked access methods that return error codes
- Unchecked access for performance-critical paths
- Const-correct API for read-only operations
- Clear naming conventions distinguishing safe vs unsafe operations

## Proposed API

### Safe Access Functions
```c
// Get element at index with bounds checking
// Returns: SAMRENA_SUCCESS or SAMRENA_ERROR_OUT_OF_BOUNDS
int samrena_vector_get(const SamrenaVector* vec, size_t index, void* out_element);

// Set element at index with bounds checking
// Returns: SAMRENA_SUCCESS or SAMRENA_ERROR_OUT_OF_BOUNDS
int samrena_vector_set(SamrenaVector* vec, size_t index, const void* element);

// Get pointer to element at index (returns NULL on bounds error)
void* samrena_vector_at(SamrenaVector* vec, size_t index);
const void* samrena_vector_at_const(const SamrenaVector* vec, size_t index);
```

### Convenience Access Functions
```c
// Get first element (returns NULL if empty)
void* samrena_vector_front(SamrenaVector* vec);
const void* samrena_vector_front_const(const SamrenaVector* vec);

// Get last element (returns NULL if empty)
void* samrena_vector_back(SamrenaVector* vec);
const void* samrena_vector_back_const(const SamrenaVector* vec);

// Get raw data pointer (never NULL after init)
void* samrena_vector_data(SamrenaVector* vec);
const void* samrena_vector_data_const(const SamrenaVector* vec);
```

### Unsafe Access Functions (Performance)
```c
// Unchecked access - caller ensures valid index
void* samrena_vector_at_unchecked(SamrenaVector* vec, size_t index);
const void* samrena_vector_at_unchecked_const(const SamrenaVector* vec, size_t index);

// Direct element access via macro
#define SAMRENA_VECTOR_ELEM(vec, type, index) \
    ((type*)((vec)->data))[(index)]
```

## Design Decisions

### Error Handling Strategy
- Safe functions return error codes for consistency
- Pointer-returning functions use NULL for errors
- Unsafe functions have no error checking

### Naming Conventions
- `_const` suffix for const-correct variants
- `_unchecked` suffix for unsafe operations
- Simple names for most common operations

### Memory Safety
- All safe functions validate vector pointer
- Bounds checking on all indexed operations
- Clear documentation of caller responsibilities

## Implementation Notes
- Use inline functions for performance
- Ensure proper alignment for element access
- Consider cache-friendly access patterns
- Add debug assertions in unsafe functions

## Testing Requirements
- Unit tests for all access methods
- Bounds checking verification
- Performance benchmarks safe vs unsafe
- Const-correctness compilation tests

## Documentation
- Clear examples for each function
- Performance characteristics documented
- Safety guarantees explicitly stated
- Migration guide from direct data access