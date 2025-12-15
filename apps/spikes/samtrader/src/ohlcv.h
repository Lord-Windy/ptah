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
  // algoritms

  double sma_20;
  double sma_50;
  double sma_200;

  double ema_9;
  double ema_21;
  double ema_50;

  double rsi_14;

  double bollinger_20_upper;
  double bollinger_20_middle;
  double bollinger_20_lower;

  double pivot_point;
  double pivot_r1;
  double pivot_r2;
  double pivot_r3;
  double pivot_s1;
  double pivot_s2;
  double pivot_s3;
} Ohlcv;

/**
 * Initializes an existing OHLCV structure with the given values
 * The strings (code, exchange) are allocated from the arena
 *
 * @param ohlcv Pointer to OHLCV structure to initialize
 * @param arena The arena to allocate strings from
 * @param code The stock/asset code
 * @param exchange The exchange name
 * @param date Unix timestamp
 * @param open Opening price
 * @param high Highest price
 * @param low Lowest price
 * @param close Closing price
 * @param volume Trading volume
 * @return 0 on success, -1 on failure
 */
int ohlcv_init(Ohlcv *ohlcv, Samrena *arena, const char *code, const char *exchange, time_t date,
               double open, double high, double low, double close, int volume);

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
Ohlcv *ohlcv_create(Samrena *arena, const char *code, const char *exchange, time_t date,
                    double open, double high, double low, double close, int volume);

/**
 * Creates a new OHLCV structure directly in a vector
 *
 * @param vec Vector to push the OHLCV into
 * @param arena The arena to allocate strings from
 * @param code The stock/asset code
 * @param exchange The exchange name
 * @param date Unix timestamp
 * @param open Opening price
 * @param high Highest price
 * @param low Lowest price
 * @param close Closing price
 * @param volume Trading volume
 * @return Pointer to the OHLCV structure in the vector, or NULL on failure
 */
Ohlcv *ohlcv_push(SamrenaVector *vec, Samrena *arena, const char *code, const char *exchange,
                  time_t date, double open, double high, double low, double close, int volume);

/**
 * Prints a vector of OHLCV records in order
 *
 * @param vec Vector containing Ohlcv structures (not pointers)
 */
void ohlcv_print_vector(SamrenaVector *vec);

void ohlcv_calculate_indicators(SamrenaVector *vec);

#endif // OHLCV_H
