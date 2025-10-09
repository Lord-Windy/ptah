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

#define _XOPEN_SOURCE
#include "db.h"
#include "ohlcv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


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


SamrenaVector* samtrader_db_fetch_ohlcv(SamtraderDb* db, const char* code,
                                        const char* exchange, time_t start_time,
                                        time_t end_time) {
  if (!db || !db->conn) {
    fprintf(stderr, "Invalid database connection\n");
    return NULL;
  }

  // Build parameterized query
  const char* query =
      "SELECT code, exchange, date, open, high, low, close, volume "
      "FROM public.ohlcv "
      "WHERE code = $1 AND exchange = $2 AND date >= to_timestamp($3) AND date <= to_timestamp($4) "
      "ORDER BY date ASC";

  // Convert timestamps to strings for parameterized query
  char start_str[32], end_str[32];
  snprintf(start_str, sizeof(start_str), "%ld", (long)start_time);
  snprintf(end_str, sizeof(end_str), "%ld", (long)end_time);

  const char* param_values[4] = {code, exchange, start_str, end_str};

  // Execute query
  PGresult* res = PQexecParams(db->conn, query, 4, NULL, param_values, NULL, NULL, 0);

  if (PQresultStatus(res) != PGRES_TUPLES_OK) {
    fprintf(stderr, "Query failed: %s\n", PQerrorMessage(db->conn));
    PQclear(res);
    return NULL;
  }

  int num_rows = PQntuples(res);

  // Create vector to hold OHLCV pointers
  SamrenaVector* vec = samrena_vector_init(db->arena, sizeof(Ohlcv*), num_rows);
  if (!vec) {
    fprintf(stderr, "Failed to create vector\n");
    PQclear(res);
    return NULL;
  }

  // Parse each row and create OHLCV records
  for (int i = 0; i < num_rows; i++) {
    const char* row_code = PQgetvalue(res, i, 0);
    const char* row_exchange = PQgetvalue(res, i, 1);
    const char* row_date = PQgetvalue(res, i, 2);
    double open_val = atof(PQgetvalue(res, i, 3));
    double high_val = atof(PQgetvalue(res, i, 4));
    double low_val = atof(PQgetvalue(res, i, 5));
    double close_val = atof(PQgetvalue(res, i, 6));
    int volume_val = atoi(PQgetvalue(res, i, 7));

    // Parse ISO 8601 timestamp (simplified - assumes format from PostgreSQL)
    struct tm tm = {0};
    strptime(row_date, "%Y-%m-%d %H:%M:%S", &tm);
    time_t date_val = mktime(&tm);

    // Create OHLCV record using arena
    Ohlcv* ohlcv = ohlcv_create(db->arena, row_code, row_exchange, date_val,
                                open_val, high_val, low_val, close_val, volume_val);
    if (!ohlcv) {
      fprintf(stderr, "Failed to create OHLCV record\n");
      PQclear(res);
      return NULL;
    }

    // Push pointer to vector
    samrena_vector_push(vec, &ohlcv);
  }

  PQclear(res);
  return vec;
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
