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

#ifndef SAMTRADER_DOMAIN_RULE_H
#define SAMTRADER_DOMAIN_RULE_H

#include <stdbool.h>
#include <stddef.h>

#include <samdata/samhashmap.h>
#include <samrena.h>
#include <samvector.h>

#include "samtrader/domain/indicator.h"

/*============================================================================
 * Rule Type Enumeration
 *============================================================================*/

/**
 * @brief Enumeration of trading rule types.
 *
 * Comparison rules compare two operands:
 *   - CROSS_ABOVE/CROSS_BELOW: Crossover detection (requires current + previous bar)
 *   - ABOVE/BELOW/EQUALS: Simple value comparison
 *   - BETWEEN: Range check (left operand between right operand and threshold)
 *
 * Composite rules combine child rules:
 *   - AND/OR: Logical combination (uses children array)
 *   - NOT: Logical negation (uses single child)
 *
 * Temporal rules add time constraints:
 *   - CONSECUTIVE: Child rule must hold for N consecutive bars
 *   - ANY_OF: Child rule must hold at least once in the last N bars
 */
typedef enum {
  SAMTRADER_RULE_CROSS_ABOVE, /**< Left crosses above right */
  SAMTRADER_RULE_CROSS_BELOW, /**< Left crosses below right */
  SAMTRADER_RULE_ABOVE,       /**< Left > right */
  SAMTRADER_RULE_BELOW,       /**< Left < right */
  SAMTRADER_RULE_BETWEEN,     /**< Right <= left <= threshold */
  SAMTRADER_RULE_EQUALS,      /**< Left == right (within tolerance) */
  SAMTRADER_RULE_AND,         /**< All children must be true */
  SAMTRADER_RULE_OR,          /**< At least one child must be true */
  SAMTRADER_RULE_NOT,         /**< Negation of child */
  SAMTRADER_RULE_CONSECUTIVE, /**< Child true for N consecutive bars */
  SAMTRADER_RULE_ANY_OF       /**< Child true at least once in last N bars */
} SamtraderRuleType;

/*============================================================================
 * Operand Structures
 *============================================================================*/

/**
 * @brief Enumeration of operand types for rule evaluation.
 *
 * Operands represent the values being compared in a rule:
 *   - Price fields: Direct access to OHLCV bar data
 *   - Indicator: Value from a pre-calculated indicator series
 *   - Constant: A literal numeric value
 */
typedef enum {
  SAMTRADER_OPERAND_PRICE_OPEN,  /**< Open price from OHLCV bar */
  SAMTRADER_OPERAND_PRICE_HIGH,  /**< High price from OHLCV bar */
  SAMTRADER_OPERAND_PRICE_LOW,   /**< Low price from OHLCV bar */
  SAMTRADER_OPERAND_PRICE_CLOSE, /**< Close price from OHLCV bar */
  SAMTRADER_OPERAND_VOLUME,      /**< Volume from OHLCV bar */
  SAMTRADER_OPERAND_INDICATOR,   /**< Value from an indicator series */
  SAMTRADER_OPERAND_CONSTANT     /**< Literal numeric constant */
} SamtraderOperandType;

/**
 * @brief An operand in a trading rule.
 *
 * Represents one side of a comparison. Access the appropriate union member
 * based on the type field:
 *   - CONSTANT -> constant
 *   - INDICATOR -> indicator.indicator_type, indicator.period, etc.
 *   - PRICE_* / VOLUME -> no union member needed (type is sufficient)
 */
typedef struct SamtraderOperand {
  SamtraderOperandType type;
  union {
    double constant; /**< Value for SAMTRADER_OPERAND_CONSTANT */
    struct {
      SamtraderIndicatorType indicator_type; /**< Which indicator */
      int period;                            /**< Primary period (e.g., 20 for SMA(20)) */
      int param2;                            /**< Second param (e.g., MACD slow period) */
      int param3;                            /**< Third param (e.g., MACD signal period) */
    } indicator; /**< Indicator reference for SAMTRADER_OPERAND_INDICATOR */
  };
} SamtraderOperand;

/*============================================================================
 * Rule Structure
 *============================================================================*/

/**
 * @brief A trading rule node in the rule AST.
 *
 * Rules form a tree structure:
 *   - Comparison rules (CROSS_ABOVE, ABOVE, BELOW, BETWEEN, EQUALS):
 *     Use left and right operands. BETWEEN also uses threshold.
 *   - Composite rules (AND, OR):
 *     Use children (NULL-terminated array of rule pointers).
 *   - NOT rule:
 *     Uses single child pointer.
 *   - Temporal rules (CONSECUTIVE, ANY_OF):
 *     Use single child pointer and lookback period.
 *
 * All memory is arena-allocated; no individual free() calls needed.
 */
typedef struct SamtraderRule {
  SamtraderRuleType type;

  SamtraderOperand left;  /**< Left operand (comparison rules) */
  SamtraderOperand right; /**< Right operand (comparison rules) */
  double threshold;       /**< Upper bound for BETWEEN rule */
  int lookback;           /**< Lookback period for CONSECUTIVE, ANY_OF */

  struct SamtraderRule **children; /**< NULL-terminated array for AND, OR */
  struct SamtraderRule *child;     /**< Single child for NOT, CONSECUTIVE, ANY_OF */
} SamtraderRule;

