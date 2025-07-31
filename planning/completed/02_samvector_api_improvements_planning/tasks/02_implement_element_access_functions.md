# Task 02: Implement Element Access Functions

## Objective
Implement the element access API defined in Task 01, ensuring safe bounds checking, proper error handling, and optimal performance.

## Dependencies
- Task 01: Define Element Access Interface (must be completed)
- Existing samrena_vector structure and basic functions

## Implementation Plan

### 1. Update Error Codes
```c
// Add to samrena.h or create samrena_errors.h
typedef enum {
    SAMRENA_SUCCESS = 0,
    SAMRENA_ERROR_NULL_POINTER = -1,
    SAMRENA_ERROR_OUT_OF_BOUNDS = -2,
    SAMRENA_ERROR_ALLOCATION_FAILED = -3,
    SAMRENA_ERROR_INVALID_SIZE = -4
} SamrenaError;
```

### 2. Safe Access Implementation
```c
// samrena_vector.c
int samrena_vector_get(const SamrenaVector* vec, size_t index, void* out_element) {
    if (!vec || !out_element) return SAMRENA_ERROR_NULL_POINTER;
    if (index >= vec->size) return SAMRENA_ERROR_OUT_OF_BOUNDS;
    
    const char* src = (const char*)vec->data + (index * vec->element_size);
    memcpy(out_element, src, vec->element_size);
    return SAMRENA_SUCCESS;
}

int samrena_vector_set(SamrenaVector* vec, size_t index, const void* element) {
    if (!vec || !element) return SAMRENA_ERROR_NULL_POINTER;
    if (index >= vec->size) return SAMRENA_ERROR_OUT_OF_BOUNDS;
    
    char* dst = (char*)vec->data + (index * vec->element_size);
    memcpy(dst, element, vec->element_size);
    return SAMRENA_SUCCESS;
}
```

### 3. Pointer Access Implementation
```c
void* samrena_vector_at(SamrenaVector* vec, size_t index) {
    if (!vec || index >= vec->size) return NULL;
    return (char*)vec->data + (index * vec->element_size);
}

const void* samrena_vector_at_const(const SamrenaVector* vec, size_t index) {
    if (!vec || index >= vec->size) return NULL;
    return (const char*)vec->data + (index * vec->element_size);
}
```

### 4. Convenience Functions
```c
void* samrena_vector_front(SamrenaVector* vec) {
    return (vec && vec->size > 0) ? vec->data : NULL;
}

void* samrena_vector_back(SamrenaVector* vec) {
    if (!vec || vec->size == 0) return NULL;
    return (char*)vec->data + ((vec->size - 1) * vec->element_size);
}

void* samrena_vector_data(SamrenaVector* vec) {
    return vec ? vec->data : NULL;
}
```

### 5. Unsafe Performance Functions
```c
static inline void* samrena_vector_at_unchecked(SamrenaVector* vec, size_t index) {
    assert(vec != NULL);
    assert(index < vec->size);
    return (char*)vec->data + (index * vec->element_size);
}

static inline const void* samrena_vector_at_unchecked_const(const SamrenaVector* vec, size_t index) {
    assert(vec != NULL);
    assert(index < vec->size);
    return (const char*)vec->data + (index * vec->element_size);
}
```

## Testing Strategy

### Unit Tests Required
1. **Bounds Checking Tests**
   - Access at valid indices
   - Access beyond size
   - Access on empty vector

2. **Null Safety Tests**
   - NULL vector parameter
   - NULL output parameter
   - NULL element parameter

3. **Data Integrity Tests**
   - Set and get same value
   - Multiple elements
   - Different data types

4. **Edge Cases**
   - Single element vector
   - Maximum size vector
   - Zero-sized elements

### Performance Tests
- Benchmark safe vs unsafe access
- Cache miss analysis
- Comparison with raw array access

## Integration Points
- Update existing functions to use new error codes
- Ensure compatibility with current API
- Add inline hints for hot paths

## Documentation Updates
- Add examples to header file
- Update API documentation
- Create migration guide
- Document performance characteristics