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
    
    # Pearl test suite
    add_executable(pearl_basic_test
        test/pearl_basic_test.c
    )
    target_link_libraries(pearl_basic_test PRIVATE datazoo samrena)
    add_test(NAME pearl_basic_test COMMAND pearl_basic_test)
    
    add_executable(pearl_collision_test
        test/pearl_collision_test.c
    )
    target_link_libraries(pearl_collision_test PRIVATE datazoo samrena)
    add_test(NAME pearl_collision_test COMMAND pearl_collision_test)
    
    add_executable(pearl_resize_test
        test/pearl_resize_test.c
    )
    target_link_libraries(pearl_resize_test PRIVATE datazoo samrena)
    add_test(NAME pearl_resize_test COMMAND pearl_resize_test)
    
    add_executable(pearl_performance_test
        test/pearl_performance_test.c
    )
    target_link_libraries(pearl_performance_test PRIVATE datazoo samrena)
    add_test(NAME pearl_performance_test COMMAND pearl_performance_test)
    
    add_executable(pearl_edge_cases_test
        test/pearl_edge_cases_test.c
    )
    target_link_libraries(pearl_edge_cases_test PRIVATE datazoo samrena)
    add_test(NAME pearl_edge_cases_test COMMAND pearl_edge_cases_test)
    
    add_executable(pearl_typesafe_test
        test/pearl_typesafe_test.c
    )
    target_link_libraries(pearl_typesafe_test PRIVATE datazoo samrena)
    add_test(NAME pearl_typesafe_test COMMAND pearl_typesafe_test)
endif()