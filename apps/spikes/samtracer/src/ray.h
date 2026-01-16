//
// Created by sam on 16/1/26.
//

#ifndef PTAH_RAY_H
#define PTAH_RAY_H

#include "vec3.h"

typedef struct Ray {
    Point3 origin;
    Vec3 direction;
} Ray;

Point3 ray_at(Ray ray, double t);

#endif // PTAH_RAY_H
