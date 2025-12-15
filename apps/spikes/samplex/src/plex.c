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

#include "plex.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ============================================================================
// Utility Functions
// ============================================================================

int64_t plex_get_time_ns(void) {
  struct timespec ts;
  if (clock_gettime(CLOCK_REALTIME, &ts) == 0) {
    return (int64_t)ts.tv_sec * 1000000000L + ts.tv_nsec;
  }
  return 0;
}

const char *plex_id_to_key(uint64_t id) {
  static char buffer[32];
  snprintf(buffer, sizeof(buffer), "%lu", id);
  return buffer;
}

// ============================================================================
// PlexItem Functions
// ============================================================================

PlexItem *plex_item_create(const char *description, Samrena *arena, PlexItemHandler handler,
                           PlexItemErrorHandler error_handler, PlexItemCleanup cleanup,
                           void *data) {
  if (arena == NULL) {
    return NULL;
  }

  PlexItem *item = SAMRENA_PUSH_TYPE(arena, PlexItem);
  if (item == NULL) {
    return NULL;
  }

  if (description != NULL) {
    size_t len = strlen(description);
    item->description = (char *)samrena_push(arena, len + 1);
    if (item->description != NULL) {
      memcpy(item->description, description, len + 1);
    }
  } else {
    item->description = NULL;
  }

  item->data = data;
  item->handler = handler;
  item->error_handler = error_handler;
  item->cleanup = cleanup;
  item->start_time_ns = 0;
  item->stop_time_ns = 0;

  return item;
}

void plex_item_destroy(PlexItem *item) {
  if (item != NULL && item->cleanup != NULL) {
    item->cleanup(item);
    item->cleanup = NULL;
  }
}

bool plex_add_item(Plex *plex, PlexItem *item) {
  if (plex == NULL || plex->items == NULL || item == NULL) {
    return false;
  }
  return samrena_vector_PlexItem_push(plex->items, item) != NULL;
}

bool plex_remove_item(Plex *plex, PlexItem *item) {
  if (plex == NULL || plex->items == NULL || item == NULL) {
    return false;
  }

  size_t size = samrena_vector_PlexItem_size(plex->items);
  for (size_t i = 0; i < size; i++) {
    PlexItem *current = samrena_vector_PlexItem_at(plex->items, i);

    // Check identification
    bool match = false;
    if (current == item) {
      match = true;
    } else if (current->data == item->data) {
      // If descriptions match
      if (current->description == item->description) {
        match = true;
      } else if (current->description != NULL && item->description != NULL &&
                 strcmp(current->description, item->description) == 0) {
        match = true;
      } else if (current->description == NULL && item->description == NULL) {
        match = true;
      }
    }

    if (match) {
      // Cleanup the existing one
      if (current->cleanup != NULL) {
        current->cleanup(current);
      }

      // Swap removal
      if (i < size - 1) {
        PlexItem *last = samrena_vector_PlexItem_at(plex->items, size - 1);
        if (last != NULL) {
          *current = *last;
        }
      }
      samrena_vector_PlexItem_pop(plex->items);
      return true;
    }
  }
  return false;
}

PlexItem *plex_find_item(Plex *plex, const char *description) {
  if (plex == NULL || plex->items == NULL || description == NULL) {
    return NULL;
  }

  size_t size = samrena_vector_PlexItem_size(plex->items);
  for (size_t i = 0; i < size; i++) {
    PlexItem *item = samrena_vector_PlexItem_at(plex->items, i);
    if (item->description != NULL && strcmp(item->description, description) == 0) {
      return item;
    }
  }
  return NULL;
}

// ============================================================================
// PlexItem Execution and Timing
// ============================================================================

void plex_item_start(PlexItem *item) {
  if (item != NULL) {
    item->start_time_ns = plex_get_time_ns();
  }
}

void plex_item_stop(PlexItem *item) {
  if (item != NULL) {
    item->stop_time_ns = plex_get_time_ns();
  }
}

void plex_item_execute(PlexItem *item, void *result) {
  if (item != NULL && item->handler != NULL) {
    item->handler(item, result);
  }
}

void plex_item_error(PlexItem *item, int32_t error) {
  if (item != NULL && item->error_handler != NULL) {
    item->error_handler(item, error);
  }
}

int64_t plex_item_duration_ns(PlexItem *item) {
  if (item != NULL && item->start_time_ns > 0 && item->stop_time_ns >= item->start_time_ns) {
    return item->stop_time_ns - item->start_time_ns;
  }
  return -1;
}

// ============================================================================
// Plex Timing and Lifecycle
// ============================================================================

void plex_start(Plex *plex) {
  if (plex != NULL) {
    plex->start_time_ns = plex_get_time_ns();
  }
}

void plex_stop(Plex *plex) {
  if (plex != NULL) {
    plex->stop_time_ns = plex_get_time_ns();
  }
}

int64_t plex_duration_ns(Plex *plex) {
  if (plex != NULL && plex->start_time_ns > 0 && plex->stop_time_ns >= plex->start_time_ns) {
    return plex->stop_time_ns - plex->start_time_ns;
  }
  return -1;
}

void plex_cleanup_items(Plex *plex) {
  if (plex == NULL || plex->items == NULL) {
    return;
  }
  size_t size = samrena_vector_PlexItem_size(plex->items);
  for (size_t i = 0; i < size; i++) {
    PlexItem *item = samrena_vector_PlexItem_at(plex->items, i);
    if (item && item->cleanup != NULL) {
      item->cleanup(item);
      item->cleanup = NULL;
    }
  }
}

void plex_get_stats(Plex *plex, uint64_t *total_items, uint64_t *completed_items,
                    int64_t *avg_duration_ns) {
  if (plex == NULL) {
    if (total_items)
      *total_items = 0;
    if (completed_items)
      *completed_items = 0;
    if (avg_duration_ns)
      *avg_duration_ns = 0;
    return;
  }

  uint64_t total = 0;
  if (plex->items != NULL) {
    total = samrena_vector_PlexItem_size(plex->items);
  }

  uint64_t completed = 0;
  int64_t total_dur = 0;

  for (size_t i = 0; i < total; i++) {
    PlexItem *item = samrena_vector_PlexItem_at(plex->items, i);
    int64_t dur = plex_item_duration_ns(item);
    if (dur >= 0) {
      completed++;
      total_dur += dur;
    }
  }

  if (total_items)
    *total_items = total;
  if (completed_items)
    *completed_items = completed;
  if (avg_duration_ns) {
    *avg_duration_ns = (completed > 0) ? (total_dur / completed) : 0;
  }
}
