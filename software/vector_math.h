#ifndef VECTOR_MATH_H
#define VECTOR_MATH_H

#include <stdint.h>

#ifndef FRAC_BITS
#define FRAC_BITS   8
#endif 

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

struct AffineTransform3D {
    float matrix[16];
};

struct Vector {
    float x, y, z;
};

struct Vector_16fixed {
    int16_t x, y, z;
} __attribute__((aligned(4)));

float Q_rsqrt(float number); // Quake III method
float sqrtf(float x); // Inverse of Q_rsqrt
float cosf(float x); // Taylor approx
float sinf(float x); // Taylor approx
float tanf(float x); //  The thing ken gave for linear approximation

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

struct Vector sub_vector(const struct Vector a, const struct Vector b);

struct Vector divide_vector(const struct Vector a, float divisor);

/** @brief Multiplies a vector with a constant and returns a new vector */
struct Vector multiply_vector(const struct Vector a, float multiplier);

/** @brief Multiplies a vector with an Affine tranform matrix and returns a new vector */
struct Vector transform_vector(const struct AffineTransform3D *transform, const struct Vector a);

/** @brief Converts Vector to Vector_16fixed where Vector_16fixed uses an 8-bit integer, 8-bit floating fixed-point data */
struct Vector_16fixed convert_vector_format(const struct Vector* a);

float max_vec(const struct Vector a);
int16_t max_vec_fixed(const struct Vector_16fixed a);

float min_vec(const struct Vector a);
int16_t min_vec_fixed(const struct Vector_16fixed a);

int16_t convert_float_to_fixed(float a);

int16_t convert_int_to_fixed(int a);

// Fixed-point vector functions (16-bit with FRAC_BITS fraction bits)

/** @brief Computes 3D cross-product given fixed-point data
 *  @param Vector_16fixed a first vector
 *  @param Vector_16fixed b second vector
 *  @param Vector_16fixed c output vector
 */
void cross_product_fixed(
    const struct Vector_16fixed *a,
    const struct Vector_16fixed *b,
    struct Vector_16fixed *c
);

/** @brief Normalizes 3D fixed-point vector
 *  @param Vector_16fixed a input and output vector
 */
void normalize_fixed(struct Vector_16fixed *a);

/** @brief Effectively multiplies the fixed-point vector with -1 */
void negative_vector_fixed(struct Vector_16fixed *a);

/** @brief Adds two fixed-point vectors and returns a new vector */
struct Vector_16fixed add_vector_fixed(const struct Vector_16fixed a, const struct Vector_16fixed b);

/** @brief Subtracts two fixed-point vectors and returns a new vector */
struct Vector_16fixed sub_vector_fixed(const struct Vector_16fixed a, const struct Vector_16fixed b);

/** @brief Divides a fixed-point vector by a fixed-point divisor and returns a new vector */
struct Vector_16fixed divide_vector_fixed(const struct Vector_16fixed a, int16_t divisor);

/** @brief Multiplies a fixed-point vector with a fixed-point multiplier and returns a new vector */
struct Vector_16fixed multiply_vector_fixed(const struct Vector_16fixed a, int16_t multiplier);

#endif