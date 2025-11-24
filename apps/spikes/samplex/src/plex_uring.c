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

#include "plex_uring.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Queue depth bounds
#define PLEX_URING_MIN_QUEUE_DEPTH 32
#define PLEX_URING_MAX_QUEUE_DEPTH 4096

PlexEventLoop *plex_event_loop_create(PlexRegistry *registry, uint32_t queue_depth) {
  // Validate input parameters
  if (registry == NULL) {
    fprintf(stderr, "plex_event_loop_create: registry cannot be NULL\n");
    return NULL;
  }

  // Validate and clamp queue_depth to reasonable bounds
  if (queue_depth < PLEX_URING_MIN_QUEUE_DEPTH) {
    queue_depth = PLEX_URING_MIN_QUEUE_DEPTH;
  } else if (queue_depth > PLEX_URING_MAX_QUEUE_DEPTH) {
    queue_depth = PLEX_URING_MAX_QUEUE_DEPTH;
  }

  // Allocate PlexEventLoop from registry's arena
  PlexEventLoop *loop = samrena_push(registry->arena, sizeof(PlexEventLoop));
  if (loop == NULL) {
    fprintf(stderr, "plex_event_loop_create: failed to allocate PlexEventLoop\n");
    return NULL;
  }

  // Initialize the io_uring ring
  int ret = io_uring_queue_init(queue_depth, &loop->ring, 0);
  if (ret < 0) {
    fprintf(stderr, "plex_event_loop_create: io_uring_queue_init failed: %s\n", strerror(-ret));
    // Note: arena-allocated memory doesn't need explicit free
    return NULL;
  }

  // Initialize the rest of the structure
  loop->registry = registry;
  loop->running = false;
  loop->arena = registry->arena;
  loop->queue_depth = queue_depth;

  return loop;
}

void plex_event_loop_destroy(PlexEventLoop *loop) {
  // NULL-safe check
  if (loop == NULL) {
    return;
  }

  // Clean up io_uring resources
  io_uring_queue_exit(&loop->ring);

  // Clear references
  loop->registry = NULL;
  loop->arena = NULL;

  // Note: The PlexEventLoop structure itself is arena-managed,
  // so we don't explicitly free it. The arena will handle cleanup
  // when the registry is destroyed.
}
