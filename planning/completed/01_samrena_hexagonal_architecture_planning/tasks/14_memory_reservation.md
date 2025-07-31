# Task: Implement Memory Reservation

## Overview
Add optional memory reservation operation that allows pre-allocating space for future allocations, improving performance for known workloads.

## Requirements
- Optional operation (not all adapters support)
- Hint about future memory needs
- No guarantee of contiguous reservation
- Performance optimization only

## Implementation Details

### 1. Reserve Operation API
```c
// In include/samrena.h
// Reserve space for at least 'size' bytes
SamrenaError samrena_reserve(Samrena* arena, uint64_t size);

// Reserve with growth hint
SamrenaError samrena_reserve_with_growth(
    Samrena* arena, 
    uint64_t immediate_size,
    uint64_t expected_total
);
```

### 2. Chained Adapter Implementation
```c
// In src/adapter_chained.c
static SamrenaError chained_reserve(void* context, uint64_t min_capacity) {
    ChainedContext* ctx = (ChainedContext*)context;
    
    // Calculate current capacity
    uint64_t current_capacity = 0;
    for (ChainedPage* page = ctx->head; page; page = page->next) {
        current_capacity += page->size;
    }
    
    if (current_capacity >= min_capacity) {
        return SAMRENA_OK;  // Already have enough
    }
    
    // Pre-allocate pages to meet capacity
    uint64_t needed = min_capacity - current_capacity;
    while (needed > 0) {
        uint64_t page_size = ctx->growth_pages * ctx->page_size;
        if (page_size > needed) {
            page_size = needed;
        }
        
        ChainedPage* page = chained_add_page(ctx, page_size);
        if (!page) {
            return SAMRENA_ERROR_OUT_OF_MEMORY;
        }
        
        needed -= page_size;
    }
    
    return SAMRENA_OK;
}
```

### 3. Virtual Adapter Implementation
```c
// In src/adapter_virtual.c
static SamrenaError virtual_reserve(void* context, uint64_t min_capacity) {
    VirtualContext* ctx = (VirtualContext*)context;
    
    // Check if within reserved space
    if (min_capacity > ctx->vm->reserved) {
        return SAMRENA_ERROR_INVALID_PARAMETER;  // Cannot exceed reservation
    }
    
    // Commit pages if needed
    if (min_capacity > ctx->vm->committed) {
        uint64_t to_commit = min_capacity - ctx->vm->committed;
        to_commit = ((to_commit + ctx->commit_size - 1) / 
                     ctx->commit_size) * ctx->commit_size;
        
        if (!vm_commit(ctx->vm, ctx->vm->committed, to_commit)) {
            return SAMRENA_ERROR_OUT_OF_MEMORY;
        }
    }
    
    // Optional: Prefetch pages
#ifdef _WIN32
    WIN32_MEMORY_RANGE_ENTRY entry = {
        .VirtualAddress = ctx->vm->base,
        .NumberOfBytes = min_capacity
    };
    PrefetchVirtualMemory(GetCurrentProcess(), 1, &entry, 0);
#elif defined(__linux__)
    madvise(ctx->vm->base, min_capacity, MADV_WILLNEED);
#endif
    
    return SAMRENA_OK;
}
```

### 4. Reserve with Growth Pattern
```c
SamrenaError samrena_reserve_with_growth(
    Samrena* arena, 
    uint64_t immediate_size,
    uint64_t expected_total
) {
    if (!arena || !arena->impl) {
        return SAMRENA_ERROR_INVALID_PARAMETER;
    }
    
    // Store growth hint for future allocations
    arena->impl->growth_hint = expected_total;
    
    // Reserve immediate size plus some headroom
    uint64_t reserve_size = immediate_size * 2;
    if (reserve_size < expected_total / 4) {
        reserve_size = expected_total / 4;  // Reserve 25% of expected
    }
    
    if (arena->impl->ops->reserve) {
        return arena->impl->ops->reserve(arena->context, reserve_size);
    }
    
    // If reserve not supported, just return OK
    return SAMRENA_OK;
}
```

### 5. Integration with Push Operations
```c
// Modified push operation that considers reservation
void* samrena_push_with_reserve(Samrena* arena, uint64_t size) {
    void* result = samrena_push(arena, size);
    
    if (!result && arena->impl->ops->reserve) {
        // Try to reserve more space
        uint64_t current = samrena_allocated(arena);
        uint64_t new_capacity = (current + size) * 2;
        
        if (arena->impl->growth_hint > new_capacity) {
            new_capacity = arena->impl->growth_hint;
        }
        
        if (samrena_reserve(arena, new_capacity) == SAMRENA_OK) {
            // Retry allocation
            result = samrena_push(arena, size);
        }
    }
    
    return result;
}
```

## Location
- `libs/samrena/src/adapter_chained.c` - Chained implementation
- `libs/samrena/src/adapter_virtual.c` - Virtual implementation
- `libs/samrena/include/samrena.h` - Public API

## Dependencies
- Tasks 05-06: Adapter implementations complete

## Verification
- [ ] Reserve pre-allocates memory
- [ ] Performance improves with reservation
- [ ] Graceful handling when unsupported
- [ ] Growth hints respected
- [ ] Memory prefetch works on supported platforms

## Notes
- Reserve is a hint, not a guarantee
- Consider memory pressure before reserving
- Document performance implications