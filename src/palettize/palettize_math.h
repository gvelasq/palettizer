// This file is part of palettize -- A palette generator based on k-means
// clustering with CIELAB colors.
// 
// MIT License
// 
// Copyright (c) 2021-2022 gvlsq
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef PALETTIZE_MATH_H
#define PALETTIZE_MATH_H

#include <float.h>
#include <math.h>

#define abs fabsf
#define cbrt cbrtf
#define pow powf
#define round roundf
#define sqrt sqrtf

#define FLOAT_MAX FLT_MAX
#define FLOAT_EPSILON FLT_EPSILON

struct Vector3 {
    float x, y, z;
};

struct Matrix3 {
    Vector3 x_axis;
    Vector3 y_axis;
    Vector3 z_axis;
};

// 
// Scalar
// 

inline float clamp(float l, float s, float h) {
    float result = s;

    if (result < l) result = l;
    else if (result > h) result = h;

    return result;
}

inline int clampi(int l, int s, int h) {
    int result = (int)clamp((float)l, (float)s, (float)h);

    return result;
}

inline float clamp01(float S) {
    float result = clamp(0.0f, S, 1.0f);

    return result;
}

inline float cube(float s) {
    float result = s*s*s;

    return result;
}

inline bool equals_approx(float a, float b) {
    bool result = abs(b - a) <= FLOAT_EPSILON;

    return result;
}

inline int roundi(float s) {
    int result = (int)round(s);

    return result;
}

inline u32 round_u32(float s) {
    u32 result = (u32)round(s);

    return result;
}

inline float safe_ratioN(float x, float y, float N) {
    float result = N;

    if (y != 0.0f) {
        result = x / y;
    }

    return result;
}

inline float safe_ratio0(float x, float y) {
    float result = safe_ratioN(x, y, 0.0f);

    return result;
}

inline float square(float s) {
    float result = s*s;

    return result;
}

// 
// Vector3
// 

inline float linear_rgb_to_srgb(float v);
inline float srgb_to_linear_rgb(float channel);

inline Vector3 V3(float x, float y, float z) {
    Vector3 result;
    result.x = x;
    result.y = y;
    result.z = z;

    return result;
}

inline Vector3 V3i(int x, int y, int z) {
    Vector3 result = V3((float)x, (float)y, (float)z);

    return result;
}

inline Vector3 operator+(Vector3 a, Vector3 b) {
    Vector3 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;

    return result;
}

inline Vector3 operator-(Vector3 a, Vector3 b) {
    Vector3 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;

    return result;
}

inline Vector3 operator*(Vector3 a, float s) {
    Vector3 result;
    result.x = a.x*s;
    result.y = a.y*s;
    result.z = a.z*s;

    return result;
}

inline Vector3 &operator+=(Vector3 &a, Vector3 b) {
    a = a + b;

    return a;
}

inline Vector3 &operator*=(Vector3 &a, float s) {
    a = a*s;

    return a;
}

inline float dot(Vector3 a, Vector3 b) {
    float result = a.x*b.x + a.y*b.y + a.z*b.z;

    return result;
}

inline bool equals_approx(Vector3 a, Vector3 b) {
    bool result = equals_approx(a.x, b.x) && equals_approx(a.y, b.y) && equals_approx(a.z, b.z);

    return result;
}

inline float length_squared(Vector3 v) {
    float result = dot(v, v);

    return result;
}

inline Vector3 linear_rgb_to_srgb(Vector3 v) {
    Vector3 result;
    result.x = linear_rgb_to_srgb(v.x);
    result.y = linear_rgb_to_srgb(v.y);
    result.z = linear_rgb_to_srgb(v.z);

    return result;
}

inline u32 pack_rgba(Vector3 v) {
    u32 result = round_u32(v.x*255.0f) << 0 | round_u32(v.y*255.0f) << 8 | round_u32(v.z*255.0f) << 16 | 255 << 24;

    return result;
}

inline Vector3 srgb_to_linear_rgb(Vector3 v) {
    Vector3 result;
    result.x = srgb_to_linear_rgb(v.x);
    result.y = srgb_to_linear_rgb(v.y);
    result.z = srgb_to_linear_rgb(v.z);

    return result;
}

