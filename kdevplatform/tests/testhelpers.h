/*
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TESTHELPERS_H
#define KDEVPLATFORM_TESTHELPERS_H

#include "testsexport.h"

#include <interfaces/idocument.h>
#include <language/editor/rangeinrevision.h>
#include <language/duchain/types/abstracttype.h>
#include <language/duchain/declaration.h>
#include <serialization/indexedstring.h>

#include <QTest>

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

namespace QTest {
template<>
inline char* toString(const KDevelop::CursorInRevision& c)
{
    return toString(c.castToSimpleCursor());
}

template<>
inline char* toString(const KDevelop::RangeInRevision& r)
{
    return toString(r.castToSimpleRange());
}

template<>
inline char* toString(const KDevelop::QualifiedIdentifier& id)
{
    return qstrdup(qPrintable(id.toString()));
}

template<>
inline char* toString(const KDevelop::Identifier& id)
{
    return qstrdup(qPrintable(id.toString()));
}

template<>
inline char* toString(const KDevelop::Declaration& dec)
{
    QString s = QStringLiteral("Declaration %1 (%2): %3")
                .arg(dec.identifier().toString(),
                     dec.qualifiedIdentifier().toString(),
                     QString::number(reinterpret_cast<qint64>(&dec), 10));
    return qstrdup(s.toLatin1().constData());
}

template<>
inline char* toString(const KDevelop::AbstractType::Ptr& type)
{
    QString s = QStringLiteral("Type: %1")
                .arg(type ? type->toString() : QStringLiteral("<null>"));
    return qstrdup(s.toLatin1().constData());
}

template<>
inline char* toString(const KDevelop::IndexedString& string)
{
    return qstrdup(qPrintable(string.str()));
}

template<>
inline char* toString(const KDevelop::IDocument::DocumentState& state)
{
    return toString(static_cast<uint>(state));
}
}

#endif // KDEVPLATFORM_TESTHELPERS_H
