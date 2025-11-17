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
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int ohlcv_init(Ohlcv *ohlcv, Samrena *arena, const char *code, const char *exchange, time_t date,
               double open, double high, double low, double close, int volume) {
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

  ohlcv->ema_9 = 0;
  ohlcv->ema_21 = 0;
  ohlcv->ema_50 = 0;

  ohlcv->rsi_14 = 0;

  ohlcv->bollinger_20_upper = 0;
  ohlcv->bollinger_20_middle = 0;
  ohlcv->bollinger_20_lower = 0;

  ohlcv->pivot_point = 0;
  ohlcv->pivot_r1 = 0;
  ohlcv->pivot_r2 = 0;
  ohlcv->pivot_r3 = 0;
  ohlcv->pivot_s1 = 0;
  ohlcv->pivot_s2 = 0;
  ohlcv->pivot_s3 = 0;

  return 0;
}

Ohlcv *ohlcv_create(Samrena *arena, const char *code, const char *exchange, time_t date,
                    double open, double high, double low, double close, int volume) {
  Ohlcv *ohlcv = samrena_push(arena, sizeof(Ohlcv));
  if (!ohlcv) {
    return NULL;
  }

  if (ohlcv_init(ohlcv, arena, code, exchange, date, open, high, low, close, volume) != 0) {
    return NULL;
  }

  return ohlcv;
}

Ohlcv *ohlcv_push(SamrenaVector *vec, Samrena *arena, const char *code, const char *exchange,
                  time_t date, double open, double high, double low, double close, int volume) {
  if (!vec || !arena) {
    return NULL;
  }

  Ohlcv ohlcv_temp;
  if (ohlcv_init(&ohlcv_temp, arena, code, exchange, date, open, high, low, close, volume) != 0) {
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
  printf("%-10s %-10s %-12s %10s %10s %12s %10s %10s %10s %10s %10s %10s\n", "Code", "Exchange",
         "Date", "Open", "Close", "Volume", "SMA-20", "SMA-50", "SMA-200", "Pivot", "R1", "S1");
  printf("-----------------------------------------------------------------------------------------"
         "-----------------------------------\n");

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

    printf("%-10s %-10s %-12s %10.2f %10.2f %12d %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f\n",
           ohlcv->code, ohlcv->exchange, date_str, ohlcv->open, ohlcv->close, ohlcv->volume,
           ohlcv->sma_20, ohlcv->sma_50, ohlcv->sma_200, ohlcv->pivot_point, ohlcv->pivot_r1,
           ohlcv->pivot_s1);
  }
}

// vec is of OHLCV
// index is the current one being worked on
// period is the number of days
double ohlcv_calculate_sma(SamrenaVector *vec, size_t index, size_t period) {
  // If we don't have enough data for a full period, calculate SMA with available data
  size_t available_points = index + 1;
  size_t points_to_use = (available_points < period) ? available_points : period;

  if (points_to_use == 0) {
    return 0.0;
  }

  // Calculate the start index for our window
  size_t start_index = index - points_to_use + 1;

  // Sum up the closing prices over the available period
  double sum = 0.0;
  for (size_t i = start_index; i <= index; i++) {
    Ohlcv *ohlcv = (Ohlcv *)samrena_vector_at(vec, i);
    if (!ohlcv) {
      return 0.0; // Invalid data
    }
    sum += ohlcv->close;
  }

  // Return the average
  return sum / (double)points_to_use;
}

double ohlcv_calculate_ema(SamrenaVector *vec, size_t index, size_t period, double prev_ema) {
  // Get current closing price
  Ohlcv *curr_ohlcv = (Ohlcv *)samrena_vector_at(vec, index);
  if (!curr_ohlcv) {
    return 0.0;
  }

  // For the first EMA value (when prev_ema is 0), use SMA as the seed
  if (prev_ema == 0.0) {
    return ohlcv_calculate_sma(vec, index, period);
  }

  // Calculate the smoothing multiplier: 2 / (period + 1)
  double multiplier = 2.0 / (double)(period + 1);

  // Calculate EMA: (Close - EMA_prev) × multiplier + EMA_prev
  // This is equivalent to: Close × multiplier + EMA_prev × (1 - multiplier)
  return (curr_ohlcv->close - prev_ema) * multiplier + prev_ema;
}

double ohlcv_calculate_rsi(SamrenaVector *vec, size_t index, size_t period) {
  // We need at least 'period' data points to calculate RSI
  if (index < period) {
    return 0.0; // Not enough data
  }

  double total_gains = 0.0;
  double total_losses = 0.0;

  // Calculate gains and losses over the period
  // Start from (index - period + 1) and compare each close with the previous close
  for (size_t i = index - period + 1; i <= index; i++) {
    Ohlcv *curr_ohlcv = (Ohlcv *)samrena_vector_at(vec, i);
    Ohlcv *prev_ohlcv = (Ohlcv *)samrena_vector_at(vec, i - 1);

    if (!curr_ohlcv || !prev_ohlcv) {
      return 0.0; // Invalid data
    }

    double change = curr_ohlcv->close - prev_ohlcv->close;

    if (change > 0) {
      total_gains += change;
    } else if (change < 0) {
      total_losses += -change; // Store as positive value
    }
  }

  double average_gains = total_gains / (double)period;
  double average_losses = total_losses / (double)period;

  // Handle division by zero case
  if (average_losses == 0.0) {
    return 100.0; // If no losses, RSI is 100
  }

  double RS = average_gains / average_losses;

  return 100.0 - (100.0 / (1.0 + RS));
}

