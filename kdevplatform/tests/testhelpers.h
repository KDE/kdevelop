/*
 * This file is part of KDevelop
 *
 * Copyright 2012 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_TESTHELPERS_H
#define KDEVPLATFORM_TESTHELPERS_H

#include "testsexport.h"

#include <language/editor/rangeinrevision.h>
#include <language/duchain/types/abstracttype.h>
#include <language/duchain/declaration.h>
#include <serialization/indexedstring.h>

#include <QTest>

#define QFAIL_RETURN(message, retval)                                                                                  \
    do {                                                                                                               \
        QTest::qFail(static_cast<const char *>(message), __FILE__, __LINE__);                                          \
        return retval;                                                                                                 \
    } while (false)

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
}

#endif // KDEVPLATFORM_TESTHELPERS_H
