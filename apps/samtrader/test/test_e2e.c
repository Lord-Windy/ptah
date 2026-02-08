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
#include <unistd.h>

#include <samdata/samhashmap.h>
#include <samrena.h>
#include <samvector.h>

#include <samtrader/adapters/file_config_adapter.h>
#include <samtrader/adapters/postgres_adapter.h>
#include <samtrader/adapters/typst_report_adapter.h>
#include <samtrader/domain/backtest.h>
#include <samtrader/domain/execution.h>
#include <samtrader/domain/indicator.h>
#include <samtrader/domain/metrics.h>
#include <samtrader/domain/ohlcv.h>
#include <samtrader/domain/portfolio.h>
#include <samtrader/domain/position.h>
#include <samtrader/domain/rule.h>
#include <samtrader/domain/strategy.h>
#include <samtrader/ports/config_port.h>
#include <samtrader/ports/data_port.h>
#include <samtrader/ports/report_port.h>

/*============================================================================
 * Assertion Macros
 *============================================================================*/

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

#define INDICATOR_KEY_BUF_SIZE 64

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

static char *write_temp_file(const char *name, const char *content) {
  static char path[256];
  snprintf(path, sizeof(path), "/tmp/test_e2e_%s_%d.ini", name, getpid());
  FILE *f = fopen(path, "w");
  if (!f)
    return NULL;
  fprintf(f, "%s", content);
  fclose(f);
  return path;
}

