# Task 13: Automatic Memory Expansion

## Objective
Add configurable expansion policies to allow arenas to grow automatically when they run out of memory.

## Current State
- Arenas have fixed capacity set at initialization
- `samrena_push` returns NULL when capacity is exceeded
- Users must pre-calculate exact memory needs

## Implementation Steps
1. Add expansion policy enum to header:
   ```c
   typedef enum {
       SAMRENA_POLICY_FIXED = 0,      // Current behavior
       SAMRENA_POLICY_DOUBLE,         // Double capacity when full
       SAMRENA_POLICY_GROW_PAGES      // Grow by N pages
   } SamrenaPolicy;
   ```

2. Add fields to Samrena structure:
   - `SamrenaPolicy policy`
   - `uint64_t growth_pages` (for GROW_PAGES policy)
   - `uint8_t **page_list` (array of allocated pages)
   - `uint64_t page_count`

3. Create new initialization function:
   ```c
   Samrena *samrena_allocate_with_policy(uint64_t initial_pages, 
                                         SamrenaPolicy policy,
                                         uint64_t growth_pages);
   ```

4. Modify `samrena_push` to handle expansion:
   - Check if allocation would exceed capacity
   - If policy allows, allocate new pages
   - Update capacity and page tracking

5. Keep original `samrena_allocate` for backward compatibility
   (defaults to FIXED policy)

## Key Considerations
- Must handle allocation failures during expansion
- Track all allocated pages for proper cleanup
- Consider maximum growth limits
- Maintain alignment guarantees across page boundaries

## Verification
- Test fixed policy matches current behavior
- Test doubling policy with multiple expansions
- Test page-based growth
- Test expansion failure handling
- Verify all pages are freed in `samrena_deallocate`