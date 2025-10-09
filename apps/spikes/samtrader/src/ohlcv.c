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

#include "ohlcv.h"
#include <stdio.h>
#include <string.h>

Ohlcv *ohlcv_create(Samrena *arena, const char *code, const char *exchange,
                    time_t date, double open, double high, double low,
                    double close, int volume) {
    Ohlcv *ohlcv = samrena_push(arena, sizeof(Ohlcv));
    if (!ohlcv) {
        return NULL;
    }

    // Allocate and copy code string
    size_t code_len = strlen(code) + 1;
    ohlcv->code = samrena_push(arena, code_len);
    if (!ohlcv->code) {
        return NULL;
    }
    memcpy(ohlcv->code, code, code_len);

    // Allocate and copy exchange string
    size_t exchange_len = strlen(exchange) + 1;
    ohlcv->exchange = samrena_push(arena, exchange_len);
    if (!ohlcv->exchange) {
        return NULL;
    }
    memcpy(ohlcv->exchange, exchange, exchange_len);

    // Set numeric fields
    ohlcv->date = date;
    ohlcv->open = open;
    ohlcv->high = high;
    ohlcv->low = low;
    ohlcv->close = close;
    ohlcv->volume = volume;

    return ohlcv;
}

void ohlcv_print_vector(SamrenaVector *vec) {
    if (!vec) {
        printf("Vector is NULL\n");
        return;
    }

    size_t size = samrena_vector_size(vec);
    if (size == 0) {
        printf("No OHLCV records found\n");
        return;
    }

    printf("OHLCV Records (%zu total):\n", size);
    printf("%-10s %-10s %-20s %10s %10s %10s %10s %12s\n",
           "Code", "Exchange", "Date", "Open", "High", "Low", "Close", "Volume");
    printf("------------------------------------------------------------------------------------\n");

    for (size_t i = 0; i < size; i++) {
        Ohlcv **ohlcv_ptr = (Ohlcv **)samrena_vector_at(vec, i);
        if (!ohlcv_ptr || !*ohlcv_ptr) {
            printf("Invalid record at index %zu\n", i);
            continue;
        }

        Ohlcv *ohlcv = *ohlcv_ptr;

        // Format the timestamp
        char date_str[20];
        struct tm *tm_info = localtime(&ohlcv->date);
        strftime(date_str, sizeof(date_str), "%Y-%m-%d %H:%M:%S", tm_info);

        printf("%-10s %-10s %-20s %10.2f %10.2f %10.2f %10.2f %12d\n",
               ohlcv->code,
               ohlcv->exchange,
               date_str,
               ohlcv->open,
               ohlcv->high,
               ohlcv->low,
               ohlcv->close,
               ohlcv->volume);
    }
}
