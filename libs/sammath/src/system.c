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

#include <math.h>

void samphysics_system_calculate_accelerations(SamPhysicsSystem* sys) {

  // Newton's law of gravitation: F = G·m₁·m₂/r² in the direction of r̂
  // reset accelerations of bodies
  for (uint64_t i = 0; i < sys->bodies_count; i++) {
    sys->bodies[i].acceleration = (SamVector3d){0, 0, 0};
  }

  // For each pair of bodies
  for (uint64_t i = 0; i < sys->bodies_count; i++) {
    for (uint64_t j = i + 1; j < sys->bodies_count; j++) {

      // Calculate separate vector from i to j
      double dx = sys->bodies[j].position.x - sys->bodies[i].position.x;
      double dy = sys->bodies[j].position.y - sys->bodies[i].position.y;
      double dz = sys->bodies[j].position.z - sys->bodies[i].position.z;

      // distance squared
      double r2 = dx * dx + dy * dy + dz * dz;

      // adding softening to avoid singularities
      // epsilon = 1e-10 for astronomical units
      double epsilon = SAMMATH_PHYSICS_EPSILON;
      r2 += epsilon * epsilon;

      // calculate 1/r^3 (since F ∝ 1/r^2, and we need direction r̂ = r/|r|)
      double r = sqrt(r2);
      double r3_inv = 1.0 / (r2 * r);

      // calculate the force magnitude G/r^3
      double force_factor = sys->G * r3_inv;

      // Apply equal and opposite accelerations
      // F_ij = G * m_j * (r_j - r_i) / |r|³
      // a_i = F_ij / m_i = G * m_j * (r_j - r_i) / |r|³
      sys->bodies[i].acceleration.x += force_factor * sys->bodies[j].mass * dx * r3_inv;
      sys->bodies[i].acceleration.y += force_factor * sys->bodies[j].mass * dy * r3_inv;
      sys->bodies[i].acceleration.z += force_factor * sys->bodies[j].mass * dz * r3_inv;

      sys->bodies[j].acceleration.x -= force_factor * sys->bodies[i].mass * dx * r3_inv;
      sys->bodies[j].acceleration.y -= force_factor * sys->bodies[i].mass * dy * r3_inv;
      sys->bodies[j].acceleration.z -= force_factor * sys->bodies[i].mass * dz * r3_inv;
    }
  }

}

/*
*What it does: Computes E = KE + PE, which should remain constant in an isolated system.
The physics:

Kinetic Energy: KE = ½mv²
Potential Energy: PE = -G·m₁·m₂/r (negative because gravity is attractive)

Why it matters: Energy drift tells you if your integration is accurate. For IAS15, expect machine precision conservation (~10⁻¹⁵).
 */
double samphysics_system_calculate_total_energy(SamPhysicsSystem* sys) {

  double kinetic_energy = 0.0;
  double potential_energy = 0.0;

  // Calculate kinetic energy: KE = Σ(½ m_i v_i²)
  for (uint64_t i = 0; i < sys->bodies_count; i++) {
    double vx = sys->bodies[i].velocity.x;
    double vy = sys->bodies[i].velocity.y;
    double vz = sys->bodies[i].velocity.z;
    double v2 = vx * vx + vy * vy + vz * vz;

    kinetic_energy += 0.5 * sys->bodies[i].mass * v2;
  }

  // Calculate potential energy: PE = -Σ(G m_i m_j / r_ij)
  // Sum over unique pairs only (i < j)
  for (uint64_t i = 0; i < sys->bodies_count; i++) {
    for (uint64_t j = i + 1; j < sys->bodies_count; j++) {
      double dx = sys->bodies[j].position.x - sys->bodies[i].position.x;
      double dy = sys->bodies[j].position.y - sys->bodies[i].position.y;
      double dz = sys->bodies[j].position.z - sys->bodies[i].position.z;

      double r2 = dx * dx + dy * dy + dz * dz;
      double r = sqrt(r2);

      double epsilon = SAMMATH_PHYSICS_EPSILON;
      r = sqrt(r*r + epsilon*epsilon);

      potential_energy -= sys->G * sys->bodies[i].mass * sys->bodies[j].mass / r;
    }
  }

  return kinetic_energy + potential_energy;
}

SamVector3d samphysics_system_calculate_total_angular_momentum(SamPhysicsSystem* sys) {
  SamVector3d total_angular_momentum = {0, 0, 0};

  for (uint64_t i = 0; i < sys->bodies_count; i++) {
    SamVector3d angular_momentum = {0, 0, 0};
    angular_momentum.x = sys->bodies[i].mass * sys->bodies[i].velocity.x;
    angular_momentum.y = sys->bodies[i].mass * sys->bodies[i].velocity.y;
    angular_momentum.z = sys->bodies[i].mass * sys->bodies[i].velocity.z;

    // Calculate L_i = r_i × p_i
    SamVector3d current_angular_momentum = samvector3d_cross(sys->bodies[i].position, angular_momentum);
    total_angular_momentum = samvector3d_add(total_angular_momentum, current_angular_momentum);
  }

  return total_angular_momentum;
}

double samphysics_system_calculate_angular_momentum_magnitude(SamPhysicsSystem* sys) {
  SamVector3d l = samphysics_system_calculate_total_angular_momentum(sys);
  return sqrt(l.x*l.x + l.y*l.y + l.z*l.z);
}

SamVector3d samphysics_system_calculate_center_of_mass(SamPhysicsSystem* sys) {
  SamVector3d center_of_mass = {0, 0, 0};
  double total_mass = 0.0;

  for (uint64_t i = 0; i < sys->bodies_count; i++) {
    center_of_mass = samvector3d_add(center_of_mass, sys->bodies[i].position);
    total_mass += sys->bodies[i].mass;
  }

  center_of_mass = samvector3d_scale(center_of_mass, 1.0 / total_mass);
  return center_of_mass;
}

SamVector3d samphysics_system_calculate_center_of_mass_velocity(SamPhysicsSystem* sys) {
  SamVector3d center_of_mass_velocity = {0, 0, 0};
  double total_mass = 0.0;

  for (uint64_t i = 0; i < sys->bodies_count; i++) {
    center_of_mass_velocity = samvector3d_add(center_of_mass_velocity, sys->bodies[i].velocity);
    total_mass += sys->bodies[i].mass;
  }
  
  center_of_mass_velocity = samvector3d_scale(center_of_mass_velocity, 1.0 / total_mass);
  return center_of_mass_velocity;
}