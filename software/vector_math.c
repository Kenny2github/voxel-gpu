#include "vector_math.h"

// Simple math functions for bare-metal environment
float Q_rsqrt(float number) {
    long i;
    float x2, y;
    const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    y = number;
    i = *(long*)&y;                      // evil floating point bit level hacking
    i = 0x5f3759df - (i >> 1);           // what the f*ck?
    y = *(float*)&i;
    y = y * (threehalfs - (x2 * y * y)); // 1st iteration
    return y;                            // returns 1/√number
}

float sqrtf(float x) {
    if (x <= 0.0f) return 0.0f;
    return 1.0f / Q_rsqrt(x);            // Convert inverse sqrt to sqrt
}

float cosf(float x) {
    // Taylor series approximation: cos(x) = 1 - x²/2! + x⁴/4! - x⁶/6! + ...
    float result = 1.0f;
    float term = 1.0f;
    float x_sq = x * x;
    
    for (int n = 1; n <= 5; n++) {
        term *= -x_sq / ((2*n-1) * (2*n));
        result += term;
    }
    return result;
}

float sinf(float x) {
    // Taylor series approximation: sin(x) = x - x³/3! + x⁵/5! - x⁷/7! + ...
    float result = x;
    float term = x;
    float x_sq = x * x;
    
    for (int n = 1; n <= 5; n++) {
        term *= -x_sq / ((2*n) * (2*n+1));
        result += term;
    }
    return result;
}

void cross_product(const struct Vector *a, const struct Vector *b, struct Vector *c) {
    if (!a || !b) return;
    c->x = a->y * b->z - a->z * b->y;
    c->y = a->z * b->x - a->x * b->z;
    c->z = a->x * b->y - a->y * b->x;
}

void normalize(struct Vector *a) {
    if (!a) return;
    
    float squared_norm = a->x * a->x + a->y * a->y + a->z * a->z;
    if(squared_norm == 0.0f)
        return;

    float inv_sqrt = Q_rsqrt(squared_norm);
    if (inv_sqrt == 0.0f) return;

    a->x *= inv_sqrt;
    a->y *= inv_sqrt;
    a->z *= inv_sqrt;
}

struct AffineTransform3D identity_transform() {
    struct AffineTransform3D identity = {{
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    }};

    return identity;
}


struct AffineTransform3D rotate_transform(float angle, struct Vector axis) {

    // Normalize axis
    float mag = sqrtf(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);
    if(mag == 0.0f)
        return identity_transform();
    float x = axis.x / mag;
    float y = axis.y / mag;
    float z = axis.z / mag;

    float c = cosf(angle);
    float s = sinf(angle);
    float one_c = 1.0f - c;

    // Row-major 4x4 rotation matrix
    struct AffineTransform3D t = {{
        c + x*x*one_c,     x*y*one_c + z*s,   x*z*one_c - y*s,   0.0f,
        y*x*one_c - z*s,   c + y*y*one_c,     y*z*one_c + x*s,   0.0f,
        z*x*one_c + y*s,   z*y*one_c - x*s,   c + z*z*one_c,     0.0f,
        0.0f,              0.0f,              0.0f,              1.0f
    }};

    return t;
}

void negative_vector(struct Vector* a) {
    if (!a) return;
    a->x = -a->x;
    a->y = -a->y;
    a->z = -a->z;
}

struct Vector add_vector(const struct Vector a, const struct Vector b) {
    struct Vector r;
    r.x = a.x + b.x;
    r.y = a.y + b.y;
    r.z = a.z + b.z;
    return r;
}


struct Vector divide_vector(const struct Vector a, float divisor) {
    struct Vector r;
    r.x = a.x / divisor;
    r.y = a.y / divisor;
    r.z = a.z / divisor;
    return r;
}

struct Vector multiply_vector(const struct Vector a, float multiplier) {
    struct Vector r;
    r.x = a.x * multiplier;
    r.y = a.y * multiplier;
    r.z = a.z * multiplier;
    return r;
}

struct Vector transform_vector(const struct AffineTransform3D *transform, const struct Vector a) {
    struct Vector r = {0, 0, 0};
    if (!transform) return r;
    // 4x4 matrix-vector multiplication for row-major matrix
    r.x = transform->matrix[0]*a.x + transform->matrix[1]*a.y + transform->matrix[2]*a.z + transform->matrix[3];
    r.y = transform->matrix[4]*a.x + transform->matrix[5]*a.y + transform->matrix[6]*a.z + transform->matrix[7];
    r.z = transform->matrix[8]*a.x + transform->matrix[9]*a.y + transform->matrix[10]*a.z + transform->matrix[11];
    return r;
}

struct Vector_16fixed convert_vector_format(const struct Vector *a) {
    struct Vector_16fixed new_vector = {
        convert_float_to_fixed(a->x),
        convert_float_to_fixed(a->y),
        convert_float_to_fixed(a->z)
    };

    return new_vector;
}

float max_vec(const struct Vector a) {
    return a.x > a.y && a.x > a.z ? a.x : a.y > a.z ? a.y : a.z;
}

float min_vec(const struct Vector a) {
    return a.x < a.y && a.x < a.z ? a.x : a.y < a.z ? a.y : a.z;
}

int16_t convert_float_to_fixed(float a) {
    return (int16_t)(a * (1 << FRAC_BITS));
}