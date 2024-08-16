/*
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TESTHELPERS_H
#define KDEVPLATFORM_TESTHELPERS_H

#include "testhelpermacros.h"
#include "testsexport.h"

#include <debugger/breakpoint/breakpoint.h>
#include <interfaces/idocument.h>
#include <language/editor/rangeinrevision.h>
#include <language/duchain/types/abstracttype.h>
#include <language/duchain/declaration.h>
#include <serialization/indexedstring.h>
#include <serialization/indexedstringview.h>

#include <QMetaEnum>
#include <QTest>

namespace KDevelop {
template<typename Enum>
const char* enumeratorName(Enum enumerator)
{
    return QMetaEnum::fromType<Enum>().valueToKey(static_cast<int>(enumerator));
}
}

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
inline char* toString(const KDevelop::IndexedStringView& string)
{
    return toString(string.toString());
}

template<>
inline char* toString(const KDevelop::IDocument::DocumentState& state)
{
    return toString(static_cast<uint>(state));
}

template<>
inline char* toString(const KDevelop::Breakpoint::BreakpointKind& value)
{
    return toString(static_cast<uint>(value));
}

template<>
inline char* toString(const KDevelop::Breakpoint::BreakpointState& value)
{
    return toString(static_cast<uint>(value));
}
}

#endif // KDEVPLATFORM_TESTHELPERS_H
