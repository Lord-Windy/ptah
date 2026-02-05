//
// Created by sam on 16/1/26.
//

#include "ray.h"

Point3 ray_at(Ray ray, double t) { return vec3_add(ray.origin, vec3_mul(ray.direction, t)); }
