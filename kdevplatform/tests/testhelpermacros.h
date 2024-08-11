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

#endif // KDEVPLATFORM_TESTHELPERMACROS_H
