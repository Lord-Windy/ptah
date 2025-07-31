# Task 06: Implement Memory Ownership

## Objective
Implement the memory ownership model designed in Task 05, providing both owned and shared arena patterns with proper lifecycle management and safety guarantees.

## Dependencies
- Task 05: Design Memory Ownership Model (must be completed)
- Understanding of current arena creation and destruction APIs
- Updated SamrenaVector structure with ownership fields

## Implementation Plan

### 1. Arena Size Calculation
```c
// Internal helper for owned arena sizing
static size_t calculate_arena_size(size_t element_size, size_t initial_capacity) {
    // Base size for initial capacity
    size_t base_size = element_size * initial_capacity;
    
    // Add growth headroom (estimate 4x total growth)
    size_t with_growth = base_size * 4;
    
    // Minimum arena size for efficiency
    const size_t min_arena_size = 4096;  // 4KB
    const size_t max_arena_size = 1024 * 1024;  // 1MB default max
    
    if (with_growth < min_arena_size) return min_arena_size;
    if (with_growth > max_arena_size) return max_arena_size;
    
    // Round up to nearest power of 2 for alignment
    size_t rounded = 1;
    while (rounded < with_growth) rounded <<= 1;
    
    return rounded;
}
```

### 2. Owned Arena Initialization
```c
SamrenaVector* samrena_vector_init_owned(size_t element_size, size_t initial_capacity) {
    if (element_size == 0) return NULL;
    
    // Calculate and create appropriately sized arena
    size_t arena_size = calculate_arena_size(element_size, initial_capacity);
    Samrena* arena = samrena_create(arena_size);
    if (!arena) return NULL;
    
    // Initialize vector with owned arena
    SamrenaVector* vec = samrena_vector_init_with_arena(arena, element_size, initial_capacity);
    if (!vec) {
        samrena_destroy(arena);
        return NULL;
    }
    
    // Mark arena as owned
    vec->owns_arena = true;
    vec->can_shrink = true;  // Owned arenas can be resized
    
    return vec;
}
```

### 3. Enhanced Shared Arena Initialization
```c
SamrenaVector* samrena_vector_init_with_arena(Samrena* arena, size_t element_size, size_t initial_capacity) {
    if (!arena || element_size == 0) return NULL;
    
    // Allocate vector structure from arena
    SamrenaVector* vec = samrena_alloc(arena, sizeof(SamrenaVector));
    if (!vec) return NULL;
    
    // Initialize vector fields
    vec->size = 0;
    vec->element_size = element_size;
    vec->capacity = initial_capacity;
    vec->arena = arena;
    vec->owns_arena = false;
    vec->growth_factor = 1.5f;
    vec->min_growth = 8;
    vec->can_shrink = samrena_supports_realloc(arena);
    
    // Allocate initial data buffer
    if (initial_capacity > 0) {
        vec->data = samrena_alloc(arena, initial_capacity * element_size);
        if (!vec->data) {
            // Don't free vec - it's in the arena
            return NULL;
        }
    } else {
        vec->data = NULL;
    }
    
    return vec;
}
```

### 4. Backward Compatible Initialization
```c
// Keep existing API for compatibility
SamrenaVector* samrena_vector_init(Samrena* arena, size_t element_size, size_t initial_capacity) {
    return samrena_vector_init_with_arena(arena, element_size, initial_capacity);
}
```

### 5. Auto-Detecting Operations
```c
int samrena_vector_push_auto(SamrenaVector* vec, const void* element) {
    if (!vec || !element) return SAMRENA_ERROR_NULL_POINTER;
    
    // Use the vector's arena regardless of ownership
    return samrena_vector_push_with_arena(vec->arena, vec, element);
}

int samrena_vector_reserve_auto(SamrenaVector* vec, size_t capacity) {
    if (!vec) return SAMRENA_ERROR_NULL_POINTER;
    
    // Check if growth is needed
    if (capacity <= vec->capacity) return SAMRENA_SUCCESS;
    
    // Use vector's arena for reallocation
    return samrena_vector_reserve_with_arena(vec->arena, vec, capacity);
}
```

