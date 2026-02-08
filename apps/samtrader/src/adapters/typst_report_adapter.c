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

#include <samtrader/adapters/typst_report_adapter.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Maximum template file size (1MB) */
#define MAX_TEMPLATE_SIZE (1024 * 1024)

/* Maximum placeholder key length */
#define MAX_KEY_LENGTH 64

/* Maximum formatted value length */
#define MAX_VALUE_LENGTH 256

/**
 * @brief Internal implementation data for Typst report adapter.
 */
typedef struct {
  const char *template_path; /**< Path to custom template, or NULL */
} TypstReportImpl;

/* Forward declarations */
static bool typst_report_write(SamtraderReportPort *port, SamtraderBacktestResult *result,
                               SamtraderStrategy *strategy, const char *output_path);
static void typst_report_close(SamtraderReportPort *port);

/* ============================================================================
 * Template Placeholder Resolution
 * ============================================================================ */

/**
 * @brief Resolve a placeholder key to its value.
 *
 * @param key Placeholder key (without {{ }} delimiters)
 * @param result Backtest results
 * @param strategy Strategy definition
 * @param buf Output buffer for the value
 * @param buf_size Size of the output buffer
 * @return true if the key was recognized, false otherwise
 */
static bool resolve_placeholder(const char *key, SamtraderBacktestResult *result,
                                SamtraderStrategy *strategy, char *buf, size_t buf_size) {
  if (strcmp(key, "STRATEGY_NAME") == 0) {
    snprintf(buf, buf_size, "%s", strategy->name ? strategy->name : "Unnamed Strategy");
    return true;
  }
  if (strcmp(key, "STRATEGY_DESCRIPTION") == 0) {
    snprintf(buf, buf_size, "%s", strategy->description ? strategy->description : "");
    return true;
  }
  if (strcmp(key, "POSITION_SIZE") == 0) {
    snprintf(buf, buf_size, "%.1f", strategy->position_size * 100.0);
    return true;
  }
  if (strcmp(key, "STOP_LOSS_PCT") == 0) {
    snprintf(buf, buf_size, "%.1f", strategy->stop_loss_pct);
    return true;
  }
  if (strcmp(key, "TAKE_PROFIT_PCT") == 0) {
    snprintf(buf, buf_size, "%.1f", strategy->take_profit_pct);
    return true;
  }
  if (strcmp(key, "MAX_POSITIONS") == 0) {
    snprintf(buf, buf_size, "%d", strategy->max_positions);
    return true;
  }
  if (strcmp(key, "TOTAL_RETURN") == 0) {
    snprintf(buf, buf_size, "%.2f", result->total_return * 100.0);
    return true;
  }
  if (strcmp(key, "ANNUALIZED_RETURN") == 0) {
    snprintf(buf, buf_size, "%.2f", result->annualized_return * 100.0);
    return true;
  }
  if (strcmp(key, "SHARPE_RATIO") == 0) {
    snprintf(buf, buf_size, "%.3f", result->sharpe_ratio);
    return true;
  }
  if (strcmp(key, "SORTINO_RATIO") == 0) {
    snprintf(buf, buf_size, "%.3f", result->sortino_ratio);
    return true;
  }
  if (strcmp(key, "MAX_DRAWDOWN") == 0) {
    snprintf(buf, buf_size, "%.2f", result->max_drawdown * 100.0);
    return true;
  }
  if (strcmp(key, "MAX_DRAWDOWN_DURATION") == 0) {
    snprintf(buf, buf_size, "%.0f", result->max_drawdown_duration);
    return true;
  }
  if (strcmp(key, "WIN_RATE") == 0) {
    snprintf(buf, buf_size, "%.1f", result->win_rate * 100.0);
    return true;
  }
  if (strcmp(key, "PROFIT_FACTOR") == 0) {
    snprintf(buf, buf_size, "%.2f", result->profit_factor);
    return true;
  }
  if (strcmp(key, "TOTAL_TRADES") == 0) {
    snprintf(buf, buf_size, "%d", result->total_trades);
    return true;
  }
  if (strcmp(key, "WINNING_TRADES") == 0) {
    snprintf(buf, buf_size, "%d", result->winning_trades);
    return true;
  }
  if (strcmp(key, "LOSING_TRADES") == 0) {
    snprintf(buf, buf_size, "%d", result->losing_trades);
    return true;
  }
  if (strcmp(key, "AVERAGE_WIN") == 0) {
    snprintf(buf, buf_size, "%.2f", result->average_win);
    return true;
  }
  if (strcmp(key, "AVERAGE_LOSS") == 0) {
    snprintf(buf, buf_size, "%.2f", result->average_loss);
    return true;
  }
  if (strcmp(key, "LARGEST_WIN") == 0) {
    snprintf(buf, buf_size, "%.2f", result->largest_win);
    return true;
  }
  if (strcmp(key, "LARGEST_LOSS") == 0) {
    snprintf(buf, buf_size, "%.2f", result->largest_loss);
    return true;
  }
  if (strcmp(key, "AVG_TRADE_DURATION") == 0) {
    snprintf(buf, buf_size, "%.1f", result->average_trade_duration);
    return true;
  }
  if (strcmp(key, "GENERATED_DATE") == 0) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buf, buf_size, "%Y-%m-%d", tm_info);
    return true;
  }

  return false;
}

