# Task 04: Implement Capacity Functions

## Objective
Implement the capacity management API from Task 03, focusing on efficient memory management while respecting arena constraints and growth strategies.

## Dependencies
- Task 03: Design Capacity Management API (must be completed)
- Understanding of current arena reallocation capabilities
- Updated SamrenaVector structure with growth configuration

## Implementation Plan

### 1. Update Vector Structure
```c
// samrena_vector.h - Add to existing SamrenaVector struct
typedef struct {
    uint64_t size;
    uint64_t element_size;
    uint64_t capacity;
    void *data;
    Samrena *arena;
    bool owns_arena;
    // New fields for capacity management
    float growth_factor;    // Default: 1.5f
    size_t min_growth;      // Default: 8
    bool can_shrink;        // Depends on arena type
} SamrenaVector;
```

### 2. Growth Calculation Helper
```c
// Internal helper for calculating new capacity
static size_t calculate_new_capacity(const SamrenaVector* vec, size_t required) {
    if (required <= vec->capacity) return vec->capacity;
    
    size_t growth_by_factor = (size_t)(vec->capacity * vec->growth_factor);
    size_t growth_by_min = vec->capacity + vec->min_growth;
    size_t new_capacity = (growth_by_factor > growth_by_min) ? 
                         growth_by_factor : growth_by_min;
    
    return (new_capacity > required) ? new_capacity : required;
}
```

### 3. Reserve Implementation
```c
int samrena_vector_reserve(SamrenaVector* vec, size_t min_capacity) {
    if (!vec) return SAMRENA_ERROR_NULL_POINTER;
    if (min_capacity <= vec->capacity) return SAMRENA_SUCCESS;
    
    size_t new_capacity = calculate_new_capacity(vec, min_capacity);
    size_t new_size = new_capacity * vec->element_size;
    
    // Attempt reallocation through arena
    void* new_data;
    if (vec->owns_arena) {
        new_data = samrena_realloc(vec->arena, vec->data, new_size);
    } else {
        // External arena - allocate new and copy
        new_data = samrena_alloc(vec->arena, new_size);
        if (new_data && vec->data) {
            memcpy(new_data, vec->data, vec->size * vec->element_size);
        }
    }
    
    if (!new_data) return SAMRENA_ERROR_ALLOCATION_FAILED;
    
    vec->data = new_data;
    vec->capacity = new_capacity;
    return SAMRENA_SUCCESS;
}
```

### 4. Shrink Implementation
```c
int samrena_vector_shrink_to_fit(SamrenaVector* vec) {
    if (!vec || !vec->can_shrink) return SAMRENA_SUCCESS;
    if (vec->size == vec->capacity) return SAMRENA_SUCCESS;
    
    if (vec->size == 0) {
        // Free all data if empty
        if (vec->owns_arena) {
            samrena_free(vec->arena, vec->data);
        }
        vec->data = NULL;
        vec->capacity = 0;
        return SAMRENA_SUCCESS;
    }
    
    size_t new_size = vec->size * vec->element_size;
    void* new_data = samrena_realloc(vec->arena, vec->data, new_size);
    
    if (new_data) {
        vec->data = new_data;
        vec->capacity = vec->size;
    }
    
    return SAMRENA_SUCCESS; // Don't fail if shrink can't happen
}
```

### 5. Clear and Truncate
```c
void samrena_vector_clear(SamrenaVector* vec) {
    if (vec) {
        vec->size = 0;
    }
}

int samrena_vector_truncate(SamrenaVector* vec, size_t new_size) {
    if (!vec) return SAMRENA_ERROR_NULL_POINTER;
    if (new_size > vec->size) return SAMRENA_ERROR_OUT_OF_BOUNDS;
    
    vec->size = new_size;
    return SAMRENA_SUCCESS;
}

int samrena_vector_reset(SamrenaVector* vec, size_t initial_capacity) {
    if (!vec) return SAMRENA_ERROR_NULL_POINTER;
    
    samrena_vector_clear(vec);
    
    if (initial_capacity != vec->capacity) {
        return samrena_vector_set_capacity(vec, initial_capacity);
    }
    
    return SAMRENA_SUCCESS;
}
```

### 6. Query Functions
```c
static inline size_t samrena_vector_size(const SamrenaVector* vec) {
    return vec ? vec->size : 0;
}

static inline size_t samrena_vector_capacity(const SamrenaVector* vec) {
    return vec ? vec->capacity : 0;
}

static inline bool samrena_vector_is_empty(const SamrenaVector* vec) {
    return !vec || vec->size == 0;
}

static inline bool samrena_vector_is_full(const SamrenaVector* vec) {
    return vec && vec->size == vec->capacity;
}

static inline size_t samrena_vector_available(const SamrenaVector* vec) {
    return vec ? (vec->capacity - vec->size) : 0;
}
```

### 7. Growth Configuration
```c
void samrena_vector_set_growth_factor(SamrenaVector* vec, float factor) {
    if (vec && factor >= 1.0f) {
        vec->growth_factor = factor;
    }
}

void samrena_vector_set_min_growth(SamrenaVector* vec, size_t min_elements) {
    if (vec) {
        vec->min_growth = min_elements;
    }
}

SamrenaVectorStats samrena_vector_get_stats(const SamrenaVector* vec) {
    SamrenaVectorStats stats = {0};
    if (vec) {
        stats.used_bytes = vec->size * vec->element_size;
        stats.allocated_bytes = vec->capacity * vec->element_size;
        stats.wasted_bytes = stats.allocated_bytes - stats.used_bytes;
        stats.utilization = stats.allocated_bytes > 0 ? 
            (float)stats.used_bytes / stats.allocated_bytes : 0.0f;
    }
    return stats;
}
```

## Arena Integration Strategy

### Detecting Arena Capabilities
```c
// During vector initialization
static bool arena_supports_shrinking(Samrena* arena) {
    // Check arena type/flags to determine if shrinking is supported
    // This depends on the arena implementation
    return arena->type == SAMRENA_TYPE_DYNAMIC;
}
```

### Reallocation Handling
1. Try realloc first (if supported)
2. Fall back to alloc + copy + free
3. Handle allocation failures gracefully
4. Update metadata only on success

## Testing Strategy

### Unit Tests
1. **Capacity Growth**
   - Reserve beyond current capacity
   - Multiple reserve calls
   - Growth factor effectiveness

2. **Shrinking Operations**
   - Shrink to fit after removals
   - Shrink on empty vector
   - Arena capability respect

3. **Configuration**
   - Growth factor changes
   - Minimum growth settings
   - Statistics accuracy

### Performance Tests
- Reserve vs repeated push performance
- Memory fragmentation with shrinking
- Statistics collection overhead

## Integration Notes
- Update init functions to set default growth parameters
- Ensure thread safety if required
- Consider alignment requirements for performance
- Document arena compatibility requirements