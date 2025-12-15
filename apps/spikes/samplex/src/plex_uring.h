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

#ifndef PLEX_URING_H
#define PLEX_URING_H

// Feature test macro for sigset_t and other POSIX types
#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <liburing.h>

#include "plex.h"

/**
 * PlexEventLoop - io_uring-based event loop for async operations
 */
typedef struct PlexEventLoop {
  struct io_uring ring;     // io_uring instance
  PlexRegistry *registry;   // Reference to plex registry
  bool running;             // Loop state flag
  Samrena *arena;           // Memory backing
  uint32_t queue_depth;     // CQ/SQ size
} PlexEventLoop;

/**
 * Create event loop with specified queue depth
 * @param registry The PlexRegistry to associate with this event loop (must not be NULL)
 * @param queue_depth The desired queue depth (will be clamped to 32-4096 range)
 * @return Newly allocated PlexEventLoop or NULL on failure
 */
PlexEventLoop *plex_event_loop_create(PlexRegistry *registry, uint32_t queue_depth);

/**
 * Destroy event loop and cleanup resources
 * @param loop The event loop to destroy (NULL-safe)
 */
void plex_event_loop_destroy(PlexEventLoop *loop);

/**
 * Run the event loop (blocking until stopped)
 * Processes io_uring completions and dispatches to PlexItem handlers
 * @param loop The event loop to run (must not be NULL)
 * @return 0 on clean shutdown, -1 on error
 */
int plex_event_loop_run(PlexEventLoop *loop);

/**
 * Signal the event loop to stop
 * Thread-safe, can be called from signal handlers or PlexItem handlers
 * @param loop The event loop to stop (NULL-safe)
 */
void plex_event_loop_stop(PlexEventLoop *loop);

#endif // PLEX_URING_H
