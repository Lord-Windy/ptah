# Task 05: Design Memory Ownership Model

## Objective
Design a flexible memory ownership model that allows vectors to either manage their own memory arenas or use externally provided arenas, improving API usability and memory management flexibility.

## Requirements
- Support for owned vs shared arena models
- Clear ownership semantics and lifecycle management
- Backward compatibility with existing API
- Efficient arena usage for both patterns
- Safe cleanup and destruction

## Proposed Ownership Models

### 1. Owned Arena Model
Vector creates and manages its own arena:
```c
// Vector owns arena and manages its lifecycle
SamrenaVector* samrena_vector_init_owned(size_t element_size, size_t initial_capacity);

// Operations don't require arena parameter
int samrena_vector_push_owned(SamrenaVector* vec, const void* element);
int samrena_vector_reserve_owned(SamrenaVector* vec, size_t capacity);

// Vector cleanup includes arena destruction
void samrena_vector_destroy(SamrenaVector* vec);
```

### 2. Shared Arena Model
Vector uses externally managed arena:
```c
// Vector uses provided arena (existing API)
SamrenaVector* samrena_vector_init_with_arena(Samrena* arena, size_t element_size, size_t initial_capacity);

// Operations require arena parameter (backward compatible)
int samrena_vector_push(Samrena* arena, SamrenaVector* vec, const void* element);
int samrena_vector_reserve(SamrenaVector* vec, size_t capacity); // Arena from vec context
```

### 3. Hybrid Operations
Functions that work with both models:
```c
// Automatically detect ownership and use appropriate arena
int samrena_vector_push_auto(SamrenaVector* vec, const void* element);
int samrena_vector_reserve_auto(SamrenaVector* vec, size_t capacity);

// Explicit arena override (useful for migration)
int samrena_vector_push_with_arena(Samrena* arena, SamrenaVector* vec, const void* element);
```

## Updated Vector Structure

```c
typedef struct {
    uint64_t size;
    uint64_t element_size;
    uint64_t capacity;
    void *data;
    Samrena *arena;         // Arena used for allocations
    bool owns_arena;        // True if vector should destroy arena
    float growth_factor;
    size_t min_growth;
    bool can_shrink;
    // Optional: reference counting for shared arenas
    uint32_t *arena_ref_count;
} SamrenaVector;
```

## API Design Patterns

### Initialization Patterns
```c
// Pattern 1: Owned arena (recommended for simple use cases)
SamrenaVector* vec1 = samrena_vector_init_owned(sizeof(int), 10);

// Pattern 2: Shared arena (recommended for bulk operations)
Samrena* shared_arena = samrena_create(1024 * 1024);
SamrenaVector* vec2 = samrena_vector_init_with_arena(shared_arena, sizeof(int), 10);
SamrenaVector* vec3 = samrena_vector_init_with_arena(shared_arena, sizeof(float), 20);

// Pattern 3: Migration from existing code (compatible)
SamrenaVector* vec4 = samrena_vector_init(shared_arena, sizeof(double), 5); // existing API
```

### Operation Patterns
```c
// Owned vectors - simple API
samrena_vector_push_owned(vec1, &value);

// Shared vectors - arena-aware
samrena_vector_push(shared_arena, vec2, &value);
// or
samrena_vector_push_auto(vec2, &value); // uses vec2->arena

// Cleanup patterns
samrena_vector_destroy(vec1);        // destroys vector and owned arena
samrena_vector_destroy(vec2);        // destroys vector only
samrena_vector_destroy(vec3);        // destroys vector only
samrena_destroy(shared_arena);       // destroys shared arena
```

## Memory Safety Guarantees

### Owned Arena Safety
- Vector has exclusive access to arena
- Arena lifetime matches vector lifetime
- No dangling pointer concerns
- Automatic cleanup on destruction

### Shared Arena Safety
- Multiple vectors can share arena safely
- Arena must outlive all vectors using it
- Optional reference counting for automatic cleanup
- Clear documentation of lifetime requirements

### Migration Safety
- Existing code continues to work unchanged
- New functions are additive only
- Clear upgrade path from old to new API
- Runtime detection of ownership model

## Implementation Considerations

### Arena Sizing for Owned Vectors
```c
// Calculate appropriate arena size based on initial capacity and growth
static size_t calculate_arena_size(size_t element_size, size_t initial_capacity) {
    size_t base_size = element_size * initial_capacity;
    // Add overhead for growth (estimate 3x growth over lifetime)
    size_t estimated_max = base_size * 3;
    // Round up to reasonable minimum (4KB)
    return (estimated_max < 4096) ? 4096 : estimated_max;
}
```

### Arena Type Selection
```c
// Choose arena type based on use case
typedef enum {
    SAMRENA_VECTOR_ARENA_DYNAMIC,    // For owned vectors - supports realloc
    SAMRENA_VECTOR_ARENA_STACK,      // For temporary vectors
    SAMRENA_VECTOR_ARENA_POOL        // For shared vectors
} SamrenaVectorArenaType;
```

## Benefits Analysis

### Owned Arena Benefits
- **Simplicity**: Single object lifecycle
- **Safety**: No external dependencies
- **Performance**: Optimized for single vector
- **Usability**: Minimal API surface

### Shared Arena Benefits
- **Efficiency**: Reduced allocation overhead
- **Control**: Fine-grained memory management
- **Compatibility**: Works with existing arena types
- **Scalability**: Better for many small vectors

## Testing Strategy
- Memory leak detection for both models
- Arena lifetime management tests
- Migration compatibility tests
- Performance comparison between models
- Error handling in edge cases

## Documentation Requirements
- Clear ownership model explanation
- Migration guide from existing API
- Best practices for model selection
- Examples for common use cases
- Performance characteristics documentation