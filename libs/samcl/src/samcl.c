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
#include <string.h>

typedef struct SamCLInternal {
  int initialized;
} SamCLInternal;

SamCL *samcl_create(void) {
  SamCL *cl = malloc(sizeof(SamCL));
  if (!cl)
    return NULL;

  SamCLInternal *internal = malloc(sizeof(SamCLInternal));
  if (!internal) {
    free(cl);
    return NULL;
  }

  internal->initialized = 0;
  cl->internal = internal;

  return cl;
}

void samcl_destroy(SamCL *cl) {
  if (!cl)
    return;

  if (cl->internal) {
    free(cl->internal);
  }
  free(cl);
}

int samcl_init(SamCL *cl) {
  if (!cl || !cl->internal)
    return -1;

  SamCLInternal *internal = (SamCLInternal *)cl->internal;
  internal->initialized = 1;

  return 0;
}

int samcl_execute(SamCL *cl, const char *command) {
  if (!cl || !cl->internal || !command)
    return -1;

  SamCLInternal *internal = (SamCLInternal *)cl->internal;
  if (!internal->initialized)
    return -1;

  printf("SamCL executing: %s\n", command);

  return 0;
}