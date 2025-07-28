# Task 02: Make Samrena Parameter Mandatory

## Objective
Update the honeycomb_create function to require a non-nullable samrena instance.

## Current State
- The API might currently allow NULL samrena parameter
- This could lead to unclear memory management behavior

## Implementation Steps
1. Update `honeycomb_create` function signature in `datazoo.h` to clearly document samrena is required
2. Add validation in `honeycomb.c` to check if samrena parameter is NULL
3. Return NULL from honeycomb_create if samrena is NULL
4. Update any existing documentation/comments to reflect this requirement

## Code Changes
```c
// In honeycomb_create function:
if (arena == NULL) {
    return NULL;  // Samrena instance is required
}
```

## Verification
- Write a test case that attempts to create a honeycomb with NULL samrena
- Ensure it returns NULL as expected
- Update existing tests to always provide valid samrena instance