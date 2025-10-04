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

#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>

#include "samrena.h"
#include "samdata.h"

int main(int argc, char *argv[]) {
    printf("SamTrader Prototype\n");
    printf("===================\n\n");

    SamrenaConfig config = samrena_default_config();
    config.initial_pages = 256;  // 256 pages * 4KB = 1MB

    Samrena *arena = samrena_create(&config);
    if (!arena) {
        fprintf(stderr, "Failed to create arena\n");
        return 1;
    }

    const char *conninfo = getenv("DATABASE_URL");
    if (!conninfo) {
        conninfo = "dbname=samtrader user=sam";
    }

    printf("Connecting to PostgreSQL...\n");
    PGconn *conn = PQconnectdb(conninfo);

    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to database failed: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        samrena_destroy(arena);
        return 1;
    }





    printf("Connected to PostgreSQL server version: %d\n", PQserverVersion(conn));

    printf("\nMemory Usage:\n");
    printf("  Arena allocated: %lu bytes\n", samrena_allocated(arena));
    printf("  Arena capacity: %lu bytes\n", samrena_capacity(arena));

    PQfinish(conn);
    samrena_destroy(arena);

    printf("\nSamTrader prototype shutdown complete.\n");
    return 0;
}
