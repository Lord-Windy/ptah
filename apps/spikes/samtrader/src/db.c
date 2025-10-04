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

#include "db.h"
#include <stdio.h>
#include <string.h>


SamtraderDb* samtrader_db_connect(Samrena* arena, char* conn_info) {

  SamtraderDb* db = SAMRENA_PUSH_TYPE(arena, SamtraderDb);

  db->arena = arena;

  // Copy connection string to arena
  size_t conn_len = strlen(conn_info) + 1;
  db->conninfo = SAMRENA_PUSH_ARRAY(arena, char, conn_len);
  memcpy(db->conninfo, conn_info, conn_len);

  // Establish PostgreSQL connection
  db->conn = PQconnectdb(conn_info);

  if (PQstatus(db->conn) != CONNECTION_OK) {
    fprintf(stderr, "Connection to database failed: %s\n", PQerrorMessage(db->conn));
    PQfinish(db->conn);
    return NULL;
  }

  return db;

}

void samtrader_db_close(SamtraderDb* db) {
  if (!db) {
    return;
  }

  if (db->conn) {
    PQfinish(db->conn);
    db->conn = NULL;
  }
}


void samtrader_db_debug_print_connection(SamtraderDb* db, bool show_conn_string){
  if (!db || !db->conn) {
    printf("Database: Not connected\n");
    return;
  }

  printf("Database Connection Info:\n");
  printf("  Status: %s\n", PQstatus(db->conn) == CONNECTION_OK ? "Connected" : "Disconnected");
  printf("  Database: %s\n", PQdb(db->conn));
  printf("  User: %s\n", PQuser(db->conn));
  printf("  Host: %s\n", PQhost(db->conn));
  printf("  Port: %s\n", PQport(db->conn));
  printf("  Protocol Version: %d\n", PQprotocolVersion(db->conn));
  printf("  Server Version: %d\n", PQserverVersion(db->conn));

  if (show_conn_string) {
    printf("  Connection String: %s\n", db->conninfo);
  }
}
