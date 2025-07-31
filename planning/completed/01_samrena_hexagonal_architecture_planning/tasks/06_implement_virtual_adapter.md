# Task: Implement Virtual Memory Adapter

## Overview
Create a high-performance adapter using virtual memory APIs to provide a contiguous memory arena with lazy physical page allocation.

## Requirements
- Reserve large virtual address space
- Commit physical pages on demand
- Platform-specific implementations
- Graceful fallback if unavailable

## Implementation Details

### 1. Platform Abstraction Layer
```c
// In src/adapter_virtual_platform.h
typedef struct {
    void* base;
    uint64_t reserved;
    uint64_t committed;
} VirtualMemory;

// Platform-specific functions
VirtualMemory* vm_reserve(uint64_t size);
bool vm_commit(VirtualMemory* vm, uint64_t offset, uint64_t size);
void vm_release(VirtualMemory* vm);
```

### 2. Windows Implementation
```c
// In src/adapter_virtual_win32.c
#ifdef _WIN32
VirtualMemory* vm_reserve(uint64_t size) {
    void* base = VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_READWRITE);
    if (!base) return NULL;
    
    VirtualMemory* vm = malloc(sizeof(VirtualMemory));
    vm->base = base;
    vm->reserved = size;
    vm->committed = 0;
    return vm;
}

bool vm_commit(VirtualMemory* vm, uint64_t offset, uint64_t size) {
    void* addr = (char*)vm->base + offset;
    void* result = VirtualAlloc(addr, size, MEM_COMMIT, PAGE_READWRITE);
    if (result) {
        vm->committed += size;
        return true;
    }
    return false;
}
#endif
```

### 3. POSIX Implementation
```c
// In src/adapter_virtual_posix.c
#ifndef _WIN32
VirtualMemory* vm_reserve(uint64_t size) {
    void* base = mmap(NULL, size, PROT_NONE, 
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (base == MAP_FAILED) return NULL;
    
    VirtualMemory* vm = malloc(sizeof(VirtualMemory));
    vm->base = base;
    vm->reserved = size;
    vm->committed = 0;
    return vm;
}

bool vm_commit(VirtualMemory* vm, uint64_t offset, uint64_t size) {
    void* addr = (char*)vm->base + offset;
    if (mprotect(addr, size, PROT_READ | PROT_WRITE) == 0) {
        vm->committed += size;
        return true;
    }
    return false;
}
#endif
```

### 4. Virtual Adapter Implementation
```c
typedef struct {
    VirtualMemory* vm;
    uint64_t used;
    uint64_t commit_size;
    bool track_stats;
} VirtualContext;

static void* virtual_push(void* context, uint64_t size) {
    VirtualContext* ctx = (VirtualContext*)context;
    
    // Check if we need to commit more memory
    uint64_t needed = ctx->used + size;
    if (needed > ctx->vm->committed) {
        uint64_t to_commit = ((needed - ctx->vm->committed + 
                              ctx->commit_size - 1) / 
                             ctx->commit_size) * ctx->commit_size;
        
        if (!vm_commit(ctx->vm, ctx->vm->committed, to_commit)) {
            return NULL;  // Out of memory
        }
    }
    
    void* result = (char*)ctx->vm->base + ctx->used;
    ctx->used += size;
    return result;
}
```

## Location
- `libs/samrena/src/adapter_virtual.c` - Main adapter
- `libs/samrena/src/adapter_virtual_platform.h` - Platform interface
- `libs/samrena/src/adapter_virtual_win32.c` - Windows implementation
- `libs/samrena/src/adapter_virtual_posix.c` - POSIX implementation

## Dependencies
- Tasks 01-03: Core interfaces defined

## Verification
- [ ] Works on Windows (VirtualAlloc)
- [ ] Works on Linux (mmap)
- [ ] Works on macOS (mmap)
- [ ] Commits memory lazily
- [ ] Handles out-of-memory gracefully

## Notes
- Default to 2GB reserved space on 64-bit
- Consider huge page support
- Test with address sanitizers