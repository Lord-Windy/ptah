# Task: Refactor Current Implementation

## Overview
Refactor the existing samrena implementation to work as the chained pages adapter within the new hexagonal architecture. This preserves current functionality while enabling the new design.

## Requirements
- Maintain exact current behavior
- Separate interface from implementation
- Prepare for multiple adapter support
- No breaking changes to existing API

## Implementation Steps

### 1. Move Implementation to Adapter File
- Create `libs/samrena/src/adapter_chained.c`
- Move current implementation from `samrena.c`
- Rename internal functions with `chained_` prefix

### 2. Implement Operations Interface
```c
static SamrenaError chained_create(void** context, const void* config) {
    const SamrenaConfig* cfg = (const SamrenaConfig*)config;
    // Allocate and initialize chained context
    // Use existing arena initialization code
}

static void* chained_push(void* context, uint64_t size) {
    // Use existing push implementation
}

// ... implement all required operations

static const SamrenaOps chained_ops = {
    .name = "chained",
    .create = chained_create,
    .destroy = chained_destroy,
    .push = chained_push,
    .push_zero = chained_push_zero,
    .allocated = chained_allocated,
    .capacity = chained_capacity,
    .reserve = NULL,  // Not supported yet
    .reset = NULL,    // Not supported yet
};
```

### 3. Update Main Implementation
```c
// In samrena.c
Samrena* samrena_allocate(uint64_t initial_pages) {
    SamrenaConfig config = samrena_default_config();
    config.initial_pages = initial_pages;
    config.strategy = SAMRENA_STRATEGY_CHAINED;
    return samrena_create(&config);
}
```

## Location
- `libs/samrena/src/adapter_chained.c` - New adapter file
- `libs/samrena/src/samrena.c` - Updated main file

## Dependencies
- Tasks 01-03: Interface and configuration defined

## Verification
- [ ] All existing tests pass unchanged
- [ ] No performance regression
- [ ] Clean separation of concerns
- [ ] Internal functions properly prefixed

## Notes
- This is a pure refactoring - no new features
- Keep git history clean with meaningful commits
- Document any subtle behavior that must be preserved