/* ============================================================================
 * Template-Based Report Generation
 * ============================================================================ */

/**
 * @brief Write a report using a custom Typst template with placeholder substitution.
 *
 * Reads the template file, replaces {{PLACEHOLDER}} markers with actual values,
 * and writes the result to the output file.
 *
 * @param template_path Path to the template file
 * @param result Backtest results
 * @param strategy Strategy definition
 * @param output_path Output file path
 * @return true on success, false on failure
 */
static bool write_template_report(const char *template_path, SamtraderBacktestResult *result,
                                  SamtraderStrategy *strategy, const char *output_path) {
  /* Read template file */
  FILE *tmpl_file = fopen(template_path, "r");
  if (tmpl_file == NULL) {
    return false;
  }

  fseek(tmpl_file, 0, SEEK_END);
  long file_size = ftell(tmpl_file);
  fseek(tmpl_file, 0, SEEK_SET);

  if (file_size <= 0 || (size_t)file_size > MAX_TEMPLATE_SIZE) {
    fclose(tmpl_file);
    return false;
  }

  char *template_buf = malloc((size_t)file_size + 1);
  if (template_buf == NULL) {
    fclose(tmpl_file);
    return false;
  }

  size_t bytes_read = fread(template_buf, 1, (size_t)file_size, tmpl_file);
  fclose(tmpl_file);
  template_buf[bytes_read] = '\0';

  /* Open output file */
  FILE *out = fopen(output_path, "w");
  if (out == NULL) {
    free(template_buf);
    return false;
  }

  /* Process template: scan for {{ }} placeholders */
  const char *pos = template_buf;
  while (*pos != '\0') {
    const char *open = strstr(pos, "{{");
    if (open == NULL) {
      /* No more placeholders, write remaining text */
      fputs(pos, out);
      break;
    }

    /* Write text before the placeholder */
    fwrite(pos, 1, (size_t)(open - pos), out);

    const char *key_start = open + 2;
    const char *close_marker = strstr(key_start, "}}");
    if (close_marker == NULL) {
      /* Unterminated placeholder, write it literally */
      fputs(open, out);
      break;
    }

    /* Extract key */
    size_t key_len = (size_t)(close_marker - key_start);
    if (key_len >= MAX_KEY_LENGTH) {
      /* Key too long, write literally */
      fwrite(open, 1, (size_t)(close_marker + 2 - open), out);
      pos = close_marker + 2;
      continue;
    }

    char key[MAX_KEY_LENGTH];
    memcpy(key, key_start, key_len);
    key[key_len] = '\0';

    /* Resolve placeholder */
    char value[MAX_VALUE_LENGTH];
    if (resolve_placeholder(key, result, strategy, value, sizeof(value))) {
      fputs(value, out);
    } else {
      /* Unknown placeholder, write it literally */
      fwrite(open, 1, (size_t)(close_marker + 2 - open), out);
    }

    pos = close_marker + 2;
  }

  free(template_buf);
  fclose(out);
  return true;
}

/* ============================================================================
 * Default Report Generation
 * ============================================================================ */

/**
 * @brief Write the Typst document preamble.
 */
static void write_preamble(FILE *out, const char *strategy_name) {
  fprintf(out, "#set document(title: \"Backtest Report: %s\")\n", strategy_name);
  fprintf(out, "#set page(paper: \"a4\", margin: 2cm)\n");
  fprintf(out, "#set text(font: \"New Computer Modern\", size: 11pt)\n");
  fprintf(out, "\n");
}