static char *read_file(const char *path) {
  FILE *f = fopen(path, "r");
  if (!f)
    return NULL;
  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  fseek(f, 0, SEEK_SET);
  if (size <= 0) {
    fclose(f);
    return NULL;
  }
  char *buf = malloc((size_t)size + 1);
  if (!buf) {
    fclose(f);
    return NULL;
  }
  size_t n = fread(buf, 1, (size_t)size, f);
  buf[n] = '\0';
  fclose(f);
  return buf;
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

/*============================================================================
 * Copies of main.c static helpers (cannot link static functions)
 *============================================================================*/

static void collect_from_operand(const SamtraderOperand *op, SamHashMap *seen_keys,
                                 SamrenaVector *operands, Samrena *arena) {
  if (op->type != SAMTRADER_OPERAND_INDICATOR)
    return;
  char key_buf[INDICATOR_KEY_BUF_SIZE];
  if (samtrader_operand_indicator_key(key_buf, sizeof(key_buf), op) < 0)
    return;
  if (samhashmap_contains(seen_keys, key_buf))
    return;
  samhashmap_put(seen_keys, key_buf, (void *)1);
  samrena_vector_push(operands, op);
  (void)arena;
}

static void collect_indicator_operands(const SamtraderRule *rule, SamHashMap *seen_keys,
                                       SamrenaVector *operands, Samrena *arena) {
  if (!rule)
    return;
  switch (rule->type) {
    case SAMTRADER_RULE_CROSS_ABOVE:
    case SAMTRADER_RULE_CROSS_BELOW:
    case SAMTRADER_RULE_ABOVE:
    case SAMTRADER_RULE_BELOW:
    case SAMTRADER_RULE_BETWEEN:
    case SAMTRADER_RULE_EQUALS:
      collect_from_operand(&rule->left, seen_keys, operands, arena);
      collect_from_operand(&rule->right, seen_keys, operands, arena);
      break;
    case SAMTRADER_RULE_AND:
    case SAMTRADER_RULE_OR:
      if (rule->children) {
        for (size_t i = 0; rule->children[i] != NULL; i++)
          collect_indicator_operands(rule->children[i], seen_keys, operands, arena);
      }
      break;
    case SAMTRADER_RULE_NOT:
      collect_indicator_operands(rule->child, seen_keys, operands, arena);
      break;
    case SAMTRADER_RULE_CONSECUTIVE:
    case SAMTRADER_RULE_ANY_OF:
      collect_indicator_operands(rule->child, seen_keys, operands, arena);
      break;
  }
}

static SamtraderIndicatorSeries *
calculate_indicator_for_operand(Samrena *arena, const SamtraderOperand *op, SamrenaVector *ohlcv) {
  switch (op->indicator.indicator_type) {
    case SAMTRADER_IND_MACD:
      return samtrader_calculate_macd(arena, ohlcv, op->indicator.period, op->indicator.param2,
                                      op->indicator.param3);
    case SAMTRADER_IND_BOLLINGER:
      return samtrader_calculate_bollinger(arena, ohlcv, op->indicator.period,
                                           op->indicator.param2 / 100.0);
    case SAMTRADER_IND_STOCHASTIC:
      return samtrader_calculate_stochastic(arena, ohlcv, op->indicator.period,
                                            op->indicator.param2);
    case SAMTRADER_IND_PIVOT:
      return samtrader_calculate_pivot(arena, ohlcv);
    default:
      return samtrader_indicator_calculate(arena, op->indicator.indicator_type, ohlcv,
                                           op->indicator.period);
  }
}

static int load_strategy_from_config(SamtraderConfigPort *config, Samrena *arena,
                                     SamtraderStrategy *strategy) {
  memset(strategy, 0, sizeof(*strategy));

  strategy->name = config->get_string(config, "strategy", "name");
  if (!strategy->name)
    strategy->name = "Unnamed Strategy";

  strategy->description = config->get_string(config, "strategy", "description");
  if (!strategy->description)
    strategy->description = "";

  const char *entry_long_str = config->get_string(config, "strategy", "entry_long");
  if (!entry_long_str || entry_long_str[0] == '\0')
    return 1;
  strategy->entry_long = samtrader_rule_parse(arena, entry_long_str);
  if (!strategy->entry_long)
    return 1;

  const char *exit_long_str = config->get_string(config, "strategy", "exit_long");
  if (!exit_long_str || exit_long_str[0] == '\0')
    return 1;
  strategy->exit_long = samtrader_rule_parse(arena, exit_long_str);
  if (!strategy->exit_long)
    return 1;

  const char *entry_short_str = config->get_string(config, "strategy", "entry_short");
  if (entry_short_str && entry_short_str[0] != '\0')
    strategy->entry_short = samtrader_rule_parse(arena, entry_short_str);

  const char *exit_short_str = config->get_string(config, "strategy", "exit_short");
  if (exit_short_str && exit_short_str[0] != '\0')
    strategy->exit_short = samtrader_rule_parse(arena, exit_short_str);

  strategy->position_size = config->get_double(config, "strategy", "position_size", 0.25);
  strategy->stop_loss_pct = config->get_double(config, "strategy", "stop_loss", 0.0);
  strategy->take_profit_pct = config->get_double(config, "strategy", "take_profit", 0.0);
  strategy->max_positions = config->get_int(config, "strategy", "max_positions", 1);

  return 0;
}

static int load_strategy_from_file(const char *strategy_path, Samrena *arena,
                                   SamtraderStrategy *strategy) {
  SamtraderConfigPort *config = samtrader_file_config_adapter_create(arena, strategy_path);
  if (!config)
    return 1;
  int rc = load_strategy_from_config(config, arena, strategy);
  config->close(config);
  return rc;
}

/*============================================================================
 * Shared E2E Pipeline Runner
 *============================================================================*/

/**
 * Run the full pipeline: collect indicators → backtest loop → metrics → report.
 * Returns 0 on success, fills out_metrics and out_portfolio for assertions.
 */
static int run_e2e_pipeline(Samrena *arena, SamrenaVector *ohlcv, SamtraderStrategy *strategy,
                            double initial_capital, double commission_flat, double commission_pct,
                            double slippage_pct, double risk_free_rate, const char *report_path,
                            SamtraderMetrics **out_metrics, SamtraderPortfolio **out_portfolio) {
  /* Collect indicator operands from all rules */
  SamHashMap *seen_keys = samhashmap_create(32, arena);
  SamrenaVector *operands = samrena_vector_init(arena, sizeof(SamtraderOperand), 16);
  collect_indicator_operands(strategy->entry_long, seen_keys, operands, arena);
  collect_indicator_operands(strategy->exit_long, seen_keys, operands, arena);
  if (strategy->entry_short)
    collect_indicator_operands(strategy->entry_short, seen_keys, operands, arena);
  if (strategy->exit_short)
    collect_indicator_operands(strategy->exit_short, seen_keys, operands, arena);

  /* Calculate indicators */
  SamHashMap *indicators = samhashmap_create(32, arena);
  for (size_t i = 0; i < samrena_vector_size(operands); i++) {
    const SamtraderOperand *op = (const SamtraderOperand *)samrena_vector_at_const(operands, i);
    SamtraderIndicatorSeries *series = calculate_indicator_for_operand(arena, op, ohlcv);
    if (!series)
      return 1;
    char key_buf[INDICATOR_KEY_BUF_SIZE];
    samtrader_operand_indicator_key(key_buf, sizeof(key_buf), op);
    samhashmap_put(indicators, key_buf, series);
  }

  /* Create portfolio */
  SamtraderPortfolio *portfolio = samtrader_portfolio_create(arena, initial_capital);
  if (!portfolio)
    return 1;

  size_t bar_count = samrena_vector_size(ohlcv);
  const char *code = "TEST";
  const char *exchange = "US";

  /* Main backtest loop (mirrors main.c) */
  for (size_t i = 0; i < bar_count; i++) {
    const SamtraderOhlcv *bar = (const SamtraderOhlcv *)samrena_vector_at_const(ohlcv, i);

    SamHashMap *price_map = build_price_map(arena, bar);
    if (!price_map)
      continue;

    samtrader_execution_check_triggers(portfolio, arena, price_map, bar->date, commission_flat,
                                       commission_pct, slippage_pct);

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

    if (!samtrader_portfolio_has_position(portfolio, code)) {
      bool enter_long = samtrader_rule_evaluate(strategy->entry_long, ohlcv, indicators, i);
      if (enter_long) {
        samtrader_execution_enter_long(portfolio, arena, code, exchange, bar->close, bar->date,
                                       strategy->position_size, strategy->stop_loss_pct,
                                       strategy->take_profit_pct, strategy->max_positions,
                                       commission_flat, commission_pct, slippage_pct);
      }
    }

    double equity = samtrader_portfolio_total_equity(portfolio, price_map);
    samtrader_portfolio_record_equity(portfolio, arena, bar->date, equity);
  }

  /* Calculate metrics */
  SamtraderMetrics *metrics = samtrader_metrics_calculate(arena, portfolio->closed_trades,
                                                          portfolio->equity_curve, risk_free_rate);
  if (!metrics)
    return 1;

  /* Generate report */
  if (report_path) {
    SamtraderReportPort *report = samtrader_typst_adapter_create(arena, NULL);
    if (!report)
      return 1;

    SamtraderBacktestResult *result = SAMRENA_PUSH_TYPE_ZERO(arena, SamtraderBacktestResult);
    if (!result)
      return 1;
    result->total_return = metrics->total_return;
    result->annualized_return = metrics->annualized_return;
    result->sharpe_ratio = metrics->sharpe_ratio;
    result->sortino_ratio = metrics->sortino_ratio;
    result->max_drawdown = metrics->max_drawdown;
    result->max_drawdown_duration = metrics->max_drawdown_duration;
    result->win_rate = metrics->win_rate;
    result->profit_factor = metrics->profit_factor;
    result->total_trades = metrics->total_trades;
    result->winning_trades = metrics->winning_trades;
    result->losing_trades = metrics->losing_trades;
    result->average_win = metrics->average_win;
    result->average_loss = metrics->average_loss;
    result->largest_win = metrics->largest_win;
    result->largest_loss = metrics->largest_loss;
    result->average_trade_duration = metrics->average_trade_duration;
    result->equity_curve = portfolio->equity_curve;
    result->trades = portfolio->closed_trades;

    bool ok = report->write(report, result, strategy, report_path);
    report->close(report);
    if (!ok)
      return 1;
  }

  *out_metrics = metrics;
  *out_portfolio = portfolio;
  return 0;
}

/*============================================================================
 * Test 1: SMA Crossover Strategy from Config
 *============================================================================*/

static int test_e2e_sma_crossover(void) {
  printf("Testing E2E SMA crossover strategy from config...\n");
  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  /* Write INI config */
  const char *ini = "[strategy]\n"
                    "name = SMA Crossover E2E\n"
                    "description = SMA(3) crosses SMA(5)\n"
                    "entry_long = CROSS_ABOVE(SMA(3), SMA(5))\n"
                    "exit_long = CROSS_BELOW(SMA(3), SMA(5))\n"
                    "position_size = 0.5\n"
                    "max_positions = 1\n";
  char *config_path = write_temp_file("sma_crossover", ini);
  ASSERT(config_path != NULL, "Failed to write config file");

  /* Parse strategy from config */
  SamtraderConfigPort *config = samtrader_file_config_adapter_create(arena, config_path);
  ASSERT(config != NULL, "Failed to create config adapter");

  SamtraderStrategy strategy;
  int rc = load_strategy_from_config(config, arena, &strategy);
  config->close(config);
  ASSERT(rc == 0, "Failed to load strategy from config");
  ASSERT(strategy.entry_long != NULL, "entry_long should be parsed");
  ASSERT(strategy.exit_long != NULL, "exit_long should be parsed");
  ASSERT(strcmp(strategy.name, "SMA Crossover E2E") == 0, "Strategy name mismatch");

  /* Generate 50 bars: decline → rise → decline → rise */
  double closes[50];
  for (int i = 0; i < 50; i++) {
    if (i < 12)
      closes[i] = 100.0 - (double)i * 1.5; /* decline */
    else if (i < 25)
      closes[i] = closes[11] + (double)(i - 11) * 2.0; /* rise */
    else if (i < 38)
      closes[i] = closes[24] - (double)(i - 24) * 1.5; /* decline */
    else
      closes[i] = closes[37] + (double)(i - 37) * 2.5; /* rise */
  }
  SamrenaVector *ohlcv = make_ohlcv(arena, closes, 50);

  /* Run pipeline */
  char report_path[256];
  snprintf(report_path, sizeof(report_path), "/tmp/test_e2e_sma_%d.typ", getpid());

  SamtraderMetrics *metrics = NULL;
  SamtraderPortfolio *portfolio = NULL;
  rc = run_e2e_pipeline(arena, ohlcv, &strategy, 100000.0, 0.0, 0.0, 0.0, 0.05, report_path,
                        &metrics, &portfolio);
  ASSERT(rc == 0, "Pipeline failed");
  ASSERT(metrics != NULL, "Metrics should not be NULL");

  /* Verify trades were generated */
  size_t trade_count = samrena_vector_size(portfolio->closed_trades);
  ASSERT(trade_count >= 1, "Should have at least 1 closed trade");
  ASSERT(metrics->total_trades >= 1, "Metrics should reflect trades");

  /* Verify report was generated and contains strategy name */
  char *report_content = read_file(report_path);
  ASSERT(report_content != NULL, "Report file should be readable");
  ASSERT(strstr(report_content, "SMA Crossover E2E") != NULL,
         "Report should contain strategy name");

  free(report_content);
  unlink(report_path);
  unlink(config_path);
  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

/*============================================================================
 * Test 2: RSI Mean Reversion Strategy
 *============================================================================*/

static int test_e2e_rsi_mean_reversion(void) {
  printf("Testing E2E RSI mean reversion strategy...\n");
  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  const char *ini = "[strategy]\n"
                    "name = RSI Mean Reversion E2E\n"
                    "description = Buy oversold, sell overbought\n"
                    "entry_long = BELOW(RSI(14), 30)\n"
                    "exit_long = ABOVE(RSI(14), 70)\n"
                    "position_size = 0.5\n"
                    "max_positions = 1\n";
  char *config_path = write_temp_file("rsi_reversion", ini);
  ASSERT(config_path != NULL, "Failed to write config file");

  SamtraderConfigPort *config = samtrader_file_config_adapter_create(arena, config_path);
  ASSERT(config != NULL, "Failed to create config adapter");

  SamtraderStrategy strategy;
  int rc = load_strategy_from_config(config, arena, &strategy);
  config->close(config);
  ASSERT(rc == 0, "Failed to load RSI strategy");

  /* Generate 50 bars: sustained decline (RSI < 30) → sustained rise (RSI > 70) */
  double closes[50];
  closes[0] = 100.0;
  for (int i = 1; i < 50; i++) {
    if (i < 20)
      closes[i] = closes[i - 1] - 2.0; /* sustained decline → RSI drops below 30 */
    else if (i < 25)
      closes[i] = closes[i - 1]; /* flat bottom */
    else
      closes[i] = closes[i - 1] + 3.0; /* sustained rise → RSI climbs above 70 */
  }
  SamrenaVector *ohlcv = make_ohlcv(arena, closes, 50);

  char report_path[256];
  snprintf(report_path, sizeof(report_path), "/tmp/test_e2e_rsi_%d.typ", getpid());

  SamtraderMetrics *metrics = NULL;
  SamtraderPortfolio *portfolio = NULL;
  rc = run_e2e_pipeline(arena, ohlcv, &strategy, 100000.0, 0.0, 0.0, 0.0, 0.05, report_path,
                        &metrics, &portfolio);
  ASSERT(rc == 0, "Pipeline failed");

  /* Verify at least 1 trade completed */
  size_t trade_count = samrena_vector_size(portfolio->closed_trades);
  ASSERT(trade_count >= 1, "Should have at least 1 closed trade");

  /* The trade should be profitable: bought oversold, sold overbought */
  const SamtraderClosedTrade *trade =
      (const SamtraderClosedTrade *)samrena_vector_at_const(portfolio->closed_trades, 0);
  ASSERT(trade != NULL, "First trade should exist");
  ASSERT(trade->pnl > 0.0, "RSI mean reversion trade should be profitable");

  /* Verify report */
  char *report_content = read_file(report_path);
  ASSERT(report_content != NULL, "Report should be readable");
  ASSERT(strstr(report_content, "RSI Mean Reversion E2E") != NULL,
         "Report should contain strategy name");

  free(report_content);
  unlink(report_path);
  unlink(config_path);
  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

/*============================================================================
 * Test 3: Commission and Slippage Costs
 *============================================================================*/

static int test_e2e_with_costs(void) {
  printf("Testing E2E with commission and slippage costs...\n");
  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  const char *ini = "[strategy]\n"
                    "name = SMA Crossover With Costs\n"
                    "entry_long = CROSS_ABOVE(SMA(3), SMA(5))\n"
                    "exit_long = CROSS_BELOW(SMA(3), SMA(5))\n"
                    "position_size = 0.5\n"
                    "max_positions = 1\n";
  char *config_path = write_temp_file("costs", ini);
  ASSERT(config_path != NULL, "Failed to write config file");

  SamtraderConfigPort *config = samtrader_file_config_adapter_create(arena, config_path);
  ASSERT(config != NULL, "Failed to create config adapter");

  SamtraderStrategy strategy;
  int rc = load_strategy_from_config(config, arena, &strategy);
  config->close(config);
  ASSERT(rc == 0, "Failed to load strategy");

  /* Same price data as SMA crossover test */
  double closes[50];
  for (int i = 0; i < 50; i++) {
    if (i < 12)
      closes[i] = 100.0 - (double)i * 1.5;
    else if (i < 25)
      closes[i] = closes[11] + (double)(i - 11) * 2.0;
    else if (i < 38)
      closes[i] = closes[24] - (double)(i - 24) * 1.5;
    else
      closes[i] = closes[37] + (double)(i - 37) * 2.5;
  }
  SamrenaVector *ohlcv = make_ohlcv(arena, closes, 50);

  /* Run without costs */
  SamtraderMetrics *metrics_no_cost = NULL;
  SamtraderPortfolio *portfolio_no_cost = NULL;
  rc = run_e2e_pipeline(arena, ohlcv, &strategy, 100000.0, 0.0, 0.0, 0.0, 0.05, NULL,
                        &metrics_no_cost, &portfolio_no_cost);
  ASSERT(rc == 0, "Pipeline without costs failed");

  /* Run with costs: commission_pct = 0.5%, slippage_pct = 0.1% */
  Samrena *arena2 = samrena_create_default();
  ASSERT(arena2 != NULL, "Failed to create second arena");

  /* Re-parse strategy in new arena */
  SamtraderConfigPort *config2 = samtrader_file_config_adapter_create(arena2, config_path);
  ASSERT(config2 != NULL, "Failed to create config adapter 2");
  SamtraderStrategy strategy2;
  rc = load_strategy_from_config(config2, arena2, &strategy2);
  config2->close(config2);
  ASSERT(rc == 0, "Failed to load strategy 2");

  SamrenaVector *ohlcv2 = make_ohlcv(arena2, closes, 50);

  char report_path[256];
  snprintf(report_path, sizeof(report_path), "/tmp/test_e2e_costs_%d.typ", getpid());

  SamtraderMetrics *metrics_with_cost = NULL;
  SamtraderPortfolio *portfolio_with_cost = NULL;
  rc = run_e2e_pipeline(arena2, ohlcv2, &strategy2, 100000.0, 0.0, 0.5, 0.1, 0.05, report_path,
                        &metrics_with_cost, &portfolio_with_cost);
  ASSERT(rc == 0, "Pipeline with costs failed");

  /* Both runs should produce trades */
  size_t trades_no_cost = samrena_vector_size(portfolio_no_cost->closed_trades);
  size_t trades_with_cost = samrena_vector_size(portfolio_with_cost->closed_trades);
  ASSERT(trades_no_cost >= 1, "Should have trades without costs");
  ASSERT(trades_with_cost >= 1, "Should have trades with costs");

  /* With costs, each trade's PnL should be less than naive (exit - entry) * qty.
   * We verify by checking the total PnL across all cost trades is less than no-cost trades. */
  double total_pnl_no_cost = 0.0;
  for (size_t i = 0; i < trades_no_cost; i++) {
    const SamtraderClosedTrade *t =
        (const SamtraderClosedTrade *)samrena_vector_at_const(portfolio_no_cost->closed_trades, i);
    total_pnl_no_cost += t->pnl;
  }
  double total_pnl_with_cost = 0.0;
  for (size_t i = 0; i < trades_with_cost; i++) {
    const SamtraderClosedTrade *t =
        (const SamtraderClosedTrade *)samrena_vector_at_const(portfolio_with_cost->closed_trades,
                                                              i);
    total_pnl_with_cost += t->pnl;
  }
  ASSERT(total_pnl_with_cost < total_pnl_no_cost, "Costs should reduce total PnL");

  /* Report should be generated */
  char *report_content = read_file(report_path);
  ASSERT(report_content != NULL, "Report should be readable");

  free(report_content);
  unlink(report_path);
  unlink(config_path);
  samrena_destroy(arena);
  samrena_destroy(arena2);
  printf("  PASS\n");
  return 0;
}

/*============================================================================
 * Test 4: Stop Loss and Take Profit Triggers
 *============================================================================*/

static int test_e2e_stop_loss_take_profit(void) {
  printf("Testing E2E stop loss and take profit triggers...\n");
  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  const char *ini = "[strategy]\n"
                    "name = SL/TP Test\n"
                    "entry_long = ABOVE(close, 95)\n"
                    "exit_long = ABOVE(close, 999)\n"
                    "position_size = 0.5\n"
                    "stop_loss = 5.0\n"
                    "take_profit = 10.0\n"
                    "max_positions = 1\n";
  char *config_path = write_temp_file("sl_tp", ini);
  ASSERT(config_path != NULL, "Failed to write config file");

  SamtraderConfigPort *config = samtrader_file_config_adapter_create(arena, config_path);
  ASSERT(config != NULL, "Failed to create config adapter");

  SamtraderStrategy strategy;
  int rc = load_strategy_from_config(config, arena, &strategy);
  config->close(config);
  ASSERT(rc == 0, "Failed to load strategy");
  ASSERT_DOUBLE_EQ(strategy.stop_loss_pct, 5.0, "Stop loss should be 5%");
  ASSERT_DOUBLE_EQ(strategy.take_profit_pct, 10.0, "Take profit should be 10%");

  /* 30 bars: enter at ~100, take profit at ~110, re-enter, stop loss on decline */
  double closes[30];
  closes[0] = 90.0;  /* no entry (below 95) */
  closes[1] = 100.0; /* entry: price > 95, SL = 95, TP = 110 */
  for (int i = 2; i < 8; i++)
    closes[i] = 100.0 + (double)(i - 1) * 2.0; /* rising to 114 */
  /* By bar 7: close=112, TP=110 → should have triggered TP */
  closes[8] = 112.0; /* still high */
  closes[9] = 100.0; /* re-enter (>95), new SL=95, TP=110 */
  for (int i = 10; i < 15; i++)
    closes[i] = 100.0; /* flat */
  for (int i = 15; i < 25; i++)
    closes[i] = 100.0 - (double)(i - 14) * 2.0; /* decline to 80 */
  /* SL at 95 triggers when close drops to 92 or below */
  for (int i = 25; i < 30; i++)
    closes[i] = 75.0 + (double)(i - 25); /* flat-ish bottom */

  SamrenaVector *ohlcv = make_ohlcv(arena, closes, 30);

  char report_path[256];
  snprintf(report_path, sizeof(report_path), "/tmp/test_e2e_sltp_%d.typ", getpid());

  SamtraderMetrics *metrics = NULL;
  SamtraderPortfolio *portfolio = NULL;
  rc = run_e2e_pipeline(arena, ohlcv, &strategy, 100000.0, 0.0, 0.0, 0.0, 0.05, report_path,
                        &metrics, &portfolio);
  ASSERT(rc == 0, "Pipeline failed");

  /* Should have at least 2 trades (TP exit + SL exit) */
  size_t trade_count = samrena_vector_size(portfolio->closed_trades);
  ASSERT(trade_count >= 2, "Should have at least 2 closed trades (TP and SL)");

  /* Check for at least one positive PnL (TP) and one negative PnL (SL) */
  bool has_positive = false;
  bool has_negative = false;
  for (size_t i = 0; i < trade_count; i++) {
    const SamtraderClosedTrade *t =
        (const SamtraderClosedTrade *)samrena_vector_at_const(portfolio->closed_trades, i);
    if (t->pnl > 0.0)
      has_positive = true;
    if (t->pnl < 0.0)
      has_negative = true;
  }
  ASSERT(has_positive, "Should have at least one profitable trade (TP)");
  ASSERT(has_negative, "Should have at least one losing trade (SL)");

  /* Report generated */
  char *report_content = read_file(report_path);
  ASSERT(report_content != NULL, "Report should be readable");

  free(report_content);
  unlink(report_path);
  unlink(config_path);
  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

/*============================================================================
 * Test 5: Strategy Loaded from Separate File
 *============================================================================*/

static int test_e2e_strategy_from_file(void) {
  printf("Testing E2E strategy loaded from separate file...\n");
  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  /* Write separate strategy INI file */
  const char *strategy_ini = "[strategy]\n"
                             "name = File Strategy\n"
                             "description = Strategy loaded from separate file\n"
                             "entry_long = ABOVE(close, 95)\n"
                             "exit_long = ABOVE(close, 115)\n"
                             "position_size = 0.25\n"
                             "max_positions = 1\n";
  char *strategy_path = write_temp_file("file_strategy", strategy_ini);
  ASSERT(strategy_path != NULL, "Failed to write strategy file");

  /* Load strategy via load_strategy_from_file */
  SamtraderStrategy strategy;
  int rc = load_strategy_from_file(strategy_path, arena, &strategy);
  ASSERT(rc == 0, "Failed to load strategy from file");
  ASSERT(strcmp(strategy.name, "File Strategy") == 0, "Strategy name should be 'File Strategy'");

  /* Generate price data */
  double closes[] = {90,  95, 100, 105, 110, 115, 120, 115, 110, 105,
                     100, 95, 100, 105, 110, 115, 120, 125, 120, 115};
  SamrenaVector *ohlcv = make_ohlcv(arena, closes, 20);

  char report_path[256];
  snprintf(report_path, sizeof(report_path), "/tmp/test_e2e_file_%d.typ", getpid());

  SamtraderMetrics *metrics = NULL;
  SamtraderPortfolio *portfolio = NULL;
  rc = run_e2e_pipeline(arena, ohlcv, &strategy, 100000.0, 0.0, 0.0, 0.0, 0.05, report_path,
                        &metrics, &portfolio);
  ASSERT(rc == 0, "Pipeline failed");
  ASSERT(metrics != NULL, "Metrics should not be NULL");

  /* Verify trades occurred */
  size_t trade_count = samrena_vector_size(portfolio->closed_trades);
  ASSERT(trade_count >= 1, "Should have at least 1 closed trade");

  /* Verify report contains the file-loaded strategy name */
  char *report_content = read_file(report_path);
  ASSERT(report_content != NULL, "Report should be readable");
  ASSERT(strstr(report_content, "File Strategy") != NULL, "Report should contain 'File Strategy'");

  free(report_content);
  unlink(report_path);
  unlink(strategy_path);
  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

/*============================================================================
 * Test 6: Full DB Pipeline (env-gated)
 *============================================================================*/

static int test_e2e_full_db_pipeline(void) {
  printf("Testing E2E full DB pipeline...\n");

  const char *conninfo = getenv("SAMTRADER_TEST_PG_CONNINFO");
  if (!conninfo) {
    printf("  SKIP (SAMTRADER_TEST_PG_CONNINFO not set)\n");
    return 0;
  }

  const char *code = getenv("SAMTRADER_TEST_CODE");
  if (!code)
    code = "BHP";
  const char *exchange = getenv("SAMTRADER_TEST_EXCHANGE");
  if (!exchange)
    exchange = "AU";

  Samrena *arena = samrena_create_default();
  ASSERT(arena != NULL, "Failed to create arena");

  /* Connect to database */
  SamtraderDataPort *data = samtrader_postgres_adapter_create(arena, conninfo);
  ASSERT(data != NULL, "Failed to connect to database");

  /* Fetch all OHLCV data */
  time_t epoch_start = 0;
  time_t epoch_end = (time_t)4102444800; /* 2100-01-01 */
  SamrenaVector *ohlcv = data->fetch_ohlcv(data, code, exchange, epoch_start, epoch_end);
  ASSERT(ohlcv != NULL, "Failed to fetch OHLCV data");

  size_t bar_count = samrena_vector_size(ohlcv);
  ASSERT(bar_count >= 30, "Need at least 30 bars");
  printf("  Fetched %zu bars for %s.%s\n", bar_count, code, exchange);

  /* Write strategy config */
  const char *strategy_ini = "[strategy]\n"
                             "name = DB E2E Test\n"
                             "entry_long = CROSS_ABOVE(SMA(10), SMA(30))\n"
                             "exit_long = CROSS_BELOW(SMA(10), SMA(30))\n"
                             "position_size = 0.5\n"
                             "max_positions = 1\n";
  char *config_path = write_temp_file("db_e2e", strategy_ini);
  ASSERT(config_path != NULL, "Failed to write config");

  SamtraderConfigPort *config = samtrader_file_config_adapter_create(arena, config_path);
  ASSERT(config != NULL, "Failed to create config adapter");

  SamtraderStrategy strategy;
  int rc = load_strategy_from_config(config, arena, &strategy);
  config->close(config);
  ASSERT(rc == 0, "Failed to load strategy");

  /* Collect indicators and calculate */
  SamHashMap *seen_keys = samhashmap_create(32, arena);
  SamrenaVector *operands = samrena_vector_init(arena, sizeof(SamtraderOperand), 16);
  collect_indicator_operands(strategy.entry_long, seen_keys, operands, arena);
  collect_indicator_operands(strategy.exit_long, seen_keys, operands, arena);

  SamHashMap *indicators = samhashmap_create(32, arena);
  for (size_t i = 0; i < samrena_vector_size(operands); i++) {
    const SamtraderOperand *op = (const SamtraderOperand *)samrena_vector_at_const(operands, i);
    SamtraderIndicatorSeries *series = calculate_indicator_for_operand(arena, op, ohlcv);
    ASSERT(series != NULL, "Indicator calculation failed");
    char key_buf[INDICATOR_KEY_BUF_SIZE];
    samtrader_operand_indicator_key(key_buf, sizeof(key_buf), op);
    samhashmap_put(indicators, key_buf, series);
  }

  /* Create portfolio and run backtest */
  SamtraderPortfolio *portfolio = samtrader_portfolio_create(arena, 100000.0);
  ASSERT(portfolio != NULL, "Failed to create portfolio");

  for (size_t i = 0; i < bar_count; i++) {
    const SamtraderOhlcv *bar = (const SamtraderOhlcv *)samrena_vector_at_const(ohlcv, i);
    SamHashMap *price_map = build_price_map(arena, bar);
    if (!price_map)
      continue;

    samtrader_execution_check_triggers(portfolio, arena, price_map, bar->date, 0.0, 0.0, 0.0);

    if (samtrader_portfolio_has_position(portfolio, bar->code)) {
      SamtraderPosition *pos = samtrader_portfolio_get_position(portfolio, bar->code);
      bool should_exit = false;
      if (pos && samtrader_position_is_long(pos))
        should_exit = samtrader_rule_evaluate(strategy.exit_long, ohlcv, indicators, i);
      if (should_exit)
        samtrader_execution_exit_position(portfolio, arena, bar->code, bar->close, bar->date, 0.0,
                                          0.0, 0.0);
    }

    if (!samtrader_portfolio_has_position(portfolio, bar->code)) {
      bool enter_long = samtrader_rule_evaluate(strategy.entry_long, ohlcv, indicators, i);
      if (enter_long)
        samtrader_execution_enter_long(portfolio, arena, bar->code, bar->exchange, bar->close,
                                       bar->date, strategy.position_size, strategy.stop_loss_pct,
                                       strategy.take_profit_pct, strategy.max_positions, 0.0, 0.0,
                                       0.0);
    }

    double equity = samtrader_portfolio_total_equity(portfolio, price_map);
    samtrader_portfolio_record_equity(portfolio, arena, bar->date, equity);
  }

  /* Metrics */
  SamtraderMetrics *metrics =
      samtrader_metrics_calculate(arena, portfolio->closed_trades, portfolio->equity_curve, 0.05);
  ASSERT(metrics != NULL, "Metrics calculation failed");
  printf("  Trades: %d, Return: %.2f%%\n", metrics->total_trades, metrics->total_return * 100.0);

  /* Generate report */
  char report_path[256];
  snprintf(report_path, sizeof(report_path), "/tmp/test_e2e_db_%d.typ", getpid());

  SamtraderReportPort *report = samtrader_typst_adapter_create(arena, NULL);
  ASSERT(report != NULL, "Failed to create report adapter");

  SamtraderBacktestResult *result = SAMRENA_PUSH_TYPE_ZERO(arena, SamtraderBacktestResult);
  ASSERT(result != NULL, "Failed to allocate result");
  result->total_return = metrics->total_return;
  result->annualized_return = metrics->annualized_return;
  result->sharpe_ratio = metrics->sharpe_ratio;
  result->sortino_ratio = metrics->sortino_ratio;
  result->max_drawdown = metrics->max_drawdown;
  result->max_drawdown_duration = metrics->max_drawdown_duration;
  result->win_rate = metrics->win_rate;
  result->profit_factor = metrics->profit_factor;
  result->total_trades = metrics->total_trades;
  result->winning_trades = metrics->winning_trades;
  result->losing_trades = metrics->losing_trades;
  result->average_win = metrics->average_win;
  result->average_loss = metrics->average_loss;
  result->largest_win = metrics->largest_win;
  result->largest_loss = metrics->largest_loss;
  result->average_trade_duration = metrics->average_trade_duration;
  result->equity_curve = portfolio->equity_curve;
  result->trades = portfolio->closed_trades;

  bool ok = report->write(report, result, &strategy, report_path);
  report->close(report);
  ASSERT(ok, "Report generation failed");

  char *report_content = read_file(report_path);
  ASSERT(report_content != NULL, "Report should be readable");
  ASSERT(strstr(report_content, "DB E2E Test") != NULL, "Report should contain strategy name");

  free(report_content);
  unlink(report_path);
  unlink(config_path);
  data->close(data);
  samrena_destroy(arena);
  printf("  PASS\n");
  return 0;
}

/*============================================================================
 * Main
 *============================================================================*/

int main(void) {
  printf("=== End-to-End Pipeline Tests ===\n\n");

  int failures = 0;

  failures += test_e2e_sma_crossover();
  failures += test_e2e_rsi_mean_reversion();
  failures += test_e2e_with_costs();
  failures += test_e2e_stop_loss_take_profit();
  failures += test_e2e_strategy_from_file();
  failures += test_e2e_full_db_pipeline();

  printf("\n=== Results: %d failures ===\n", failures);

  return failures > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
