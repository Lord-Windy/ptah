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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "samtrader/domain/indicator.h"
#include "samtrader/domain/ohlcv.h"

#define ASSERT(cond, msg)                                                                          \
  do {                                                                                             \
    if (!(cond)) {                                                                                 \
      printf("FAIL: %s\n", msg);                                                                   \
      return 1;                                                                                    \
    }                                                                                              \
  } while (0)

#define ASSERT_DOUBLE_EQ(a, b, msg)                                                                \
  do {                                                                                             \
    if (fabs((a) - (b)) > 0.0001) {                                                                \
      printf("FAIL: %s (expected %f, got %f)\n", msg, (b), (a));                                   \
      return 1;                                                                                    \
    }                                                                                              \
  } while (0)

/* Helper to create test OHLCV data with known close prices */
static SamrenaVector *create_test_ohlcv(Samrena *arena, const double *closes, size_t count) {
  SamrenaVector *vec = samtrader_ohlcv_vector_create(arena, count);
  if (!vec) {
    return NULL;
  }

  for (size_t i = 0; i < count; i++) {
    SamtraderOhlcv bar = {.code = "TEST",
                          .exchange = "US",
                          .date = (time_t)(1704067200 + i * 86400),
                          .open = closes[i],
                          .high = closes[i] + 1.0,
                          .low = closes[i] - 1.0,
                          .close = closes[i],
                          .volume = 1000000};
    samrena_vector_push(vec, &bar);
  }

  return vec;
}

/*============================================================================
 * SMA Tests
 *============================================================================*/

