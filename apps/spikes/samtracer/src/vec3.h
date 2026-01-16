//
// Created by sam on 15/1/26.
//

#ifndef PTAH_VEC3_H
#define PTAH_VEC3_H


typedef struct {
  double x;
  double y;
  double z;
} Vec3;

typedef Vec3 Point3;

Vec3 vec3_add(Vec3 a, Vec3 b);
Vec3 vec3_sub(Vec3 a, Vec3 b);
Vec3 vec3_mul(Vec3 a, double b);
Vec3 vec3_div(Vec3 a, double b);
double vec3_dot(Vec3 a, Vec3 b);
Vec3 vec3_cross(Vec3 a, Vec3 b);
Vec3 vec3_unit(Vec3 a);

double vec3_distance(Vec3 a, Vec3 b);
double vec3_length(Vec3 a);
double vec3_squared_length(Vec3 a);

#endif // PTAH_VEC3_H
