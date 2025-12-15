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

ptah_add_library(samdata
    INSTALL
    SOURCES
        src/samhash.c
        src/samhashmap.c
        src/samrng.c
        src/samset.c
    PUBLIC_HEADERS
        include/samdata.h
    DEPENDENCIES
        samrena
)

# Install the individual component headers in the samdata subdirectory
install(FILES
    include/samdata/samhash.h
    include/samdata/samhashmap.h
    include/samdata/samrng.h
    include/samdata/samset.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/samdata
)

# Build tests if testing is enabled
if(BUILD_TESTING)
    enable_testing()
    
    add_executable(samdata_test
        test/samdata_test.c
        test/samhashmap_tests.c
    )
    target_link_libraries(samdata_test PRIVATE samdata samrena)
    add_test(NAME samdata_test COMMAND samdata_test)
    
    # Samset test suite
    add_executable(samset_basic_test
        test/samset_basic_test.c
    )
    target_link_libraries(samset_basic_test PRIVATE samdata samrena)
    add_test(NAME samset_basic_test COMMAND samset_basic_test)
    
    add_executable(samset_collision_test
        test/samset_collision_test.c
    )
    target_link_libraries(samset_collision_test PRIVATE samdata samrena)
    add_test(NAME samset_collision_test COMMAND samset_collision_test)
    
    add_executable(samset_resize_test
        test/samset_resize_test.c
    )
    target_link_libraries(samset_resize_test PRIVATE samdata samrena)
    add_test(NAME samset_resize_test COMMAND samset_resize_test)
    
    add_executable(samset_performance_test
        test/samset_performance_test.c
    )
    target_link_libraries(samset_performance_test PRIVATE samdata samrena)
    add_test(NAME samset_performance_test COMMAND samset_performance_test)
    
    add_executable(samset_edge_case_test
        test/samset_edge_case_test.c
    )
    target_link_libraries(samset_edge_case_test PRIVATE samdata samrena)
    add_test(NAME samset_edge_case_test COMMAND samset_edge_case_test)
    
    add_executable(samset_typesafe_test
        test/samset_typesafe_test.c
    )
    target_link_libraries(samset_typesafe_test PRIVATE samdata samrena)
    add_test(NAME samset_typesafe_test COMMAND samset_typesafe_test)
    
    add_executable(samset_utility_test
        test/samset_utility_test.c
    )
    target_link_libraries(samset_utility_test PRIVATE samdata samrena)
    add_test(NAME samset_utility_test COMMAND samset_utility_test)
    
    add_executable(samset_functional_test
        test/samset_functional_test.c
    )
    target_link_libraries(samset_functional_test PRIVATE samdata samrena)
    add_test(NAME samset_functional_test COMMAND samset_functional_test)
    
    add_executable(samrng_test
        test/samrng_test.c
    )
    target_link_libraries(samrng_test PRIVATE samdata samrena m)
    add_test(NAME samrng_test COMMAND samrng_test)
endif()