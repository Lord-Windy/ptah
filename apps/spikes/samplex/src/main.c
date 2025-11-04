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

    printf("\nSamplex completed successfully!\n");
    return EXIT_SUCCESS;
}
