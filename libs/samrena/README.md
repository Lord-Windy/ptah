# Samrena - High-Performance Memory Arena Library

Samrena is a fast, flexible C memory arena (bump allocator) library with multiple backend strategies, designed for efficient bulk memory management without individual deallocation overhead.

## Features

- **Zero-overhead allocations** - O(1) allocation with simple pointer bumping
- **Multiple allocation strategies**:
  - **Default**: Simple single-block arena
  - **Chained**: Growable linked-list of memory blocks
  - **Virtual**: OS virtual memory-backed arena with lazy commits
- **Arena-backed dynamic arrays** via SamrenaVector
- **Type-safe macros** for common operations
- **Cross-platform** support (Linux, macOS, Windows)
- **Hexagonal architecture** with pluggable memory adapters
- **Thread-safe** allocation operations
- **Configurable growth policies** (linear/exponential)

## Quick Start

### Basic Usage

```c
#include "samrena.h"

int main() {
    // Create a default arena
    Samrena* arena = samrena_create_default();
    
    // Allocate memory - no need to free individual allocations
    int* numbers = SAMRENA_PUSH_ARRAY(arena, int, 1000);
    char* buffer = SAMRENA_PUSH_ARRAY(arena, char, 4096);
    
    // Use the allocated memory
    for (int i = 0; i < 1000; i++) {
        numbers[i] = i * 2;
    }
    
    // Destroy arena - frees all memory at once
    samrena_destroy(arena);
    return 0;
}
```

### Using Different Strategies

```c
// Chained arena - grows by adding new blocks
SamrenaConfig config = samrena_config_chained(10);  // 10 pages per block
Samrena* chained = samrena_create(&config);

// Virtual memory arena - reserves large space, commits as needed
config = samrena_config_virtual(100);  // Reserve 100MB
Samrena* virtual = samrena_create(&config);

// Custom configuration
config = samrena_default_config();
config.strategy = SAMRENA_STRATEGY_CHAINED;
config.initial_pages = 5;
config.growth_pages = 10;
config.enable_stats = true;
Samrena* custom = samrena_create(&config);
```

### Type-Safe Allocation

```c
typedef struct {
    float x, y, z;
} Vec3;

Samrena* arena = samrena_create_default();

// Allocate single object
Vec3* position = SAMRENA_PUSH_TYPE(arena, Vec3);

// Allocate array
Vec3* vertices = SAMRENA_PUSH_ARRAY(arena, Vec3, 1000);

// Allocate and zero-initialize
Vec3* origins = SAMRENA_PUSH_ARRAY_ZERO(arena, Vec3, 10);

// Aligned allocation
Vec3* aligned = SAMRENA_PUSH_ALIGNED_TYPE(arena, Vec3, 64);
```

## SamrenaVector - Dynamic Arrays

SamrenaVector provides growable arrays backed by arena allocation:

### Basic Vector Usage

```c
#include "samvector.h"

Samrena* arena = samrena_create_default();

// Create a vector for integers
SamrenaVector* vec = samrena_vector_init(arena, sizeof(int), 10);

// Push elements
int value = 42;
samrena_vector_push(vec, &value);

// Access elements
int* elem = (int*)samrena_vector_at(vec, 0);
printf("Element: %d\n", *elem);

// Iterate
for (size_t i = 0; i < samrena_vector_size(vec); i++) {
    int* val = (int*)samrena_vector_at(vec, i);
    printf("%d ", *val);
}

samrena_destroy(arena);  // Cleans up vector too
```

### Type-Safe Vectors

```c
// Declare a type-safe vector for your struct
typedef struct {
    int id;
    char name[32];
} Person;

SAMRENA_DECLARE_VECTOR(Person)

// Use the type-safe API
Samrena* arena = samrena_create_default();
SamrenaVector_Person* people = samrena_vector_Person_init(arena, 100);

Person p = {.id = 1, .name = "Alice"};
samrena_vector_Person_push(people, &p);

Person* first = samrena_vector_Person_at(people, 0);
printf("First person: %s\n", first->name);
```

### Functional Operations

