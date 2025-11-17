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

#include <cjson/cJSON.h>
#include <curl/curl.h>
#include <samdata.h>
#include <samrena.h>
#include <stdio.h>

int main(int argc, char **argv) {
  printf("samspacetrader starting...\n");

  Samrena *arena = samrena_create_default();
  if (!arena) {
    fprintf(stderr, "Failed to create arena\n");
    return 1;
  }

  SamHashMap *map = samhashmap_create(16, arena);
  samhashmap_put(map, "test", "value");
  printf("Samdata test: %s\n", (char *)samhashmap_get(map, "test"));

  curl_global_init(CURL_GLOBAL_DEFAULT);
  CURL *curl = curl_easy_init();
  if (curl) {
    printf("CURL initialized\n");
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();

  cJSON *json = cJSON_CreateObject();
  cJSON_AddStringToObject(json, "test", "value");
  char *json_str = cJSON_Print(json);
  printf("cJSON test: %s\n", json_str);
  cJSON_free(json_str);
  cJSON_Delete(json);

  samrena_destroy(arena);
  printf("All libraries working!\n");
  return 0;
}