/*
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TESTHELPERMACROS_H
#define KDEVPLATFORM_TESTHELPERMACROS_H

#include <QtVersionChecks>

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

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
#define QCOMPARE_OP_IMPL_RETURN(lhs, rhs, op, opId, retval)                                                            \
    do {                                                                                                               \
        if (!QTest::qCompareOp<QTest::ComparisonOperation::opId>(lhs, rhs, #lhs, #rhs, __FILE__, __LINE__))            \
            return retval;                                                                                             \
    } while (false)
#else
// NOTE: this version of the macro is based on the initial implementation of QCOMPARE_OP_IMPL()
// in Qt 6.4. The improved version above requires QTest::qCompareOp() introduced in Qt 6.8.

// A wrapper lambda is introduced to extend the lifetime of lhs and rhs in
// case they are temporary objects.
// We also use IILE to prevent potential name clashes and shadowing of variables
// from user code. A drawback of the approach is that it looks ugly :(
#define QCOMPARE_OP_IMPL_RETURN(lhs, rhs, op, opId, retval)                                                            \
    do {                                                                                                               \
        if (![](auto&& qt_lhs_arg, auto&& qt_rhs_arg) {                                                                \
                /* assumes that op does not actually move from qt_{lhs, rhs}_arg */                                    \
                return QTest::reportResult(                                                                            \
                    std::forward<decltype(qt_lhs_arg)>(qt_lhs_arg) op std::forward<decltype(qt_rhs_arg)>(qt_rhs_arg),  \
                    [&qt_lhs_arg] {                                                                                    \
                        return QTest::toString(qt_lhs_arg);                                                            \
                    },                                                                                                 \
                    [&qt_rhs_arg] {                                                                                    \
                        return QTest::toString(qt_rhs_arg);                                                            \
                    },                                                                                                 \
                    #lhs, #rhs, QTest::ComparisonOperation::opId, __FILE__, __LINE__);                                 \
            }(lhs, rhs)) {                                                                                             \
            return retval;                                                                                             \
        }                                                                                                              \
    } while (false)
#endif

#define QCOMPARE_EQ_RETURN(computed, baseline, retval) QCOMPARE_OP_IMPL_RETURN(computed, baseline, ==, Equal, retval)
#define QCOMPARE_NE_RETURN(computed, baseline, retval) QCOMPARE_OP_IMPL_RETURN(computed, baseline, !=, NotEqual, retval)
#define QCOMPARE_LT_RETURN(computed, baseline, retval) QCOMPARE_OP_IMPL_RETURN(computed, baseline, <, LessThan, retval)
#define QCOMPARE_LE_RETURN(computed, baseline, retval)                                                                 \
    QCOMPARE_OP_IMPL_RETURN(computed, baseline, <=, LessThanOrEqual, retval)
#define QCOMPARE_GT_RETURN(computed, baseline, retval)                                                                 \
    QCOMPARE_OP_IMPL_RETURN(computed, baseline, >, GreaterThan, retval)
#define QCOMPARE_GE_RETURN(computed, baseline, retval)                                                                 \
    QCOMPARE_OP_IMPL_RETURN(computed, baseline, >=, GreaterThanOrEqual, retval)

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