```c
// Filter elements
bool is_even(const void* elem, void* user_data) {
    return (*(int*)elem) % 2 == 0;
}

SamrenaVector* evens = samrena_vector_filter(vec, is_even, NULL, arena);

// Map transformation
void double_value(const void* src, void* dst, void* user_data) {
    *(int*)dst = *(int*)src * 2;
}

SamrenaVector* doubled = samrena_vector_map(vec, sizeof(int), 
                                           double_value, NULL, arena);

// ForEach iteration
void print_int(const void* elem, void* user_data) {
    printf("%d ", *(int*)elem);
}

samrena_vector_foreach(vec, print_int, NULL);
```

## Advanced Features

### Memory Reservation

```c
Samrena* arena = samrena_create_default();

// Reserve space for expected usage
samrena_reserve(arena, 1024 * 1024);  // Reserve 1MB

// Reserve with growth hint
samrena_reserve_with_growth(arena, 
    100 * 1024,    // Need 100KB immediately
    1024 * 1024);  // Expect 1MB total

// Check if allocation will succeed
if (samrena_can_allocate(arena, size)) {
    void* mem = samrena_push(arena, size);
}
```

### Capability Detection

```c
// Check available strategies
SamrenaStrategy strategies[10];
int count = samrena_available_strategies(strategies, 10);

for (int i = 0; i < count; i++) {
    printf("Available: %s\n", samrena_strategy_name(strategies[i]));
}

// Check specific capabilities
if (samrena_has_capability(arena, SAMRENA_CAP_ZERO_COPY_GROWTH)) {
    printf("Arena supports zero-copy growth\n");
}

// Get full capability info
SamrenaCapabilities caps = samrena_get_capabilities(arena);
printf("Max allocation: %lu bytes\n", caps.max_allocation_size);
printf("Alignment guarantee: %lu\n", caps.alignment_guarantee);
```

### Arena Information

```c
// Get current usage
uint64_t used = samrena_allocated(arena);
uint64_t total = samrena_capacity(arena);
printf("Using %lu of %lu bytes (%.1f%%)\n", 
       used, total, (100.0 * used) / total);

// Get detailed info
SamrenaInfo info;
samrena_get_info(arena, &info);
printf("Adapter: %s\n", info.adapter_name);
printf("Can grow: %s\n", info.can_grow ? "yes" : "no");
printf("Contiguous: %s\n", info.is_contiguous ? "yes" : "no");
```

## Use Cases

Samrena is ideal for:

- **Temporary allocations** - Request handlers, frame allocations in games
- **Bulk data processing** - Load file, process, discard
- **Parser/compiler passes** - AST nodes, symbol tables
- **Scratch memory** - Temporary buffers for algorithms
- **Memory pooling** - Pre-allocate for known workloads
- **Simplified memory management** - No manual free() calls

## Performance

- **Allocation**: O(1) - simple pointer increment
- **Deallocation**: O(1) - destroy entire arena
- **No fragmentation** - linear allocation pattern
- **Cache-friendly** - sequential memory layout
- **Zero overhead** - no metadata per allocation

## Building

Samrena is part of the Ptah monorepo. From the repository root:

```bash
mkdir build
cd build
cmake ..
make

# Run tests
ctest -R samrena
```

## API Reference

### Core Functions

- `samrena_create()` - Create arena with configuration
- `samrena_create_default()` - Create with default settings
- `samrena_destroy()` - Destroy arena and free all memory
- `samrena_push()` - Allocate memory
- `samrena_push_zero()` - Allocate zero-initialized memory
- `samrena_push_aligned()` - Allocate with specific alignment

### Configuration

- `samrena_default_config()` - Get default configuration
- `samrena_config_chained()` - Configure chained strategy
- `samrena_config_virtual()` - Configure virtual memory strategy

### Vector Operations

- `samrena_vector_init()` - Create vector with arena
- `samrena_vector_push()` - Add element
- `samrena_vector_pop()` - Remove last element
- `samrena_vector_at()` - Access element by index
- `samrena_vector_resize()` - Change capacity
- `samrena_vector_clear()` - Remove all elements

## License

Copyright 2025 Samuel "Lord-Windy" Brown

Licensed under the Apache License, Version 2.0