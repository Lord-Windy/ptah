# Task 07: Design Error Handling System

## Objective
Design a comprehensive error handling system for Samvector that provides clear error reporting, consistent error codes, and debugging support while maintaining performance for success paths.

## Requirements
- Consistent error codes across all operations
- Detailed error information for debugging
- Performance-optimized success paths
- Thread-safe error reporting
- Backward compatibility with existing code

## Error Classification

### Core Error Categories
```c
typedef enum {
    // Success
    SAMRENA_SUCCESS = 0,
    
    // Parameter errors
    SAMRENA_ERROR_NULL_POINTER = -1,
    SAMRENA_ERROR_INVALID_SIZE = -2,
    SAMRENA_ERROR_OUT_OF_BOUNDS = -3,
    SAMRENA_ERROR_INVALID_OPERATION = -4,
    
    // Memory errors
    SAMRENA_ERROR_ALLOCATION_FAILED = -10,
    SAMRENA_ERROR_ARENA_EXHAUSTED = -11,
    SAMRENA_ERROR_INSUFFICIENT_CAPACITY = -12,
    
    // Ownership errors
    SAMRENA_ERROR_OWNERSHIP_CONFLICT = -20,
    SAMRENA_ERROR_ARENA_MISMATCH = -21,
    
    // State errors
    SAMRENA_ERROR_VECTOR_EMPTY = -30,
    SAMRENA_ERROR_VECTOR_FULL = -31,
    SAMRENA_ERROR_INVALID_STATE = -32
} SamrenaError;
```

## Error Context System

### Error Information Structure
```c
typedef struct {
    SamrenaError code;           // Error code
    const char* message;         // Human-readable message
    const char* function;        // Function where error occurred
    const char* file;            // Source file
    int line;                    // Line number
    void* context;               // Additional context (vector, arena, etc.)
} SamrenaErrorInfo;
```

### Thread-Local Error Storage
```c
// Thread-local storage for last error
_Thread_local SamrenaErrorInfo g_last_error = {0};

// Macro for setting error with location info
#define SAMRENA_SET_ERROR(code, msg, ctx) \
    samrena_set_error_impl((code), (msg), __func__, __FILE__, __LINE__, (ctx))

// Implementation function
void samrena_set_error_impl(SamrenaError code, const char* message, 
                           const char* function, const char* file, 
                           int line, void* context);
```

## API Design Patterns

### Return Value Conventions
```c
// Pattern 1: Functions that can fail return error codes
int samrena_vector_push(SamrenaVector* vec, const void* element);
int samrena_vector_get(const SamrenaVector* vec, size_t index, void* out);

// Pattern 2: Query functions return values directly (infallible)
size_t samrena_vector_size(const SamrenaVector* vec);
bool samrena_vector_is_empty(const SamrenaVector* vec);

// Pattern 3: Pointer-returning functions use NULL for errors
void* samrena_vector_at(SamrenaVector* vec, size_t index);
SamrenaVector* samrena_vector_init_owned(size_t element_size, size_t capacity);
```

### Error Checking Functions
```c
// Get last error information for current thread
SamrenaErrorInfo samrena_get_last_error(void);

// Check if operation succeeded
static inline bool samrena_succeeded(SamrenaError result) {
    return result == SAMRENA_SUCCESS;
}

// Check if operation failed
static inline bool samrena_failed(SamrenaError result) {
    return result != SAMRENA_SUCCESS;
}

// Clear last error
void samrena_clear_error(void);

// Get error message string
const char* samrena_error_string(SamrenaError code);
```

## Error Reporting API

### Basic Error Queries
```c
// Get error code only (fast path)
SamrenaError samrena_last_error_code(void);

// Get error message only
const char* samrena_last_error_message(void);

// Check if specific error occurred
bool samrena_is_error(SamrenaError code);

// Print error to stderr with context
void samrena_print_error(void);

// Format error as string
int samrena_format_error(char* buffer, size_t buffer_size);
```

### Debug and Logging Support
```c
// Error callback for custom handling
typedef void (*SamrenaErrorCallback)(const SamrenaErrorInfo* error);

// Set global error callback
void samrena_set_error_callback(SamrenaErrorCallback callback);

// Enable/disable error location tracking
void samrena_set_error_tracking(bool enabled);

// Dump error with full context
void samrena_dump_error_context(FILE* output);
```

## Parameter Validation

