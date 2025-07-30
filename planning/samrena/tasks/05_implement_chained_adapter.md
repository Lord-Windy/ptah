# Task: Complete Chained Pages Adapter

## Overview
Enhance the refactored chained pages adapter with all planned features, including growth policies, statistics, and optional operations.

## Requirements
- Dynamic page growth
- Handle allocations larger than page size
- Optional statistics tracking
- Reset capability

## Implementation Details

### 1. Enhanced Context Structure
```c
typedef struct ChainedPage {
    struct ChainedPage* next;
    uint64_t size;
    uint64_t used;
    uint8_t data[];
} ChainedPage;

typedef struct {
    ChainedPage* head;
    ChainedPage* current;
    uint64_t page_size;
    uint64_t growth_pages;
    
    // Statistics (optional)
    uint64_t total_allocated;
    uint64_t total_capacity;
    uint64_t allocation_count;
    uint64_t page_count;
    bool track_stats;
} ChainedContext;
```

### 2. Implement Growth Policy
```c
static ChainedPage* chained_add_page(ChainedContext* ctx, uint64_t min_size) {
    // Calculate size: max(min_size, growth_pages * page_size)
    uint64_t size = ctx->growth_pages * ctx->page_size;
    if (size < min_size) {
        size = min_size;  // Handle large allocations
    }
    
    ChainedPage* page = malloc(sizeof(ChainedPage) + size);
    if (!page) return NULL;
    
    page->size = size;
    page->used = 0;
    page->next = NULL;
    
    // Link to chain
    if (ctx->current) {
        ctx->current->next = page;
    } else {
        ctx->head = page;
    }
    ctx->current = page;
    
    // Update stats
    if (ctx->track_stats) {
        ctx->total_capacity += size;
        ctx->page_count++;
    }
    
    return page;
}
```

### 3. Implement Reset Operation
```c
static void chained_reset(void* context) {
    ChainedContext* ctx = (ChainedContext*)context;
    
    // Reset all pages to unused
    for (ChainedPage* page = ctx->head; page; page = page->next) {
        page->used = 0;
    }
    
    // Reset to first page
    ctx->current = ctx->head;
    
    // Reset statistics
    if (ctx->track_stats) {
        ctx->total_allocated = 0;
        ctx->allocation_count = 0;
    }
}
```

## Location
- `libs/samrena/src/adapter_chained.c` - Enhanced implementation

## Dependencies
- Task 04: Basic refactoring complete

## Verification
- [ ] Growth policy works correctly
- [ ] Large allocations handled properly
- [ ] Reset clears all allocations
- [ ] Statistics accurate when enabled
- [ ] No memory leaks

## Notes
- Consider alignment requirements
- Test edge cases (zero size, huge allocations)
- Document growth behavior clearly