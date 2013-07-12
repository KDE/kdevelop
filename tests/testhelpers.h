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

#include "kdevplatformtestsexport.h"

#include <language/editor/rangeinrevision.h>
#include <language/duchain/types/abstracttype.h>
#include <language/duchain/declaration.h>

#include <QTest>

namespace QTest {

template<>
inline char* toString(const KDevelop::SimpleCursor& c)
{
    return qstrdup(qPrintable(QString("(%1, %2)").arg(c.line).arg(c.column)));
}

template<>
inline char* toString(const KDevelop::SimpleRange& r)
{
    return qstrdup(qPrintable(QString("[%1, %2]").arg(toString(r.start)).arg(toString(r.end))));
}

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
inline char* toString(const KTextEditor::Cursor& c)
{
    return qstrdup(qPrintable(QString("(%1, %2)").arg(c.line()).arg(c.column())));
}

template<>
inline char* toString(const KTextEditor::Range& r)
{
    return qstrdup(qPrintable(QString("[%1, %2]").arg(toString(r.start())).arg(toString(r.end()))));
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
    QString s = QString("Declaration %1 (%2): %3")
        .arg(dec.identifier().toString())
        .arg(dec.qualifiedIdentifier().toString())
        .arg(reinterpret_cast<qint64>(&dec));
    return qstrdup(s.toLatin1().constData());
}

template<>
inline char* toString(const TypePtr<KDevelop::AbstractType>& type)
{
    QString s = QString("Type: %1")
        .arg(type ? type->toString() : QString("<null>"));
    return qstrdup(s.toLatin1().constData());
}

}

#endif // KDEVPLATFORM_TESTHELPERS_H
