/*
    SPDX-FileCopyrightText: 2026 Fernando Vilas <fvilas@iname.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TEST_MODULE_EXPORT_H
#define TEST_MODULE_EXPORT_H

#ifdef TEST_MODULE_STATIC_DEFINE
#  define TEST_MODULE_EXPORT
#  define TEST_MODULE_NO_EXPORT
#else
#  ifndef TEST_MODULE_EXPORT
#    ifdef test_module_EXPORTS
        /* We are building this library */
#      define TEST_MODULE_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define TEST_MODULE_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef TEST_MODULE_NO_EXPORT
#    define TEST_MODULE_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef TEST_MODULE_DEPRECATED
#  define TEST_MODULE_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef TEST_MODULE_DEPRECATED_EXPORT
#  define TEST_MODULE_DEPRECATED_EXPORT TEST_MODULE_EXPORT TEST_MODULE_DEPRECATED
#endif

#ifndef TEST_MODULE_DEPRECATED_NO_EXPORT
#  define TEST_MODULE_DEPRECATED_NO_EXPORT TEST_MODULE_NO_EXPORT TEST_MODULE_DEPRECATED
#endif

#endif /* TEST_MODULE_EXPORT_H */
