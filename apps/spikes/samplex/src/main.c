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

#define _GNU_SOURCE
#include <fcntl.h>
#include <libpq-fe.h>
#include <liburing.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "plex.h"
#include "plex_uring.h"

void test_plex_registry(void) {
  printf("\n=== Plex Registry Test ===\n\n");

  // Test plex_registry_create with different capacities
  printf("Testing plex_registry_create with default capacity (0)...\n");
  PlexRegistry *registry = plex_registry_create(0);
  if (registry == NULL) {
    printf("ERROR: Failed to create registry with default capacity\n");
    return;
  }
  printf("SUCCESS: Registry created with default capacity\n");

  // Test basic properties
  printf("Registry arena: %p\n", (void *)registry->arena);
  printf("Registry plex_map: %p\n", (void *)registry->plex_map);
  printf("Registry id_tracker: %lu\n", registry->id_tracker);
  printf("Registry rwlock initialized: YES\n");

  // Test another registry with specific capacity
  printf("\nTesting plex_registry_create with specific capacity (64)...\n");
  PlexRegistry *registry2 = plex_registry_create(64);
  if (registry2 == NULL) {
    printf("ERROR: Failed to create registry with capacity 64\n");
    plex_registry_destroy(registry); // Clean up first one
    return;
  }
  printf("SUCCESS: Registry created with capacity 64\n");

  printf("Registry2 arena: %p\n", (void *)registry2->arena);
  printf("Registry2 plex_map: %p\n", (void *)registry2->plex_map);
  printf("Registry2 id_tracker: %lu\n", registry2->id_tracker);

  // Test the size function (should be 0 for both empty registries)
  printf("\nTesting plex_registry_size...\n");
  uint64_t size = plex_registry_size(registry);
  printf("Registry 1 size: %lu (should be 0)\n", size);

  uint64_t size2 = plex_registry_size(registry2);
  printf("Registry 2 size: %lu (should be 0)\n", size2);

  // Test plex_create
  printf("\n=== Testing plex_create ===\n");
  Plex *plex1 = plex_create(registry, "Test Plex 1");
  if (plex1 == NULL) {
    printf("ERROR: Failed to create plex1\n");
    plex_registry_destroy(registry);
    plex_registry_destroy(registry2);
    return;
  }
  printf("SUCCESS: Created plex1 with ID: %lu\n", plex1->id);
  printf("  Description: %s\n", plex1->description);
  printf("  Arena: %p\n", (void *)plex1->arena);
  printf("  Items vector: %p (size: %zu)\n", (void *)plex1->items,
         samrena_vector_PlexItem_size(plex1->items));

  Plex *plex2 = plex_create(registry, "Test Plex 2");
  if (plex2 == NULL) {
    printf("ERROR: Failed to create plex2\n");
    plex_destroy(registry, plex1);
    plex_registry_destroy(registry);
    plex_registry_destroy(registry2);
    return;
  }
  printf("SUCCESS: Created plex2 with ID: %lu\n", plex2->id);
  printf("  Description: %s\n", plex2->description);

  // Test registry size after adding plexes
  size = plex_registry_size(registry);
  printf("\nRegistry size after adding 2 plexes: %lu (should be 2)\n", size);

  // Test plex_get_by_id
  printf("\n=== Testing plex_get_by_id ===\n");
  Plex *retrieved_plex1 = plex_get_by_id(registry, plex1->id);
  if (retrieved_plex1 == NULL) {
    printf("ERROR: Failed to retrieve plex1 by ID\n");
  } else if (retrieved_plex1 == plex1) {
    printf("SUCCESS: Retrieved plex1 by ID %lu\n", plex1->id);
    printf("  Pointers match: %p == %p\n", (void *)retrieved_plex1, (void *)plex1);
  } else {
    printf("ERROR: Retrieved plex does not match original\n");
  }

  Plex *retrieved_plex2 = plex_get_by_id(registry, plex2->id);
  if (retrieved_plex2 == NULL) {
    printf("ERROR: Failed to retrieve plex2 by ID\n");
  } else if (retrieved_plex2 == plex2) {
    printf("SUCCESS: Retrieved plex2 by ID %lu\n", plex2->id);
    printf("  Pointers match: %p == %p\n", (void *)retrieved_plex2, (void *)plex2);
  } else {
    printf("ERROR: Retrieved plex does not match original\n");
  }

  // Test retrieval of non-existent ID
  Plex *non_existent = plex_get_by_id(registry, 999);
  if (non_existent == NULL) {
    printf("SUCCESS: Non-existent ID 999 returned NULL (expected)\n");
  } else {
    printf("ERROR: Non-existent ID returned a plex (should be NULL)\n");
  }

  // Test plex_destroy
  printf("\n=== Testing plex_destroy ===\n");
  uint64_t plex1_id = plex1->id;  // Save ID before destroying
  plex_destroy(registry, plex1);
  printf("Destroyed plex1\n");

  size = plex_registry_size(registry);
  printf("Registry size after destroying plex1: %lu (should be 1)\n", size);

  // Verify plex1 is no longer retrievable
  retrieved_plex1 = plex_get_by_id(registry, plex1_id);
  if (retrieved_plex1 == NULL) {
    printf("SUCCESS: plex1 no longer in registry after destroy\n");
  } else {
    printf("ERROR: plex1 still in registry after destroy\n");
  }

  // Verify plex2 is still retrievable
  retrieved_plex2 = plex_get_by_id(registry, plex2->id);
  if (retrieved_plex2 == plex2) {
    printf("SUCCESS: plex2 still retrievable after plex1 destroy\n");
  } else {
    printf("ERROR: plex2 not retrievable after plex1 destroy\n");
  }

  // Clean up
  printf("\n=== Cleaning up ===\n");
  plex_destroy(registry, plex2);
  printf("Destroyed plex2\n");

  size = plex_registry_size(registry);
  printf("Registry size after destroying all plexes: %lu (should be 0)\n", size);

  plex_registry_destroy(registry);
  printf("Registry 1 destroyed\n");
  plex_registry_destroy(registry2);
  printf("Registry 2 destroyed\n");

  printf("\nPlex Registry Test completed successfully!\n");
}