/**
 * @brief Write the report title and generation date.
 */
static void write_title(FILE *out, const char *strategy_name) {
  time_t now = time(NULL);
  struct tm *tm_info = localtime(&now);
  char date_buf[32];
  strftime(date_buf, sizeof(date_buf), "%Y-%m-%d", tm_info);

  fprintf(out, "= Backtest Report: %s\n", strategy_name);
  fprintf(out, "\n");
  fprintf(out, "_Generated on %s_\n", date_buf);
  fprintf(out, "\n");
}

/**
 * @brief Write the strategy summary section.
 */
static void write_strategy_summary(FILE *out, SamtraderStrategy *strategy) {
  const char *name = strategy->name ? strategy->name : "Unnamed Strategy";
  const char *desc = strategy->description ? strategy->description : "No description provided.";

  fprintf(out, "== Strategy Summary\n");
  fprintf(out, "\n");
  fprintf(out, "#table(\n");
  fprintf(out, "  columns: (auto, 1fr),\n");
  fprintf(out, "  stroke: none,\n");
  fprintf(out, "  inset: 6pt,\n");
  fprintf(out, "  [*Name*], [%s],\n", name);
  fprintf(out, "  [*Description*], [%s],\n", desc);
  fprintf(out, ")\n");
  fprintf(out, "\n");
}

/**
 * @brief Write the strategy parameters section.
 */
static void write_strategy_parameters(FILE *out, SamtraderStrategy *strategy) {
  fprintf(out, "== Strategy Parameters\n");
  fprintf(out, "\n");
  fprintf(out, "#table(\n");
  fprintf(out, "  columns: (1fr, 1fr),\n");
  fprintf(out, "  inset: 8pt,\n");
  fprintf(out, "  fill: (x, y) => if y == 0 { luma(230) },\n");
  fprintf(out, "  [*Parameter*], [*Value*],\n");
  fprintf(out, "  [Position Size], [%.1f%%],\n", strategy->position_size * 100.0);

  if (strategy->stop_loss_pct > 0.0) {
    fprintf(out, "  [Stop Loss], [%.1f%%],\n", strategy->stop_loss_pct);
  } else {
    fprintf(out, "  [Stop Loss], [None],\n");
  }

  if (strategy->take_profit_pct > 0.0) {
    fprintf(out, "  [Take Profit], [%.1f%%],\n", strategy->take_profit_pct);
  } else {
    fprintf(out, "  [Take Profit], [None],\n");
  }

  fprintf(out, "  [Max Positions], [%d],\n", strategy->max_positions);
  fprintf(out, "  [Long Entry], [%s],\n", strategy->entry_long ? "Defined" : "None");
  fprintf(out, "  [Long Exit], [%s],\n", strategy->exit_long ? "Defined" : "None");
  fprintf(out, "  [Short Entry], [%s],\n", strategy->entry_short ? "Defined" : "None");
  fprintf(out, "  [Short Exit], [%s],\n", strategy->exit_short ? "Defined" : "None");
  fprintf(out, ")\n");
  fprintf(out, "\n");
}

/**
 * @brief Write the performance metrics section.
 */
