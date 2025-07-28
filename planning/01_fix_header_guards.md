# Task 01: Fix Header Guard Naming

## Objective
Fix the incorrect header guard naming in datazoo.h file.

## Current State
- The header guard is currently named `ZOOKEEPER_H` which doesn't match the library name
- This is inconsistent and could cause issues if there's another library with the same guard name

## Implementation Steps
1. Open `/home/sam/dev/ptah/libs/datazoo/include/datazoo.h`
2. Find the header guard definitions at the top and bottom of the file
3. Replace `ZOOKEEPER_H` with `DATAZOO_H` in both locations:
   - Change `#ifndef ZOOKEEPER_H` to `#ifndef DATAZOO_H`
   - Change `#define ZOOKEEPER_H` to `#define DATAZOO_H`
   - Change `#endif // ZOOKEEPER_H` to `#endif // DATAZOO_H`

## Verification
- Ensure the project still builds successfully after the change
- Run existing tests to confirm no functionality is broken