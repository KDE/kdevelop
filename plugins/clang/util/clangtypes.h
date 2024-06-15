/*
    SPDX-FileCopyrightText: 2013 Olivier de Gaalon <olivier.jg@gmail.com>
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
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

inline size_t qHash(const CXCursor& cursor) noexcept
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
    explicit ClangString(CXString string);
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
    explicit ClangLocation(CXSourceLocation cursor);
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
    explicit ClangRange(CXSourceRange range);

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
