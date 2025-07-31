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

ptah_add_library(datazoo
    SOURCES
        src/starfish.c
        src/honeycomb.c
        src/pearl.c
    PUBLIC_HEADERS
        include/datazoo.h
    DEPENDENCIES
        samrena
)

# Build tests if testing is enabled
if(BUILD_TESTING)
    enable_testing()
    
    add_executable(datazoo_test
        test/datazoo_test.c
        test/honeycomb_tests.c
    )
    target_link_libraries(datazoo_test PRIVATE datazoo samrena)
    add_test(NAME datazoo_test COMMAND datazoo_test)
endif()