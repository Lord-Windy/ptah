# Task: Implement Factory Function

## Overview
Create the main factory function that ties together all components: configuration, strategy selection, adapter instantiation, and error handling.

## Requirements
- Single entry point for arena creation
- Validate configuration
- Handle adapter selection and fallback
- Clean error reporting
- Thread-safe initialization

## Implementation Details

### 1. Internal Implementation Structure
```c
// In src/samrena_internal.h
typedef struct SamrenaImpl {
    const SamrenaOps* ops;
    SamrenaStrategy strategy;
    uint64_t page_size;
    
    // Statistics (common to all adapters)
    struct {
        uint64_t total_allocations;
        uint64_t failed_allocations;
        uint64_t peak_usage;
    } stats;
    
    // Debug info
    char adapter_name[32];
} SamrenaImpl;
```

### 2. Configuration Validation
```c
static SamrenaError validate_config(const SamrenaConfig* config) {
    if (config->initial_pages == 0) {
        return SAMRENA_ERROR_INVALID_PARAMETER;
    }
    
    if (config->page_size != 0 && config->page_size < 4096) {
        return SAMRENA_ERROR_INVALID_PARAMETER;  // Minimum page size
    }
    
    if (config->strategy < 0 || config->strategy > SAMRENA_STRATEGY_VIRTUAL) {
        return SAMRENA_ERROR_INVALID_PARAMETER;
    }
    
    // Virtual memory specific validation
    if (config->strategy == SAMRENA_STRATEGY_VIRTUAL) {
        if (config->max_reserve != 0 && config->max_reserve < config->page_size) {
            return SAMRENA_ERROR_INVALID_PARAMETER;
        }
    }
    
    return SAMRENA_OK;
}
```

### 3. Main Factory Implementation
```c
Samrena* samrena_create(const SamrenaConfig* config) {
    // Use defaults if no config provided
    SamrenaConfig cfg = config ? *config : samrena_default_config();
    
    // Validate configuration
    SamrenaError err = validate_config(&cfg);
    if (err != SAMRENA_OK) {
        if (cfg.log_callback) {
            log_message(&cfg, "Invalid configuration: error %d", err);
        }
        return NULL;
    }
    
    // Apply defaults for zero values
    if (cfg.page_size == 0) {
        cfg.page_size = SAMRENA_DEFAULT_PAGE_SIZE;
    }
    
    // Select strategy with fallback
    const char* fallback_reason = NULL;
    SamrenaStrategy selected = apply_fallback(
        cfg.strategy, &cfg, &fallback_reason
    );
    
    if (selected == SAMRENA_STRATEGY_DEFAULT) {
        // Fallback failed in strict mode
        return NULL;
    }
    
    // Find adapter
    const SamrenaOps* ops = find_adapter(selected);
    if (!ops) {
        log_message(&cfg, "Internal error: adapter not found");
        return NULL;
    }
    
    // Allocate arena structure
    Samrena* arena = calloc(1, sizeof(Samrena));
    if (!arena) {
        return NULL;
    }
    
    arena->impl = calloc(1, sizeof(SamrenaImpl));
    if (!arena->impl) {
        free(arena);
        return NULL;
    }
    
    // Initialize implementation
    arena->impl->ops = ops;
    arena->impl->strategy = selected;
    arena->impl->page_size = cfg.page_size;
    strncpy(arena->impl->adapter_name, ops->name, 
            sizeof(arena->impl->adapter_name) - 1);
    
    // Create adapter context
    err = ops->create(&arena->context, &cfg);
    if (err != SAMRENA_OK) {
        log_message(&cfg, "Failed to create %s adapter: error %d", 
                    ops->name, err);
        free(arena->impl);
        free(arena);
        return NULL;
    }
    
    // Log successful creation
    if (cfg.log_callback && selected != cfg.strategy) {
        log_message(&cfg, "Created %s arena (requested: %s)", 
                    ops->name, samrena_strategy_name(cfg.strategy));
    }
    
    return arena;
}
```

### 4. Destruction Function
```c
void samrena_destroy(Samrena* arena) {
    if (!arena) return;
    
    if (arena->impl && arena->impl->ops && arena->context) {
        // Call adapter-specific cleanup
        arena->impl->ops->destroy(arena->context);
    }
    
    free(arena->impl);
    free(arena);
}
```

### 5. Thread Safety
```c
// Optional: Global initialization for thread safety
static pthread_once_t init_once = PTHREAD_ONCE_INIT;
static pthread_mutex_t factory_mutex = PTHREAD_MUTEX_INITIALIZER;

static void samrena_global_init(void) {
    // One-time initialization if needed
    // e.g., detect page size, CPU features
}

Samrena* samrena_create_threadsafe(const SamrenaConfig* config) {
    pthread_once(&init_once, samrena_global_init);
    
    pthread_mutex_lock(&factory_mutex);
    Samrena* arena = samrena_create(config);
    pthread_mutex_unlock(&factory_mutex);
    
    return arena;
}
```

## Location
- `libs/samrena/src/samrena.c` - Factory implementation

## Dependencies
- Tasks 01-09: All core components ready

## Verification
- [ ] Creates arenas with all strategies
- [ ] Validates configuration properly
- [ ] Handles errors gracefully
- [ ] Fallback works as configured
- [ ] No memory leaks on error paths
- [ ] Thread-safe if needed

## Notes
- Consider arena pooling for performance
- May want to add arena "templates"
- Document all error conditions