void test_plex_event_loop(void) {
  printf("\n=== PlexEventLoop Test ===\n\n");

  // Create a registry for the event loop
  printf("Creating PlexRegistry...\n");
  PlexRegistry *registry = plex_registry_create(0);
  if (registry == NULL) {
    printf("ERROR: Failed to create registry\n");
    return;
  }
  printf("SUCCESS: Registry created\n");

  // Test creating event loop with various queue depths
  printf("\nTesting plex_event_loop_create with queue_depth=128...\n");
  PlexEventLoop *loop = plex_event_loop_create(registry, 128);
  if (loop == NULL) {
    printf("ERROR: Failed to create event loop\n");
    plex_registry_destroy(registry);
    return;
  }
  printf("SUCCESS: Event loop created\n");
  printf("  Registry: %p\n", (void *)loop->registry);
  printf("  Arena: %p\n", (void *)loop->arena);
  printf("  Queue depth: %u\n", loop->queue_depth);
  printf("  Running: %s\n", loop->running ? "true" : "false");

  // Verify queue depth
  if (loop->queue_depth != 128) {
    printf("ERROR: Queue depth mismatch (expected 128, got %u)\n", loop->queue_depth);
  } else {
    printf("SUCCESS: Queue depth is correct (128)\n");
  }

  // Verify initial state
  if (loop->running) {
    printf("ERROR: Loop should not be running initially\n");
  } else {
    printf("SUCCESS: Loop is not running initially\n");
  }

  // Test destroying event loop
  printf("\nTesting plex_event_loop_destroy...\n");
  plex_event_loop_destroy(loop);
  printf("SUCCESS: Event loop destroyed\n");

  // Test with minimum queue depth (should be clamped to 32)
  printf("\nTesting with queue_depth=10 (should be clamped to 32)...\n");
  PlexEventLoop *loop2 = plex_event_loop_create(registry, 10);
  if (loop2 == NULL) {
    printf("ERROR: Failed to create event loop with small queue depth\n");
  } else {
    printf("SUCCESS: Event loop created with clamped queue depth\n");
    printf("  Queue depth: %u (expected 32)\n", loop2->queue_depth);
    if (loop2->queue_depth == 32) {
      printf("SUCCESS: Queue depth correctly clamped to minimum\n");
    } else {
      printf("ERROR: Queue depth not clamped correctly\n");
    }
    plex_event_loop_destroy(loop2);
  }

  // Test with maximum queue depth (should be clamped to 4096)
  printf("\nTesting with queue_depth=10000 (should be clamped to 4096)...\n");
  PlexEventLoop *loop3 = plex_event_loop_create(registry, 10000);
  if (loop3 == NULL) {
    printf("ERROR: Failed to create event loop with large queue depth\n");
  } else {
    printf("SUCCESS: Event loop created with clamped queue depth\n");
    printf("  Queue depth: %u (expected 4096)\n", loop3->queue_depth);
    if (loop3->queue_depth == 4096) {
      printf("SUCCESS: Queue depth correctly clamped to maximum\n");
    } else {
      printf("ERROR: Queue depth not clamped correctly\n");
    }
    plex_event_loop_destroy(loop3);
  }

  // Test NULL safety
  printf("\nTesting NULL safety for plex_event_loop_destroy...\n");
  plex_event_loop_destroy(NULL);
  printf("SUCCESS: NULL destroy handled safely\n");

  // Test NULL registry
  printf("\nTesting NULL registry for plex_event_loop_create...\n");
  PlexEventLoop *loop4 = plex_event_loop_create(NULL, 128);
  if (loop4 == NULL) {
    printf("SUCCESS: NULL registry correctly rejected\n");
  } else {
    printf("ERROR: NULL registry should have been rejected\n");
    plex_event_loop_destroy(loop4);
  }

  // Clean up
  printf("\n=== Cleaning up ===\n");
  plex_registry_destroy(registry);
  printf("Registry destroyed\n");

  printf("\nPlexEventLoop Test completed successfully!\n");
}

