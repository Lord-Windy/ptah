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

#ifndef SAMMATH_PHYSICS_H
#define SAMMATH_PHYSICS_H

#include <stdint.h>

#include "vector.h"

typedef struct {
  SamVector3d position;
  SamVector3d velocity;
  SamVector3d acceleration;
  double mass;
} SamPhysicsBody;

typedef struct {
  SamPhysicsBody* bodies;
  uint64_t bodies_count;
  double time;
  double G; // gravitational constant
} SamPhysicsSystem;

typedef struct {
  double h; //timestep
  double h_last;
  double error_b5;
  double error_last;

  // Gauss-Radau nodes and weights
  double nodes[8];
  double weights[8];

  // Predictor-corrector arrays (per body)
  SamVector3d* B;  // B[stage][body_index]
  SamVector3d* G;  // G[stage][body_index]
  SamVector3d* E;  // Error estimates

  // Compensated summation for high precision
  SamVector3d* cs_pos;  // position compensation
  SamVector3d* cs_vel;  // velocity compensation  
 
} SamIAS15State;

#endif //SAMMATH_PHYSICS_H


