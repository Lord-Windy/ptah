# Task: Implement Fallback Mechanisms

## Overview
Ensure graceful degradation when requested features are unavailable, providing clear feedback and sensible alternatives.

## Requirements
- Automatic fallback to available adapters
- Optional strict mode that fails instead
- Clear logging of fallback decisions
- Maintain performance when possible

## Implementation Details

### 1. Fallback Configuration
```c
// In include/samrena.h
typedef enum {
    SAMRENA_FALLBACK_AUTO = 0,     // Automatically choose best alternative
    SAMRENA_FALLBACK_WARN,         // Fallback with warning
    SAMRENA_FALLBACK_STRICT        // Fail if exact strategy unavailable
} SamrenaFallbackMode;

// Extended configuration
typedef struct {
    // ... existing fields ...
    
    // Fallback behavior
    SamrenaFallbackMode fallback_mode;
    
    // Logging callback
    void (*log_callback)(const char* message, void* user_data);
    void* log_user_data;
} SamrenaConfig;
```

### 2. Fallback Decision Logic
```c
// In src/samrena.c
typedef struct {
    SamrenaStrategy from;
    SamrenaStrategy to;
    const char* reason;
} FallbackRule;

static const FallbackRule fallback_rules[] = {
    { SAMRENA_STRATEGY_VIRTUAL, SAMRENA_STRATEGY_CHAINED, 
      "Virtual memory not available on this platform" },
    { SAMRENA_STRATEGY_DEFAULT, SAMRENA_STRATEGY_CHAINED,
      "No optimal strategy available, using chained" },
    { 0, 0, NULL }
};

static SamrenaStrategy apply_fallback(
    SamrenaStrategy requested,
    const SamrenaConfig* config,
    const char** reason
) {
    // Check if requested is available
    if (find_adapter(requested)) {
        return requested;
    }
    
    // Handle based on fallback mode
    if (config->fallback_mode == SAMRENA_FALLBACK_STRICT) {
        *reason = "Requested strategy not available";
        return SAMRENA_STRATEGY_DEFAULT;  // Signal error
    }
    
    // Find appropriate fallback
    for (const FallbackRule* rule = fallback_rules; rule->reason; rule++) {
        if (rule->from == requested && find_adapter(rule->to)) {
            *reason = rule->reason;
            return rule->to;
        }
    }
    
    // Last resort
    *reason = "No suitable adapter found";
    return SAMRENA_STRATEGY_CHAINED;
}
```

### 3. Logging Integration
```c
static void log_message(const SamrenaConfig* config, const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    if (config->log_callback) {
        config->log_callback(buffer, config->log_user_data);
    } else {
        fprintf(stderr, "samrena: %s\n", buffer);
    }
}

Samrena* samrena_create(const SamrenaConfig* config) {
    SamrenaConfig cfg = config ? *config : samrena_default_config();
    
    const char* fallback_reason = NULL;
    SamrenaStrategy selected = apply_fallback(
        cfg.strategy, &cfg, &fallback_reason
    );
    
    if (selected != cfg.strategy && fallback_reason) {
        if (cfg.fallback_mode == SAMRENA_FALLBACK_STRICT) {
            log_message(&cfg, "ERROR: %s", fallback_reason);
            return NULL;
        } else if (cfg.fallback_mode == SAMRENA_FALLBACK_WARN) {
            log_message(&cfg, "WARNING: %s", fallback_reason);
        }
    }
    
    // Continue with selected strategy...
}
```

### 4. Performance Hints
```c
// Provide hints about performance characteristics
typedef struct {
    bool contiguous_memory;    // All allocations in single block
    bool zero_copy_growth;     // Can grow without copying
    bool constant_time_alloc;  // O(1) allocation
    uint64_t max_single_alloc; // Largest supported allocation
} SamrenaPerformanceHints;

// In include/samrena.h
SamrenaPerformanceHints samrena_get_performance_hints(Samrena* arena);

// Implementation
static const SamrenaPerformanceHints chained_hints = {
    .contiguous_memory = false,
    .zero_copy_growth = true,
    .constant_time_alloc = true,
    .max_single_alloc = UINT64_MAX
};

static const SamrenaPerformanceHints virtual_hints = {
    .contiguous_memory = true,
    .zero_copy_growth = true,
    .constant_time_alloc = true,
    .max_single_alloc = 0  // Set based on reserved size
};
```

## Location
- `libs/samrena/src/samrena.c` - Fallback logic
- `libs/samrena/include/samrena.h` - Extended configuration

## Dependencies
- Task 08: Conditional compilation for available adapters

## Verification
- [ ] Fallback works when virtual unavailable
- [ ] Strict mode prevents fallback
- [ ] Warning mode logs appropriately
- [ ] Custom logging callback works
- [ ] Performance hints accurate

## Notes
- Consider environment variable for default fallback mode
- May want to rank adapters by performance
- Test all fallback paths thoroughly