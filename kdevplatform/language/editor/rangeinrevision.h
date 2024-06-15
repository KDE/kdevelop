/*
    SPDX-FileCopyrightText: 2010 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_RANGEINREVISION_H
#define KDEVPLATFORM_RANGEINREVISION_H

#include <language/languageexport.h>
#include "cursorinrevision.h"

#include <KTextEditor/Range>

#include <QDebug>

namespace KDevelop {
/**
 * Represents a range (start- and end cursor) within a text document.
 *
 * In KDevelop, this object is used when referencing a ranges that do _not_ point into the
 * most current document revision. Therefore, before applying such a range in the text
 * documents, it has to be translated into the current document revision explicitly, thereby replaying
 * eventual changes (see DUChainBase::translate...)
 */

class KDEVPLATFORMLANGUAGE_EXPORT RangeInRevision
{
public:

    CursorInRevision start, end;

    RangeInRevision(const CursorInRevision& _start, const CursorInRevision& _end) : start(_start)
        , end(_end)
    {
    }

    RangeInRevision(const CursorInRevision& _start, int length) : start(_start)
        , end(_start.line, _start.column + length)
    {
    }

    RangeInRevision()
    {
    }

    RangeInRevision(int sLine, int sCol, int eLine, int eCol) : start(sLine, sCol)
        , end(eLine, eCol)
    {
    }

    static RangeInRevision invalid()
    {
        return RangeInRevision(-1, -1, -1, -1);
    }

    bool isValid() const
    {
        return start.column != -1 || start.line != -1 || end.column != -1 || end.line != -1;
    }

    bool isEmpty() const
    {
        return start == end;
    }

    enum ContainsBehavior {
        Default = 0,
        IncludeBackEdge = 1
    };
    /**
     * Checks if @p position is contained within this range (i.e. >= start and < end)
     * If @p cb is IncludeBackEdge, also checks that @p position == end
     */
    bool contains(const CursorInRevision& position, ContainsBehavior cb = Default) const
    {
        return (position >= start && position < end) || (cb == IncludeBackEdge && position == end);
    }

    bool contains(const RangeInRevision& range) const
    {
        return range.start >= start && range.end <= end;
    }

    bool operator ==(const RangeInRevision& rhs) const
    {
        return start == rhs.start && end == rhs.end;
    }

    bool operator !=(const RangeInRevision& rhs) const
    {
        return !(*this == rhs);
    }

    bool operator <(const RangeInRevision& rhs) const
    {
        return start < rhs.start;
    }

    /// @warning Using this is wrong in most cases! If you want
    ///  to transform this range to the current revision, you should do a proper
    ///  mapping instead through @ref KDevelop::DUChainBase or @ref KDevelop::RevisionReference
    ///  or @ref KDevelop::DocumentChangeTracker
    KTextEditor::Range castToSimpleRange() const
    {
        return KTextEditor::Range(start.castToSimpleCursor(), end.castToSimpleCursor());
    }

    /// @warning Using this is wrong in most cases! If you want
    ///  to transform this range to the current revision, you should do a proper
    ///  mapping instead through @ref KDevelop::DUChainBase or @ref KDevelop::RevisionReference
    ///  or @ref KDevelop::DocumentChangeTracker
    static RangeInRevision castFromSimpleRange(const KTextEditor::Range& range)
    {
        return RangeInRevision(range.start().line(), range.start().column(), range.end().line(), range.end().column());
    }
    ///qDebug() stream operator.  Writes this range to the debug output in a nicely formatted way.
    inline friend QDebug operator<<(QDebug s, const RangeInRevision& range)
    {
        s.nospace() << '[' << range.start << ", " << range.end << ']';
        return s.space();
    }
};

inline size_t qHash(const KDevelop::RangeInRevision& range)
{
    return qHash(range.start) + qHash(range.end) * 41;
}
} // namespace KDevelop

Q_DECLARE_TYPEINFO(KDevelop::RangeInRevision, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(KDevelop::RangeInRevision)

#endif
