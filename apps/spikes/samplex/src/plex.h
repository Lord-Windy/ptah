/*
 * Copyright 2025 Samuel "Lord-Windy" Brown
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PLEX_H
#define PLEX_H

// POSIX feature test macro for pthread_rwlock_t
#if !defined(_XOPEN_SOURCE)
#define _XOPEN_SOURCE 600
#endif

#include <pthread.h>
#include <stdint.h>

#include "samdata/samhashmap.h"
#include "samrena.h"
#include "samvector.h"

// Forward declaration
typedef struct PlexItem PlexItem;

// Function pointer types
typedef void (*PlexItemHandler)(PlexItem *self, void *result);
typedef void (*PlexItemErrorHandler)(PlexItem *self, int error);
typedef void (*PlexItemCleanup)(PlexItem *self);

typedef struct PlexItem {
  char *description;

  void *data;
  PlexItemHandler handler;
  PlexItemErrorHandler error_handler;
  PlexItemCleanup cleanup;

  // Unix timestamps in nanoseconds
  int64_t start_time_ns;
  int64_t stop_time_ns;
} PlexItem;

// Declare type-safe vector for PlexItem
SAMRENA_DECLARE_VECTOR(PlexItem)

typedef struct Plex Plex;

typedef struct Plex {
  uint64_t id;
  char *description;

  Samrena *arena;
  SamrenaVector_PlexItem *items;

  // Unix timestamps in nanoseconds
  int64_t start_time_ns;
  int64_t stop_time_ns;
} Plex;

// Declare type-safe hashmap for Plex registry (key: uint64_t id as string, value: Plex*)
SAMHASHMAP_DEFINE_TYPED(PlexMap, const char *, Plex *)

typedef struct PlexRegistry {
  uint64_t id_tracker;          // Atomic counter for generating unique Plex IDs
  pthread_rwlock_t rwlock;      // Read-write lock: exclusive for add/remove, shared for reads
  PlexMap_samhashmap *plex_map; // HashMap to store Plex instances by ID
  Samrena *arena;               // Arena for registry allocations
} PlexRegistry;

// ============================================================================
// PlexRegistry Functions
// ============================================================================

/**
 * Create a new PlexRegistry with thread-safe access
 * @param initial_capacity Initial capacity for the hashmap (0 for default)
 * @return Newly allocated PlexRegistry or NULL on failure
 */
PlexRegistry *plex_registry_create(uint64_t initial_capacity);

/**
 * Destroy a PlexRegistry and all contained Plex instances
 * @param registry The registry to destroy
 */
void plex_registry_destroy(PlexRegistry *registry);

/**
 * Get the total number of Plex instances in the registry
 * @param registry The registry to query
 * @return Number of Plex instances
 */
uint64_t plex_registry_size(PlexRegistry *registry);

// Helper functions for thread-safe operations
void plex_registry_lock_write(PlexRegistry *registry);
void plex_registry_unlock_write(PlexRegistry *registry);
void plex_registry_lock_read(PlexRegistry *registry);
void plex_registry_unlock_read(PlexRegistry *registry);

// ============================================================================
// Plex Functions
// ============================================================================

/**
 * Create a new Plex with the given description and register it in the registry
 * @param registry The PlexRegistry to register the new Plex in
 * @param description The description of the Plex
 * @return Newly allocated Plex with assigned ID or NULL on failure
 */
Plex *plex_create(PlexRegistry *registry, const char *description);

/**
 * Destroy a Plex and all its items, removing it from the registry
 * @param registry The PlexRegistry to unregister the Plex from
 * @param plex The Plex to destroy
 */
void plex_destroy(PlexRegistry *registry, Plex *plex);

/**
 * Get a Plex from the registry by its ID
 * @param registry The PlexRegistry to search in
 * @param id The ID of the Plex to retrieve
 * @return Pointer to Plex or NULL if not found
 */