inline Vector3 unpack_rgba(u32 u) {
    float inv_255 = 1.0f / 255.0f;

    Vector3 result;
    result.x = ((u >> 0) & 0xFF)*inv_255;
    result.y = ((u >> 8) & 0xFF)*inv_255;
    result.z = ((u >> 16) & 0xFF)*inv_255;

    return result;
}

// 
// Matrix3
// 

inline Vector3 operator*(Matrix3 m, Vector3 v) {
    Vector3 result;
    result.x = dot(V3(m.x_axis.x, m.y_axis.x, m.z_axis.x), v);
    result.y = dot(V3(m.x_axis.y, m.y_axis.y, m.z_axis.y), v);
    result.z = dot(V3(m.x_axis.z, m.y_axis.z, m.z_axis.z), v);

    return result;
}

// 
// Color space
// 

// White point coords for Illuminant D65
#define Xn 0.950470f
#define Yn 1.0f
#define Zn 1.088830f

inline float inv_ft(float t) {
    float result;

    float sigma = 6.0f / 29.0f;
    if (t > sigma) {
        result = cube(t);
    } else {
        result = (3.0f*square(sigma))*(t - (4.0f / 29.0f));
    }

    return result;
}

inline Vector3 cielab_to_ciexyz(Vector3 v) {
    Vector3 result;
    result.x = Xn*inv_ft(((v.x + 16.0f) / 116.0f) + (v.y / 500.0f));
    result.y = Yn*inv_ft((v.x + 16.0f) / 116.0f);
    result.z = Zn*inv_ft(((v.x + 16.0f) / 116.0f) - (v.z / 200.0f));

    return result;
}

inline float Ft(float t) {
    float result;

    float sigma = 6.0f / 29.0f;
    if (t > cube(sigma)) {
        result = cbrt(t);
    } else {
        result = (t / (3.0f*square(sigma))) + (4.0f / 29.0f);
    }

    return result;
}

inline Vector3 ciexyz_to_cielab(Vector3 v) {
    Vector3 result;
    result.x = 116.0f*Ft(v.y / Yn) - 16.0f;
    result.y = 500.0f*(Ft(v.x / Xn) - Ft(v.y / Yn));
    result.z = 200.0f*(Ft(v.y / Yn) - Ft(v.z / Zn));

    return result;
}

inline Vector3 ciexyz_to_linear_rgb(Vector3 v) {
    Matrix3 m;
    m.x_axis = V3(3.2404542f, -0.9692660f, 0.0556434f);
    m.y_axis = V3(-1.5371385f, 1.8760108f, -0.2040259f);
    m.z_axis = V3(-0.4985314f, 0.0415560f, 1.0572252f);

    Vector3 result = m*v;

    return result;
}

inline Vector3 linear_rgb_to_ciexyz(Vector3 v) {
    Matrix3 m;
    m.x_axis = V3(0.4124564f, 0.2126729f, 0.0193339f);
    m.y_axis = V3(0.3575761f, 0.7151522f, 0.1191920f);
    m.z_axis = V3(0.1804375f, 0.0721750f, 0.9503041f);

    Vector3 result = m*v;

    return result;
}

inline float linear_rgb_to_srgb(float s) {
    s = clamp01(s);

    float result;
    if (s <= 0.0031308f) {
        result = 12.92f*s;
    } else {
        result = (1.055f*pow(s, (1.0f / 2.4f))) - 0.055f;
    }

    return result;
}

inline u32 pack_cielab_to_rgba(Vector3 v) {
    Vector3 cielab = v;
    Vector3 ciexyz = cielab_to_ciexyz(cielab);
    Vector3 linear_rgb = ciexyz_to_linear_rgb(ciexyz);
    Vector3 srgb = linear_rgb_to_srgb(linear_rgb);
    u32 result = pack_rgba(srgb);

    return result;
}

inline float srgb_to_linear_rgb(float s) {
    s = clamp01(s);

    float result;
    if (s <= 0.04045f) {
        result = s / 12.92f;
    } else {
        result = pow(((s + 0.055f) / 1.055f), 2.4f);
    }

    assert(0.0f <= result && result <= 1.0f);

    return result;
}

inline Vector3 unpack_rgba_to_cielab(u32 u) {
    Vector3 srgb = unpack_rgba(u);
    Vector3 linear_rgb = srgb_to_linear_rgb(srgb);
    Vector3 ciexyz = linear_rgb_to_ciexyz(linear_rgb);
    Vector3 cielab = ciexyz_to_cielab(ciexyz);

    return cielab;
}

#endif
