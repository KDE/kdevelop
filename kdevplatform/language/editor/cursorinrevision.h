/*
    SPDX-FileCopyrightText: 2010 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_CURSORINREVISION_H
#define KDEVPLATFORM_CURSORINREVISION_H

#include <language/languageexport.h>

#include <KTextEditor/Cursor>

#include <QDebug>

namespace KDevelop {
/**
 * Represents a cursor (line-number and column-number) within a text document.
 *
 * In KDevelop, this object is used when referencing a cursor that does _not_ point into the
 * most current document revision. Therefore, before applying such a cursor in the text
 * documents, it has to be translated into the current document revision explicitly, thereby replaying
 * eventual changes (see DUChainBase::translate...)
 */

class KDEVPLATFORMLANGUAGE_EXPORT CursorInRevision
{
public:

    int line = 0, column = 0;

    CursorInRevision()
    {
    }

    CursorInRevision(int _line, int _column) : line(_line)
        , column(_column)
    {
    }

    static CursorInRevision invalid()
    {
        return CursorInRevision(-1, -1);
    }

    bool isValid() const
    {
        return line != -1 || column != -1;
    }

    bool operator<(const CursorInRevision& rhs) const
    {
        return line < rhs.line || (line == rhs.line && column < rhs.column);
    }

    bool operator<=(const CursorInRevision& rhs) const
    {
        return line < rhs.line || (line == rhs.line && column <= rhs.column);
    }

    bool operator>(const CursorInRevision& rhs) const
    {
        return line > rhs.line || (line == rhs.line && column > rhs.column);
    }

    bool operator>=(const CursorInRevision& rhs) const
    {
        return line > rhs.line || (line == rhs.line && column >= rhs.column);
    }

    bool operator ==(const CursorInRevision& rhs) const
    {
        return line == rhs.line && column == rhs.column;
    }

    bool operator !=(const CursorInRevision& rhs) const
    {
        return !(*this == rhs);
    }

    CursorInRevision operator +(const CursorInRevision& rhs) const
    {
        return CursorInRevision(line + rhs.line, column + rhs.column);
    }

    /// @warning Using this is wrong in most cases! If you want
    ///  to transform this cursor to the current revision, you should do a proper
    ///  mapping instead through @ref KDevelop::DUChainBase or @ref KDevelop::RevisionReference
    ///  or @ref KDevelop::DocumentChangeTracker
    KTextEditor::Cursor castToSimpleCursor() const
    {
        return KTextEditor::Cursor(line, column);
    }

    /// @warning Using this is wrong in most cases! If you want
    ///  to transform this cursor to the current revision, you should do a proper
    ///  mapping instead through @ref KDevelop::DUChainBase or @ref KDevelop::RevisionReference
    ///  or @ref KDevelop::DocumentChangeTracker
    static CursorInRevision castFromSimpleCursor(const KTextEditor::Cursor& cursor)
    {
        return CursorInRevision(cursor.line(), cursor.column());
    }

    /// qDebug() stream operator.  Writes this cursor to the debug output in a nicely formatted way.
    inline friend QDebug operator<<(QDebug s, const CursorInRevision& cursor)
    {
        s.nospace() << "(" << cursor.line << ", " << cursor.column << ")";
        return s.space();
    }
};

inline size_t qHash(const KDevelop::CursorInRevision& cursor)
{
    return cursor.line * 53 + cursor.column * 47;
}
} // namespace KDevelop

Q_DECLARE_TYPEINFO(KDevelop::CursorInRevision, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(KDevelop::CursorInRevision)

#endif