typedef struct {
  double upper;
  double middle;
  double lower;
} Bollinger;

typedef struct {
  double pivot;
  double r1;
  double r2;
  double r3;
  double s1;
  double s2;
  double s3;
} PivotPoints;

PivotPoints ohlcv_calculate_pivot_points(SamrenaVector *vec, size_t index) {
  PivotPoints pivot;

  pivot.pivot = 0;
  pivot.r1 = 0;
  pivot.r2 = 0;
  pivot.r3 = 0;
  pivot.s1 = 0;
  pivot.s2 = 0;
  pivot.s3 = 0;

  // Need at least one previous period to calculate pivot points
  if (index == 0) {
    return pivot;
  }

  // Get the previous period's data
  Ohlcv *prev_ohlcv = (Ohlcv *)samrena_vector_at(vec, index - 1);
  if (!prev_ohlcv) {
    return pivot;
  }

  // Calculate Pivot Point: PP = (High + Low + Close) / 3
  pivot.pivot = (prev_ohlcv->high + prev_ohlcv->low + prev_ohlcv->close) / 3.0;

  // Calculate Support and Resistance levels
  // R1 = (2 × PP) - Low
  pivot.r1 = (2.0 * pivot.pivot) - prev_ohlcv->low;

  // S1 = (2 × PP) - High
  pivot.s1 = (2.0 * pivot.pivot) - prev_ohlcv->high;

  // R2 = PP + (High - Low)
  pivot.r2 = pivot.pivot + (prev_ohlcv->high - prev_ohlcv->low);

  // S2 = PP - (High - Low)
  pivot.s2 = pivot.pivot - (prev_ohlcv->high - prev_ohlcv->low);

  // R3 = High + 2 × (PP - Low)
  pivot.r3 = prev_ohlcv->high + 2.0 * (pivot.pivot - prev_ohlcv->low);

  // S3 = Low - 2 × (High - PP)
  pivot.s3 = prev_ohlcv->low - 2.0 * (prev_ohlcv->high - pivot.pivot);

  return pivot;
}

Bollinger ohlcv_calculate_bollinger(SamrenaVector *vec, size_t index, double sma, size_t period) {

  Bollinger bol;

  bol.upper = 0;
  bol.middle = sma;
  bol.lower = 0;

  // If we don't have enough data for a full period, use available data
  size_t available_points = index + 1;
  size_t points_to_use = (available_points < period) ? available_points : period;

  if (points_to_use == 0) {
    return bol;
  }

  // Calculate the start index for our window
  size_t start_index = index - points_to_use + 1;

  // Calculate variance (sum of squared differences from the mean)
  double variance_sum = 0.0;
  for (size_t i = start_index; i <= index; i++) {
    Ohlcv *ohlcv = (Ohlcv *)samrena_vector_at(vec, i);
    if (!ohlcv) {
      return bol; // Invalid data
    }
    double diff = ohlcv->close - sma;
    variance_sum += diff * diff;
  }

  // Calculate standard deviation
  double variance = variance_sum / (double)points_to_use;
  double std_dev = sqrt(variance);

  // Bollinger Bands are typically SMA ± 2 standard deviations
  bol.upper = sma + (2.0 * std_dev);
  bol.lower = sma - (2.0 * std_dev);

  return bol;
}

void ohlcv_calculate_indicators(SamrenaVector *vec) {
  if (!vec) {
    return;
  }

  for (size_t i = 0; i < vec->size; i++) {
    Ohlcv *ohlcv = (Ohlcv *)samrena_vector_at(vec, i);
    if (!ohlcv) {
      continue;
    }

    // Calculate SMAs
    ohlcv->sma_20 = ohlcv_calculate_sma(vec, i, 20);
    ohlcv->sma_50 = ohlcv_calculate_sma(vec, i, 50);
    ohlcv->sma_200 = ohlcv_calculate_sma(vec, i, 200);

    // Calculate EMAs using previous values (0.0 for first element)
    double prev_ema_9 = 0.0;
    double prev_ema_21 = 0.0;
    double prev_ema_50 = 0.0;

    if (i > 0) {
      Ohlcv *prev_ohlcv = (Ohlcv *)samrena_vector_at(vec, i - 1);
      if (prev_ohlcv) {
        prev_ema_9 = prev_ohlcv->ema_9;
        prev_ema_21 = prev_ohlcv->ema_21;
        prev_ema_50 = prev_ohlcv->ema_50;
      }
    }

    ohlcv->ema_9 = ohlcv_calculate_ema(vec, i, 9, prev_ema_9);
    ohlcv->ema_21 = ohlcv_calculate_ema(vec, i, 21, prev_ema_21);
    ohlcv->ema_50 = ohlcv_calculate_ema(vec, i, 50, prev_ema_50);

    Bollinger bol = ohlcv_calculate_bollinger(vec, i, ohlcv->sma_20, 20);

    ohlcv->bollinger_20_upper = bol.upper;
    ohlcv->bollinger_20_middle = bol.middle;
    ohlcv->bollinger_20_lower = bol.lower;

    // Calculate pivot points
    PivotPoints pivot = ohlcv_calculate_pivot_points(vec, i);

    ohlcv->pivot_point = pivot.pivot;
    ohlcv->pivot_r1 = pivot.r1;
    ohlcv->pivot_r2 = pivot.r2;
    ohlcv->pivot_r3 = pivot.r3;
    ohlcv->pivot_s1 = pivot.s1;
    ohlcv->pivot_s2 = pivot.s2;
    ohlcv->pivot_s3 = pivot.s3;
  }
}
