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

  /* Test unsupported type */
  SamtraderIndicatorSeries *rsi =
      samtrader_indicator_calculate(arena, SAMTRADER_IND_RSI, ohlcv, 14);
  ASSERT(rsi == NULL, "Unsupported type should return NULL");

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

  /* Dispatcher test */
  failures += test_indicator_calculate_dispatcher();

  /* Comparison test */
  failures += test_moving_averages_comparison();

  printf("\n=== Results: %d failures ===\n", failures);

  return failures > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
