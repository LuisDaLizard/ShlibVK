#include "ShlibVK/Math/Vec3.h"
#include <stdio.h>

void Vec3Print(Vec3 vector)
{
    printf("(%f, %f, %f)\n", vector.x, vector.y, vector.z);
}

Vec3 Vec3Add(Vec3 left, Vec3 right)
{
    Vec3 result;

    result.x = left.x + right.x;
    result.y = left.y + right.y;
    result.z = left.z + right.z;

    return result;
}

Vec3 Vec3Sub(Vec3 left, Vec3 right)
{
    Vec3 result;

    result.x = left.x - right.x;
    result.y = left.y - right.y;
    result.z = left.z - right.z;

    return result;
}
float Vec3Dot(Vec3 left, Vec3 right)
{
    float result;

    result = left.x * right.x + left.y * right.y + left.z * right.z;

    return result;
}

Vec3 Vec3Normalize(Vec3 vector)
{
    float magnitude = sqrtf(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);

    if (magnitude == 0)
        return (Vec3){ 0, 0, 0 };

    float invMagnitude = 1 / magnitude;

    vector.x *= invMagnitude;
    vector.y *= invMagnitude;
    vector.z *= invMagnitude;

    return vector;
}

Vec3 Vec3Cross(Vec3 left, Vec3 right)
{
    Vec3 result;

    result.x = left.y * right.z - left.z * right.y;
    result.y = left.z * right.x - left.x * right.z;
    result.z = left.x * right.y - left.y * right.x;

    return result;
}

Vec3 Vec3Negate(Vec3 vector)
{
    Vec3 result;

    result.x = -vector.x;
    result.y = -vector.y;
    result.z = -vector.z;

    return result;
}

Vec3 Vec3Scale(Vec3 vector, float scalar)
{
    Vec3 result;

    result.x = vector.x * scalar;
    result.y = vector.y * scalar;
    result.z = vector.z * scalar;

    return result;
}

Vec3 Vec3Mul(Vec3 left, Vec3 right)
{
    Vec3 result;

    result.x = left.x * right.x;
    result.y = left.y * right.y;
    result.z = left.z * right.z;

    return result;
}

float Vec3Magnitude(Vec3 vector)
{
    return sqrtf(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
}