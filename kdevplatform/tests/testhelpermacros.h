/*
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TESTHELPERMACROS_H
#define KDEVPLATFORM_TESTHELPERMACROS_H

#define QVERIFY_RETURN(statement, retval)                                                                              \
    do {                                                                                                               \
        if (!QTest::qVerify(static_cast<bool>(statement), #statement, "", __FILE__, __LINE__))                         \
            return retval;                                                                                             \
    } while (false)

#define QCOMPARE_RETURN(actual, expected, retval)                                                                      \
    do {                                                                                                               \
        if (!QTest::qCompare(actual, expected, #actual, #expected, __FILE__, __LINE__))                                \
            return retval;                                                                                             \
    } while (false)

#define RETURN_IF_TEST_FAILED(...)                                                                                     \
    do {                                                                                                               \
        if (QTest::currentTestFailed()) {                                                                              \
            qCritical("FAILED AT: %s:%d", __FILE__, __LINE__);                                                         \
            return __VA_ARGS__;                                                                                        \
        }                                                                                                              \
    } while (false)

/**  @note Unlike simpler *_RETURN() macros above, this macro relies on an IILE and RETURN_IF_TEST_FAILED()
 *         to avoid copying a lot of potentially changeable code from qtestcase.h.
 *         Therefore, @p condition must work inside a lambda with @c & as the capture-default.
 *         Furthermore, this macro returns @p retval not only if the QTRY_VERIFY() call fails, but also if
 *         the current test has already failed before the macro's invocation. If the distinction matters,
 *         check whether the current test has failed before invoking this macro.
 */
#define QTRY_VERIFY_RETURN(condition, retval)                                                                          \
    [&]() { /* IILE to allow QTRY_VERIFY to return void on failure */                                                  \
            QTRY_VERIFY(condition);                                                                                    \
    }();                                                                                                               \
    RETURN_IF_TEST_FAILED(retval);

#endif // KDEVPLATFORM_TESTHELPERMACROS_H
