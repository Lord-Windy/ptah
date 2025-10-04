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
#include <libpq-fe.h>
#include "db.h"
#include "samrena.h"
#include "samdata.h"


SamtraderDb* setup_db(Samrena* arena) {

  char* conn_info = "postgres://sam:password@localhost:5432/samtrader";
  SamtraderDb* db = samtrader_db_connect(arena, conn_info); 
  samtrader_db_debug_print_connection(db, true);

  return db;
}

int main(int argc, char *argv[]) {
    printf("samtrader prototype\n");

    // Initialize arena
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    SamtraderDb* db = setup_db(arena);


    // Cleanup
    samtrader_db_close(db);
    samrena_destroy(arena);

    return 0;
}
