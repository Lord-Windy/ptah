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
#include <string.h>

#include "samtrader/domain/execution.h"
#include "samtrader/domain/indicator.h"
#include "samtrader/domain/metrics.h"
#include "samtrader/domain/ohlcv.h"
#include "samtrader/domain/portfolio.h"
#include "samtrader/domain/rule.h"
#include "samtrader/domain/strategy.h"

#define ASSERT(cond, msg)                                                                          \
  do {                                                                                             \
    if (!(cond)) {                                                                                 \
      printf("FAIL: %s\n", msg);                                                                   \
      return 1;                                                                                    \
    }                                                                                              \
  } while (0)

#define ASSERT_DOUBLE_EQ(a, b, msg)                                                                \
  do {                                                                                             \
    if (fabs((a) - (b)) > 0.01) {                                                                  \
      printf("FAIL: %s (expected %f, got %f)\n", msg, (b), (a));                                   \
      return 1;                                                                                    \
    }                                                                                              \
  } while (0)

/*============================================================================
 * Helpers
 *============================================================================*/

static time_t day_time(int day) {
  return (time_t)(1704067200 + day * 86400); /* 2024-01-01 + day offset */
}

static SamrenaVector *make_ohlcv(Samrena *arena, const double *closes, size_t count) {
  SamrenaVector *vec = samrena_vector_init(arena, sizeof(SamtraderOhlcv), count);
  for (size_t i = 0; i < count; i++) {
    SamtraderOhlcv bar = {
        .code = "TEST",
        .exchange = "US",
        .date = day_time((int)i),
        .open = closes[i] - 1.0,
        .high = closes[i] + 1.0,
        .low = closes[i] - 2.0,
        .close = closes[i],
        .volume = (int64_t)(1000 * (i + 1)),
    };
    samrena_vector_push(vec, &bar);
  }
  return vec;
}

static SamHashMap *build_price_map(Samrena *arena, const SamtraderOhlcv *bar) {
  SamHashMap *price_map = samhashmap_create(4, arena);
  if (!price_map)
    return NULL;
  double *price = SAMRENA_PUSH_TYPE(arena, double);
  if (!price)
    return NULL;
  *price = bar->close;
  samhashmap_put(price_map, bar->code, price);
  return price_map;
}

/**
 * Run the core backtest loop (replicating main.c logic) with an invariant check
 * at each bar: cash + |qty| * close == total_equity().
 */
