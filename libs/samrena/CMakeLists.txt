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

# Source files - virtual memory is always enabled
set(SAMRENA_SOURCES
    src/samrena.c
    src/samrena_vector.c
)

ptah_add_library(samrena
    SOURCES ${SAMRENA_SOURCES}
    PUBLIC_HEADERS
        include/samrena.h
        include/samvector.h
)

# Build tests if testing is enabled
if(BUILD_TESTING)
    enable_testing()
    
    # Find pthread for thread safety tests
    find_package(Threads REQUIRED)
    
    add_executable(samrena_test
        test/samrena_test.c
    )
    target_link_libraries(samrena_test PRIVATE samrena)
    add_test(NAME samrena_test COMMAND samrena_test)
    
    add_executable(samrena_vector_test
        test/samrena_vector_test.c
    )
    target_link_libraries(samrena_vector_test PRIVATE samrena)
    add_test(NAME samrena_vector_test COMMAND samrena_vector_test)
    
    add_executable(samrena_feature_test
        test/test_features.c
    )
    target_link_libraries(samrena_feature_test PRIVATE samrena)
    add_test(NAME samrena_feature_test COMMAND samrena_feature_test)
    
    # Comprehensive adapter test suite
    add_executable(samrena_adapter_test
        test/test_samrena_adapters.c
    )
    target_link_libraries(samrena_adapter_test PRIVATE samrena Threads::Threads)
    add_test(NAME samrena_adapter_test COMMAND samrena_adapter_test)

    # Arena types test (default, session, global)
    add_executable(samrena_arena_types_test
        test/test_arena_types.c
    )
    target_link_libraries(samrena_arena_types_test PRIVATE samrena)
    add_test(NAME samrena_arena_types_test COMMAND samrena_arena_types_test)

    # Comprehensive samvector test suite
    add_executable(samvector_lifecycle_test
        test/test_samvector_lifecycle.c
    )
    target_link_libraries(samvector_lifecycle_test PRIVATE samrena)
    add_test(NAME samvector_lifecycle_test COMMAND samvector_lifecycle_test)
    
    add_executable(samvector_access_test
        test/test_samvector_access.c
    )
    target_link_libraries(samvector_access_test PRIVATE samrena)
    add_test(NAME samvector_access_test COMMAND samvector_access_test)
    
    add_executable(samvector_capacity_test
        test/test_samvector_capacity.c
    )
    target_link_libraries(samvector_capacity_test PRIVATE samrena)
    add_test(NAME samvector_capacity_test COMMAND samvector_capacity_test)
    
    add_executable(samvector_operations_test
        test/test_samvector_operations.c
    )
    target_link_libraries(samvector_operations_test PRIVATE samrena)
    add_test(NAME samvector_operations_test COMMAND samvector_operations_test)
    
    add_executable(samvector_edge_cases_test
        test/test_samvector_edge_cases.c
    )
    target_link_libraries(samvector_edge_cases_test PRIVATE samrena)
    add_test(NAME samvector_edge_cases_test COMMAND samvector_edge_cases_test)
    
    add_executable(samvector_performance_test
        test/test_samvector_performance.c
    )
    target_link_libraries(samvector_performance_test PRIVATE samrena)
    add_test(NAME samvector_performance_test COMMAND samvector_performance_test)
    
    add_executable(samvector_typesafe_test
        test/test_samvector_typesafe.c
    )
    target_link_libraries(samvector_typesafe_test PRIVATE samrena)
    add_test(NAME samvector_typesafe_test COMMAND samvector_typesafe_test)
    
    
    # Valgrind integration - only add tests if explicitly requested
    option(ENABLE_VALGRIND_TESTS "Enable Valgrind memory leak tests (may fail on systems with AVX-512)" OFF)
    
    if(ENABLE_VALGRIND_TESTS)
        find_program(VALGRIND_PROGRAM valgrind)
        if(VALGRIND_PROGRAM)
            # Standard valgrind tests
            add_test(NAME samrena_test_valgrind 
                     COMMAND ${VALGRIND_PROGRAM} 
                             --tool=memcheck 
                             --leak-check=full 
                             --show-leak-kinds=all 
                             --track-origins=yes 
                             --error-exitcode=1
                             --suppressions=${CMAKE_SOURCE_DIR}/valgrind_suppressions.supp
                             $<TARGET_FILE:samrena_test>)
            
            add_test(NAME samrena_vector_test_valgrind 
                     COMMAND ${VALGRIND_PROGRAM} 
                             --tool=memcheck 
                             --leak-check=full 
                             --show-leak-kinds=all 
                             --track-origins=yes 
                             --error-exitcode=1
                             --suppressions=${CMAKE_SOURCE_DIR}/valgrind_suppressions.supp
                             $<TARGET_FILE:samrena_vector_test>)
            
            add_test(NAME samrena_feature_test_valgrind 
                     COMMAND ${VALGRIND_PROGRAM} 
                             --tool=memcheck 
                             --leak-check=full 
                             --show-leak-kinds=all 
                             --track-origins=yes 
                             --error-exitcode=1
                             --suppressions=${CMAKE_SOURCE_DIR}/valgrind_suppressions.supp
                             $<TARGET_FILE:samrena_feature_test>)
            
            add_test(NAME samrena_adapter_test_valgrind
                     COMMAND ${VALGRIND_PROGRAM}
                             --tool=memcheck
                             --leak-check=full
                             --show-leak-kinds=all
                             --track-origins=yes
                             --error-exitcode=1
                             --suppressions=${CMAKE_SOURCE_DIR}/valgrind_suppressions.supp
                             $<TARGET_FILE:samrena_adapter_test>)

            add_test(NAME samrena_arena_types_test_valgrind
                     COMMAND ${VALGRIND_PROGRAM}
                             --tool=memcheck
                             --leak-check=full
                             --show-leak-kinds=all
                             --track-origins=yes
                             --error-exitcode=1
                             --suppressions=${CMAKE_SOURCE_DIR}/valgrind_suppressions.supp
                             $<TARGET_FILE:samrena_arena_types_test>)

            # Valgrind tests for comprehensive samvector test suite
            add_test(NAME samvector_lifecycle_test_valgrind 
                     COMMAND ${VALGRIND_PROGRAM} 
                             --tool=memcheck 
                             --leak-check=full 
                             --show-leak-kinds=all 
                             --track-origins=yes 
                             --error-exitcode=1
                             --suppressions=${CMAKE_SOURCE_DIR}/valgrind_suppressions.supp
                             $<TARGET_FILE:samvector_lifecycle_test>)
            
            add_test(NAME samvector_access_test_valgrind 
                     COMMAND ${VALGRIND_PROGRAM} 
                             --tool=memcheck 
                             --leak-check=full 
                             --show-leak-kinds=all 
                             --track-origins=yes 
                             --error-exitcode=1
                             --suppressions=${CMAKE_SOURCE_DIR}/valgrind_suppressions.supp
                             $<TARGET_FILE:samvector_access_test>)
            
            add_test(NAME samvector_capacity_test_valgrind 
                     COMMAND ${VALGRIND_PROGRAM} 
                             --tool=memcheck 
                             --leak-check=full 
                             --show-leak-kinds=all 
                             --track-origins=yes 
                             --error-exitcode=1
                             --suppressions=${CMAKE_SOURCE_DIR}/valgrind_suppressions.supp
                             $<TARGET_FILE:samvector_capacity_test>)
            
            add_test(NAME samvector_operations_test_valgrind 
                     COMMAND ${VALGRIND_PROGRAM} 
                             --tool=memcheck 
                             --leak-check=full 
                             --show-leak-kinds=all 
                             --track-origins=yes 
                             --error-exitcode=1
                             --suppressions=${CMAKE_SOURCE_DIR}/valgrind_suppressions.supp
                             $<TARGET_FILE:samvector_operations_test>)
            
            add_test(NAME samvector_edge_cases_test_valgrind 
                     COMMAND ${VALGRIND_PROGRAM} 
                             --tool=memcheck 
                             --leak-check=full 
                             --show-leak-kinds=all 
                             --track-origins=yes 
                             --error-exitcode=1
                             --suppressions=${CMAKE_SOURCE_DIR}/valgrind_suppressions.supp
                             $<TARGET_FILE:samvector_edge_cases_test>)
            
            message(STATUS "Samrena: Valgrind tests enabled (use -DENABLE_VALGRIND_TESTS=OFF to disable)")
        else()
            message(WARNING "Samrena: Valgrind requested but not found")
        endif()
    else()
        message(STATUS "Samrena: Valgrind tests disabled (use -DENABLE_VALGRIND_TESTS=ON to enable)")
    endif()
endif()