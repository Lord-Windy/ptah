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

#include <samrena.h>

#include "vector.h"

#define SAMMATH_PHYSICS_EPSILON 1e-10

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

void samphysics_system_calculate_accelerations(SamPhysicsSystem* sys);
double samphysics_system_calculate_total_energy(SamPhysicsSystem* sys);
SamVector3d samphysics_system_calculate_total_angular_momentum(SamPhysicsSystem* sys);
double samphysics_system_calculate_angular_momentum_magnitude(SamPhysicsSystem* sys);
SamVector3d samphysics_system_calculate_center_of_mass(SamPhysicsSystem* sys);
SamVector3d samphysics_system_calculate_center_of_mass_velocity(SamPhysicsSystem* sys);

typedef struct {
  double h; //timestep
  double h_last;
  double error_b5;
  double error_last;

  // Gauss-Radau nodes and weights
  double nodes[8];
  double weights[8];

  // Predictor-corrector arrays (per body)
  // B array: 7 coefficients per body (B₁ through B₇)
  // These represent the polynomial interpolation of acceleration
  SamVector3d* B;  // B[stage][body_index]
  // G array: 7 Gauss-Radau stage values per body
  // These are linear combinations of B values
  SamVector3d* G;  // G[stage][body_index]
  SamVector3d* E;  // Error estimates

  // Compensated summation arrays for position and velocity
  // These track numerical error to maintain precision
  SamVector3d* cs_pos;  // position compensation
  SamVector3d* cs_vel;  // velocity compensation  
 
} SamIAS15State;

// 3. IAS15 initialization
void samphysics_ias15_init_nodes_weights(SamIAS15State* state);
void samphysics_ias15_allocate_arrays(SamIAS15State* state, Samrena* arena, int n_bodies);

// 4. IAS15 core algorithm
void samphysics_ias15_predict_B_values(SamIAS15State* state, SamPhysicsSystem* sys);
void samphysics_ias15_update_G_from_B(SamIAS15State* state, SamPhysicsSystem* sys);
void samphysics_ias15_evaluate_F(SamIAS15State* state, SamPhysicsSystem* sys, int stage);
void samphysics_ias15_correct_B_and_G(SamIAS15State* state, SamPhysicsSystem* sys);
double samphysics_ias15_estimate_error(SamIAS15State* state, SamPhysicsSystem* sys);
void samphysics_ias15_step_accept(SamIAS15State* state, SamPhysicsSystem* sys);
double samphysics_ias15_compute_new_timestep(SamIAS15State* state);

#endif //SAMMATH_PHYSICS_H


