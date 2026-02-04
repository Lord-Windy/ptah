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

#include "samtrader/domain/indicator.h"

const char *samtrader_indicator_type_name(SamtraderIndicatorType type) {
    switch (type) {
    case SAMTRADER_IND_SMA:
        return "SMA";
    case SAMTRADER_IND_EMA:
        return "EMA";
    case SAMTRADER_IND_WMA:
        return "WMA";
    case SAMTRADER_IND_RSI:
        return "RSI";
    case SAMTRADER_IND_MACD:
        return "MACD";
    case SAMTRADER_IND_MACD_SIGNAL:
        return "MACD Signal";
    case SAMTRADER_IND_MACD_HISTOGRAM:
        return "MACD Histogram";
    case SAMTRADER_IND_STOCHASTIC_K:
        return "Stochastic %K";
    case SAMTRADER_IND_STOCHASTIC_D:
        return "Stochastic %D";
    case SAMTRADER_IND_ROC:
        return "ROC";
    case SAMTRADER_IND_BOLLINGER_UPPER:
        return "Bollinger Upper";
    case SAMTRADER_IND_BOLLINGER_MIDDLE:
        return "Bollinger Middle";
    case SAMTRADER_IND_BOLLINGER_LOWER:
        return "Bollinger Lower";
    case SAMTRADER_IND_ATR:
        return "ATR";
    case SAMTRADER_IND_STDDEV:
        return "StdDev";
    case SAMTRADER_IND_OBV:
        return "OBV";
    case SAMTRADER_IND_VWAP:
        return "VWAP";
    case SAMTRADER_IND_PIVOT:
        return "Pivot";
    case SAMTRADER_IND_PIVOT_R1:
        return "Pivot R1";
    case SAMTRADER_IND_PIVOT_R2:
        return "Pivot R2";
    case SAMTRADER_IND_PIVOT_R3:
        return "Pivot R3";
    case SAMTRADER_IND_PIVOT_S1:
        return "Pivot S1";
    case SAMTRADER_IND_PIVOT_S2:
        return "Pivot S2";
    case SAMTRADER_IND_PIVOT_S3:
        return "Pivot S3";
    default:
        return "Unknown";
    }
}

SamtraderIndicatorSeries *samtrader_indicator_series_create(
    Samrena *arena, SamtraderIndicatorType type, int period,
    uint64_t initial_capacity) {
    return samtrader_indicator_series_create_full(arena, type, period, 0, 0,
                                                  0.0, initial_capacity);
}

SamtraderIndicatorSeries *samtrader_indicator_series_create_full(
    Samrena *arena, SamtraderIndicatorType type, int period, int param2,
    int param3, double param_double, uint64_t initial_capacity) {
    if (!arena) {
        return NULL;
    }

    SamtraderIndicatorSeries *series =
        SAMRENA_PUSH_TYPE_ZERO(arena, SamtraderIndicatorSeries);
    if (!series) {
        return NULL;
    }

    series->values = samrena_vector_init(arena, sizeof(SamtraderIndicatorValue),
                                         initial_capacity);
    if (!series->values) {
        return NULL;
    }

    series->type = type;
    series->period = period;
    series->param2 = param2;
    series->param3 = param3;
    series->param_double = param_double;

    return series;
}

SamtraderIndicatorValue *samtrader_indicator_series_add(
    SamtraderIndicatorSeries *series, time_t date, double value, bool valid) {
    if (!series || !series->values) {
        return NULL;
    }

    SamtraderIndicatorValue val = {.date = date, .value = value, .valid = valid};

    return (SamtraderIndicatorValue *)samrena_vector_push(series->values, &val);
}

const SamtraderIndicatorValue *samtrader_indicator_series_at(
    const SamtraderIndicatorSeries *series, size_t index) {
    if (!series || !series->values) {
        return NULL;
    }

    return (const SamtraderIndicatorValue *)samrena_vector_at_const(
        series->values, index);
}

size_t samtrader_indicator_series_size(const SamtraderIndicatorSeries *series) {
    if (!series || !series->values) {
        return 0;
    }

    return samrena_vector_size(series->values);
}

bool samtrader_indicator_series_latest(const SamtraderIndicatorSeries *series,
                                       double *out_value) {
    if (!series || !series->values || !out_value) {
        return false;
    }

    size_t size = samrena_vector_size(series->values);
    if (size == 0) {
        return false;
    }

    for (size_t i = size; i > 0; i--) {
        const SamtraderIndicatorValue *val =
            (const SamtraderIndicatorValue *)samrena_vector_at_const(
                series->values, i - 1);
        if (val && val->valid) {
            *out_value = val->value;
            return true;
        }
    }

    return false;
}
