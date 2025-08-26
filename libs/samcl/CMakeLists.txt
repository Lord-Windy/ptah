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

ptah_add_library(samcl
    SOURCES
        src/samcl.c
    PUBLIC_HEADERS
        include/samcl.h
)

if(BUILD_TESTING)
    ptah_add_executable(samcl_test
        SOURCES
            test/samcl_test.c
        DEPENDENCIES
            samcl
    )
    add_test(NAME samcl_basic COMMAND samcl_test)
endif()