### Validation Macros
```c
// Parameter validation macros
#define SAMRENA_CHECK_NULL(ptr, name) \
    do { \
        if (!(ptr)) { \
            SAMRENA_SET_ERROR(SAMRENA_ERROR_NULL_POINTER, \
                            "Parameter '" name "' is NULL", (ptr)); \
            return SAMRENA_ERROR_NULL_POINTER; \
        } \
    } while(0)

#define SAMRENA_CHECK_BOUNDS(vec, index) \
    do { \
        if ((index) >= (vec)->size) { \
            SAMRENA_SET_ERROR(SAMRENA_ERROR_OUT_OF_BOUNDS, \
                            "Index out of bounds", (vec)); \
            return SAMRENA_ERROR_OUT_OF_BOUNDS; \
        } \
    } while(0)

#define SAMRENA_CHECK_CAPACITY(vec, required) \
    do { \
        if ((required) > (vec)->capacity) { \
            SAMRENA_SET_ERROR(SAMRENA_ERROR_INSUFFICIENT_CAPACITY, \
                            "Insufficient capacity", (vec)); \
            return SAMRENA_ERROR_INSUFFICIENT_CAPACITY; \
        } \
    } while(0)
```

### Validation Functions
```c
// Validate vector pointer and state
static inline SamrenaError validate_vector(const SamrenaVector* vec) {
    if (!vec) return SAMRENA_ERROR_NULL_POINTER;
    if (!vec->arena) return SAMRENA_ERROR_INVALID_STATE;
    if (vec->size > vec->capacity) return SAMRENA_ERROR_INVALID_STATE;
    return SAMRENA_SUCCESS;
}

// Validate index bounds
static inline SamrenaError validate_index(const SamrenaVector* vec, size_t index) {
    SamrenaError err = validate_vector(vec);
    if (err != SAMRENA_SUCCESS) return err;
    if (index >= vec->size) return SAMRENA_ERROR_OUT_OF_BOUNDS;
    return SAMRENA_SUCCESS;
}
```

## Performance Considerations

### Optimized Error Paths
```c
// Fast path for success cases
#ifdef NDEBUG
    #define SAMRENA_LIKELY_SUCCESS(expr) __builtin_expect(!!(expr == SAMRENA_SUCCESS), 1)
    #define SAMRENA_UNLIKELY_ERROR(expr) __builtin_expect(!!(expr != SAMRENA_SUCCESS), 0)
#else
    #define SAMRENA_LIKELY_SUCCESS(expr) (expr == SAMRENA_SUCCESS)
    #define SAMRENA_UNLIKELY_ERROR(expr) (expr != SAMRENA_SUCCESS)
#endif

// Example usage in hot path
int samrena_vector_push_fast(SamrenaVector* vec, const void* element) {
    // Fast path validation
    if (SAMRENA_UNLIKELY_ERROR(!vec || !element)) {
        return SAMRENA_ERROR_NULL_POINTER;
    }
    
    // Hot path continues...
}
```

### Conditional Error Tracking
```c
// Compile-time error tracking control
#ifdef SAMRENA_DETAILED_ERRORS
    #define SAMRENA_TRACK_ERROR 1
#else
    #define SAMRENA_TRACK_ERROR 0
#endif

// Runtime error tracking
extern bool g_samrena_error_tracking_enabled;
```

## Integration with Existing Code

### Backward Compatibility
```c
// Legacy functions continue to work
void* samrena_vector_push_legacy(Samrena* arena, SamrenaVector* vec, const void* element) {
    int result = samrena_vector_push_auto(vec, element);
    return (result == SAMRENA_SUCCESS) ? samrena_vector_back(vec) : NULL;
}
```

### Migration Helpers
```c
// Check if last operation failed
bool samrena_last_operation_failed(void);

// Convert error code to boolean
bool samrena_check_result(SamrenaError result);

// Assert success in debug builds
#ifdef DEBUG
    #define SAMRENA_ASSERT_SUCCESS(expr) \
        do { \
            SamrenaError __result = (expr); \
            assert(__result == SAMRENA_SUCCESS); \
        } while(0)
#else
    #define SAMRENA_ASSERT_SUCCESS(expr) (expr)
#endif
```

## Error Message Localization

### Message System
```c
// Error message lookup
typedef struct {
    SamrenaError code;
    const char* english;
    const char* localized;
} SamrenaErrorMessage;

// Set locale for error messages
void samrena_set_error_locale(const char* locale);

// Custom message provider
typedef const char* (*SamrenaMessageProvider)(SamrenaError code);
void samrena_set_message_provider(SamrenaMessageProvider provider);
```

## Testing Strategy
- Error condition coverage for all functions
- Thread safety of error reporting
- Performance impact measurement
- Error callback functionality
- Message formatting and localization

## Documentation Requirements
- Error code reference
- Error handling best practices
- Migration guide for error checking
- Performance implications
- Thread safety guarantees