// Context structure to pass event loop to handler
typedef struct {
  PlexEventLoop *loop;
  PlexRegistry *registry;
} TimeoutContext;

// Handler that gets called when timeout completes
void timeout_handler(PlexItem *self, void *result) {
  printf("Timeout completed! Result: %d\n", (int)(intptr_t)result);

  TimeoutContext *ctx = (TimeoutContext *)self->data;
  if (ctx && ctx->loop) {
    printf("Stopping event loop...\n");
    plex_event_loop_stop(ctx->loop);
  }
}

// Error handler for timeout operations
void timeout_error_handler(PlexItem *self, int error) {
  printf("ERROR: Timeout operation failed with error: %d (%s)\n", error, strerror(-error));

  TimeoutContext *ctx = (TimeoutContext *)self->data;
  if (ctx && ctx->loop) {
    printf("Stopping event loop due to error...\n");
    plex_event_loop_stop(ctx->loop);
  }
}

void test_plex_event_loop_run_stop(void) {
  printf("\n=== PlexEventLoop Run/Stop Test ===\n\n");

  // Create a registry for the event loop
  printf("Creating PlexRegistry...\n");
  PlexRegistry *registry = plex_registry_create(0);
  if (registry == NULL) {
    printf("ERROR: Failed to create registry\n");
    return;
  }
  printf("SUCCESS: Registry created\n");

  // Create event loop
  printf("Creating PlexEventLoop with queue_depth=128...\n");
  PlexEventLoop *loop = plex_event_loop_create(registry, 128);
  if (loop == NULL) {
    printf("ERROR: Failed to create event loop\n");
    plex_registry_destroy(registry);
    return;
  }
  printf("SUCCESS: Event loop created\n");

  // Create context for the handler
  TimeoutContext *ctx = SAMRENA_PUSH_TYPE(registry->arena, TimeoutContext);
  if (ctx == NULL) {
    printf("ERROR: Failed to allocate context\n");
    plex_event_loop_destroy(loop);
    plex_registry_destroy(registry);
    return;
  }
  ctx->loop = loop;
  ctx->registry = registry;

  // Create a PlexItem for the timeout operation
  printf("Creating PlexItem for timeout test...\n");
  PlexItem *timeout_item = plex_item_create(
      "Timeout Test",
      registry->arena,
      timeout_handler,
      timeout_error_handler,
      NULL,  // no cleanup needed
      ctx    // pass context as user data
  );
  if (timeout_item == NULL) {
    printf("ERROR: Failed to create PlexItem\n");
    plex_event_loop_destroy(loop);
    plex_registry_destroy(registry);
    return;
  }
  printf("SUCCESS: PlexItem created\n");

  // Submit a timeout operation (1 second)
  printf("Submitting 1-second timeout operation to io_uring...\n");
  struct __kernel_timespec ts = {.tv_sec = 1, .tv_nsec = 0};
  struct io_uring_sqe *sqe = io_uring_get_sqe(&loop->ring);
  if (sqe == NULL) {
    printf("ERROR: Failed to get SQE\n");
    plex_event_loop_destroy(loop);
    plex_registry_destroy(registry);
    return;
  }

  io_uring_prep_timeout(sqe, &ts, 0, 0);
  io_uring_sqe_set_data(sqe, timeout_item);
  printf("SUCCESS: Timeout operation queued\n");

  // Run the event loop
  printf("\nRunning event loop (should run for ~1 second)...\n");
  printf("Loop running flag: %s\n", loop->running ? "true" : "false");

  int64_t start_time = plex_get_time_ns();
  int result = plex_event_loop_run(loop);
  int64_t end_time = plex_get_time_ns();

  double duration_sec = (end_time - start_time) / 1e9;

  printf("\nEvent loop exited with result: %d\n", result);
  printf("Loop running flag after exit: %s\n", loop->running ? "true" : "false");
  printf("Duration: %.3f seconds\n", duration_sec);

  // Verify results
  if (result == 0) {
    printf("SUCCESS: Event loop exited cleanly\n");
  } else {
    printf("ERROR: Event loop exited with error\n");
  }

  if (!loop->running) {
    printf("SUCCESS: Loop running flag is false after stop\n");
  } else {
    printf("ERROR: Loop running flag is still true\n");
  }

  if (duration_sec >= 0.9 && duration_sec <= 1.5) {
    printf("SUCCESS: Duration is approximately 1 second (%.3f sec)\n", duration_sec);
  } else {
    printf("WARNING: Duration is not close to 1 second (%.3f sec)\n", duration_sec);
  }

  // Clean up
  printf("\n=== Cleaning up ===\n");
  plex_event_loop_destroy(loop);
  printf("Event loop destroyed\n");
  plex_registry_destroy(registry);
  printf("Registry destroyed\n");

  printf("\nPlexEventLoop Run/Stop Test completed successfully!\n");
}

