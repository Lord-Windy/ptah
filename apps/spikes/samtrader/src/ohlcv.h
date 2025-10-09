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

#ifndef OHLCV_H
#define OHLCV_H

#include <samrena.h>
#include <samvector.h>
#include <time.h>

typedef struct {
    char *code;
    char *exchange;
    time_t date;
    double open;
    double high;
    double low;
    double close;
    int volume;
} Ohlcv;

/**
 * Creates a new OHLCV structure using arena allocation
 *
 * @param arena The arena to allocate from
 * @param code The stock/asset code
 * @param exchange The exchange name
 * @param date Unix timestamp
 * @param open Opening price
 * @param high Highest price
 * @param low Lowest price
 * @param close Closing price
 * @param volume Trading volume
 * @return Pointer to the allocated OHLCV structure
 */
Ohlcv *ohlcv_create(Samrena *arena, const char *code, const char *exchange,
                    time_t date, double open, double high, double low,
                    double close, int volume);

/**
 * Prints a vector of OHLCV records in order
 *
 * @param vec Vector containing Ohlcv* pointers
 */
void ohlcv_print_vector(SamrenaVector *vec);

#endif // OHLCV_H
