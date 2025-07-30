# Task: Create Comprehensive Test Suite

## Overview
Develop a comprehensive test suite that validates all adapters against the same set of requirements, ensuring consistent behavior across implementations.

## Requirements
- Test all adapters with same test cases
- Validate edge cases and error conditions
- Performance benchmarks
- Memory leak detection
- Thread safety verification

## Implementation Details

### 1. Test Framework Structure
```c
// In test/test_samrena_adapters.c
typedef struct {
    const char* name;
    void (*test_func)(SamrenaStrategy strategy);
    bool (*should_run)(SamrenaStrategy strategy);
} AdapterTest;

typedef struct {
    SamrenaStrategy strategy;
    const char* name;
    bool available;
} AdapterInfo;

// Test runner
static void run_adapter_tests(void) {
    AdapterInfo adapters[] = {
        { SAMRENA_STRATEGY_CHAINED, "chained", true },
        { SAMRENA_STRATEGY_VIRTUAL, "virtual", 
          samrena_strategy_available(SAMRENA_STRATEGY_VIRTUAL) },
    };
    
    AdapterTest tests[] = {
        { "basic_allocation", test_basic_allocation, NULL },
        { "large_allocation", test_large_allocation, NULL },
        { "many_small_allocations", test_many_small_allocations, NULL },
        { "growth_behavior", test_growth_behavior, NULL },
        { "reset_operation", test_reset_operation, requires_reset },
        { "reserve_operation", test_reserve_operation, requires_reserve },
        { "edge_cases", test_edge_cases, NULL },
        { "thread_safety", test_thread_safety, NULL },
        { NULL, NULL, NULL }
    };
    
    for (size_t i = 0; i < sizeof(adapters)/sizeof(adapters[0]); i++) {
        if (!adapters[i].available) {
            printf("Skipping %s adapter (not available)\n", adapters[i].name);
            continue;
        }
        
        printf("\nTesting %s adapter:\n", adapters[i].name);
        
        for (AdapterTest* test = tests; test->name; test++) {
            if (test->should_run && !test->should_run(adapters[i].strategy)) {
                printf("  %s: SKIPPED\n", test->name);
                continue;
            }
            
            printf("  %s: ", test->name);
            fflush(stdout);
            
            test->test_func(adapters[i].strategy);
            printf("PASS\n");
        }
    }
}
```

### 2. Core Functionality Tests
```c
static void test_basic_allocation(SamrenaStrategy strategy) {
    SamrenaConfig config = samrena_default_config();
    config.strategy = strategy;
    
    Samrena* arena = samrena_create(&config);
    assert(arena != NULL);
    
    // Test various allocation sizes
    size_t sizes[] = {1, 8, 16, 32, 64, 128, 256, 512, 1024, 4096};
    void* ptrs[sizeof(sizes)/sizeof(sizes[0])];
    
    for (size_t i = 0; i < sizeof(sizes)/sizeof(sizes[0]); i++) {
        ptrs[i] = samrena_push(arena, sizes[i]);
        assert(ptrs[i] != NULL);
        
        // Verify memory is writable
        memset(ptrs[i], 0xAA, sizes[i]);
    }
    
    // Verify allocations don't overlap
    for (size_t i = 0; i < sizeof(sizes)/sizeof(sizes[0]); i++) {
        for (size_t j = i + 1; j < sizeof(sizes)/sizeof(sizes[0]); j++) {
            assert(ptrs[i] != ptrs[j]);
            assert((char*)ptrs[i] + sizes[i] <= (char*)ptrs[j] ||
                   (char*)ptrs[j] + sizes[j] <= (char*)ptrs[i]);
        }
    }
    
    samrena_destroy(arena);
}

static void test_large_allocation(SamrenaStrategy strategy) {
    SamrenaConfig config = samrena_default_config();
    config.strategy = strategy;
    config.initial_pages = 1;
    
    Samrena* arena = samrena_create(&config);
    assert(arena != NULL);
    
    // Allocate larger than initial page
    size_t large_size = config.page_size * 3;
    void* large_ptr = samrena_push(arena, large_size);
    assert(large_ptr != NULL);
    
    // Verify entire allocation is usable
    memset(large_ptr, 0xBB, large_size);
    
    // Verify we can still allocate after large allocation
    void* after = samrena_push(arena, 1024);
    assert(after != NULL);
    assert(after != large_ptr);
    
    samrena_destroy(arena);
}
```

