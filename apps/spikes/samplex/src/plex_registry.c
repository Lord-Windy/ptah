#include "plex.h"
#include <stdio.h>
#include <string.h>

PlexRegistry *plex_registry_create(uint64_t initial_capacity) {
  // Create Arena for registry allocations
  Samrena *arena = samrena_create_default();
  if (arena == NULL) {
    return NULL;
  }

  // Create PlexRegistry via arena allocation
  PlexRegistry *registry = samrena_push(arena, sizeof(PlexRegistry));
  if (registry == NULL) {
    samrena_destroy(arena);
    return NULL;
  }

  // Set the arena on the registry to be used there
  registry->arena = arena;

  // Create new plex_map based on the defined type (PlexMap_samhashmap)
  if (initial_capacity == 0) {
    initial_capacity = 16; // reasonable default
  }
  registry->plex_map = PlexMap_create(initial_capacity, arena);
  if (registry->plex_map == NULL) {
    samrena_destroy(arena);
    return NULL;
  }

  // Set id tracker to 0
  registry->id_tracker = 0;

  // Create and init the rwlock. If it needs heap, it uses the arena
  if (pthread_rwlock_init(&registry->rwlock, NULL) != 0) {
    samrena_destroy(arena);
    return NULL;
  }

  return registry;
}

void plex_registry_destroy(PlexRegistry *registry) {
  if (registry == NULL) {
    return;
  }

  // Destroy the read-write lock
  pthread_rwlock_destroy(&registry->rwlock);

  // The arena will clean up all the memory including the registry and the hashmap
  // since they were both allocated through the arena
  if (registry->arena != NULL) {
    samrena_destroy(registry->arena);
  }
}

uint64_t plex_registry_size(PlexRegistry *registry) {
  if (registry == NULL || registry->plex_map == NULL) {
    return 0;
  }

  // Use read lock for size query
  pthread_rwlock_rdlock(&registry->rwlock);
  uint64_t size = PlexMap_size(registry->plex_map);
  pthread_rwlock_unlock(&registry->rwlock);

  return size;
}

void plex_registry_lock_write(PlexRegistry *registry) {
  if (registry != NULL) {
    pthread_rwlock_wrlock(&registry->rwlock);
  }
}

void plex_registry_unlock_write(PlexRegistry *registry) {
  if (registry != NULL) {
    pthread_rwlock_unlock(&registry->rwlock);
  }
}

void plex_registry_lock_read(PlexRegistry *registry) {
  if (registry != NULL) {
    pthread_rwlock_rdlock(&registry->rwlock);
  }
}

void plex_registry_unlock_read(PlexRegistry *registry) {
  if (registry != NULL) {
    pthread_rwlock_unlock(&registry->rwlock);
  }
}

Plex *plex_create(PlexRegistry *registry, const char *description) {
  if (registry == NULL) {
    return NULL;
  }

  // Create new arena for this Plex using default config (256MB reserve)
  Samrena *plex_arena = samrena_create_default();
  if (plex_arena == NULL) {
    return NULL;
  }

  // Allocate Plex structure from its arena
  Plex *plex = SAMRENA_PUSH_TYPE(plex_arena, Plex);
  if (plex == NULL) {
    samrena_destroy(plex_arena);
    return NULL;
  }

  // Set the arena
  plex->arena = plex_arena;

  // Copy description string to arena
  if (description != NULL) {
    size_t desc_len = strlen(description);
    plex->description = (char *)samrena_push(plex_arena, desc_len + 1);
    if (plex->description == NULL) {
      samrena_destroy(plex_arena);
      return NULL;
    }
    memcpy(plex->description, description, desc_len + 1);
  } else {
    plex->description = NULL;
  }

  // Create vector of PlexItems with initial capacity
  plex->items = samrena_vector_PlexItem_init(plex_arena, 16);
  if (plex->items == NULL) {
    samrena_destroy(plex_arena);
    return NULL;
  }

  // Initialize timestamps to 0 (not started)
  plex->start_time_ns = 0;
  plex->stop_time_ns = 0;

  // Acquire write lock to modify registry
  plex_registry_lock_write(registry);

  // Assign unique ID and increment tracker
  plex->id = registry->id_tracker++;

  // Convert ID to string key for hashmap
  char key_buffer[32];
  snprintf(key_buffer, sizeof(key_buffer), "%lu", plex->id);

  // Allocate key string in registry arena
  size_t key_len = strlen(key_buffer);
  char *key = (char *)samrena_push(registry->arena, key_len + 1);
  if (key == NULL) {
    plex_registry_unlock_write(registry);
    samrena_destroy(plex_arena);
    return NULL;
  }
  memcpy(key, key_buffer, key_len + 1);

  // Add to registry hashmap
  if (!PlexMap_put(registry->plex_map, key, plex)) {
    plex_registry_unlock_write(registry);
    samrena_destroy(plex_arena);
    return NULL;
  }

  // Release write lock
  plex_registry_unlock_write(registry);

  return plex;
}

void plex_destroy(PlexRegistry *registry, Plex *plex) {
  if (plex == NULL) {
    return;
  }

  // Clean up all items (call their cleanup functions if provided)
  if (plex->items != NULL) {
    size_t item_count = samrena_vector_PlexItem_size(plex->items);
    for (size_t i = 0; i < item_count; i++) {
      PlexItem *item = samrena_vector_PlexItem_at(plex->items, i);
      if (item != NULL && item->cleanup != NULL) {
        item->cleanup(item);
      }
    }
  }

  // Remove from registry if registry is provided
  if (registry != NULL) {
    // Acquire write lock for removal
    plex_registry_lock_write(registry);

    // Convert ID to string key for removal
    char key_buffer[32];
    snprintf(key_buffer, sizeof(key_buffer), "%lu", plex->id);

    // Remove from hashmap
    PlexMap_remove(registry->plex_map, key_buffer);

    // Release write lock
    plex_registry_unlock_write(registry);
  }

  // Destroy the arena (this frees all plex memory including items vector and description)
  if (plex->arena != NULL) {
    samrena_destroy(plex->arena);
  }
}

Plex *plex_get_by_id(PlexRegistry *registry, uint64_t id) {
  if (registry == NULL || registry->plex_map == NULL) {
    return NULL;
  }

  // Convert ID to string key
  char key_buffer[32];
  snprintf(key_buffer, sizeof(key_buffer), "%lu", id);

  // Acquire read lock for safe access
  plex_registry_lock_read(registry);

  // Get plex from hashmap
  Plex *plex = PlexMap_get(registry->plex_map, key_buffer);

  // Release read lock
  plex_registry_unlock_read(registry);

  return plex;
}