static int test_sma_basic(void) {
  printf("Testing SMA basic calculation...\n");

  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  /* Simple data: 1, 2, 3, 4, 5 with period 3 */
  double closes[] = {1.0, 2.0, 3.0, 4.0, 5.0};
  SamrenaVector *ohlcv = create_test_ohlcv(arena, closes, 5);
  ASSERT(ohlcv != NULL, "Failed to create OHLCV data");

  SamtraderIndicatorSeries *sma = samtrader_calculate_sma(arena, ohlcv, 3);
  ASSERT(sma != NULL, "Failed to calculate SMA");
  ASSERT(sma->type == SAMTRADER_IND_SMA, "Type should be SMA");
  ASSERT(sma->params.period == 3, "Period should be 3");
  ASSERT(samtrader_indicator_series_size(sma) == 5, "Should have 5 values");

  /* First two values should be invalid (warmup) */
  const SamtraderIndicatorValue *val = samtrader_indicator_series_at(sma, 0);
  ASSERT(val != NULL && val->valid == false, "Index 0 should be invalid");

  val = samtrader_indicator_series_at(sma, 1);
  ASSERT(val != NULL && val->valid == false, "Index 1 should be invalid");

  /* SMA(3) at index 2: (1+2+3)/3 = 2.0 */
  val = samtrader_indicator_series_at(sma, 2);
  ASSERT(val != NULL && val->valid == true, "Index 2 should be valid");
  ASSERT_DOUBLE_EQ(val->data.simple.value, 2.0, "SMA at index 2");

  /* SMA(3) at index 3: (2+3+4)/3 = 3.0 */
  val = samtrader_indicator_series_at(sma, 3);
  ASSERT(val != NULL && val->valid == true, "Index 3 should be valid");
  ASSERT_DOUBLE_EQ(val->data.simple.value, 3.0, "SMA at index 3");

  /* SMA(3) at index 4: (3+4+5)/3 = 4.0 */
  val = samtrader_indicator_series_at(sma, 4);
  ASSERT(val != NULL && val->valid == true, "Index 4 should be valid");
  ASSERT_DOUBLE_EQ(val->data.simple.value, 4.0, "SMA at index 4");

  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

static int test_sma_period_1(void) {
  printf("Testing SMA with period 1...\n");

  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  double closes[] = {10.0, 20.0, 30.0};
  SamrenaVector *ohlcv = create_test_ohlcv(arena, closes, 3);
  ASSERT(ohlcv != NULL, "Failed to create OHLCV data");

  SamtraderIndicatorSeries *sma = samtrader_calculate_sma(arena, ohlcv, 1);
  ASSERT(sma != NULL, "Failed to calculate SMA");

  /* All values should be valid and equal to the close price */
  for (size_t i = 0; i < 3; i++) {
    const SamtraderIndicatorValue *val = samtrader_indicator_series_at(sma, i);
    ASSERT(val != NULL && val->valid == true, "All values should be valid");
    ASSERT_DOUBLE_EQ(val->data.simple.value, closes[i], "SMA(1) should equal close price");
  }

  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

static int test_sma_invalid_params(void) {
  printf("Testing SMA with invalid parameters...\n");

  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  double closes[] = {1.0, 2.0, 3.0};
  SamrenaVector *ohlcv = create_test_ohlcv(arena, closes, 3);

  /* NULL arena */
  ASSERT(samtrader_calculate_sma(NULL, ohlcv, 3) == NULL, "NULL arena should fail");

  /* NULL ohlcv */
  ASSERT(samtrader_calculate_sma(arena, NULL, 3) == NULL, "NULL ohlcv should fail");

  /* Invalid period */
  ASSERT(samtrader_calculate_sma(arena, ohlcv, 0) == NULL, "Period 0 should fail");
  ASSERT(samtrader_calculate_sma(arena, ohlcv, -1) == NULL, "Negative period should fail");

  /* Empty vector */
  SamrenaVector *empty = samtrader_ohlcv_vector_create(arena, 10);
  ASSERT(samtrader_calculate_sma(arena, empty, 3) == NULL, "Empty vector should fail");

  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

/*============================================================================
 * EMA Tests
 *============================================================================*/

static int test_ema_basic(void) {
  printf("Testing EMA basic calculation...\n");

  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  /* Data: 1, 2, 3, 4, 5 with period 3 */
  double closes[] = {1.0, 2.0, 3.0, 4.0, 5.0};
  SamrenaVector *ohlcv = create_test_ohlcv(arena, closes, 5);
  ASSERT(ohlcv != NULL, "Failed to create OHLCV data");

  SamtraderIndicatorSeries *ema = samtrader_calculate_ema(arena, ohlcv, 3);
  ASSERT(ema != NULL, "Failed to calculate EMA");
  ASSERT(ema->type == SAMTRADER_IND_EMA, "Type should be EMA");
  ASSERT(ema->params.period == 3, "Period should be 3");

  /* First two values should be invalid */
  const SamtraderIndicatorValue *val = samtrader_indicator_series_at(ema, 0);
  ASSERT(val != NULL && val->valid == false, "Index 0 should be invalid");

  val = samtrader_indicator_series_at(ema, 1);
  ASSERT(val != NULL && val->valid == false, "Index 1 should be invalid");

  /* EMA at index 2: initial value = SMA = (1+2+3)/3 = 2.0 */
  val = samtrader_indicator_series_at(ema, 2);
  ASSERT(val != NULL && val->valid == true, "Index 2 should be valid");
  ASSERT_DOUBLE_EQ(val->data.simple.value, 2.0, "EMA at index 2 (initial SMA)");

  /* EMA at index 3: k = 2/(3+1) = 0.5, EMA = 4*0.5 + 2.0*0.5 = 3.0 */
  val = samtrader_indicator_series_at(ema, 3);
  ASSERT(val != NULL && val->valid == true, "Index 3 should be valid");
  ASSERT_DOUBLE_EQ(val->data.simple.value, 3.0, "EMA at index 3");

  /* EMA at index 4: EMA = 5*0.5 + 3.0*0.5 = 4.0 */
  val = samtrader_indicator_series_at(ema, 4);
  ASSERT(val != NULL && val->valid == true, "Index 4 should be valid");
  ASSERT_DOUBLE_EQ(val->data.simple.value, 4.0, "EMA at index 4");

  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

static int test_ema_convergence(void) {
  printf("Testing EMA convergence to constant value...\n");

  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  /* Constant data should converge EMA to that value */
  double closes[] = {10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0};
  SamrenaVector *ohlcv = create_test_ohlcv(arena, closes, 10);

  SamtraderIndicatorSeries *ema = samtrader_calculate_ema(arena, ohlcv, 3);
  ASSERT(ema != NULL, "Failed to calculate EMA");

  /* All valid values should equal 10.0 */
  for (size_t i = 2; i < 10; i++) {
    const SamtraderIndicatorValue *val = samtrader_indicator_series_at(ema, i);
    ASSERT(val != NULL && val->valid == true, "Should be valid");
    ASSERT_DOUBLE_EQ(val->data.simple.value, 10.0, "EMA should converge to constant");
  }

  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

static int test_ema_invalid_params(void) {
  printf("Testing EMA with invalid parameters...\n");

  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  double closes[] = {1.0, 2.0, 3.0};
  SamrenaVector *ohlcv = create_test_ohlcv(arena, closes, 3);

  ASSERT(samtrader_calculate_ema(NULL, ohlcv, 3) == NULL, "NULL arena should fail");
  ASSERT(samtrader_calculate_ema(arena, NULL, 3) == NULL, "NULL ohlcv should fail");
  ASSERT(samtrader_calculate_ema(arena, ohlcv, 0) == NULL, "Period 0 should fail");

  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

/*============================================================================
 * WMA Tests
 *============================================================================*/

static int test_wma_basic(void) {
  printf("Testing WMA basic calculation...\n");

  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  /* Data: 1, 2, 3, 4, 5 with period 3 */
  double closes[] = {1.0, 2.0, 3.0, 4.0, 5.0};
  SamrenaVector *ohlcv = create_test_ohlcv(arena, closes, 5);
  ASSERT(ohlcv != NULL, "Failed to create OHLCV data");

  SamtraderIndicatorSeries *wma = samtrader_calculate_wma(arena, ohlcv, 3);
  ASSERT(wma != NULL, "Failed to calculate WMA");
  ASSERT(wma->type == SAMTRADER_IND_WMA, "Type should be WMA");
  ASSERT(wma->params.period == 3, "Period should be 3");

  /* First two values should be invalid */
  const SamtraderIndicatorValue *val = samtrader_indicator_series_at(wma, 0);
  ASSERT(val != NULL && val->valid == false, "Index 0 should be invalid");

  val = samtrader_indicator_series_at(wma, 1);
  ASSERT(val != NULL && val->valid == false, "Index 1 should be invalid");

  /* WMA at index 2: (1*1 + 2*2 + 3*3) / (1+2+3) = (1+4+9)/6 = 14/6 = 2.333... */
  val = samtrader_indicator_series_at(wma, 2);
  ASSERT(val != NULL && val->valid == true, "Index 2 should be valid");
  ASSERT_DOUBLE_EQ(val->data.simple.value, 14.0 / 6.0, "WMA at index 2");

  /* WMA at index 3: (2*1 + 3*2 + 4*3) / 6 = (2+6+12)/6 = 20/6 = 3.333... */
  val = samtrader_indicator_series_at(wma, 3);
  ASSERT(val != NULL && val->valid == true, "Index 3 should be valid");
  ASSERT_DOUBLE_EQ(val->data.simple.value, 20.0 / 6.0, "WMA at index 3");

  /* WMA at index 4: (3*1 + 4*2 + 5*3) / 6 = (3+8+15)/6 = 26/6 = 4.333... */
  val = samtrader_indicator_series_at(wma, 4);
  ASSERT(val != NULL && val->valid == true, "Index 4 should be valid");
  ASSERT_DOUBLE_EQ(val->data.simple.value, 26.0 / 6.0, "WMA at index 4");

  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

static int test_wma_weighting(void) {
  printf("Testing WMA weighting behavior...\n");

  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  /* WMA should weight recent prices higher */
  /* With period 2: WMA = (old*1 + new*2) / 3 */
  double closes[] = {10.0, 20.0};
  SamrenaVector *ohlcv = create_test_ohlcv(arena, closes, 2);

  SamtraderIndicatorSeries *wma = samtrader_calculate_wma(arena, ohlcv, 2);
  ASSERT(wma != NULL, "Failed to calculate WMA");

  /* WMA at index 1: (10*1 + 20*2) / 3 = 50/3 = 16.666... */
  /* This is closer to 20 than SMA would be (15.0) */
  const SamtraderIndicatorValue *val = samtrader_indicator_series_at(wma, 1);
  ASSERT(val != NULL && val->valid == true, "Should be valid");
  ASSERT_DOUBLE_EQ(val->data.simple.value, 50.0 / 3.0, "WMA should weight recent higher");

  /* Verify it's greater than SMA */
  double sma = (10.0 + 20.0) / 2.0;
  ASSERT(val->data.simple.value > sma, "WMA should be > SMA when prices are rising");

  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

static int test_wma_invalid_params(void) {
  printf("Testing WMA with invalid parameters...\n");

  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  double closes[] = {1.0, 2.0, 3.0};
  SamrenaVector *ohlcv = create_test_ohlcv(arena, closes, 3);

  ASSERT(samtrader_calculate_wma(NULL, ohlcv, 3) == NULL, "NULL arena should fail");
  ASSERT(samtrader_calculate_wma(arena, NULL, 3) == NULL, "NULL ohlcv should fail");
  ASSERT(samtrader_calculate_wma(arena, ohlcv, 0) == NULL, "Period 0 should fail");

  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

/*============================================================================
 * RSI Tests
 *============================================================================*/

static int test_rsi_basic(void) {
  printf("Testing RSI basic calculation...\n");

  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  /* Data with alternating gains and losses, period 5 */
  double closes[] = {44.0,  44.25, 44.50, 43.75, 44.50, 44.25, 43.75, 44.00,
                     43.50, 44.00, 44.50, 44.25, 44.75, 45.00, 45.50};
  SamrenaVector *ohlcv = create_test_ohlcv(arena, closes, 15);
  ASSERT(ohlcv != NULL, "Failed to create OHLCV data");

  SamtraderIndicatorSeries *rsi = samtrader_calculate_rsi(arena, ohlcv, 5);
  ASSERT(rsi != NULL, "Failed to calculate RSI");
  ASSERT(rsi->type == SAMTRADER_IND_RSI, "Type should be RSI");
  ASSERT(rsi->params.period == 5, "Period should be 5");
  ASSERT(samtrader_indicator_series_size(rsi) == 15, "Should have 15 values");

  /* First 5 values (indices 0-4) should be invalid (warmup) */
  for (size_t i = 0; i < 5; i++) {
    const SamtraderIndicatorValue *val = samtrader_indicator_series_at(rsi, i);
    ASSERT(val != NULL && val->valid == false, "Warmup values should be invalid");
  }

  /* Index 5 should be the first valid RSI value */
  const SamtraderIndicatorValue *val = samtrader_indicator_series_at(rsi, 5);
  ASSERT(val != NULL && val->valid == true, "Index 5 should be valid");

  /* All valid RSI values should be in [0, 100] */
  for (size_t i = 5; i < 15; i++) {
    val = samtrader_indicator_series_at(rsi, i);
    ASSERT(val != NULL && val->valid == true, "Should be valid");
    ASSERT(val->data.simple.value >= 0.0 && val->data.simple.value <= 100.0,
           "RSI should be between 0 and 100");
  }

  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

static int test_rsi_all_gains(void) {
  printf("Testing RSI with all gains (monotonically rising)...\n");

  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  /* Monotonically rising prices */
  double closes[] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
  SamrenaVector *ohlcv = create_test_ohlcv(arena, closes, 10);

  SamtraderIndicatorSeries *rsi = samtrader_calculate_rsi(arena, ohlcv, 5);
  ASSERT(rsi != NULL, "Failed to calculate RSI");

  /* With all gains and no losses, RSI should be 100 */
  for (size_t i = 5; i < 10; i++) {
    const SamtraderIndicatorValue *val = samtrader_indicator_series_at(rsi, i);
    ASSERT(val != NULL && val->valid == true, "Should be valid");
    ASSERT_DOUBLE_EQ(val->data.simple.value, 100.0, "RSI should be 100 with all gains");
  }

  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

static int test_rsi_all_losses(void) {
  printf("Testing RSI with all losses (monotonically falling)...\n");

  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  /* Monotonically falling prices */
  double closes[] = {10.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0};
  SamrenaVector *ohlcv = create_test_ohlcv(arena, closes, 10);

  SamtraderIndicatorSeries *rsi = samtrader_calculate_rsi(arena, ohlcv, 5);
  ASSERT(rsi != NULL, "Failed to calculate RSI");

  /* With all losses and no gains, RSI should be 0 */
  for (size_t i = 5; i < 10; i++) {
    const SamtraderIndicatorValue *val = samtrader_indicator_series_at(rsi, i);
    ASSERT(val != NULL && val->valid == true, "Should be valid");
    ASSERT_DOUBLE_EQ(val->data.simple.value, 0.0, "RSI should be 0 with all losses");
  }

  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

static int test_rsi_constant(void) {
  printf("Testing RSI with constant prices...\n");

  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  /* Constant prices - no gains or losses */
  double closes[] = {50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0};
  SamrenaVector *ohlcv = create_test_ohlcv(arena, closes, 8);

  SamtraderIndicatorSeries *rsi = samtrader_calculate_rsi(arena, ohlcv, 3);
  ASSERT(rsi != NULL, "Failed to calculate RSI");

  /* With no gains and no losses, RSI should be 50 */
  for (size_t i = 3; i < 8; i++) {
    const SamtraderIndicatorValue *val = samtrader_indicator_series_at(rsi, i);
    ASSERT(val != NULL && val->valid == true, "Should be valid");
    ASSERT_DOUBLE_EQ(val->data.simple.value, 50.0, "RSI should be 50 with constant prices");
  }

  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

static int test_rsi_period_1(void) {
  printf("Testing RSI with period 1...\n");

  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  double closes[] = {10.0, 12.0, 11.0, 13.0};
  SamrenaVector *ohlcv = create_test_ohlcv(arena, closes, 4);

  SamtraderIndicatorSeries *rsi = samtrader_calculate_rsi(arena, ohlcv, 1);
  ASSERT(rsi != NULL, "Failed to calculate RSI");

  /* Index 0 is invalid, rest should be valid */
  const SamtraderIndicatorValue *val = samtrader_indicator_series_at(rsi, 0);
  ASSERT(val != NULL && val->valid == false, "Index 0 should be invalid");

  /* Index 1: gain of 2.0, no loss -> RSI = 100 */
  val = samtrader_indicator_series_at(rsi, 1);
  ASSERT(val != NULL && val->valid == true, "Index 1 should be valid");
  ASSERT_DOUBLE_EQ(val->data.simple.value, 100.0, "RSI should be 100 for pure gain");

  /* Index 2: loss of 1.0, smoothed avg_gain decays -> RSI < 100 */
  val = samtrader_indicator_series_at(rsi, 2);
  ASSERT(val != NULL && val->valid == true, "Index 2 should be valid");
  ASSERT_DOUBLE_EQ(val->data.simple.value, 0.0, "RSI period 1 pure loss");

  /* Index 3: gain of 2.0 -> RSI = 100 */
  val = samtrader_indicator_series_at(rsi, 3);
  ASSERT(val != NULL && val->valid == true, "Index 3 should be valid");
  ASSERT_DOUBLE_EQ(val->data.simple.value, 100.0, "RSI should be 100 for pure gain");

  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

static int test_rsi_invalid_params(void) {
  printf("Testing RSI with invalid parameters...\n");

  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  double closes[] = {1.0, 2.0, 3.0};
  SamrenaVector *ohlcv = create_test_ohlcv(arena, closes, 3);

  ASSERT(samtrader_calculate_rsi(NULL, ohlcv, 14) == NULL, "NULL arena should fail");
  ASSERT(samtrader_calculate_rsi(arena, NULL, 14) == NULL, "NULL ohlcv should fail");
  ASSERT(samtrader_calculate_rsi(arena, ohlcv, 0) == NULL, "Period 0 should fail");
  ASSERT(samtrader_calculate_rsi(arena, ohlcv, -1) == NULL, "Negative period should fail");

  SamrenaVector *empty = samtrader_ohlcv_vector_create(arena, 10);
  ASSERT(samtrader_calculate_rsi(arena, empty, 14) == NULL, "Empty vector should fail");

  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

static int test_rsi_known_values(void) {
  printf("Testing RSI with hand-calculated values...\n");

  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  /* Simple dataset for manual verification with period 3:
   * Prices: 10, 12, 11, 13, 12, 14
   * Changes:    +2, -1, +2, -1, +2
   * First 3 changes (i=1,2,3): gains={2,0,2}=4, losses={0,1,0}=1
   * Avg gain = 4/3 = 1.3333, Avg loss = 1/3 = 0.3333
   * RS = 1.3333/0.3333 = 4.0, RSI = 100 - 100/(1+4) = 80.0
   */
  double closes[] = {10.0, 12.0, 11.0, 13.0, 12.0, 14.0};
  SamrenaVector *ohlcv = create_test_ohlcv(arena, closes, 6);

  SamtraderIndicatorSeries *rsi = samtrader_calculate_rsi(arena, ohlcv, 3);
  ASSERT(rsi != NULL, "Failed to calculate RSI");

  /* First valid RSI at index 3 */
  const SamtraderIndicatorValue *val = samtrader_indicator_series_at(rsi, 3);
  ASSERT(val != NULL && val->valid == true, "Index 3 should be valid");
  ASSERT_DOUBLE_EQ(val->data.simple.value, 80.0, "RSI at index 3");

  /* Index 4: change = -1 (loss)
   * Avg gain = (1.3333 * 2 + 0) / 3 = 0.8889
   * Avg loss = (0.3333 * 2 + 1) / 3 = 0.5556
   * RS = 0.8889 / 0.5556 = 1.6, RSI = 100 - 100/2.6 = 61.5385
   */
  val = samtrader_indicator_series_at(rsi, 4);
  ASSERT(val != NULL && val->valid == true, "Index 4 should be valid");
  ASSERT_DOUBLE_EQ(val->data.simple.value, 100.0 - 100.0 / 2.6, "RSI at index 4");

  /* Index 5: change = +2 (gain)
   * Avg gain = (0.8889 * 2 + 2) / 3 = 1.2593
   * Avg loss = (0.5556 * 2 + 0) / 3 = 0.3704
   * RS = 1.2593 / 0.3704 = 3.4, RSI = 100 - 100/4.4 = 77.2727
   */
  val = samtrader_indicator_series_at(rsi, 5);
  ASSERT(val != NULL && val->valid == true, "Index 5 should be valid");
  double expected_avg_gain = ((4.0 / 3.0) * 2.0 + 0.0) / 3.0;
  double expected_avg_loss = ((1.0 / 3.0) * 2.0 + 1.0) / 3.0;
  expected_avg_gain = (expected_avg_gain * 2.0 + 2.0) / 3.0;
  expected_avg_loss = (expected_avg_loss * 2.0 + 0.0) / 3.0;
  double expected_rs = expected_avg_gain / expected_avg_loss;
  double expected_rsi = 100.0 - (100.0 / (1.0 + expected_rs));
  ASSERT_DOUBLE_EQ(val->data.simple.value, expected_rsi, "RSI at index 5");

  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

/*============================================================================
 * Bollinger Bands Tests
 *============================================================================*/

static int test_bollinger_basic(void) {
  printf("Testing Bollinger Bands basic calculation...\n");

  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  /* Data: 1, 2, 3, 4, 5 with period 3, stddev multiplier 2.0 */
  double closes[] = {1.0, 2.0, 3.0, 4.0, 5.0};
  SamrenaVector *ohlcv = create_test_ohlcv(arena, closes, 5);
  ASSERT(ohlcv != NULL, "Failed to create OHLCV data");

  SamtraderIndicatorSeries *bb = samtrader_calculate_bollinger(arena, ohlcv, 3, 2.0);
  ASSERT(bb != NULL, "Failed to calculate Bollinger Bands");
  ASSERT(bb->type == SAMTRADER_IND_BOLLINGER, "Type should be BOLLINGER");
  ASSERT(bb->params.period == 3, "Period should be 3");
  ASSERT(samtrader_indicator_series_size(bb) == 5, "Should have 5 values");

  /* First two values should be invalid (warmup) */
  const SamtraderIndicatorValue *val = samtrader_indicator_series_at(bb, 0);
  ASSERT(val != NULL && val->valid == false, "Index 0 should be invalid");

  val = samtrader_indicator_series_at(bb, 1);
  ASSERT(val != NULL && val->valid == false, "Index 1 should be invalid");

  /* Index 2: SMA = (1+2+3)/3 = 2.0
   * StdDev = sqrt(((1-2)^2 + (2-2)^2 + (3-2)^2) / 3) = sqrt(2/3) = 0.8165
   * Upper = 2.0 + 2.0 * 0.8165 = 3.6330
   * Lower = 2.0 - 2.0 * 0.8165 = 0.3670 */
  val = samtrader_indicator_series_at(bb, 2);
  ASSERT(val != NULL && val->valid == true, "Index 2 should be valid");
  ASSERT_DOUBLE_EQ(val->data.bollinger.middle, 2.0, "Middle at index 2");
  double stddev_2 = sqrt(2.0 / 3.0);
  ASSERT_DOUBLE_EQ(val->data.bollinger.upper, 2.0 + 2.0 * stddev_2, "Upper at index 2");
  ASSERT_DOUBLE_EQ(val->data.bollinger.lower, 2.0 - 2.0 * stddev_2, "Lower at index 2");

  /* Index 3: SMA = (2+3+4)/3 = 3.0
   * StdDev = sqrt(((2-3)^2 + (3-3)^2 + (4-3)^2) / 3) = sqrt(2/3) */
  val = samtrader_indicator_series_at(bb, 3);
  ASSERT(val != NULL && val->valid == true, "Index 3 should be valid");
  ASSERT_DOUBLE_EQ(val->data.bollinger.middle, 3.0, "Middle at index 3");
  ASSERT_DOUBLE_EQ(val->data.bollinger.upper, 3.0 + 2.0 * stddev_2, "Upper at index 3");
  ASSERT_DOUBLE_EQ(val->data.bollinger.lower, 3.0 - 2.0 * stddev_2, "Lower at index 3");

  /* Index 4: SMA = (3+4+5)/3 = 4.0
   * StdDev = sqrt(((3-4)^2 + (4-4)^2 + (5-4)^2) / 3) = sqrt(2/3) */
  val = samtrader_indicator_series_at(bb, 4);
  ASSERT(val != NULL && val->valid == true, "Index 4 should be valid");
  ASSERT_DOUBLE_EQ(val->data.bollinger.middle, 4.0, "Middle at index 4");
  ASSERT_DOUBLE_EQ(val->data.bollinger.upper, 4.0 + 2.0 * stddev_2, "Upper at index 4");
  ASSERT_DOUBLE_EQ(val->data.bollinger.lower, 4.0 - 2.0 * stddev_2, "Lower at index 4");

  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

static int test_bollinger_constant_prices(void) {
  printf("Testing Bollinger Bands with constant prices...\n");

  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  /* Constant prices -> stddev = 0, bands collapse to SMA */
  double closes[] = {50.0, 50.0, 50.0, 50.0, 50.0, 50.0};
  SamrenaVector *ohlcv = create_test_ohlcv(arena, closes, 6);

  SamtraderIndicatorSeries *bb = samtrader_calculate_bollinger(arena, ohlcv, 3, 2.0);
  ASSERT(bb != NULL, "Failed to calculate Bollinger Bands");

  for (size_t i = 2; i < 6; i++) {
    const SamtraderIndicatorValue *val = samtrader_indicator_series_at(bb, i);
    ASSERT(val != NULL && val->valid == true, "Should be valid");
    ASSERT_DOUBLE_EQ(val->data.bollinger.middle, 50.0, "Middle should be 50");
    ASSERT_DOUBLE_EQ(val->data.bollinger.upper, 50.0, "Upper should equal middle");
    ASSERT_DOUBLE_EQ(val->data.bollinger.lower, 50.0, "Lower should equal middle");
  }

  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

static int test_bollinger_band_symmetry(void) {
  printf("Testing Bollinger Bands symmetry around middle...\n");

  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  double closes[] = {10.0, 12.0, 11.0, 13.0, 12.0, 14.0, 11.0, 15.0};
  SamrenaVector *ohlcv = create_test_ohlcv(arena, closes, 8);

  SamtraderIndicatorSeries *bb = samtrader_calculate_bollinger(arena, ohlcv, 5, 2.0);
  ASSERT(bb != NULL, "Failed to calculate Bollinger Bands");

  /* Upper and lower should be equidistant from middle */
  for (size_t i = 4; i < 8; i++) {
    const SamtraderIndicatorValue *val = samtrader_indicator_series_at(bb, i);
    ASSERT(val != NULL && val->valid == true, "Should be valid");

    double upper_dist = val->data.bollinger.upper - val->data.bollinger.middle;
    double lower_dist = val->data.bollinger.middle - val->data.bollinger.lower;
    ASSERT_DOUBLE_EQ(upper_dist, lower_dist, "Bands should be symmetric");
    ASSERT(val->data.bollinger.upper >= val->data.bollinger.middle, "Upper should be >= middle");
    ASSERT(val->data.bollinger.lower <= val->data.bollinger.middle, "Lower should be <= middle");
  }

  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

static int test_bollinger_stddev_multiplier(void) {
  printf("Testing Bollinger Bands with different stddev multipliers...\n");

  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  double closes[] = {10.0, 12.0, 11.0, 13.0, 12.0};
  SamrenaVector *ohlcv = create_test_ohlcv(arena, closes, 5);

  SamtraderIndicatorSeries *bb1 = samtrader_calculate_bollinger(arena, ohlcv, 3, 1.0);
  SamtraderIndicatorSeries *bb2 = samtrader_calculate_bollinger(arena, ohlcv, 3, 2.0);
  SamtraderIndicatorSeries *bb3 = samtrader_calculate_bollinger(arena, ohlcv, 3, 3.0);
  ASSERT(bb1 && bb2 && bb3, "All calculations should succeed");

  /* Wider multiplier = wider bands, same middle */
  for (size_t i = 2; i < 5; i++) {
    const SamtraderIndicatorValue *v1 = samtrader_indicator_series_at(bb1, i);
    const SamtraderIndicatorValue *v2 = samtrader_indicator_series_at(bb2, i);
    const SamtraderIndicatorValue *v3 = samtrader_indicator_series_at(bb3, i);

    /* Middle should be the same for all multipliers */
    ASSERT_DOUBLE_EQ(v1->data.bollinger.middle, v2->data.bollinger.middle,
                     "Middle should be same regardless of multiplier");
    ASSERT_DOUBLE_EQ(v2->data.bollinger.middle, v3->data.bollinger.middle,
                     "Middle should be same regardless of multiplier");

    /* Wider multiplier = wider bands */
    double width1 = v1->data.bollinger.upper - v1->data.bollinger.lower;
    double width2 = v2->data.bollinger.upper - v2->data.bollinger.lower;
    double width3 = v3->data.bollinger.upper - v3->data.bollinger.lower;
    ASSERT(width2 > width1, "2x multiplier should be wider than 1x");
    ASSERT(width3 > width2, "3x multiplier should be wider than 2x");
  }

  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

static int test_bollinger_invalid_params(void) {
  printf("Testing Bollinger Bands with invalid parameters...\n");

  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  double closes[] = {1.0, 2.0, 3.0};
  SamrenaVector *ohlcv = create_test_ohlcv(arena, closes, 3);

  ASSERT(samtrader_calculate_bollinger(NULL, ohlcv, 20, 2.0) == NULL, "NULL arena should fail");
  ASSERT(samtrader_calculate_bollinger(arena, NULL, 20, 2.0) == NULL, "NULL ohlcv should fail");
  ASSERT(samtrader_calculate_bollinger(arena, ohlcv, 0, 2.0) == NULL, "Period 0 should fail");
  ASSERT(samtrader_calculate_bollinger(arena, ohlcv, -1, 2.0) == NULL,
         "Negative period should fail");

  SamrenaVector *empty = samtrader_ohlcv_vector_create(arena, 10);
  ASSERT(samtrader_calculate_bollinger(arena, empty, 20, 2.0) == NULL, "Empty vector should fail");

  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

static int test_bollinger_latest_value(void) {
  printf("Testing Bollinger Bands latest value retrieval...\n");

  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  double closes[] = {1.0, 2.0, 3.0, 4.0, 5.0};
  SamrenaVector *ohlcv = create_test_ohlcv(arena, closes, 5);

  SamtraderIndicatorSeries *bb = samtrader_calculate_bollinger(arena, ohlcv, 3, 2.0);
  ASSERT(bb != NULL, "Failed to calculate Bollinger Bands");

  SamtraderBollingerValue latest;
  ASSERT(samtrader_indicator_latest_bollinger(bb, &latest) == true,
         "Should find latest valid value");
  ASSERT_DOUBLE_EQ(latest.middle, 4.0, "Latest middle should be SMA of last 3");

  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

/*============================================================================
 * Dispatcher Tests
 *============================================================================*/

static int test_indicator_calculate_dispatcher(void) {
  printf("Testing samtrader_indicator_calculate dispatcher...\n");

  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  double closes[] = {1.0, 2.0, 3.0, 4.0, 5.0};
  SamrenaVector *ohlcv = create_test_ohlcv(arena, closes, 5);

  /* Test SMA dispatch */
  SamtraderIndicatorSeries *sma = samtrader_indicator_calculate(arena, SAMTRADER_IND_SMA, ohlcv, 3);
  ASSERT(sma != NULL, "SMA dispatch should work");
  ASSERT(sma->type == SAMTRADER_IND_SMA, "Should be SMA type");

  /* Test EMA dispatch */
  SamtraderIndicatorSeries *ema = samtrader_indicator_calculate(arena, SAMTRADER_IND_EMA, ohlcv, 3);
  ASSERT(ema != NULL, "EMA dispatch should work");
  ASSERT(ema->type == SAMTRADER_IND_EMA, "Should be EMA type");

  /* Test WMA dispatch */
  SamtraderIndicatorSeries *wma = samtrader_indicator_calculate(arena, SAMTRADER_IND_WMA, ohlcv, 3);
  ASSERT(wma != NULL, "WMA dispatch should work");
  ASSERT(wma->type == SAMTRADER_IND_WMA, "Should be WMA type");

  /* Test RSI dispatch */
  SamtraderIndicatorSeries *rsi = samtrader_indicator_calculate(arena, SAMTRADER_IND_RSI, ohlcv, 3);
  ASSERT(rsi != NULL, "RSI dispatch should work");
  ASSERT(rsi->type == SAMTRADER_IND_RSI, "Should be RSI type");

  /* Test Bollinger dispatch (uses default 2.0 stddev) */
  SamtraderIndicatorSeries *bb =
      samtrader_indicator_calculate(arena, SAMTRADER_IND_BOLLINGER, ohlcv, 3);
  ASSERT(bb != NULL, "Bollinger dispatch should work");
  ASSERT(bb->type == SAMTRADER_IND_BOLLINGER, "Should be BOLLINGER type");

  /* Test MACD dispatch (uses default 12/26/9) */
  SamtraderIndicatorSeries *macd =
      samtrader_indicator_calculate(arena, SAMTRADER_IND_MACD, ohlcv, 14);
  ASSERT(macd != NULL, "MACD dispatch should work");
  ASSERT(macd->type == SAMTRADER_IND_MACD, "Should be MACD type");

  /* Test Stochastic dispatch (uses period for %K, default 3 for %D) */
  SamtraderIndicatorSeries *stoch =
      samtrader_indicator_calculate(arena, SAMTRADER_IND_STOCHASTIC, ohlcv, 3);
  ASSERT(stoch != NULL, "Stochastic dispatch should work");
  ASSERT(stoch->type == SAMTRADER_IND_STOCHASTIC, "Should be STOCHASTIC type");

  /* Test unsupported type */
  SamtraderIndicatorSeries *roc =
      samtrader_indicator_calculate(arena, SAMTRADER_IND_ROC, ohlcv, 14);
  ASSERT(roc == NULL, "Unsupported type should return NULL");

  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

/*============================================================================
 * Comparison Tests (SMA vs EMA vs WMA)
 *============================================================================*/

static int test_moving_averages_comparison(void) {
  printf("Testing moving averages relative behavior...\n");

  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  /* Rising prices - EMA and WMA should be higher than SMA */
  double rising[] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
  SamrenaVector *ohlcv = create_test_ohlcv(arena, rising, 10);

  SamtraderIndicatorSeries *sma = samtrader_calculate_sma(arena, ohlcv, 5);
  SamtraderIndicatorSeries *ema = samtrader_calculate_ema(arena, ohlcv, 5);
  SamtraderIndicatorSeries *wma = samtrader_calculate_wma(arena, ohlcv, 5);

  ASSERT(sma != NULL && ema != NULL && wma != NULL, "All calculations should succeed");

  /* Check last value - with rising prices, EMA and WMA should lead SMA */
  const SamtraderIndicatorValue *sma_val = samtrader_indicator_series_at(sma, 9);
  const SamtraderIndicatorValue *ema_val = samtrader_indicator_series_at(ema, 9);
  const SamtraderIndicatorValue *wma_val = samtrader_indicator_series_at(wma, 9);

  ASSERT(sma_val && ema_val && wma_val, "All values should exist");

  /* SMA(5) at last: (6+7+8+9+10)/5 = 8.0 */
  ASSERT_DOUBLE_EQ(sma_val->data.simple.value, 8.0, "SMA at last index");

  /* EMA and WMA should be >= SMA for rising prices */
  ASSERT(ema_val->data.simple.value >= sma_val->data.simple.value - 0.0001,
         "EMA should be >= SMA for rising prices");
  ASSERT(wma_val->data.simple.value >= sma_val->data.simple.value - 0.0001,
         "WMA should be >= SMA for rising prices");

  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

int main(void) {
  printf("=== Indicator Calculation Tests ===\n\n");

  int failures = 0;

  /* SMA tests */
  failures += test_sma_basic();
  failures += test_sma_period_1();
  failures += test_sma_invalid_params();

  /* EMA tests */
  failures += test_ema_basic();
  failures += test_ema_convergence();
  failures += test_ema_invalid_params();

  /* WMA tests */
  failures += test_wma_basic();
  failures += test_wma_weighting();
  failures += test_wma_invalid_params();

  /* RSI tests */
  failures += test_rsi_basic();
  failures += test_rsi_all_gains();
  failures += test_rsi_all_losses();
  failures += test_rsi_constant();
  failures += test_rsi_period_1();
  failures += test_rsi_invalid_params();
  failures += test_rsi_known_values();

  /* Bollinger Bands tests */
  failures += test_bollinger_basic();
  failures += test_bollinger_constant_prices();
  failures += test_bollinger_band_symmetry();
  failures += test_bollinger_stddev_multiplier();
  failures += test_bollinger_invalid_params();
  failures += test_bollinger_latest_value();

  /* Dispatcher test */
  failures += test_indicator_calculate_dispatcher();

  /* Comparison test */
  failures += test_moving_averages_comparison();

  printf("\n=== Results: %d failures ===\n", failures);

  return failures > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
