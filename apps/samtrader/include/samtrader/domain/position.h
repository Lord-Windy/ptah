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

#ifndef SAMTRADER_DOMAIN_POSITION_H
#define SAMTRADER_DOMAIN_POSITION_H

#include <stdint.h>
#include <time.h>

/**
 * @brief Represents an open position in the portfolio.
 *
 * Quantity is signed: positive = long, negative = short.
 * Stop loss and take profit are 0 if not set.
 * All strings are arena-allocated and owned by the arena.
 */
typedef struct {
  const char *code;     /**< Stock symbol (e.g., "AAPL", "BHP") */
  const char *exchange; /**< Exchange identifier ("US", "AU") */
  int64_t quantity;     /**< Position size (positive = long, negative = short) */
  double entry_price;   /**< Average entry price */
  time_t entry_date;    /**< Date position was opened */
  double stop_loss;     /**< Stop loss price (0 if not set) */
  double take_profit;   /**< Take profit price (0 if not set) */
} SamtraderPosition;

#endif /* SAMTRADER_DOMAIN_POSITION_H */