### 3. Stress Tests
```c
static void test_many_small_allocations(SamrenaStrategy strategy) {
    SamrenaConfig config = samrena_default_config();
    config.strategy = strategy;
    
    Samrena* arena = samrena_create(&config);
    assert(arena != NULL);
    
    const size_t count = 10000;
    void** ptrs = malloc(count * sizeof(void*));
    
    // Allocate many small objects
    for (size_t i = 0; i < count; i++) {
        size_t size = (i % 256) + 1;  // 1-256 bytes
        ptrs[i] = samrena_push(arena, size);
        assert(ptrs[i] != NULL);
        
        // Write pattern
        memset(ptrs[i], i & 0xFF, size);
    }
    
    // Verify patterns
    for (size_t i = 0; i < count; i++) {
        size_t size = (i % 256) + 1;
        unsigned char* p = (unsigned char*)ptrs[i];
        for (size_t j = 0; j < size; j++) {
            assert(p[j] == (i & 0xFF));
        }
    }
    
    free(ptrs);
    samrena_destroy(arena);
}
```

### 4. Feature-Specific Tests
```c
static bool requires_reset(SamrenaStrategy strategy) {
    SamrenaCapabilities caps = samrena_strategy_capabilities(strategy);
    return (caps.flags & SAMRENA_CAP_RESET) != 0;
}

static void test_reset_operation(SamrenaStrategy strategy) {
    SamrenaConfig config = samrena_default_config();
    config.strategy = strategy;
    
    Samrena* arena = samrena_create(&config);
    assert(arena != NULL);
    
    // First allocation cycle
    void* p1 = samrena_push(arena, 1024);
    assert(p1 != NULL);
    memset(p1, 0xAA, 1024);
    
    uint64_t allocated_before = samrena_allocated(arena);
    assert(allocated_before >= 1024);
    
    // Reset
    bool reset_ok = samrena_reset_if_supported(arena);
    assert(reset_ok);
    
    uint64_t allocated_after = samrena_allocated(arena);
    assert(allocated_after == 0);
    
    // Second allocation cycle - should reuse memory
    void* p2 = samrena_push(arena, 1024);
    assert(p2 != NULL);
    
    // On some adapters, might get same address
    if (strategy == SAMRENA_STRATEGY_VIRTUAL) {
        assert(p2 == p1);  // Virtual reuses same address
    }
    
    samrena_destroy(arena);
}
```

### 5. Thread Safety Tests
```c
#include <pthread.h>

typedef struct {
    Samrena* arena;
    size_t thread_id;
    size_t allocation_count;
} ThreadTestData;

static void* thread_allocator(void* arg) {
    ThreadTestData* data = (ThreadTestData*)arg;
    
    for (size_t i = 0; i < data->allocation_count; i++) {
        size_t size = (data->thread_id * 100 + i) % 1024 + 1;
        void* ptr = samrena_push(data->arena, size);
        assert(ptr != NULL);
        
        // Write unique pattern
        memset(ptr, (data->thread_id + i) & 0xFF, size);
    }
    
    return NULL;
}

static void test_thread_safety(SamrenaStrategy strategy) {
    // Note: This tests that the API is thread-safe when used correctly
    // (i.e., external synchronization for shared arena)
    
    const size_t thread_count = 4;
    const size_t allocs_per_thread = 1000;
    
    pthread_t threads[thread_count];
    ThreadTestData thread_data[thread_count];
    
    // Create separate arena for each thread
    for (size_t i = 0; i < thread_count; i++) {
        SamrenaConfig config = samrena_default_config();
        config.strategy = strategy;
        
        thread_data[i].arena = samrena_create(&config);
        thread_data[i].thread_id = i;
        thread_data[i].allocation_count = allocs_per_thread;
        
        pthread_create(&threads[i], NULL, thread_allocator, &thread_data[i]);
    }
    
    // Wait for completion
    for (size_t i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Cleanup
    for (size_t i = 0; i < thread_count; i++) {
        samrena_destroy(thread_data[i].arena);
    }
}
```

### 6. Memory Leak Detection
```c
static void test_memory_leaks(void) {
    // Run with valgrind or AddressSanitizer
    for (int i = 0; i < 100; i++) {
        Samrena* arena = samrena_create(NULL);
        
        // Random operations
        for (int j = 0; j < 100; j++) {
            size_t size = rand() % 4096 + 1;
            void* ptr = samrena_push(arena, size);
            assert(ptr != NULL);
        }
        
        samrena_destroy(arena);
    }
}
```

## Location
- `libs/samrena/test/test_samrena_adapters.c` - Main test suite
- `libs/samrena/test/CMakeLists.txt` - Test configuration

## Dependencies
- All adapter tasks completed
- CMake test infrastructure

## Verification
- [ ] All tests pass for all adapters
- [ ] No memory leaks detected
- [ ] Thread safety verified
- [ ] Performance meets expectations
- [ ] Edge cases handled properly

## Notes
- Run tests with sanitizers enabled
- Consider fuzzing for edge cases
- Add performance regression tests