### 6. Owned Vector Convenience Operations
```c
int samrena_vector_push_owned(SamrenaVector* vec, const void* element) {
    if (!vec || !vec->owns_arena) return SAMRENA_ERROR_INVALID_OPERATION;
    return samrena_vector_push_auto(vec, element);
}

int samrena_vector_reserve_owned(SamrenaVector* vec, size_t capacity) {
    if (!vec || !vec->owns_arena) return SAMRENA_ERROR_INVALID_OPERATION;
    return samrena_vector_reserve_auto(vec, capacity);
}
```

### 7. Destruction and Cleanup
```c
void samrena_vector_destroy(SamrenaVector* vec) {
    if (!vec) return;
    
    if (vec->owns_arena) {
        // Destroy the entire arena (includes vector and data)
        samrena_destroy(vec->arena);
    } else {
        // Vector allocated in external arena - just mark as invalid
        // Note: Can't actually free from arena-based allocation
        // External arena owner is responsible for cleanup
        vec->data = NULL;
        vec->size = 0;
        vec->capacity = 0;
        vec->arena = NULL;
    }
}

// Alternative for shared arena vectors - explicit cleanup
void samrena_vector_cleanup(SamrenaVector* vec) {
    if (!vec || vec->owns_arena) return;
    
    // Clear vector but don't attempt to free arena memory
    vec->data = NULL;
    vec->size = 0;
    vec->capacity = 0;
}
```

### 8. Ownership Query Functions
```c
bool samrena_vector_owns_arena(const SamrenaVector* vec) {
    return vec && vec->owns_arena;
}

Samrena* samrena_vector_get_arena(const SamrenaVector* vec) {
    return vec ? vec->arena : NULL;
}

bool samrena_vector_can_grow(const SamrenaVector* vec) {
    return vec && vec->arena && samrena_has_space(vec->arena);
}
```

### 9. Migration Helpers
```c
// Convert shared vector to owned (creates new arena and copies data)
SamrenaVector* samrena_vector_make_owned(const SamrenaVector* shared_vec) {
    if (!shared_vec || shared_vec->owns_arena) return NULL;
    
    // Create new owned vector with same configuration
    SamrenaVector* owned = samrena_vector_init_owned(
        shared_vec->element_size, 
        shared_vec->capacity
    );
    if (!owned) return NULL;
    
    // Copy configuration
    owned->growth_factor = shared_vec->growth_factor;
    owned->min_growth = shared_vec->min_growth;
    
    // Copy data
    if (shared_vec->size > 0) {
        memcpy(owned->data, shared_vec->data, 
               shared_vec->size * shared_vec->element_size);
        owned->size = shared_vec->size;
    }
    
    return owned;
}
```

## Error Handling Strategy

### New Error Codes
```c
// Add to existing error enumeration
SAMRENA_ERROR_INVALID_OPERATION = -5,  // Operation not valid for ownership model
SAMRENA_ERROR_ARENA_EXHAUSTED = -6,    // Arena out of space
SAMRENA_ERROR_OWNERSHIP_CONFLICT = -7   // Ownership model mismatch
```

### Validation Functions
```c
static bool validate_vector_ownership(const SamrenaVector* vec, bool requires_owned) {
    if (!vec) return false;
    if (requires_owned && !vec->owns_arena) return false;
    if (!requires_owned && vec->owns_arena) return false;
    return true;
}
```

## Testing Strategy

### Unit Tests Required
1. **Ownership Model Tests**
   - Owned arena creation and destruction
   - Shared arena usage patterns
   - Mixed ownership scenarios

2. **Memory Safety Tests**
   - Proper cleanup in both models
   - No memory leaks
   - Double-free protection

3. **API Compatibility Tests**
   - Existing code still works
   - New functions integrate properly
   - Migration path validation

4. **Error Handling Tests**
   - Invalid ownership operations
   - Arena exhaustion scenarios
   - Null pointer safety

### Integration Tests
- Multiple vectors sharing one arena
- Arena lifetime management
- Performance impact of ownership tracking

## Performance Considerations
- Owned arenas optimized for single vector
- Shared arenas benefit from bulk allocation
- Minimal overhead for ownership tracking
- Inline functions for hot paths

## Documentation Updates
- API reference for new functions
- Ownership model explanation
- Migration examples
- Best practices guide