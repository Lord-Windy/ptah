# Task 20: Custom Memory Source Support

## Objective
Allow alternative backing allocators instead of hardcoded malloc/free for specialized memory regions.

## Current State
- Arena memory is always allocated via malloc
- No support for custom memory sources (mmap, pools, etc.)
- Limits usage in embedded or specialized environments

## Implementation Steps
1. Define allocator interface in header:
   ```c
   typedef struct {
       void *(*alloc)(size_t size, void *context);
       void (*free)(void *ptr, void *context);
       void *context;
   } SamrenaAllocator;
   ```

2. Add default allocator implementation:
   ```c
   extern const SamrenaAllocator SAMRENA_DEFAULT_ALLOCATOR;
   ```

3. Modify Samrena structure to store allocator:
   ```c
   typedef struct {
       uint8_t *bytes;
       uint64_t allocated;
       uint64_t capacity;
       SamrenaAllocator allocator;  // Add this field
   } Samrena;
   ```

4. Add new initialization function:
   ```c
   Samrena *samrena_allocate_with_allocator(uint64_t page_count, 
                                            const SamrenaAllocator *allocator);
   ```

5. Update existing functions to use stored allocator

## Key Considerations
- Maintain backward compatibility with existing `samrena_allocate`
- Handle allocator failure gracefully
- Document allocator interface requirements
- Consider allocator context lifetime

## Use Cases
- Memory-mapped files for persistence
- Custom memory pools
- Embedded systems with special memory regions
- Debug allocators with tracking

## Code Structure
```c
static void *default_alloc(size_t size, void *context) {
    (void)context;
    return malloc(size);
}

static void default_free(void *ptr, void *context) {
    (void)context;
    free(ptr);
}

const SamrenaAllocator SAMRENA_DEFAULT_ALLOCATOR = {
    .alloc = default_alloc,
    .free = default_free,
    .context = NULL
};
```

## Verification
- Test with default allocator (should match current behavior)
- Test with custom allocator implementation
- Test allocator failure handling
- Verify context is passed correctly to allocator functions