static void write_performance_metrics(FILE *out, SamtraderBacktestResult *result) {
  fprintf(out, "== Performance Metrics\n");
  fprintf(out, "\n");
  fprintf(out, "=== Return Metrics\n");
  fprintf(out, "\n");
  fprintf(out, "#table(\n");
  fprintf(out, "  columns: (1fr, 1fr),\n");
  fprintf(out, "  inset: 8pt,\n");
  fprintf(out, "  fill: (x, y) => if y == 0 { luma(230) },\n");
  fprintf(out, "  [*Metric*], [*Value*],\n");
  fprintf(out, "  [Total Return], [%.2f%%],\n", result->total_return * 100.0);
  fprintf(out, "  [Annualized Return], [%.2f%%],\n", result->annualized_return * 100.0);
  fprintf(out, "  [Sharpe Ratio], [%.3f],\n", result->sharpe_ratio);
  fprintf(out, "  [Sortino Ratio], [%.3f],\n", result->sortino_ratio);
  fprintf(out, ")\n");
  fprintf(out, "\n");

  fprintf(out, "=== Risk Metrics\n");
  fprintf(out, "\n");
  fprintf(out, "#table(\n");
  fprintf(out, "  columns: (1fr, 1fr),\n");
  fprintf(out, "  inset: 8pt,\n");
  fprintf(out, "  fill: (x, y) => if y == 0 { luma(230) },\n");
  fprintf(out, "  [*Metric*], [*Value*],\n");
  fprintf(out, "  [Max Drawdown], [%.2f%%],\n", result->max_drawdown * 100.0);
  fprintf(out, "  [Max Drawdown Duration], [%.0f days],\n", result->max_drawdown_duration);
  fprintf(out, "  [Profit Factor], [%.2f],\n", result->profit_factor);
  fprintf(out, ")\n");
  fprintf(out, "\n");

  fprintf(out, "=== Trade Statistics\n");
  fprintf(out, "\n");
  fprintf(out, "#table(\n");
  fprintf(out, "  columns: (1fr, 1fr),\n");
  fprintf(out, "  inset: 8pt,\n");
  fprintf(out, "  fill: (x, y) => if y == 0 { luma(230) },\n");
  fprintf(out, "  [*Metric*], [*Value*],\n");
  fprintf(out, "  [Total Trades], [%d],\n", result->total_trades);
  fprintf(out, "  [Winning Trades], [%d],\n", result->winning_trades);
  fprintf(out, "  [Losing Trades], [%d],\n", result->losing_trades);
  fprintf(out, "  [Win Rate], [%.1f%%],\n", result->win_rate * 100.0);
  fprintf(out, "  [Average Win], [$%.2f],\n", result->average_win);
  fprintf(out, "  [Average Loss], [$%.2f],\n", result->average_loss);
  fprintf(out, "  [Largest Win], [$%.2f],\n", result->largest_win);
  fprintf(out, "  [Largest Loss], [$%.2f],\n", result->largest_loss);
  fprintf(out, "  [Avg Trade Duration], [%.1f days],\n", result->average_trade_duration);
  fprintf(out, ")\n");
  fprintf(out, "\n");
}

/**
 * @brief Write the default report (no custom template).
 */
static bool write_default_report(SamtraderBacktestResult *result, SamtraderStrategy *strategy,
                                 const char *output_path) {
  FILE *out = fopen(output_path, "w");
  if (out == NULL) {
    return false;
  }

  const char *name = strategy->name ? strategy->name : "Unnamed Strategy";

  write_preamble(out, name);
  write_title(out, name);
  write_strategy_summary(out, strategy);
  write_strategy_parameters(out, strategy);
  write_performance_metrics(out, result);

  fclose(out);
  return true;
}

/* ============================================================================
 * Port Interface Implementations
 * ============================================================================ */

static bool typst_report_write(SamtraderReportPort *port, SamtraderBacktestResult *result,
                               SamtraderStrategy *strategy, const char *output_path) {
  if (port == NULL || port->impl == NULL || result == NULL || strategy == NULL ||
      output_path == NULL) {
    return false;
  }

  TypstReportImpl *impl = (TypstReportImpl *)port->impl;

  if (impl->template_path != NULL) {
    return write_template_report(impl->template_path, result, strategy, output_path);
  }

  return write_default_report(result, strategy, output_path);
}

static void typst_report_close(SamtraderReportPort *port) {
  if (port == NULL) {
    return;
  }

  /* All memory is arena-allocated, nothing to free manually */
  (void)port;
}

/* ============================================================================
 * Public API
 * ============================================================================ */

SamtraderReportPort *samtrader_typst_adapter_create(Samrena *arena, const char *template_path) {
  if (arena == NULL) {
    return NULL;
  }

  /* Allocate port structure */
  SamtraderReportPort *port = samrena_push(arena, sizeof(SamtraderReportPort));
  if (port == NULL) {
    return NULL;
  }

  /* Allocate implementation data */
  TypstReportImpl *impl = samrena_push(arena, sizeof(TypstReportImpl));
  if (impl == NULL) {
    return NULL;
  }

  /* Copy template path to arena if provided */
  if (template_path != NULL) {
    size_t path_len = strlen(template_path) + 1;
    char *path_copy = samrena_push(arena, path_len);
    if (path_copy == NULL) {
      return NULL;
    }
    memcpy(path_copy, template_path, path_len);
    impl->template_path = path_copy;
  } else {
    impl->template_path = NULL;
  }

  /* Initialize port structure */
  port->impl = impl;
  port->arena = arena;
  port->write = typst_report_write;
  port->close = typst_report_close;

  return port;
}
