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

#include <stdio.h>
#include <string.h>
#include "samrena.h"
#include "samvector.h"

typedef struct {
    int id;
    char name[32];
    float score;
} Student;

typedef struct {
    double x, y, z;
} Point3D;

SAMRENA_DECLARE_VECTOR(Student)
SAMRENA_DECLARE_VECTOR(Point3D)
SAMRENA_DECLARE_VECTOR(int)

int main() {
    printf("=== Type-safe SamVector Examples ===\n\n");
    
    Samrena* arena = samrena_create_default();
    if (!arena) {
        fprintf(stderr, "Failed to initialize arena\n");
        return 1;
    }

    printf("1. Student Vector Example:\n");
    SamrenaVector_Student* students = samrena_vector_Student_init(arena, 10);
    if (!students) {
        fprintf(stderr, "Failed to create student vector\n");
        samrena_destroy(arena);
        return 1;
    }

    Student alice = {1, "Alice Johnson", 95.5f};
    Student bob = {2, "Bob Smith", 87.2f};
    Student charlie = {3, "Charlie Brown", 92.8f};

    samrena_vector_Student_push(students, &alice);
    samrena_vector_Student_push(students, &bob);
    samrena_vector_Student_push(students, &charlie);

    printf("   Students added: %zu\n", samrena_vector_Student_size(students));
    for (size_t i = 0; i < samrena_vector_Student_size(students); i++) {
        const Student* s = samrena_vector_Student_at_const(students, i);
        if (s) {
            printf("   ID: %d, Name: %s, Score: %.1f\n", s->id, s->name, s->score);
        }
    }

    printf("\n2. Point3D Vector Example:\n");
    SamrenaVector_Point3D* points = samrena_vector_Point3D_init(arena, 5);
    if (!points) {
        fprintf(stderr, "Failed to create point vector\n");
        samrena_destroy(arena);
        return 1;
    }

    Point3D p1 = {1.0, 2.0, 3.0};
    Point3D p2 = {4.5, 5.5, 6.5};
    Point3D p3 = {-1.2, 0.0, 2.8};

    samrena_vector_Point3D_push(points, &p1);
    samrena_vector_Point3D_push(points, &p2);
    samrena_vector_Point3D_push(points, &p3);

    printf("   Points added: %zu\n", samrena_vector_Point3D_size(points));
    for (size_t i = 0; i < samrena_vector_Point3D_size(points); i++) {
        const Point3D* p = samrena_vector_Point3D_at_const(points, i);
        if (p) {
            printf("   Point[%zu]: (%.1f, %.1f, %.1f)\n", i, p->x, p->y, p->z);
        }
    }

    printf("\n3. Safe Access Operations:\n");
    Student retrieved_student;
    SamrenaVectorError err = samrena_vector_Student_get(students, 1, &retrieved_student);
    if (err == SAMRENA_VECTOR_SUCCESS) {
        printf("   Retrieved student at index 1: %s (Score: %.1f)\n", 
               retrieved_student.name, retrieved_student.score);
    }

    Student new_student = {4, "Diana Prince", 98.7f};
    err = samrena_vector_Student_set(students, 1, &new_student);
    if (err == SAMRENA_VECTOR_SUCCESS) {
        printf("   Updated student at index 1: %s (Score: %.1f)\n", 
               new_student.name, new_student.score);
    }

    printf("\n4. Vector Properties:\n");
    printf("   Student vector - Size: %zu, Capacity: %zu, Empty: %s\n",
           samrena_vector_Student_size(students),
           samrena_vector_Student_capacity(students),
           samrena_vector_Student_is_empty(students) ? "Yes" : "No");

    printf("   Point vector - Size: %zu, Capacity: %zu, Full: %s\n",
           samrena_vector_Point3D_size(points),
           samrena_vector_Point3D_capacity(points),
           samrena_vector_Point3D_is_full(points) ? "Yes" : "No");

    samrena_destroy(arena);
    printf("\nType-safe vector example completed successfully!\n");
    return 0;
}