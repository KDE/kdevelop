/*
    This file is part of KDevelop

    Copyright 2013 Olivier de Gaalon <olivier.jg@gmail.com>
    Copyright 2013 Milian Wolff <mail@milianw.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef CLANGTYPES_H
#define CLANGTYPES_H

#include <clang-c/Index.h>

#include <util/path.h>

#include "clangprivateexport.h"

#include <iterator>

class QTextStream;

namespace KTextEditor {
class Cursor;
class Range;
}

namespace KDevelop {
class DocumentCursor;
class DocumentRange;
class CursorInRevision;
class RangeInRevision;
class IndexedString;
}

inline uint qHash(const CXCursor& cursor) noexcept
{
    return clang_hashCursor(cursor);
}

inline bool operator==(const CXCursor& lhs, const CXCursor& rhs) noexcept
{
    return clang_equalCursors(lhs, rhs);
}

class ClangString
{
public:
    ClangString(CXString string);
    ~ClangString();

    ClangString(const ClangString&) = delete;
    ClangString& operator=(const ClangString&) = delete;

    /**
     * Might return nullptr for invalid strings
     */
    const char* c_str() const;

    bool isEmpty() const;

    QString toString() const;
    QByteArray toByteArray() const;
    KDevelop::IndexedString toIndexed() const;

private:
    CXString string;
};

QTextStream& operator<<(QTextStream& stream, const ClangString& str);
QDebug operator<<(QDebug stream, const ClangString& str);

class ClangLocation
{
public:
    ClangLocation(CXSourceLocation cursor);
    ~ClangLocation();

    operator KDevelop::DocumentCursor() const;

    operator KTextEditor::Cursor() const;

    operator KDevelop::CursorInRevision() const;

    operator CXSourceLocation() const;

private:
    CXSourceLocation location;
};

QDebug operator<<(QDebug stream, const ClangLocation& str);

class KDEVCLANGPRIVATE_EXPORT ClangRange
{
public:
    ClangRange(CXSourceRange range);

    ~ClangRange();

    ClangLocation start() const;
    ClangLocation end() const;

    CXSourceRange range() const;

    KDevelop::DocumentRange toDocumentRange() const;

    KTextEditor::Range toRange() const;

    KDevelop::RangeInRevision toRangeInRevision() const;

private:
    CXSourceRange m_range;
};

QDebug operator<<(QDebug stream, const ClangRange& str);

class KDEVCLANGPRIVATE_EXPORT ClangTokens
{
public:
    ClangTokens(CXTranslationUnit unit, CXSourceRange range);
    ClangTokens(const ClangTokens&) = delete;
    ClangTokens& operator=(const ClangTokens&) = delete;
    ~ClangTokens();
    CXToken* begin() const;
    CXToken* end() const;
    std::reverse_iterator<CXToken*> rbegin() const;
    std::reverse_iterator<CXToken*> rend() const;
    uint size() const;
    CXToken at(uint index) const;
    CXTranslationUnit unit() const;
private:
    CXTranslationUnit m_unit;
    CXToken* m_tokens;
    uint m_numTokens;
};

QDebug operator<<(QDebug stream, const ClangTokens& str);
QDebug operator<<(QDebug stream, const CXToken& token);

#endif // CLANGTYPES_H
