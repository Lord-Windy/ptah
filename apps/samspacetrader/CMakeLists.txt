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

find_package(CURL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(CJSON REQUIRED libcjson)

ptah_add_executable(samspacetrader
    SOURCES
        src/main.c
    DEPENDENCIES
        samrena
        samdata
        ${CURL_LIBRARIES}
        ${CJSON_LIBRARIES}
)

target_include_directories(samspacetrader PRIVATE
    ${CURL_INCLUDE_DIRS}
    ${CJSON_INCLUDE_DIRS}
)

target_link_directories(samspacetrader PRIVATE
    ${CJSON_LIBRARY_DIRS}
)