static int run_backtest_loop(Samrena *arena, SamrenaVector *ohlcv, SamtraderStrategy *strategy,
                             SamtraderPortfolio *portfolio, SamHashMap *indicators,
                             const char *code, const char *exchange, double commission_flat,
                             double commission_pct, double slippage_pct, bool allow_shorting) {
  size_t bar_count = samrena_vector_size(ohlcv);

  for (size_t i = 0; i < bar_count; i++) {
    const SamtraderOhlcv *bar = (const SamtraderOhlcv *)samrena_vector_at_const(ohlcv, i);

    SamHashMap *price_map = build_price_map(arena, bar);
    if (!price_map)
      continue;

    /* Check stop loss / take profit triggers */
    samtrader_execution_check_triggers(portfolio, arena, price_map, bar->date, commission_flat,
                                       commission_pct, slippage_pct);

    /* Evaluate exit rules for existing positions */
    if (samtrader_portfolio_has_position(portfolio, code)) {
      SamtraderPosition *pos = samtrader_portfolio_get_position(portfolio, code);
      bool should_exit = false;
      if (pos && samtrader_position_is_long(pos)) {
        should_exit = samtrader_rule_evaluate(strategy->exit_long, ohlcv, indicators, i);
      } else if (pos && samtrader_position_is_short(pos) && strategy->exit_short) {
        should_exit = samtrader_rule_evaluate(strategy->exit_short, ohlcv, indicators, i);
      }
      if (should_exit) {
        samtrader_execution_exit_position(portfolio, arena, code, bar->close, bar->date,
                                          commission_flat, commission_pct, slippage_pct);
      }
    }

    /* Evaluate entry rules */
    if (!samtrader_portfolio_has_position(portfolio, code)) {
      bool enter_long = samtrader_rule_evaluate(strategy->entry_long, ohlcv, indicators, i);
      bool enter_short = allow_shorting && strategy->entry_short
                             ? samtrader_rule_evaluate(strategy->entry_short, ohlcv, indicators, i)
                             : false;

      if (enter_long) {
        samtrader_execution_enter_long(portfolio, arena, code, exchange, bar->close, bar->date,
                                       strategy->position_size, strategy->stop_loss_pct,
                                       strategy->take_profit_pct, strategy->max_positions,
                                       commission_flat, commission_pct, slippage_pct);
      } else if (enter_short) {
        samtrader_execution_enter_short(portfolio, arena, code, exchange, bar->close, bar->date,
                                        strategy->position_size, strategy->stop_loss_pct,
                                        strategy->take_profit_pct, strategy->max_positions,
                                        commission_flat, commission_pct, slippage_pct);
      }
    }

    /* Record equity */
    double equity = samtrader_portfolio_total_equity(portfolio, price_map);
    samtrader_portfolio_record_equity(portfolio, arena, bar->date, equity);

    /* Portfolio invariant check: cash + |qty| * close == total_equity */
    double manual_equity = portfolio->cash;
    if (samtrader_portfolio_has_position(portfolio, code)) {
      SamtraderPosition *pos = samtrader_portfolio_get_position(portfolio, code);
      if (pos) {
        int64_t abs_qty = pos->quantity >= 0 ? pos->quantity : -pos->quantity;
        manual_equity += (double)abs_qty * bar->close;
      }
    }
    if (fabs(manual_equity - equity) > 0.01) {
      printf("INVARIANT FAIL at bar %zu: manual=%f, total_equity=%f\n", i, manual_equity, equity);
      return 1;
    }
  }

  return 0;
}

/*============================================================================
 * Test 1: Simple Long Backtest
 *============================================================================*/

