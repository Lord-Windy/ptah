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

#include "3d/physics.h"

// 3. IAS15 initialization
void samphysics_ias15_init_nodes_weights(SamIAS15State *state) {
  // Gauss-Radau nodes (h-values where we evaluate accelerations)
  // These are roots of a specific Legendre polynomial
  state->nodes[0] = 0.0;
  state->nodes[1] = 0.05626256053692215;
  state->nodes[2] = 0.18024069173706659;
  state->nodes[3] = 0.35262471711316964;
  state->nodes[4] = 0.54715362633055538;
  state->nodes[5] = 0.73421017721541053;
  state->nodes[6] = 0.88532094683909577;
  state->nodes[7] = 0.97752061356128750;

  // Weights for Gauss-Radau quadrature
  // These determine how much each evaluation contributes
  state->weights[0] = 0.03125; // 1/32
  state->weights[1] = 0.18535724066864462;
  state->weights[2] = 0.30453357106518506;
  state->weights[3] = 0.37695308340449744;
  state->weights[4] = 0.39132282678815924;
  state->weights[5] = 0.34700768414597336;
  state->weights[6] = 0.24924320445092235;
  state->weights[7] = 0.11462852679651851;

  // Initial timestep (will be adapted)
  state->h = 0.01; // Start conservative
  state->h_last = 0.01;
  state->error_b5 = 1e-10; // Initial error estimate
  state->error_last = 1e-10;
}

void samphysics_ias15_allocate_arrays(SamIAS15State *state, Samrena *arena, int n_bodies) {

  state->B =
      SAMRENA_PUSH_ARRAY_ZERO(arena, SamVector3d, 7 * n_bodies); // 7 arrays for each n_bodies
  state->G = SAMRENA_PUSH_ARRAY_ZERO(arena, SamVector3d, 7 * n_bodies);
  state->E = SAMRENA_PUSH_ARRAY_ZERO(arena, SamVector3d, n_bodies);

  state->cs_pos = SAMRENA_PUSH_ARRAY_ZERO(arena, SamVector3d, n_bodies);
  state->cs_vel = SAMRENA_PUSH_ARRAY_ZERO(arena, SamVector3d, n_bodies);
}

// 4. IAS15 core algorithm
void samphysics_ias15_predict_B_values(SamIAS15State *state, SamPhysicsSystem *sys) {

  double h_ratio = state->h / state->h_last;

  for (uint64_t body = 0; body < sys->bodies_count; body++) {
    // First step or after a rejected step: zero prediction
    if (state->h_last == 0.0 || h_ratio > 2.0 || h_ratio < 0.5) {
      for (int i = 0; i < 7; i++) {
        state->B[i * sys->bodies_count + body] = (SamVector3d){0, 0, 0};
      }
    } else {
      // Predict B values using Gauss-Radau quadrature
      double h_ration_n = h_ratio;
      for (int i = 0; i < 7; i++) {
        state->B[i * sys->bodies_count + body].x *= h_ration_n;
        state->B[i * sys->bodies_count + body].y *= h_ration_n;
        state->B[i * sys->bodies_count + body].z *= h_ration_n;
        h_ration_n *= h_ratio; // Next power of h_ratio
      }
    }
  }
}

void samphysics_ias15_update_G_from_B(SamIAS15State *state, SamPhysicsSystem *sys) {
  // Transformation matrix from B to G (precomputed for IAS15)
  // These come from the mathematical derivation of Gauss-Radau
  static const double c[7][7] = {
      {-0.0562625605369221, 0.0101408028300636, -0.0036547780859120, 0.0023647894439182,
       -0.0018570164693494, 0.0016209752478099, -0.0015211303600147},
      {0.0562625605369221, 0.0885791904665270, 0.0192151928158083, -0.0074624778360019,
       0.0039602412824051, -0.0026895240508051, 0.0022444497498254},
      {0.0, 0.1885791904665270, 0.0919576730967419, 0.0205804081487128, -0.0069930402063490,
       0.0032739351595440, -0.0020655790538141},
      {0.0, 0.0, 0.2551956730967419, 0.1311997881664594, 0.0379809442922142, -0.0103070843576332,
       0.0040779842524935},
      {0.0, 0.0, 0.0, 0.3757940114994557, 0.2088792771166580, 0.0715455469151816,
       -0.0184135511127340},
      {0.0, 0.0, 0.0, 0.0, 0.5847318077879892, 0.3520139156198372, 0.1395792497270812},
      {0.0, 0.0, 0.0, 0.0, 0.0, 0.8653207530163325, 0.5711734340884451}};

  for (uint64_t body = 0; body < sys->bodies_count; body++) {
    for (int i = 0; i < 7; i++) {
      state->G[i * sys->bodies_count + body] = (SamVector3d){0, 0, 0};

      for (int j = 0; j < 7; j++) {
        state->G[i * sys->bodies_count + body].x +=
            c[i][j] * state->B[j * sys->bodies_count + body].x;
        state->G[i * sys->bodies_count + body].y +=
            c[i][j] * state->B[j * sys->bodies_count + body].y;
        state->G[i * sys->bodies_count + body].z +=
            c[i][j] * state->B[j * sys->bodies_count + body].z;
      }
    }
  }
}

void samphysics_ias15_evaluate_F(SamIAS15State *state, SamPhysicsSystem *sys, int stage) {

  SamPhysicsBody temp_bodies[sys->bodies_count];

  double tau = state->h * state->nodes[stage];

  for (uint64_t body = 0; body < sys->bodies_count; body++) {
    temp_bodies[body].position = sys->bodies[body].position;
    temp_bodies[body].velocity = sys->bodies[body].velocity;
    temp_bodies[body].mass = sys->bodies[body].mass;

    // Add v0*tau
    temp_bodies[body].position.x += sys->bodies[body].velocity.x * tau;
    temp_bodies[body].position.y += sys->bodies[body].velocity.y * tau;
    temp_bodies[body].position.z += sys->bodies[body].velocity.z * tau;
  }

  SamPhysicsBody *original_bodies = sys->bodies;
  sys->bodies = temp_bodies;

  samphysics_system_calculate_accelerations(sys);

  sys->bodies = original_bodies;
}

void samphysics_ias15_correct_B_and_G(SamIAS15State *state, SamPhysicsSystem *sys) {}

double samphysics_ias15_estimate_error(SamIAS15State *state, SamPhysicsSystem *sys) {}

void samphysics_ias15_step_accept(SamIAS15State *state, SamPhysicsSystem *sys) {}

double samphysics_ias15_compute_new_timestep(SamIAS15State *state) {}