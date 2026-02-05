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

#include "vec3.h"

#include <math.h>

Vec3 vec3_add(Vec3 a, Vec3 b) { return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z}; }

Vec3 vec3_sub(Vec3 a, Vec3 b) { return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z}; }

Vec3 vec3_mul(Vec3 a, double b) { return (Vec3){a.x * b, a.y * b, a.z * b}; }

Vec3 vec3_div(Vec3 a, double b) { return (Vec3){a.x / b, a.y / b, a.z / b}; }

double vec3_dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

Vec3 vec3_cross(Vec3 a, Vec3 b) {
  return (Vec3){
      a.y * b.z - a.z * b.y,
      a.z * b.x - a.x * b.z,
      a.x * b.y - a.y * b.x,
  };
}

Vec3 vec3_unit(Vec3 a) {
  double len = vec3_length(a);
  return vec3_div(a, len);
}

double vec3_distance(Vec3 a, Vec3 b) { return vec3_length(vec3_sub(a, b)); }

double vec3_length(Vec3 a) { return sqrt(vec3_squared_length(a)); }

double vec3_squared_length(Vec3 a) { return a.x * a.x + a.y * a.y + a.z * a.z; }