Plex *plex_get_by_id(PlexRegistry *registry, uint64_t id);

// ============================================================================
// PlexItem Functions
// ============================================================================

/**
 * Create a new PlexItem with the given description and handlers
 * @param description The description of the item
 * @param handler The success handler function
 * @param error_handler The error handler function
 * @param cleanup The cleanup function (can be NULL)
 * @param data User data to associate with the item
 * @return Newly allocated PlexItem or NULL on failure
 */
PlexItem *plex_item_create(const char *description, PlexItemHandler handler,
                           PlexItemErrorHandler error_handler, PlexItemCleanup cleanup, void *data);

/**
 * Destroy a PlexItem (calls cleanup if provided)
 * @param item The item to destroy
 */
void plex_item_destroy(PlexItem *item);

/**
 * Add a PlexItem to a Plex
 * @param plex The Plex to add to
 * @param item The item to add
 * @return true on success, false on failure
 */
bool plex_add_item(Plex *plex, PlexItem *item);

/**
 * Remove a PlexItem from a Plex
 * @param plex The Plex to remove from
 * @param item The item to remove
 * @return true on success, false on failure
 */
bool plex_remove_item(Plex *plex, PlexItem *item);

/**
 * Find an item by description (first match)
 * @param plex The Plex to search
 * @param description The description to search for
 * @return Pointer to PlexItem or NULL if not found
 */
PlexItem *plex_find_item(Plex *plex, const char *description);

// ============================================================================
// PlexItem Execution and Timing
// ============================================================================

/**
 * Mark a PlexItem as started (records start time)
 * @param item The item to start
 */
void plex_item_start(PlexItem *item);

/**
 * Mark a PlexItem as stopped (records stop time)
 * @param item The item to stop
 */
void plex_item_stop(PlexItem *item);

/**
 * Execute a PlexItem's handler with success result
 * @param item The item to execute
 * @param result Result data to pass to handler
 */
void plex_item_execute(PlexItem *item, void *result);

/**
 * Execute a PlexItem's error handler
 * @param item The item to execute
 * @param error Error code to pass to handler
 */
void plex_item_error(PlexItem *item, int32_t error);

/**
 * Get the duration of a PlexItem in nanoseconds
 * @param item The item to query
 * @return Duration in nanoseconds, or -1 if not completed
 */
int64_t plex_item_duration_ns(PlexItem *item);

// ============================================================================
// Plex Timing and Lifecycle
// ============================================================================

/**
 * Mark a Plex as started (records start time)
 * @param plex The Plex to start
 */
void plex_start(Plex *plex);

/**
 * Mark a Plex as stopped (records stop time)
 * @param plex The Plex to stop
 */
void plex_stop(Plex *plex);

/**
 * Get the duration of a Plex in nanoseconds
 * @param plex The Plex to query
 * @return Duration in nanoseconds, or -1 if not completed
 */
int64_t plex_duration_ns(Plex *plex);

// ============================================================================
// Utility Functions
// ============================================================================

/**
 * Get current time in nanoseconds since Unix epoch
 * @return Current time in nanoseconds
 */
int64_t plex_get_time_ns(void);

/**
 * Convert Plex ID to string key for hashmap (uses internal buffer, not thread-safe)
 * @param id The ID to convert
 * @return String representation
 */
const char *plex_id_to_key(uint64_t id);

/**
 * Clean up all items in a Plex (calls cleanup handlers)
 * @param plex The Plex to clean up
 */
void plex_cleanup_items(Plex *plex);

/**
 * Get statistics for a Plex
 * @param plex The Plex to analyze
 * @param total_items Output: total number of items
 * @param completed_items Output: number of items with both start and stop times
 * @param avg_duration_ns Output: average duration in nanoseconds
 */
void plex_get_stats(Plex *plex, uint64_t *total_items, uint64_t *completed_items,
                    int64_t *avg_duration_ns);

#endif // PLEX_H
