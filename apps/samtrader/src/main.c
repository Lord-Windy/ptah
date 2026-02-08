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

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* TRD Section 10.2 exit codes */
#define EXIT_GENERAL_ERROR 1
#define EXIT_CONFIG_ERROR 2
#define EXIT_DB_ERROR 3
#define EXIT_INVALID_STRATEGY 4
#define EXIT_INSUFFICIENT_DATA 5

typedef struct {
  const char *config_path;   /* -c / --config */
  const char *strategy_path; /* -s / --strategy */
  const char *output_path;   /* -o / --output */
  const char *exchange;      /* --exchange */
  const char *code;          /* --code */
} CliArgs;

typedef enum { CMD_BACKTEST, CMD_LIST_SYMBOLS, CMD_VALIDATE, CMD_INFO, CMD_HELP } Command;

static void print_usage(const char *prog) {
  fprintf(stderr,
          "Usage: %s <command> [options]\n"
          "\n"
          "samtrader - Algorithmic Trading Backtester\n"
          "\n"
          "Commands:\n"
          "  backtest       Run a backtest\n"
          "  list-symbols   List available symbols\n"
          "  validate       Validate a strategy file\n"
          "  info           Show data range for a symbol\n"
          "\n"
          "Options:\n"
          "  -c, --config <path>     Config file path (required for backtest)\n"
          "  -s, --strategy <path>   Strategy file path\n"
          "  -o, --output <path>     Output report path\n"
          "      --exchange <name>   Exchange name\n"
          "      --code <symbol>     Symbol code\n"
          "  -h, --help              Show this help message\n",
          prog);
}

static int parse_command(const char *arg) {
  if (strcmp(arg, "backtest") == 0)
    return CMD_BACKTEST;
  if (strcmp(arg, "list-symbols") == 0)
    return CMD_LIST_SYMBOLS;
  if (strcmp(arg, "validate") == 0)
    return CMD_VALIDATE;
  if (strcmp(arg, "info") == 0)
    return CMD_INFO;
  if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0)
    return CMD_HELP;
  return -1;
}

static int parse_args(int argc, char *argv[], Command *cmd, CliArgs *args) {
  static const struct option long_options[] = {{"config", required_argument, NULL, 'c'},
                                               {"strategy", required_argument, NULL, 's'},
                                               {"output", required_argument, NULL, 'o'},
                                               {"exchange", required_argument, NULL, 'E'},
                                               {"code", required_argument, NULL, 'C'},
                                               {"help", no_argument, NULL, 'h'},
                                               {NULL, 0, NULL, 0}};

  memset(args, 0, sizeof(*args));

  if (argc < 2) {
    print_usage(argv[0]);
    return EXIT_GENERAL_ERROR;
  }

  int parsed = parse_command(argv[1]);
  if (parsed < 0) {
    fprintf(stderr, "Error: unknown command '%s'\n\n", argv[1]);
    print_usage(argv[0]);
    return EXIT_GENERAL_ERROR;
  }
  *cmd = (Command)parsed;

  if (*cmd == CMD_HELP) {
    print_usage(argv[0]);
    return -1; /* signal: help printed, exit 0 */
  }

  /* Reset getopt and parse flags from argv[1] onward.
     We shift optind to 2 so getopt skips argv[0] and the subcommand. */
  optind = 2;
  int opt;
  while ((opt = getopt_long(argc, argv, "c:s:o:h", long_options, NULL)) != -1) {
    switch (opt) {
      case 'c':
        args->config_path = optarg;
        break;
      case 's':
        args->strategy_path = optarg;
        break;
      case 'o':
        args->output_path = optarg;
        break;
      case 'E':
        args->exchange = optarg;
        break;
      case 'C':
        args->code = optarg;
        break;
      case 'h':
        print_usage(argv[0]);
        return -1;
      default:
        print_usage(argv[0]);
        return EXIT_GENERAL_ERROR;
    }
  }

  return 0;
}

static int validate_args(Command cmd, const CliArgs *args) {
  switch (cmd) {
    case CMD_BACKTEST:
      if (!args->config_path) {
        fprintf(stderr, "Error: backtest requires -c/--config\n");
        return EXIT_CONFIG_ERROR;
      }
      break;
    case CMD_LIST_SYMBOLS:
      if (!args->exchange) {
        fprintf(stderr, "Error: list-symbols requires --exchange\n");
        return EXIT_GENERAL_ERROR;
      }
      break;
    case CMD_VALIDATE:
      if (!args->strategy_path) {
        fprintf(stderr, "Error: validate requires -s/--strategy\n");
        return EXIT_INVALID_STRATEGY;
      }
      break;
    case CMD_INFO:
      if (!args->code) {
        fprintf(stderr, "Error: info requires --code\n");
        return EXIT_GENERAL_ERROR;
      }
      if (!args->exchange) {
        fprintf(stderr, "Error: info requires --exchange\n");
        return EXIT_GENERAL_ERROR;
      }
      break;
    case CMD_HELP:
      break;
  }
  return 0;
}

static int cmd_backtest(const CliArgs *args) {
  printf("Backtest: config=%s", args->config_path);
  if (args->strategy_path)
    printf(", strategy=%s", args->strategy_path);
  if (args->output_path)
    printf(", output=%s", args->output_path);
  printf("\n");
  return EXIT_SUCCESS;
}

static int cmd_list_symbols(const CliArgs *args) {
  printf("List symbols: exchange=%s\n", args->exchange);
  return EXIT_SUCCESS;
}

static int cmd_validate(const CliArgs *args) {
  printf("Validate: strategy=%s\n", args->strategy_path);
  return EXIT_SUCCESS;
}

static int cmd_info(const CliArgs *args) {
  printf("Info: code=%s, exchange=%s\n", args->code, args->exchange);
  return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
  Command cmd;
  CliArgs args;

  int rc = parse_args(argc, argv, &cmd, &args);
  if (rc == -1)
    return EXIT_SUCCESS; /* --help */
  if (rc != 0)
    return rc;

  rc = validate_args(cmd, &args);
  if (rc != 0)
    return rc;

  switch (cmd) {
    case CMD_BACKTEST:
      return cmd_backtest(&args);
    case CMD_LIST_SYMBOLS:
      return cmd_list_symbols(&args);
    case CMD_VALIDATE:
      return cmd_validate(&args);
    case CMD_INFO:
      return cmd_info(&args);
    case CMD_HELP:
      return EXIT_SUCCESS;
  }

  return EXIT_GENERAL_ERROR;
}
