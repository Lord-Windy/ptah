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
#define _XOPEN_SOURCE 600

#include <stdint.h>
#include <pthread.h>

#include <samrena.h>
#include <samvector.h>
#include <samdata/samhashmap.h>

// Forward declaration
typedef struct PlexItem PlexItem;

// Function pointer types
typedef void (*PlexItemHandler)(PlexItem* self, void* result);
typedef void (*PlexItemErrorHandler)(PlexItem* self, int error);
typedef void (*PlexItemCleanup)(PlexItem* self);

typedef struct PlexItem{
  char* description;
  
  void* data;
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
  char* description;
  
  Samrena* arena;
  SamrenaVector_PlexItem* items;
  
  // Unix timestamps in nanoseconds
  int64_t start_time_ns;
  int64_t stop_time_ns;
} Plex;

// Declare type-safe hashmap for Plex registry (key: uint64_t id as string, value: Plex*)
SAMHASHMAP_DEFINE_TYPED(PlexMap, const char*, Plex*)

typedef struct PlexRegistry {
  uint64_t id_tracker;              // Atomic counter for generating unique Plex IDs
  pthread_rwlock_t rwlock;          // Read-write lock: exclusive for add/remove, shared for reads
  PlexMap_samhashmap* plex_map;     // HashMap to store Plex instances by ID
  Samrena* arena;                   // Arena for registry allocations
} PlexRegistry;

#endif // PLEX_H
