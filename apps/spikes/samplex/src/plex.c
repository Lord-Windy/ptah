#include "plex.h"

PlexRegistry* plex_registry_create(uint64_t initial_capacity) {
  // Create Arena for registry allocations
  Samrena* arena = samrena_create_default();
  if (arena == NULL) {
    return NULL;
  }

  // Create PlexRegistry via arena allocation
  PlexRegistry* registry = samrena_push(arena, sizeof(PlexRegistry));
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

void plex_registry_destroy(PlexRegistry* registry) {
  if (registry == NULL) {
    return;
  }

  // Destroy the read-write lock
  pthread_rwlock_destroy(&registry->rwlock);
  
  // The arena will clean up all the memory including the registry and the hashmap
  // since they were both allocated through the arena
  if (registry->arena != NULL ) {
    samrena_destroy(registry->arena);
  }
}

uint64_t plex_registry_size(PlexRegistry* registry) {
  if (registry == NULL || registry->plex_map == NULL) {
    return 0;
  }
  
  // Use read lock for size query
  pthread_rwlock_rdlock(&registry->rwlock);
  uint64_t size = PlexMap_size(registry->plex_map);
  pthread_rwlock_unlock(&registry->rwlock);
  
  return size;
}

void plex_registry_lock_write(PlexRegistry* registry) {
    if (registry != NULL) {
        pthread_rwlock_wrlock(&registry->rwlock);
    }
}

void plex_registry_unlock_write(PlexRegistry* registry) {
    if (registry != NULL) {
        pthread_rwlock_unlock(&registry->rwlock);
    }
}

void plex_registry_lock_read(PlexRegistry* registry) {
    if (registry != NULL) {
        pthread_rwlock_rdlock(&registry->rwlock);
    }
}

void plex_registry_unlock_read(PlexRegistry* registry) {
    if (registry != NULL) {
        pthread_rwlock_unlock(&registry->rwlock);
    }
}
