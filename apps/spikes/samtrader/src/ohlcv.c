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
#include "samvector.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int ohlcv_init(Ohlcv *ohlcv, Samrena *arena, const char *code,
               const char *exchange, time_t date, double open, double high,
               double low, double close, int volume) {
    if (!ohlcv || !arena || !code || !exchange) {
        return -1;
    }

    // Allocate and copy code string
    size_t code_len = strlen(code) + 1;
    ohlcv->code = samrena_push(arena, code_len);
    if (!ohlcv->code) {
        return -1;
    }
    memcpy(ohlcv->code, code, code_len);

    // Allocate and copy exchange string
    size_t exchange_len = strlen(exchange) + 1;
    ohlcv->exchange = samrena_push(arena, exchange_len);
    if (!ohlcv->exchange) {
        return -1;
    }
    memcpy(ohlcv->exchange, exchange, exchange_len);

    // Set numeric fields
    ohlcv->date = date;
    ohlcv->open = open;
    ohlcv->high = high;
    ohlcv->low = low;
    ohlcv->close = close;
    ohlcv->volume = volume;

    // Initialize indicators
    ohlcv->sma_20 = 0;
    ohlcv->sma_50 = 0;
    ohlcv->sma_200 = 0;

    return 0;
}

Ohlcv *ohlcv_create(Samrena *arena, const char *code, const char *exchange,
                    time_t date, double open, double high, double low,
                    double close, int volume) {
    Ohlcv *ohlcv = samrena_push(arena, sizeof(Ohlcv));
    if (!ohlcv) {
        return NULL;
    }

    if (ohlcv_init(ohlcv, arena, code, exchange, date, open, high, low, close,
                   volume) != 0) {
        return NULL;
    }

    return ohlcv;
}

Ohlcv *ohlcv_push(SamrenaVector *vec, Samrena *arena, const char *code,
                  const char *exchange, time_t date, double open, double high,
                  double low, double close, int volume) {
    if (!vec || !arena) {
        return NULL;
    }

    Ohlcv ohlcv_temp;
    if (ohlcv_init(&ohlcv_temp, arena, code, exchange, date, open, high, low,
                   close, volume) != 0) {
        return NULL;
    }

    Ohlcv *pushed = samrena_vector_push(vec, &ohlcv_temp);
    return pushed;
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
    printf("%-10s %-10s %-12s %10s %10s %12s %10s %10s %10s\n",
           "Code", "Exchange", "Date", "Open", "Close", "Volume", "SMA-20", "SMA-50", "SMA-200");
    printf("----------------------------------------------------------------------------------------------------------\n");

    for (size_t i = 0; i < size; i++) {
        Ohlcv *ohlcv = (Ohlcv *)samrena_vector_at(vec, i);
        if (!ohlcv) {
            printf("Invalid record at index %zu\n", i);
            continue;
        }

        // Format the timestamp
        char date_str[12];
        struct tm *tm_info = localtime(&ohlcv->date);
        strftime(date_str, sizeof(date_str), "%Y-%m-%d", tm_info);

        printf("%-10s %-10s %-12s %10.2f %10.2f %12d %10.2f %10.2f %10.2f\n",
               ohlcv->code,
               ohlcv->exchange,
               date_str,
               ohlcv->open,
               ohlcv->close,
               ohlcv->volume,
               ohlcv->sma_20,
               ohlcv->sma_50,
               ohlcv->sma_200);
    }
}

// vec is of OHLCV
// index is the current one being worked on
// period is the number of days
double ohlcv_calculate_sma(SamrenaVector* vec, size_t index, size_t period) {
    // SMA requires at least 'period' data points
    // If we don't have enough data before the current index, return 0
    if (index + 1 < period) {
        return 0.0;
    }

    // Calculate the start index for our window
    size_t start_index = index - period + 1;

    // Sum up the closing prices over the period
    double sum = 0.0;
    for (size_t i = start_index; i <= index; i++) {
        Ohlcv *ohlcv = (Ohlcv *)samrena_vector_at(vec, i);
        if (!ohlcv) {
            return 0.0;  // Invalid data
        }
        sum += ohlcv->close;
    }

    // Return the average
    return sum / (double)period;
}

void ohlcv_calculate_indicators(SamrenaVector* vec) {
    if (!vec) {
        return;
    }

    for (size_t i = 0; i < vec->size; i++) {
        Ohlcv *ohlcv = (Ohlcv *)samrena_vector_at(vec, i);
        if (!ohlcv) {
            continue;
        }

        ohlcv->sma_20 = ohlcv_calculate_sma(vec, i, 20);
        ohlcv->sma_50 = ohlcv_calculate_sma(vec, i, 50);
        ohlcv->sma_200 = ohlcv_calculate_sma(vec, i, 200);
    }
}
