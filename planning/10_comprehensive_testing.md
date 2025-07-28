# Task 10: Create Comprehensive Test Suite

## Objective
Develop complete test coverage for all datazoo functionality using samrena.

## Current State
- Basic tests may exist
- Need comprehensive coverage of new features

## Test Categories to Implement

### 1. Basic Operations Tests
- Create hash map with various initial capacities
- Insert, get, and remove single elements
- Verify NULL parameter handling
- Test empty hash map operations

### 2. Key Copying Tests
- Insert key from temporary buffer
- Modify original buffer after insertion
- Verify hash map retains correct key
- Test with various key lengths

### 3. Resize Tests
- Insert elements to trigger resize
- Verify all elements accessible after resize
- Test multiple resize operations
- Test resize with nearly exhausted arena

### 4. Load Factor Tests
- Test with different load factors (0.5, 0.75, 0.9)
- Verify resize triggers at correct thresholds
- Performance tests at various load factors

### 5. Iterator Tests
- Iterate empty hash map
- Iterate and count all elements
- Verify each element visited once
- Test iterator with concurrent modifications

### 6. Bulk Operation Tests
- Bulk insert triggering resize
- Bulk insert within capacity
- Mixed bulk operations
- Error handling in bulk operations

### 7. Stress Tests
- Large dataset handling (10000+ elements)
- Random insert/remove patterns
- Arena exhaustion scenarios
- Performance benchmarks

### 8. Edge Cases
- Hash collision handling
- Maximum capacity limits
- Zero-length keys
- Very long keys

## Test Implementation Structure
```c
// In datazoo_test.c
void test_key_copying() {
    Samrena *arena = samrena_allocate(1000);
    Honeycomb *comb = honeycomb_create(16, 0.75, arena);
    
    char buffer[32];
    sprintf(buffer, "test_key");
    honeycomb_put(comb, buffer, "value");
    
    // Modify buffer
    sprintf(buffer, "modified");
    
    // Original key should still work
    void *value = honeycomb_get(comb, "test_key");
    assert(value != NULL);
    assert(strcmp(value, "value") == 0);
}
```

## Verification
- All tests pass consistently
- Code coverage > 90%
- No memory issues under Valgrind
- Performance meets requirements