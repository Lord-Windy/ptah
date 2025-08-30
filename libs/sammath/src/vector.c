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

#include "3d/vector.h"
#include <math.h>

SamVector3d samvector3d_add(SamVector3d a, SamVector3d b) {
    return (SamVector3d){a.x + b.x, a.y + b.y, a.z + b.z};
}

SamVector3d samvector3d_subtract(SamVector3d a, SamVector3d b) {
    return (SamVector3d){a.x - b.x, a.y - b.y, a.z - b.z};
}

SamVector3d samvector3d_scale(SamVector3d a, double scale) {
    return (SamVector3d){a.x * scale, a.y * scale, a.z * scale};
}

SamVector3d samvector3d_cross(SamVector3d a, SamVector3d b) {
  SamVector3d result;
  result.x = a.y * b.z - a.z * b.y;
  result.y = a.z * b.x - a.x * b.z;
  result.z = a.x * b.y - a.y * b.x;
  return result;
}

double samvector3d_dot(SamVector3d a, SamVector3d b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

double samvector3d_magnitude(SamVector3d v) {
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}