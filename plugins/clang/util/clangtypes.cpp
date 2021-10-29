/*
    SPDX-FileCopyrightText: 2013 Olivier de Gaalon <olivier.jg@gmail.com>
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "clangtypes.h"

#include <QReadLocker>

#include <language/editor/rangeinrevision.h>
#include <language/editor/documentcursor.h>
#include <language/editor/documentrange.h>
#include <util/path.h>

using namespace KDevelop;

namespace {

template<typename T>
inline T cursorForCXSrcLoc(CXSourceLocation loc)
{
    uint line = 0;
    uint column = 0;
    clang_getFileLocation(loc, nullptr, &line, &column, nullptr);
    return {static_cast<int>(line-1), static_cast<int>(column-1)};
}

}

ClangString::ClangString(CXString string)
    : string(string)
{
}

ClangString::~ClangString()
{
    clang_disposeString(string);
}

const char* ClangString::c_str() const
{
    return clang_getCString(string);
}

bool ClangString::isEmpty() const
{
    auto str = c_str();
    return !str || !str[0];
}

QString ClangString::toString() const
{
    return QString::fromUtf8(c_str());
}

QByteArray ClangString::toByteArray() const
{
    return QByteArray(c_str());
}

IndexedString ClangString::toIndexed() const
{
    return IndexedString(c_str());
}

QTextStream& operator<<(QTextStream& stream, const ClangString& str)
{
    return stream << str.toString();
}

QDebug operator<<(QDebug stream, const ClangString& str)
{
    return stream << str.toString();
}

ClangLocation::ClangLocation(CXSourceLocation location)
    : location(location)
{

}

ClangLocation::operator DocumentCursor() const
{
    uint line = 0;
    uint column = 0;
    CXFile file;
    clang_getFileLocation(location, &file, &line, &column, nullptr);
    ClangString fileName(clang_getFileName(file));
    return {IndexedString(fileName.c_str()), {static_cast<int>(line-1), static_cast<int>(column-1)}};
}

ClangLocation::operator KTextEditor::Cursor() const
{
    return cursorForCXSrcLoc<KTextEditor::Cursor>(location);
}

ClangLocation::operator CursorInRevision() const
{
    return cursorForCXSrcLoc<CursorInRevision>(location);
}

ClangLocation::operator CXSourceLocation() const
{
    return location;
}

ClangLocation::~ClangLocation()
{
}

QDebug operator<<(QDebug stream, const ClangLocation& location)
{
    return stream << static_cast<DocumentCursor>(location);
}

ClangRange::ClangRange(CXSourceRange range)
    : m_range(range)
{

}

ClangLocation ClangRange::start() const
{
    return ClangLocation(clang_getRangeStart(m_range));
}

ClangLocation ClangRange::end() const
{
    return ClangLocation(clang_getRangeEnd(m_range));
}

CXSourceRange ClangRange::range() const
{
    return m_range;
}

DocumentRange ClangRange::toDocumentRange() const
{
    auto start = clang_getRangeStart(m_range);
    CXFile file;
    clang_getFileLocation(start, &file, nullptr, nullptr, nullptr);
    ClangString fileName(clang_getFileName(file));
    return {IndexedString(QUrl::fromLocalFile(fileName.toString()).adjusted(QUrl::NormalizePathSegments)), toRange()};
}

KTextEditor::Range ClangRange::toRange() const
{
    return {start(), end()};
}

RangeInRevision ClangRange::toRangeInRevision() const
{
    return {start(), end()};
}

ClangRange::~ClangRange()
{
}

QDebug operator<<(QDebug stream, const ClangRange& range)
{
    return stream << range.toDocumentRange();
}

ClangTokens::ClangTokens(CXTranslationUnit unit, CXSourceRange range)
    : m_unit(unit)
{
    clang_tokenize(m_unit, range, &m_tokens, &m_numTokens);
}
ClangTokens::~ClangTokens()
{
    clang_disposeTokens(m_unit, m_tokens, m_numTokens);
}
CXToken* ClangTokens::begin() const
{
    return m_tokens;
}
CXToken* ClangTokens::end() const
{
    return m_tokens + m_numTokens;
}
std::reverse_iterator<CXToken*> ClangTokens::rbegin() const
{
    return std::reverse_iterator<CXToken*>(end());
}
std::reverse_iterator<CXToken*> ClangTokens::rend() const
{
    return std::reverse_iterator<CXToken*>(begin());
}
uint ClangTokens::size() const
{
    return m_numTokens;
}
CXToken ClangTokens::at(uint index) const {
    Q_ASSERT(index < m_numTokens);
    return m_tokens[index];
}

CXTranslationUnit ClangTokens::unit() const
{
    return m_unit;
}

QDebug operator<<(QDebug stream, const ClangTokens& tokens)
{
    stream << "ClangTokens {";
    for (uint i = 0; i < tokens.size(); ++i) {
        stream << i << tokens.at(i) << ClangString(clang_getTokenSpelling(tokens.unit(), tokens.at(i))) << ",";
    }
    return stream << "}";
}

QDebug operator<<(QDebug stream, const CXToken& token)
{
    return stream << clang_getTokenKind(token);
}
