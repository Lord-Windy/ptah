# Copyright 2025 Samuel "Lord-Windy" Brown
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

ptah_add_executable(samtrader
    SOURCES
        src/main.c
        src/domain/ohlcv.c
        src/domain/indicator.c
        src/domain/indicator_sma.c
        src/domain/indicator_ema.c
        src/domain/indicator_wma.c
        src/domain/indicator_rsi.c
        src/domain/indicator_bollinger.c
        src/domain/indicator_macd.c
        src/domain/indicator_stochastic.c
        src/domain/indicator_atr.c
        src/domain/indicator_pivot.c
        src/adapters/file_config_adapter.c
        src/adapters/postgres_adapter.c
    DEPENDENCIES
        samrena
        samdata
        PostgreSQL::PostgreSQL
)

target_include_directories(samtrader PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Build tests if testing is enabled
if(BUILD_TESTING)
    enable_testing()

    # OHLCV data structure tests
    add_executable(samtrader_ohlcv_test
        test/test_ohlcv.c
        src/domain/ohlcv.c
        src/domain/indicator.c
        src/adapters/postgres_adapter.c
    )
    target_include_directories(samtrader_ohlcv_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(samtrader_ohlcv_test PRIVATE samrena samdata PostgreSQL::PostgreSQL m)
    add_test(NAME samtrader_ohlcv_test COMMAND samtrader_ohlcv_test)

    # Config adapter tests
    add_executable(samtrader_config_test
        test/test_config.c
        src/adapters/file_config_adapter.c
    )
    target_include_directories(samtrader_config_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(samtrader_config_test PRIVATE samrena samdata)
    add_test(NAME samtrader_config_test COMMAND samtrader_config_test)

    # Indicator data structure tests
    add_executable(samtrader_indicator_test
        test/test_indicator.c
        src/domain/indicator.c
        src/domain/indicator_sma.c
        src/domain/indicator_ema.c
        src/domain/indicator_wma.c
        src/domain/indicator_rsi.c
        src/domain/indicator_bollinger.c
        src/domain/indicator_macd.c
        src/domain/indicator_stochastic.c
        src/domain/indicator_atr.c
        src/domain/indicator_pivot.c
    )
    target_include_directories(samtrader_indicator_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(samtrader_indicator_test PRIVATE samrena samdata m)
    add_test(NAME samtrader_indicator_test COMMAND samtrader_indicator_test)

    # Indicator calculation tests (SMA, EMA, WMA, RSI, MACD, Stochastic, Bollinger, ATR, Pivot)
    add_executable(samtrader_indicator_calc_test
        test/test_indicator_calc.c
        src/domain/ohlcv.c
        src/domain/indicator.c
        src/domain/indicator_sma.c
        src/domain/indicator_ema.c
        src/domain/indicator_wma.c
        src/domain/indicator_rsi.c
        src/domain/indicator_bollinger.c
        src/domain/indicator_macd.c
        src/domain/indicator_stochastic.c
        src/domain/indicator_atr.c
        src/domain/indicator_pivot.c
    )
    target_include_directories(samtrader_indicator_calc_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(samtrader_indicator_calc_test PRIVATE samrena samdata m)
    add_test(NAME samtrader_indicator_calc_test COMMAND samtrader_indicator_calc_test)
endif()
