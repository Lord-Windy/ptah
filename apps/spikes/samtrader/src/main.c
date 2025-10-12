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
#include <time.h>
#include "db.h"
#include "ohlcv.h"
#include "samrena.h"
#include "samdata.h"


SamtraderDb* setup_db(Samrena* arena) {

  char* conn_info = "postgres://sam:password@localhost:5432/samtrader";
  SamtraderDb* db = samtrader_db_connect(arena, conn_info);
  samtrader_db_debug_print_connection(db, true);

  return db;
}

void debug_fetch_and_print_ohlcv(SamtraderDb* db, const char* code, const char* exchange, int start_year, int num_years) {
    // Create timestamps for the year range
    struct tm start_tm = {0};
    start_tm.tm_year = start_year - 1900;  // years since 1900
    start_tm.tm_mon = 0;                   // January
    start_tm.tm_mday = 1;
    time_t start_time = mktime(&start_tm);

    struct tm end_tm = {0};
    end_tm.tm_year = (start_year + num_years - 1) - 1900;
    end_tm.tm_mon = 11;                    // December
    end_tm.tm_mday = 31;
    end_tm.tm_hour = 23;
    end_tm.tm_min = 59;
    end_tm.tm_sec = 59;
    time_t end_time = mktime(&end_tm);

    // Fetch OHLCV data
    printf("\nFetching %s data from %s exchange for %d-%d (%d years)...\n\n",
           code, exchange, start_year, start_year + num_years - 1, num_years);
    SamrenaVector* ohlcv_data = samtrader_db_fetch_ohlcv(db, code, exchange, start_time, end_time);

    if (ohlcv_data) {
        size_t total_records = samrena_vector_size(ohlcv_data);
        printf("Total records fetched: %zu\n\n", total_records);

        // Calculate indicators for all records
        printf("Calculating indicators...\n");
        ohlcv_calculate_indicators(ohlcv_data);
        printf("Indicators calculated.\n\n");

        // Print only the last 10 records using slice
        size_t records_to_show = 10;
        if (total_records > records_to_show) {
            size_t start_index = total_records - records_to_show;
            SamrenaVector* slice = samrena_vector_slice(ohlcv_data, start_index, total_records, db->arena);
            if (slice) {
                printf("Showing last %zu records:\n", records_to_show);
                ohlcv_print_vector(slice);
            } else {
                printf("Failed to slice vector\n");
            }
        } else {
            ohlcv_print_vector(ohlcv_data);
        }
    } else {
        printf("Failed to fetch OHLCV data\n");
    }
}

int main(int argc, char *argv[]) {
    printf("samtrader prototype\n");

    // Initialize arena
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    SamtraderDb* db = setup_db(arena);

    // Debug: Fetch and print IBM data from US exchange for 2001-2003 (3 years)
    debug_fetch_and_print_ohlcv(db, "IBM", "US", 2001, 3);

    // Cleanup
    samtrader_db_close(db);
    samrena_destroy(arena);

    return 0;
}