int main(void) {
  printf("=== Samplex: libpq Hello World ===\n\n");

  // Get libpq version
  int version = PQlibVersion();
  int major = version / 10000;
  int minor = (version % 10000) / 100;
  int patch = version % 100;

  printf("libpq version: %d.%d.%d\n", major, minor, patch);
  printf("libpq is working correctly!\n\n");

  // Demonstrate basic connection string parsing (without actually connecting)
  const char *keywords[] = {"host", "port", "dbname", "user", NULL};
  const char *values[] = {"localhost", "5432", "testdb", "testuser", NULL};

  PGconn *conn = PQconnectStartParams(keywords, values, 0);

  if (conn) {
    ConnStatusType status = PQstatus(conn);
    printf("Connection object created successfully\n");
    printf("Connection status: %s\n", status == CONNECTION_BAD
                                          ? "BAD (expected - not actually connecting)"
                                      : status == CONNECTION_OK ? "OK"
                                                                : "STARTED");

    PQfinish(conn);
  } else {
    printf("Failed to create connection object\n");
  }

  // io_uring demonstration
  printf("\n=== io_uring Hello World ===\n\n");

  struct io_uring ring;
  int ret = io_uring_queue_init(8, &ring, 0);

  if (ret < 0) {
    fprintf(stderr, "Failed to initialize io_uring: %s\n", strerror(-ret));
    return EXIT_FAILURE;
  }

  printf("io_uring initialized successfully!\n");
  printf("io_uring queue depth: 8\n");

  // Simple no-op operation to demonstrate io_uring is working
  struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
  if (sqe) {
    io_uring_prep_nop(sqe);
    io_uring_sqe_set_data(sqe, (void *)1);

    ret = io_uring_submit(&ring);
    if (ret < 0) {
      fprintf(stderr, "Failed to submit io_uring operation: %s\n", strerror(-ret));
      io_uring_queue_exit(&ring);
      return EXIT_FAILURE;
    }

    printf("Submitted no-op operation to io_uring\n");

    // Wait for completion
    struct io_uring_cqe *cqe;
    ret = io_uring_wait_cqe(&ring, &cqe);
    if (ret < 0) {
      fprintf(stderr, "Failed to wait for completion: %s\n", strerror(-ret));
      io_uring_queue_exit(&ring);
      return EXIT_FAILURE;
    }

    printf("io_uring operation completed successfully!\n");
    printf("Result: %d\n", cqe->res);

    io_uring_cqe_seen(&ring, cqe);
  } else {
    fprintf(stderr, "Failed to get submission queue entry\n");
    io_uring_queue_exit(&ring);
    return EXIT_FAILURE;
  }

  io_uring_queue_exit(&ring);

  // Run the Plex Registry test
  test_plex_registry();

  // Run the PlexEventLoop test
  test_plex_event_loop();

  // Run the PlexEventLoop Run/Stop test
  test_plex_event_loop_run_stop();

  printf("\nSamplex completed successfully!\n");
  return EXIT_SUCCESS;
}
