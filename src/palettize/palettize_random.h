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

#ifndef PALETTIZE_RANDOM_H
#define PALETTIZE_RANDOM_H

struct Random_Series {
    u32 seed;
};

inline Random_Series seed_series(u32 seed) {
    Random_Series result;
    result.seed = seed;

    return result;
}

inline u32 random_u32(Random_Series *series) {
    // 32-bit Xorshift
    u32 result = series->seed;
    result ^= result << 13;
    result ^= result >> 17;
    result ^= result << 5;

    series->seed = result;

    return result;
}

inline u32 random_u32_between(Random_Series *series, u32 min, u32 max) {
    u32 result = min + (random_u32(series) % (max - min));
    assert(min <= result && result <= max);

    return result;
}

#endif
