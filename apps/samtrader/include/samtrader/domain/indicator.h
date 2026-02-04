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

#ifndef SAMTRADER_DOMAIN_INDICATOR_H
#define SAMTRADER_DOMAIN_INDICATOR_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include <samrena.h>
#include <samvector.h>

/**
 * @brief Enumeration of supported technical indicator types.
 */
typedef enum {
    /* Trend Indicators */
    SAMTRADER_IND_SMA,              /**< Simple Moving Average */
    SAMTRADER_IND_EMA,              /**< Exponential Moving Average */
    SAMTRADER_IND_WMA,              /**< Weighted Moving Average */

    /* Momentum Indicators */
    SAMTRADER_IND_RSI,              /**< Relative Strength Index */
    SAMTRADER_IND_MACD,             /**< MACD Line */
    SAMTRADER_IND_MACD_SIGNAL,      /**< MACD Signal Line */
    SAMTRADER_IND_MACD_HISTOGRAM,   /**< MACD Histogram */
    SAMTRADER_IND_STOCHASTIC_K,     /**< Stochastic %K */
    SAMTRADER_IND_STOCHASTIC_D,     /**< Stochastic %D */
    SAMTRADER_IND_ROC,              /**< Rate of Change */

    /* Volatility Indicators */
    SAMTRADER_IND_BOLLINGER_UPPER,  /**< Bollinger Upper Band */
    SAMTRADER_IND_BOLLINGER_MIDDLE, /**< Bollinger Middle Band (SMA) */
    SAMTRADER_IND_BOLLINGER_LOWER,  /**< Bollinger Lower Band */
    SAMTRADER_IND_ATR,              /**< Average True Range */
    SAMTRADER_IND_STDDEV,           /**< Standard Deviation */

    /* Volume Indicators */
    SAMTRADER_IND_OBV,              /**< On-Balance Volume */
    SAMTRADER_IND_VWAP,             /**< Volume-Weighted Average Price */

    /* Support/Resistance */
    SAMTRADER_IND_PIVOT,            /**< Pivot Point */
    SAMTRADER_IND_PIVOT_R1,         /**< Resistance Level 1 */
    SAMTRADER_IND_PIVOT_R2,         /**< Resistance Level 2 */
    SAMTRADER_IND_PIVOT_R3,         /**< Resistance Level 3 */
    SAMTRADER_IND_PIVOT_S1,         /**< Support Level 1 */
    SAMTRADER_IND_PIVOT_S2,         /**< Support Level 2 */
    SAMTRADER_IND_PIVOT_S3          /**< Support Level 3 */
} SamtraderIndicatorType;

/**
 * @brief A single indicator value at a specific point in time.
 */
typedef struct {
    time_t date;    /**< Unix timestamp for this value */
    double value;   /**< The indicator value */
    bool valid;     /**< False during warmup period (insufficient data) */
} SamtraderIndicatorValue;

/**
 * @brief A time series of indicator values.
 */
typedef struct {
    SamtraderIndicatorType type;   /**< Type of indicator */
    int period;                    /**< Primary period parameter (e.g., 20 for SMA(20)) */
    int param2;                    /**< Secondary parameter (e.g., MACD slow period) */
    int param3;                    /**< Tertiary parameter (e.g., MACD signal period) */
    double param_double;           /**< Double parameter (e.g., Bollinger stddev multiplier) */
    SamrenaVector *values;         /**< Vector of SamtraderIndicatorValue */
} SamtraderIndicatorSeries;

/**
 * @brief Get a human-readable name for an indicator type.
 *
 * @param type The indicator type
 * @return Pointer to a static string with the indicator name
 */
const char *samtrader_indicator_type_name(SamtraderIndicatorType type);

/**
 * @brief Create an indicator series structure.
 *
 * @param arena Memory arena for allocation
 * @param type Indicator type
 * @param period Primary period parameter
 * @param initial_capacity Initial capacity for the values vector
 * @return Pointer to the created series, or NULL on failure
 */
SamtraderIndicatorSeries *samtrader_indicator_series_create(
    Samrena *arena, SamtraderIndicatorType type, int period,
    uint64_t initial_capacity);

/**
 * @brief Create an indicator series with multiple parameters.
 *
 * Used for indicators like MACD that require multiple period settings.
 *
 * @param arena Memory arena for allocation
 * @param type Indicator type
 * @param period Primary period
 * @param param2 Secondary parameter
 * @param param3 Tertiary parameter
 * @param param_double Double parameter (e.g., stddev multiplier)
 * @param initial_capacity Initial capacity for the values vector
 * @return Pointer to the created series, or NULL on failure
 */
SamtraderIndicatorSeries *samtrader_indicator_series_create_full(
    Samrena *arena, SamtraderIndicatorType type, int period, int param2,
    int param3, double param_double, uint64_t initial_capacity);

/**
 * @brief Add a value to an indicator series.
 *
 * @param series The indicator series
 * @param date Timestamp for the value
 * @param value The indicator value
 * @param valid Whether the value is valid (false during warmup)
 * @return Pointer to the added value, or NULL on failure
 */
SamtraderIndicatorValue *samtrader_indicator_series_add(
    SamtraderIndicatorSeries *series, time_t date, double value, bool valid);

/**
 * @brief Get an indicator value at a specific index.
 *
 * @param series The indicator series
 * @param index Index into the series (0 = oldest)
 * @return Pointer to the value, or NULL if index out of bounds
 */
const SamtraderIndicatorValue *samtrader_indicator_series_at(
    const SamtraderIndicatorSeries *series, size_t index);

/**
 * @brief Get the number of values in an indicator series.
 *
 * @param series The indicator series
 * @return Number of values, or 0 if series is NULL
 */
size_t samtrader_indicator_series_size(const SamtraderIndicatorSeries *series);

/**
 * @brief Get the latest valid value from an indicator series.
 *
 * @param series The indicator series
 * @param out_value Pointer to store the value (if found)
 * @return true if a valid value was found, false otherwise
 */
bool samtrader_indicator_series_latest(const SamtraderIndicatorSeries *series,
                                       double *out_value);

#endif /* SAMTRADER_DOMAIN_INDICATOR_H */
