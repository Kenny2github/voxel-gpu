#ifndef VECTOR_MATH_H
#define VECTOR_MATH_H

#include <stdint.h>

#define FRAC_BITS   8
#define M_PI        3.14159265358979323846264

struct AffineTransform3D {
    float matrix[16];
};

struct Vector {
    float x, y, z;
};

struct Vector_16fixed {
    uint16_t x, y, z;
};

float Q_rsqrt(float number); // Quake III method
float sqrtf(float x); // Inverse of Q_rsqrt
float cosf(float x); // Taylor approx
float sinf(float x); // Taylor approx

/** @brief Computes 3D cross-product given float data
 *  @param Vector a first vector
 *  @param Vector b second vector
 *  @param Vector c output vector
 */
void cross_product(
    const struct Vector *a,
    const struct Vector *b,
    struct Vector *c
);

/** @brief Normalizes 3D vector given vector data
 *  @param Vector a input and output vector
 */
void normalize(struct Vector *a);

/** @brief Creates a 3D affine rotation transform on a given x-axis and angle. Uses Rodrigues' formula defined here: https://www.songho.ca/opengl/gl_rotate.html
 *  @param float angle in radians
 *  @param struct Vector axis of rotation
 */
struct AffineTransform3D rotate_transform(float angle, struct Vector axis);

/** @brief Returns an identity affine transform matrix */
struct AffineTransform3D identity_transform();

/** @brief Effectively multiplies the vector with -1 */
void negative_vector(struct Vector* a);

/** @brief Adds two vectors and returns a new vector */
struct Vector add_vector(const struct Vector a, const struct Vector b);

/** @brief Multiplies a vector with a constant and returns a new vector */
struct Vector multiply_vector(const struct Vector a, float multiplier);

/** @brief Multiplies a vector with an Affine tranform matrix and returns a new vector */
struct Vector transform_vector(const struct AffineTransform3D *transform, const struct Vector a);

/** @brief Converts Vector to Vector_16fixed where Vector_16fixed uses an 8-bit integer, 8-bit floating fixed-point data */
struct Vector_16fixed convert_vector_format(const struct Vector* a);

int16_t convert_float_to_fixed(float a);


#endif