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

#include <sammath.h>
#include <stdio.h>

void test_two_body_circular_orbit(Samrena *arena) {
  SamPhysicsSystem sys;
  sys.bodies_count = 2;
  sys.G = 1.0;
  sys.bodies = SAMRENA_PUSH_ARRAY(arena, SamPhysicsBody, 2);

  // Sun at origin
  sys.bodies[0].mass = 1.0;
  sys.bodies[0].position = (SamVector3d){0, 0, 0};
  sys.bodies[0].velocity = (SamVector3d){0, 0, 0};

  // Planet in circular orbit
  sys.bodies[1].mass = 1e-3;
  sys.bodies[1].position= (SamVector3d){1, 0, 0};
  sys.bodies[1].velocity = (SamVector3d){0, 1, 0};  // v = sqrt(GM/r) = 1

  samphysics_system_calculate_accelerations(&sys);

  // Check: acceleration should be -1 in x-direction (centripetal)
  printf("Planet acceleration: (%f, %f, %f)\n",
         sys.bodies[1].acceleration.x, sys.bodies[1].acceleration.y, sys.bodies[1].acceleration.z);
  // Expected: (-1.0, 0.0, 0.0) approximately

  double E_initial = samphysics_system_calculate_total_energy(&sys);
  printf("Total energy: %f\n", E_initial);
  // Expected: -0.5 for this setup
}

int main(void) {
    sammath_hello();
    
    printf("\nDemonstrating SamMath with samrena and samdata:\n");
    
    Samrena *arena = samrena_create_default();
    if (!arena) {
        printf("Failed to create arena\n");
        return 1;
    }
    
    SamMath *math = sammath_create(arena);
    if (!math) {
        printf("Failed to create SamMath\n");
        samrena_destroy(arena);
        return 1;
    }

    double *pi = (double *)samhashmap_get(math->constants, "pi");
    double *e = (double *)samhashmap_get(math->constants, "e");

    if (pi) printf("Pi constant: %.15f\n", *pi);
    if (e) printf("Euler's number: %.15f\n", *e);

    printf("Hash map size: %zu\n", samhashmap_size(math->constants));

    test_two_body_circular_orbit(arena);

    sammath_destroy(math);
    samrena_destroy(arena);
    
    printf("SamMath example completed successfully!\n");
    return 0;
}