static int test_simple_long_backtest(void) {
  printf("Testing simple long backtest...\n");
  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  /* Prices: rising then plateau then falling */
  double closes[] = {90, 95, 100, 105, 110, 115, 120, 115, 110, 105};
  size_t count = sizeof(closes) / sizeof(closes[0]);
  SamrenaVector *ohlcv = make_ohlcv(arena, closes, count);

  /* Entry: close > 95, Exit: close > 115, position_size=0.5, no stops/commission */
  SamtraderOperand close_op = samtrader_operand_price(SAMTRADER_OPERAND_PRICE_CLOSE);
  SamtraderOperand const_95 = samtrader_operand_constant(95.0);
  SamtraderOperand const_115 = samtrader_operand_constant(115.0);

  SamtraderRule *entry_rule =
      samtrader_rule_create_comparison(arena, SAMTRADER_RULE_ABOVE, close_op, const_95);
  SamtraderRule *exit_rule =
      samtrader_rule_create_comparison(arena, SAMTRADER_RULE_ABOVE, close_op, const_115);

  SamtraderStrategy strategy = {.name = "test",
                                .entry_long = entry_rule,
                                .exit_long = exit_rule,
                                .entry_short = NULL,
                                .exit_short = NULL,
                                .position_size = 0.5,
                                .stop_loss_pct = 0.0,
                                .take_profit_pct = 0.0,
                                .max_positions = 10};

  SamtraderPortfolio *portfolio = samtrader_portfolio_create(arena, 100000.0);
  ASSERT(portfolio != NULL, "Failed to create portfolio");

  SamHashMap *indicators = samhashmap_create(4, arena);

  int loop_result = run_backtest_loop(arena, ohlcv, &strategy, portfolio, indicators, "TEST", "US",
                                      0.0, 0.0, 0.0, false);
  ASSERT(loop_result == 0, "Backtest loop invariant failed");

  /* Trace through the loop:
   * Bar 0: close=90, entry rule: 90 > 95? no  → no position
   * Bar 1: close=95, entry rule: 95 > 95? no (ABOVE is strict) → no position
   * Bar 2: close=100, entry rule: 100 > 95? yes → enter long
   *   available = 100000 * 0.5 = 50000, qty = floor(50000/100) = 500
   *   cash = 100000 - 50000 = 50000
   * Bar 3: close=105, exit rule: 105 > 115? no → hold
   * Bar 4: close=110, exit rule: 110 > 115? no → hold
   * Bar 5: close=115, exit rule: 115 > 115? no → hold
   * Bar 6: close=120, exit rule: 120 > 115? yes → exit
   *   cash = 50000 + 500*120 = 110000, PnL = 500*(120-100) = 10000
   *   entry rule: 120 > 95? yes → re-enter
   *   available = 110000 * 0.5 = 55000, qty = floor(55000/120) = 458
   *   cash = 110000 - 458*120 = 110000 - 54960 = 55040
   * Bar 7: close=115, exit rule: 115 > 115? no → hold
   * Bar 8: close=110, exit rule: 110 > 115? no → hold
   * Bar 9: close=105, exit rule: 105 > 115? no → hold (still open)
   */

  /* 1 closed trade with PnL = 10000 */
  ASSERT(samrena_vector_size(portfolio->closed_trades) == 1, "Should have 1 closed trade");
  const SamtraderClosedTrade *trade =
      (const SamtraderClosedTrade *)samrena_vector_at_const(portfolio->closed_trades, 0);
  ASSERT(trade != NULL, "Trade should not be NULL");
  ASSERT(trade->quantity == 500, "First trade: 500 shares");
  ASSERT_DOUBLE_EQ(trade->entry_price, 100.0, "First trade entry at 100");
  ASSERT_DOUBLE_EQ(trade->exit_price, 120.0, "First trade exit at 120");
  ASSERT_DOUBLE_EQ(trade->pnl, 10000.0, "First trade PnL");

  /* Second position still open */
  ASSERT(samtrader_portfolio_has_position(portfolio, "TEST"), "Should have open position");
  SamtraderPosition *pos = samtrader_portfolio_get_position(portfolio, "TEST");
  ASSERT(pos != NULL, "Open position should not be NULL");
  ASSERT(pos->quantity == 458, "Second position: 458 shares");
  ASSERT_DOUBLE_EQ(pos->entry_price, 120.0, "Second position entry at 120");

  /* Equity curve has 10 points */
  ASSERT(samrena_vector_size(portfolio->equity_curve) == 10, "Equity curve: 10 points");

  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

/*============================================================================
 * Test 2: Stop Loss Trigger
 *============================================================================*/

static int test_stop_loss_trigger(void) {
  printf("Testing stop loss trigger...\n");
  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  double closes[] = {90, 100, 110, 105, 100, 92, 88, 85};
  size_t count = sizeof(closes) / sizeof(closes[0]);
  SamrenaVector *ohlcv = make_ohlcv(arena, closes, count);

  /* Entry: close > 95, Exit: never fires (close > 999), stop_loss=10% */
  SamtraderOperand close_op = samtrader_operand_price(SAMTRADER_OPERAND_PRICE_CLOSE);
  SamtraderOperand const_95 = samtrader_operand_constant(95.0);
  SamtraderOperand const_999 = samtrader_operand_constant(999.0);

  SamtraderRule *entry_rule =
      samtrader_rule_create_comparison(arena, SAMTRADER_RULE_ABOVE, close_op, const_95);
  SamtraderRule *exit_rule =
      samtrader_rule_create_comparison(arena, SAMTRADER_RULE_ABOVE, close_op, const_999);

  SamtraderStrategy strategy = {.name = "test_sl",
                                .entry_long = entry_rule,
                                .exit_long = exit_rule,
                                .entry_short = NULL,
                                .exit_short = NULL,
                                .position_size = 0.5,
                                .stop_loss_pct = 10.0,
                                .take_profit_pct = 0.0,
                                .max_positions = 10};

  SamtraderPortfolio *portfolio = samtrader_portfolio_create(arena, 100000.0);
  ASSERT(portfolio != NULL, "Failed to create portfolio");

  SamHashMap *indicators = samhashmap_create(4, arena);

  int loop_result = run_backtest_loop(arena, ohlcv, &strategy, portfolio, indicators, "TEST", "US",
                                      0.0, 0.0, 0.0, false);
  ASSERT(loop_result == 0, "Backtest loop invariant failed");

  /* Trace:
   * Bar 0: close=90, 90 > 95? no
   * Bar 1: close=100, 100 > 95? yes → enter long
   *   qty = floor(50000/100) = 500, cash = 50000
   *   stop_loss = 100 * (1 - 10/100) = 90.0
   * Bar 2: close=110, SL check: 110 <= 90? no. Exit rule: 110 > 999? no
   * Bar 3: close=105, SL check: 105 <= 90? no
   * Bar 4: close=100, SL check: 100 <= 90? no
   * Bar 5: close=92,  SL check: 92 <= 90? no
   * Bar 6: close=88,  SL check: 88 <= 90? YES → trigger exit at 88
   *   cash = 50000 + 500*88 = 94000
   *   PnL = 500*(88-100) = -6000
   *   entry rule: 88 > 95? no → no re-entry
   * Bar 7: close=85, no position, 85 > 95? no
   */

  ASSERT(samrena_vector_size(portfolio->closed_trades) == 1, "Should have 1 closed trade");
  const SamtraderClosedTrade *trade =
      (const SamtraderClosedTrade *)samrena_vector_at_const(portfolio->closed_trades, 0);
  ASSERT(trade != NULL, "Trade should not be NULL");
  ASSERT(trade->quantity == 500, "SL trade: 500 shares");
  ASSERT_DOUBLE_EQ(trade->entry_price, 100.0, "SL trade entry at 100");
  ASSERT_DOUBLE_EQ(trade->exit_price, 88.0, "SL trade exit at 88 (trigger price)");
  ASSERT_DOUBLE_EQ(trade->pnl, -6000.0, "SL trade PnL = -6000");

  ASSERT(!samtrader_portfolio_has_position(portfolio, "TEST"), "Position should be closed");

  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

/*============================================================================
 * Test 3: Multiple Trades
 *============================================================================*/

static int test_multiple_trades(void) {
  printf("Testing multiple trades...\n");
  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  double closes[] = {90, 100, 110, 120, 130, 125, 115, 110, 100, 105, 110, 120, 130, 125, 115};
  size_t count = sizeof(closes) / sizeof(closes[0]);
  SamrenaVector *ohlcv = make_ohlcv(arena, closes, count);

  /* Entry: close > 105, Exit: close > 125 */
  SamtraderOperand close_op = samtrader_operand_price(SAMTRADER_OPERAND_PRICE_CLOSE);
  SamtraderOperand const_105 = samtrader_operand_constant(105.0);
  SamtraderOperand const_125 = samtrader_operand_constant(125.0);

  SamtraderRule *entry_rule =
      samtrader_rule_create_comparison(arena, SAMTRADER_RULE_ABOVE, close_op, const_105);
  SamtraderRule *exit_rule =
      samtrader_rule_create_comparison(arena, SAMTRADER_RULE_ABOVE, close_op, const_125);

  SamtraderStrategy strategy = {.name = "test_multi",
                                .entry_long = entry_rule,
                                .exit_long = exit_rule,
                                .entry_short = NULL,
                                .exit_short = NULL,
                                .position_size = 0.5,
                                .stop_loss_pct = 0.0,
                                .take_profit_pct = 0.0,
                                .max_positions = 10};

  SamtraderPortfolio *portfolio = samtrader_portfolio_create(arena, 100000.0);
  ASSERT(portfolio != NULL, "Failed to create portfolio");

  SamHashMap *indicators = samhashmap_create(4, arena);

  int loop_result = run_backtest_loop(arena, ohlcv, &strategy, portfolio, indicators, "TEST", "US",
                                      0.0, 0.0, 0.0, false);
  ASSERT(loop_result == 0, "Backtest loop invariant failed");

  /* Trace:
   * Bar 0: close=90, 90 > 105? no
   * Bar 1: close=100, 100 > 105? no
   * Bar 2: close=110, 110 > 105? yes → enter long
   *   qty = floor(50000/110) = 454, cash = 100000 - 454*110 = 100000 - 49940 = 50060
   * Bar 3: close=120, 120 > 125? no
   * Bar 4: close=130, 130 > 125? yes → exit
   *   cash = 50060 + 454*130 = 50060 + 59020 = 109080
   *   PnL = 454*(130-110) = 9080
   *   entry: 130 > 105? yes → re-enter
   *   qty = floor(54540/130) = 419, cash = 109080 - 419*130 = 109080 - 54470 = 54610
   * Bar 5: close=125, 125 > 125? no
   * Bar 6: close=115, no
   * Bar 7: close=110, no
   * Bar 8: close=100, no
   * Bar 9: close=105, no
   * Bar 10: close=110, no
   * Bar 11: close=120, no
   * Bar 12: close=130, 130 > 125? yes → exit
   *   cash = 54610 + 419*130 = 54610 + 54470 = 109080
   *   PnL = 419*(130-130) = 0
   *   entry: 130 > 105? yes → re-enter
   *   qty = floor(54540/130) = 419, cash = 109080 - 419*130 = 54610
   * Bar 13: close=125, 125 > 125? no
   * Bar 14: close=115, no → still holding
   */

  /* 2 closed trades */
  ASSERT(samrena_vector_size(portfolio->closed_trades) == 2, "Should have 2 closed trades");

  /* First trade: entry 110, exit 130, PnL = 454 * 20 = 9080 */
  const SamtraderClosedTrade *t1 =
      (const SamtraderClosedTrade *)samrena_vector_at_const(portfolio->closed_trades, 0);
  ASSERT(t1 != NULL, "Trade 1 should not be NULL");
  ASSERT_DOUBLE_EQ(t1->entry_price, 110.0, "Trade 1 entry");
  ASSERT_DOUBLE_EQ(t1->exit_price, 130.0, "Trade 1 exit");
  ASSERT_DOUBLE_EQ(t1->pnl, 9080.0, "Trade 1 PnL");

  /* Second trade: entry 130, exit 130, PnL = 0 */
  const SamtraderClosedTrade *t2 =
      (const SamtraderClosedTrade *)samrena_vector_at_const(portfolio->closed_trades, 1);
  ASSERT(t2 != NULL, "Trade 2 should not be NULL");
  ASSERT_DOUBLE_EQ(t2->entry_price, 130.0, "Trade 2 entry");
  ASSERT_DOUBLE_EQ(t2->exit_price, 130.0, "Trade 2 exit");
  ASSERT_DOUBLE_EQ(t2->pnl, 0.0, "Trade 2 PnL");

  /* Compute metrics */
  SamtraderMetrics *metrics =
      samtrader_metrics_calculate(arena, portfolio->closed_trades, portfolio->equity_curve, 0.0);
  ASSERT(metrics != NULL, "Metrics should not be NULL");
  ASSERT(metrics->total_trades == 2, "2 total trades");
  ASSERT(metrics->winning_trades == 1, "1 winning trade (PnL > 0)");
  ASSERT(metrics->losing_trades == 1, "1 losing trade (PnL = 0 counts as loss)");
  ASSERT_DOUBLE_EQ(metrics->win_rate, 0.5, "50% win rate");

  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

/*============================================================================
 * Test 4: SMA Strategy (Indicator Pipeline Integration)
 *============================================================================*/

static int test_sma_strategy(void) {
  printf("Testing SMA strategy...\n");
  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  double closes[] = {100, 102, 104, 103, 101, 99, 97, 98, 100, 103};
  size_t count = sizeof(closes) / sizeof(closes[0]);
  SamrenaVector *ohlcv = make_ohlcv(arena, closes, count);

  /* Calculate SMA(3) from the OHLCV data */
  SamtraderIndicatorSeries *sma3 =
      samtrader_indicator_calculate(arena, SAMTRADER_IND_SMA, ohlcv, 3);
  ASSERT(sma3 != NULL, "SMA(3) calculation failed");
  ASSERT(samtrader_indicator_series_size(sma3) == count, "SMA series should have same length");

  /* Build indicators hashmap */
  SamHashMap *indicators = samhashmap_create(4, arena);
  SamtraderOperand sma_operand = samtrader_operand_indicator(SAMTRADER_IND_SMA, 3);
  char key[64];
  samtrader_operand_indicator_key(key, sizeof(key), &sma_operand);
  samhashmap_put(indicators, key, sma3);

  /* Entry: close > SMA(3), Exit: close < SMA(3) */
  SamtraderOperand close_op = samtrader_operand_price(SAMTRADER_OPERAND_PRICE_CLOSE);

  SamtraderRule *entry_rule =
      samtrader_rule_create_comparison(arena, SAMTRADER_RULE_ABOVE, close_op, sma_operand);
  SamtraderRule *exit_rule =
      samtrader_rule_create_comparison(arena, SAMTRADER_RULE_BELOW, close_op, sma_operand);

  SamtraderStrategy strategy = {.name = "test_sma",
                                .entry_long = entry_rule,
                                .exit_long = exit_rule,
                                .entry_short = NULL,
                                .exit_short = NULL,
                                .position_size = 0.5,
                                .stop_loss_pct = 0.0,
                                .take_profit_pct = 0.0,
                                .max_positions = 10};

  SamtraderPortfolio *portfolio = samtrader_portfolio_create(arena, 100000.0);
  ASSERT(portfolio != NULL, "Failed to create portfolio");

  int loop_result = run_backtest_loop(arena, ohlcv, &strategy, portfolio, indicators, "TEST", "US",
                                      0.0, 0.0, 0.0, false);
  ASSERT(loop_result == 0, "Backtest loop invariant failed");

  /* SMA(3) values:
   * Bar 0: invalid (warmup)
   * Bar 1: invalid (warmup)
   * Bar 2: (100+102+104)/3 = 102.0, valid. close=104 > 102? yes → enter
   *   qty = floor(50000/104) = 480, cash = 100000 - 480*104 = 100000 - 49920 = 50080
   * Bar 3: SMA=(102+104+103)/3=103.0, close=103 < 103? no (not strictly below)
   * Bar 4: SMA=(104+103+101)/3=102.67, close=101 < 102.67? yes → exit
   *   cash = 50080 + 480*101 = 50080 + 48480 = 98560
   *   PnL = 480*(101-104) = -1440
   *   entry: 101 < 102.67, so 101 > 102.67? no → no re-entry
   * Bar 5: SMA=(103+101+99)/3=101.0, close=99 < 101? below. No pos to exit.
   *   entry: 99 > 101? no
   * Bar 6: SMA=(101+99+97)/3=99.0, close=97 < 99? no position.
   *   entry: 97 > 99? no
   * Bar 7: SMA=(99+97+98)/3=98.0, close=98 > 98? no (not strictly above)
   * Bar 8: SMA=(97+98+100)/3=98.33, close=100 > 98.33? yes → enter
   *   qty = floor(49280/100) = 492, cash = 98560 - 492*100 = 98560 - 49200 = 49360
   * Bar 9: SMA=(98+100+103)/3=100.33, close=103 > 100.33 → not exit (not below).
   *   Still holding.
   */

  ASSERT(samrena_vector_size(portfolio->closed_trades) == 1, "Should have 1 closed trade");
  const SamtraderClosedTrade *trade =
      (const SamtraderClosedTrade *)samrena_vector_at_const(portfolio->closed_trades, 0);
  ASSERT(trade != NULL, "Trade should not be NULL");
  ASSERT(trade->quantity == 480, "SMA trade: 480 shares");
  ASSERT_DOUBLE_EQ(trade->entry_price, 104.0, "SMA trade entry at 104");
  ASSERT_DOUBLE_EQ(trade->exit_price, 101.0, "SMA trade exit at 101");
  ASSERT_DOUBLE_EQ(trade->pnl, -1440.0, "SMA trade PnL = -1440");

  /* Second position still open */
  ASSERT(samtrader_portfolio_has_position(portfolio, "TEST"), "Should have open position");
  SamtraderPosition *pos = samtrader_portfolio_get_position(portfolio, "TEST");
  ASSERT(pos != NULL, "Open position should not be NULL");
  ASSERT(pos->quantity == 492, "Second position: 492 shares");
  ASSERT_DOUBLE_EQ(pos->entry_price, 100.0, "Second position entry at 100");

  /* Equity curve has 10 points */
  ASSERT(samrena_vector_size(portfolio->equity_curve) == 10, "Equity curve: 10 points");

  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

/*============================================================================
 * Test 5: Portfolio Invariant Stress Test
 *============================================================================*/

static int test_portfolio_invariant_stress(void) {
  printf("Testing portfolio invariant under volatile prices...\n");
  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  /* 20 bars of volatile sawtooth prices:
   * closes: 100, 98, 105, 96, 110, 94, 115, 92, 120, 90, 125, 88, 130, 86, 135, 84, 140, 82,
   *         145, 80 */
  double closes[20];
  for (int i = 0; i < 20; i++) {
    double base = 100.0;
    if (i % 2 == 0) {
      closes[i] = base + 5.0 * (double)(i / 2);
    } else {
      closes[i] = base - 2.0 * (double)((i + 1) / 2);
    }
  }

  size_t count = 20;
  SamrenaVector *ohlcv = make_ohlcv(arena, closes, count);

  /* Entry: close > 50 (always true after bar 0), Exit: close > 999 (never fires) */
  SamtraderOperand close_op = samtrader_operand_price(SAMTRADER_OPERAND_PRICE_CLOSE);
  SamtraderOperand const_50 = samtrader_operand_constant(50.0);
  SamtraderOperand const_999 = samtrader_operand_constant(999.0);

  SamtraderRule *entry_rule =
      samtrader_rule_create_comparison(arena, SAMTRADER_RULE_ABOVE, close_op, const_50);
  SamtraderRule *exit_rule =
      samtrader_rule_create_comparison(arena, SAMTRADER_RULE_ABOVE, close_op, const_999);

  SamtraderStrategy strategy = {.name = "test_invariant",
                                .entry_long = entry_rule,
                                .exit_long = exit_rule,
                                .entry_short = NULL,
                                .exit_short = NULL,
                                .position_size = 0.5,
                                .stop_loss_pct = 0.0,
                                .take_profit_pct = 0.0,
                                .max_positions = 10};

  SamtraderPortfolio *portfolio = samtrader_portfolio_create(arena, 100000.0);
  ASSERT(portfolio != NULL, "Failed to create portfolio");

  SamHashMap *indicators = samhashmap_create(4, arena);

  /* The main assertion: invariant check passes at every bar */
  int loop_result = run_backtest_loop(arena, ohlcv, &strategy, portfolio, indicators, "TEST", "US",
                                      0.0, 0.0, 0.0, false);
  ASSERT(loop_result == 0, "Portfolio invariant failed during stress test");

  /* Position should be held the entire time (exit never fires) */
  ASSERT(samtrader_portfolio_has_position(portfolio, "TEST"), "Should still have position");
  ASSERT(samrena_vector_size(portfolio->closed_trades) == 0, "No closed trades");
  ASSERT(samrena_vector_size(portfolio->equity_curve) == 20, "Equity curve: 20 points");

  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

/*============================================================================
 * Main
 *============================================================================*/

int main(void) {
  printf("=== Backtest Integration Tests ===\n\n");

  int failures = 0;

  failures += test_simple_long_backtest();
  failures += test_stop_loss_trigger();
  failures += test_multiple_trades();
  failures += test_sma_strategy();
  failures += test_portfolio_invariant_stress();

  printf("\n=== Results: %d failures ===\n", failures);

  return failures > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
