/*
    SPDX-FileCopyrightText: 2023 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEV_CLANG_SUPPORT_ADDITIONAL_FLOATING_TYPES_H
#define KDEV_CLANG_SUPPORT_ADDITIONAL_FLOATING_TYPES_H

#pragma clang system_header

// This file is automatically included by KDevelop's Clang backend for C/C++ projects to improve compatibility with GCC.
// This is required to remove parse errors in GCC headers when using a GCC toolchain.

#ifdef __cplusplus
inline namespace __KDevelopClangGccCompat {
#endif

typedef long double __float80;

#if __GNUC__ >= 13 || !defined(__cplusplus)
// based on bits/floatn-common.h from glibc
typedef float _Float32;
typedef double _Float32x;
typedef double _Float64;
typedef long double _Float64x;

typedef __float128 _Float128;
#endif

#ifdef __cplusplus
}
#endif

#endif // KDEV_CLANG_SUPPORT_ADDITIONAL_FLOATING_TYPES_H
