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
endif()
