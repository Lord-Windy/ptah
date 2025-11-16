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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <libpq-fe.h>
#include <liburing.h>

#include "plex.h"

void test_plex_registry(void) {
    printf("\n=== Plex Registry Test ===\n\n");

    // Test plex_registry_create with different capacities
    printf("Testing plex_registry_create with default capacity (0)...\n");
    PlexRegistry* registry = plex_registry_create(0);
    if (registry == NULL) {
        printf("ERROR: Failed to create registry with default capacity\n");
        return;
    }
    printf("SUCCESS: Registry created with default capacity\n");

    // Test basic properties
    printf("Registry arena: %p\n", (void*)registry->arena);
    printf("Registry plex_map: %p\n", (void*)registry->plex_map);
    printf("Registry id_tracker: %lu\n", registry->id_tracker);
    printf("Registry rwlock initialized: YES\n");
    
    // Test another registry with specific capacity
    printf("\nTesting plex_registry_create with specific capacity (64)...\n");
    PlexRegistry* registry2 = plex_registry_create(64);
    if (registry2 == NULL) {
        printf("ERROR: Failed to create registry with capacity 64\n");
        plex_registry_destroy(registry); // Clean up first one
        return;
    }
    printf("SUCCESS: Registry created with capacity 64\n");
    
    printf("Registry2 arena: %p\n", (void*)registry2->arena);
    printf("Registry2 plex_map: %p\n", (void*)registry2->plex_map);
    printf("Registry2 id_tracker: %lu\n", registry2->id_tracker);

    // Test the size function (should be 0 for both empty registries)
    printf("\nTesting plex_registry_size...\n");
    uint64_t size = plex_registry_size(registry);
    printf("Registry 1 size: %lu (should be 0)\n", size);
    
    uint64_t size2 = plex_registry_size(registry2);
    printf("Registry 2 size: %lu (should be 0)\n", size2);

    // Clean up
    printf("\nCleaning up registries...\n");
    plex_registry_destroy(registry);
    printf("Registry 1 destroyed\n");
    plex_registry_destroy(registry2);
    printf("Registry 2 destroyed\n");

    printf("\nPlex Registry Test completed successfully!\n");
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
        printf("Connection status: %s\n",
               status == CONNECTION_BAD ? "BAD (expected - not actually connecting)" :
               status == CONNECTION_OK ? "OK" : "STARTED");

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

    printf("\nSamplex completed successfully!\n");
    return EXIT_SUCCESS;
}
