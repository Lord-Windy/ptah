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

#include "samcl.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  printf("SamCL Example Application\n");
  printf("=========================\n\n");

  SamCL *cl = samcl_create();
  if (!cl) {
    fprintf(stderr, "Failed to create SamCL instance\n");
    return 1;
  }

  printf("Created SamCL instance\n");

  int result = samcl_init(cl);
  if (result != 0) {
    fprintf(stderr, "Failed to initialize SamCL\n");
    samcl_destroy(cl);
    return 1;
  }

  printf("Initialized SamCL\n");

  const char *commands[] = {"kernel1", "kernel2", "kernel3"};

  for (int i = 0; i < 3; i++) {
    printf("Executing: %s\n", commands[i]);
    result = samcl_execute(cl, commands[i]);
    if (result != 0) {
      fprintf(stderr, "Failed to execute command: %s\n", commands[i]);
    }
  }

  samcl_destroy(cl);
  printf("\nSamCL destroyed successfully\n");

  return 0;
}