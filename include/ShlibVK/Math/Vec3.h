#ifndef SHLIBVK_VEC3_H
#define SHLIBVK_VEC3_H

#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES
#endif

#include <math.h>

typedef struct sVec3
{
    float x, y, z;
} Vec3;

void Vec3Print(Vec3 vector);

Vec3 Vec3Add(Vec3 left, Vec3 right);

Vec3 Vec3Sub(Vec3 left, Vec3 right);

float Vec3Dot(Vec3 left, Vec3 right);

Vec3 Vec3Normalize(Vec3 vector);

Vec3 Vec3Cross(Vec3 left, Vec3 right);

Vec3 Vec3Negate(Vec3 vector);

Vec3 Vec3Scale(Vec3 vector, float scalar);

Vec3 Vec3Mul(Vec3 left, Vec3 right);

float Vec3Magnitude(Vec3 vector);

#endif //SHLIBVK_VEC3_H
