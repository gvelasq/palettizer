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

#ifndef PALETTIZE_STRING_H
#define PALETTIZE_STRING_H

inline char flip_case(char c) {
    char result = '\0';

    if ('a' <= c && c <= 'z') {
        result = 'A' + (c - 'a');
    } else if ('A' <= c && c <= 'Z') {
        result = 'a' + (c - 'A');
    }

    return result;
}

inline bool strings_match(char *a, char *b, bool case_sensitive = true) {
    while (*a && *b) {
        if (*a == *b ||
            (!case_sensitive && *a == flip_case(*b))) {
            a++;
            b++;
        } else {
            break;
        }
    }

    bool result = *a == *b;

    return result;
}

#endif