/*============================================================================
 * Rule Construction API
 *============================================================================*/

/**
 * @brief Create a comparison rule with two operands.
 *
 * Valid types: CROSS_ABOVE, CROSS_BELOW, ABOVE, BELOW, EQUALS.
 *
 * @param arena Memory arena for allocation
 * @param type Rule type (must be a comparison type)
 * @param left Left operand
 * @param right Right operand
 * @return Pointer to the created rule, or NULL on failure
 */
SamtraderRule *samtrader_rule_create_comparison(Samrena *arena, SamtraderRuleType type,
                                                SamtraderOperand left, SamtraderOperand right);

/**
 * @brief Create a BETWEEN range rule.
 *
 * Evaluates true when: right <= left <= threshold
 *
 * @param arena Memory arena for allocation
 * @param left Operand to check (the value being tested)
 * @param lower Lower bound operand
 * @param upper Upper bound value (threshold)
 * @return Pointer to the created rule, or NULL on failure
 */
SamtraderRule *samtrader_rule_create_between(Samrena *arena, SamtraderOperand left,
                                             SamtraderOperand lower, double upper);

/**
 * @brief Create a composite rule (AND or OR) from child rules.
 *
 * @param arena Memory arena for allocation
 * @param type Rule type (SAMTRADER_RULE_AND or SAMTRADER_RULE_OR)
 * @param children Array of child rule pointers (will be copied)
 * @param count Number of children
 * @return Pointer to the created rule, or NULL on failure
 */
SamtraderRule *samtrader_rule_create_composite(Samrena *arena, SamtraderRuleType type,
                                               SamtraderRule **children, size_t count);

/**
 * @brief Create a NOT rule.
 *
 * @param arena Memory arena for allocation
 * @param child Rule to negate
 * @return Pointer to the created rule, or NULL on failure
 */
SamtraderRule *samtrader_rule_create_not(Samrena *arena, SamtraderRule *child);

/**
 * @brief Create a temporal rule (CONSECUTIVE or ANY_OF).
 *
 * - CONSECUTIVE: child must be true for `lookback` consecutive bars
 * - ANY_OF: child must be true at least once in the last `lookback` bars
 *
 * @param arena Memory arena for allocation
 * @param type Rule type (SAMTRADER_RULE_CONSECUTIVE or SAMTRADER_RULE_ANY_OF)
 * @param child The child rule to apply temporally
 * @param lookback Number of bars for the temporal window
 * @return Pointer to the created rule, or NULL on failure
 */
SamtraderRule *samtrader_rule_create_temporal(Samrena *arena, SamtraderRuleType type,
                                              SamtraderRule *child, int lookback);

/*============================================================================
 * Operand Construction Helpers
 *============================================================================*/

/**
 * @brief Create a constant operand.
 *
 * @param value The numeric constant
 * @return The operand (stack-allocated struct)
 */
SamtraderOperand samtrader_operand_constant(double value);

/**
 * @brief Create a price field operand.
 *
 * @param type The price field type (must be PRICE_OPEN, PRICE_HIGH,
 *             PRICE_LOW, PRICE_CLOSE, or VOLUME)
 * @return The operand (stack-allocated struct)
 */
SamtraderOperand samtrader_operand_price(SamtraderOperandType type);

/**
 * @brief Create a simple indicator operand (single period parameter).
 *
 * Use for: SMA, EMA, WMA, RSI, ROC, ATR, STDDEV, OBV, VWAP
 *
 * @param indicator_type The indicator type
 * @param period Primary period parameter
 * @return The operand (stack-allocated struct)
 */
SamtraderOperand samtrader_operand_indicator(SamtraderIndicatorType indicator_type, int period);

/**
 * @brief Create a multi-parameter indicator operand.
 *
 * Use for: MACD (fast, slow, signal), Stochastic (k, d), Bollinger (period, 0)
 *
 * @param indicator_type The indicator type
 * @param period Primary period parameter
 * @param param2 Secondary parameter
 * @param param3 Tertiary parameter (0 if unused)
 * @return The operand (stack-allocated struct)
 */
SamtraderOperand samtrader_operand_indicator_multi(SamtraderIndicatorType indicator_type,
                                                   int period, int param2, int param3);

/*============================================================================
 * Rule Information
 *============================================================================*/

/**
 * @brief Get a human-readable name for a rule type.
 *
 * @param type The rule type
 * @return Static string name (never NULL)
 */
const char *samtrader_rule_type_name(SamtraderRuleType type);

/**
 * @brief Get a human-readable name for an operand type.
 *
 * @param type The operand type
 * @return Static string name (never NULL)
 */
const char *samtrader_operand_type_name(SamtraderOperandType type);

/**
 * @brief Count the number of children in a composite rule.
 *
 * @param rule The rule (must be AND or OR type)
 * @return Number of children, or 0 if not a composite rule or NULL
 */
size_t samtrader_rule_child_count(const SamtraderRule *rule);

#endif /* SAMTRADER_DOMAIN_RULE_H */
