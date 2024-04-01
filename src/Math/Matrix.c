#include "ShlibVK/Math/Matrix.h"
#include "ShlibVK/Utils.h"
#include <stdio.h>

void MatrixPrint(Matrix matrix)
{
    printf("|%f, %f, %f, %f|\n", matrix.m00, matrix.m01, matrix.m02, matrix.m03);
    printf("|%f, %f, %f, %f|\n", matrix.m10, matrix.m11, matrix.m12, matrix.m13);
    printf("|%f, %f, %f, %f|\n", matrix.m20, matrix.m21, matrix.m22, matrix.m23);
    printf("|%f, %f, %f, %f|\n", matrix.m30, matrix.m31, matrix.m32, matrix.m33);
}

Matrix MatrixIdentity(void)
{
    Matrix result = { 0 };

    result.m00 = 1;
    result.m11 = 1;
    result.m22 = 1;
    result.m33 = 1;

    return result;
}

Matrix MatrixMul(Matrix left, Matrix right)
{
    Matrix result;

    result.m00 = left.m00 * right.m00 + left.m01 * right.m10 + left.m02 * right.m20 + left.m03 * right.m30;
    result.m01 = left.m00 * right.m01 + left.m01 * right.m11 + left.m02 * right.m21 + left.m03 * right.m31;
    result.m02 = left.m00 * right.m02 + left.m01 * right.m12 + left.m02 * right.m22 + left.m03 * right.m32;
    result.m03 = left.m00 * right.m03 + left.m01 * right.m13 + left.m02 * right.m23 + left.m03 * right.m33;

    result.m10 = left.m10 * right.m00 + left.m11 * right.m10 + left.m12 * right.m20 + left.m13 * right.m30;
    result.m11 = left.m10 * right.m01 + left.m11 * right.m11 + left.m12 * right.m21 + left.m13 * right.m31;
    result.m12 = left.m10 * right.m02 + left.m11 * right.m12 + left.m12 * right.m22 + left.m13 * right.m32;
    result.m13 = left.m10 * right.m03 + left.m11 * right.m13 + left.m12 * right.m23 + left.m13 * right.m33;

    result.m20 = left.m20 * right.m00 + left.m21 * right.m10 + left.m22 * right.m20 + left.m23 * right.m30;
    result.m21 = left.m20 * right.m01 + left.m21 * right.m11 + left.m22 * right.m21 + left.m23 * right.m31;
    result.m22 = left.m20 * right.m02 + left.m21 * right.m12 + left.m22 * right.m22 + left.m23 * right.m32;
    result.m23 = left.m20 * right.m03 + left.m21 * right.m13 + left.m22 * right.m23 + left.m23 * right.m33;

    result.m30 = left.m30 * right.m00 + left.m31 * right.m10 + left.m32 * right.m20 + left.m33 * right.m30;
    result.m31 = left.m30 * right.m01 + left.m31 * right.m11 + left.m32 * right.m21 + left.m33 * right.m31;
    result.m32 = left.m30 * right.m02 + left.m31 * right.m12 + left.m32 * right.m22 + left.m33 * right.m32;
    result.m33 = left.m30 * right.m03 + left.m31 * right.m13 + left.m32 * right.m23 + left.m33 * right.m33;

    return result;
}

Vec3 MatrixMulVec3(Matrix left, Vec3 right)
{
    Vec3 result;

    result.x = left.m00 * right.x + left.m01 * right.y + left.m02 * right.z + left.m03;
    result.y = left.m10 * right.x + left.m11 * right.y + left.m12 * right.z + left.m13;
    result.z = left.m20 * right.x + left.m21 * right.y + left.m22 * right.z + left.m23;

    return result;
}

Vec4 MatrixMulVec4(Matrix left, Vec4 right)
{
    Vec4 result;

    result.x = left.m00 * right.x + left.m01 * right.y + left.m02 * right.z + left.m03 * right.w;
    result.y = left.m10 * right.x + left.m11 * right.y + left.m12 * right.z + left.m13 * right.w;
    result.z = left.m20 * right.x + left.m21 * right.y + left.m22 * right.z + left.m23 * right.w;
    result.w = left.m30 * right.x + left.m31 * right.y + left.m32 * right.z + left.m33 * right.w;

    return result;
}

Matrix MatrixCreateScale(Vec3 scale)
{
    Matrix result = MatrixIdentity();

    result.m00 *= scale.x;
    result.m11 *= scale.y;
    result.m22 *= scale.z;

    return result;
}

Matrix MatrixCreateUScale(float scale)
{
    Matrix result = MatrixIdentity();

    result.m00 *= scale;
    result.m11 *= scale;
    result.m22 *= scale;

    return result;
}

Matrix MatrixCreateTranslation(Vec3 translation)
{
    Matrix result = MatrixIdentity();

    result.m03 = translation.x;
    result.m13 = translation.y;
    result.m23 = translation.z;

    return result;
}

Matrix MatrixScale(Matrix matrix, Vec3 scale)
{
    return MatrixMul(matrix, MatrixCreateScale(scale));
}

Matrix MatrixUScale(Matrix matrix, float scale)
{
    return MatrixMul(matrix, MatrixCreateUScale(scale));
}

Matrix MatrixTranslate(Matrix matrix, Vec3 translation)
{
    return MatrixMul(matrix, MatrixCreateTranslation(translation));
}

Matrix MatrixOrtho(float left, float right, float top, float bottom, float near, float far)
{
    Matrix result = MatrixIdentity();

    result.m00 = 2 / (right - left);
    result.m11 = 2 / (top - bottom);
    result.m22 = -2 / (far - near);

    result.m03 = -((right + left) / (right - left));
    result.m13 = -((top + bottom) / (top - bottom));
    result.m23 = -((far + near) / (far - near));

    return result;
}

Matrix MatrixPerspective(float aspect, float fov, float near, float far)
{
    Matrix result = MatrixIdentity();

    float tan_fov = tanf((fov * DEG2RADF) / 2.0f);

    result.m00 = 1 / (aspect * tan_fov);
    result.m11 = 1 / tan_fov;
    result.m22 = -(far + near) / (far - near);
    result.m33 = 0;
    result.m23 = -(2 * far * near) / (far - near);
    result.m32 = -1;

    return result;
}

Matrix MatrixLookAt(Vec3 eye, Vec3 target, Vec3 up) {
    Matrix result = MatrixIdentity();

    Vec3 zaxis = Vec3Normalize(Vec3Sub(target, eye));
    Vec3 xaxis = Vec3Normalize(Vec3Cross(zaxis, up));
    Vec3 yaxis = Vec3Cross(xaxis, zaxis);

    zaxis = Vec3Negate(zaxis);

    result.m00 = xaxis.x;
    result.m01 = xaxis.y;
    result.m02 = xaxis.z;
    result.m03 = -Vec3Dot(xaxis, eye);

    result.m10 = yaxis.x;
    result.m11 = yaxis.y;
    result.m12 = yaxis.z;
    result.m13 = -Vec3Dot(yaxis, eye);

    result.m20 = zaxis.x;
    result.m21 = zaxis.y;
    result.m22 = zaxis.z;
    result.m23 = -Vec3Dot(zaxis, eye);

    result.m33 = 1;


    return result;
}