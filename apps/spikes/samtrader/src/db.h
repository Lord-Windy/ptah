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

#ifndef SAMTRADER_DB_H
#define SAMTRADER_DB_H

#include <libpq-fe.h>
#include "samrena.h"

typedef struct {
  char* conninfo;
  PGconn* conn;
  Samrena* arena;
} SamtraderDb;

// Database connection and utilities for samtrader
SamtraderDb* samtrader_db_connect(Samrena* arena, char* conn_info);
void samtrader_db_close(SamtraderDb* db);



// Debug
// Will print out information about the connection
void samtrader_db_debug_print_connection(SamtraderDb* db, bool show_conn_string);

#endif // SAMTRADER_DB_H
