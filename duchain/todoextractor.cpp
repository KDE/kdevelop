/*
 * Copyright 2014 Kevin Funk <kfunk@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "todoextractor.h"

#include "clangtypes.h"

#include <language/duchain/problem.h>
#include <language/duchain/stringhelpers.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/icompletionsettings.h>

#include <KDebug>

#include <QStringList>

#include <algorithm>

using namespace KDevelop;

namespace {

inline int findEndOfLine(const QString& str, int from = 0)
{
    const int index = str.indexOf('\n', from);
    return (index == -1 ? str.length() : index);
}

inline int findBeginningOfLine(const QString& str, int from = 0)
{
    const int index = str.lastIndexOf('\n', from);
    return (index == -1 ? 0 : index+1);
}

/**
 * Chop everything after characters in string @p str that match the regular expression @p regexp
 *
 * @return Number of chars trimmed
 */
int rtrim(QString& str, const QRegExp regexp = QRegExp("[^ \t\r\n\v\f]"))
{
    const int last = str.lastIndexOf(regexp);
    if (last == -1 || last == (str.size() - 1)) {
        return 0;
    }

    const int trim = (str.size() - 1) - last;
    str.chop(trim);
    return trim;
}

/**
 * @brief Class for parsing to-do items out of a given comment string
 *
 * Make sure this class is very performant as it will be used for every comment string
 * throughout the code base.
 *
 * So: No unnecessary deep-copies of strings if possible!
 */
class CommentTodoParser
{
public:
    struct Result {
        /// Description of the problem
        QString description;
        /// Range within the comment
        SimpleRange localRange;
    };

    CommentTodoParser(const QString& str, const QStringList& markerWords)
        : m_str(str)
        , m_todoMarkerWords(markerWords)
    {
        skipUntilMarkerWord();
    }

    QVector<Result> results() const
    {
        return m_results;
    }

private:
    void skipUntilMarkerWord()
    {
        // in the most-cases, we won't find a to-do item
        // make sure this case is sufficiently fast

        // for each to-do marker, scan the comment text
        foreach (const QString& todoMarker, m_todoMarkerWords) {
            int offset = m_str.indexOf(todoMarker, m_offset);
            if (offset != -1) {
                m_offset = offset;
                parseTodoMarker();
            }
        }
    }

    void skipUntilNewline()
    {
        m_offset = findEndOfLine(m_str, m_offset);
    }

    void parseTodoMarker()
    {
        // okay, we've found something
        // m_offset points to the start of a "FIXME" or "TODO"
        const int lineStart = findBeginningOfLine(m_str, m_offset);
        const int lineEnd = findEndOfLine(m_str, m_offset);
        Q_ASSERT(lineStart <= m_offset);
        Q_ASSERT(lineEnd > m_offset);

        QString text = m_str.mid(m_offset, lineEnd - m_offset);
        Q_ASSERT(!text.contains('\n'));

        // there's nothing to be stripped on the left side, hence ignore that
        int stripped_right;
        text = KDevelop::extractComment(text, nullptr, &stripped_right);
        // remove additional whitespace
        stripped_right += rtrim(text);

        // check at what line within the comment we are by just counting the newlines until now
        const int line = std::count(m_str.constBegin(), m_str.constBegin() + m_offset, '\n');
        SimpleCursor start = {line, m_offset - lineStart};
        SimpleCursor end = {line, start.column + text.length()};
        m_results << Result{text, {start, end}};

        skipUntilNewline();
        skipUntilMarkerWord();
    }

    inline bool atEnd() const
    {
        return m_offset < m_str.length();
    }

private:
    const QString m_str;
    const QStringList m_todoMarkerWords;

    int m_offset = 0;

    QVector<Result> m_results;
};

}

TodoExtractor::TodoExtractor(CXTranslationUnit unit, const KDevelop::IndexedString& file)
    : m_unit(unit)
    , m_file(file)
    , m_todoMarkerWords(KDevelop::ICore::self()->languageController()->completionSettings()->todoMarkerWords())
{
    extractTodos();
}

void TodoExtractor::extractTodos()
{
    auto cursor = clang_getTranslationUnitCursor(m_unit);
    CXSourceRange range = clang_getCursorExtent(cursor);

    CXToken* tokens = nullptr;
    unsigned int nTokens = 0;
    clang_tokenize(m_unit, range, &tokens, &nTokens);
    for (unsigned int i = 0; i < nTokens; ++i) {
        CXToken token = tokens[i];
        CXTokenKind tokenKind = clang_getTokenKind(token);
        if (tokenKind != CXToken_Comment) {
            continue;
        }

        CXString tokenSpelling = clang_getTokenSpelling(m_unit, token);
        auto tokenRange = ClangRange(clang_getTokenExtent(m_unit, token)).toSimpleRange();
        const QString text = ClangString(tokenSpelling).toString();

        CommentTodoParser parser(text, m_todoMarkerWords);
        foreach (const CommentTodoParser::Result& result, parser.results()) {
            ProblemPointer problem(new Problem);
            problem->setDescription(result.description);
            problem->setSeverity(ProblemData::Hint);
            problem->setSource(ProblemData::ToDo);

            // move the local range to the correct location
            // note: localRange is the range *within* the comment only
            auto localRange = result.localRange;
            SimpleRange todoRange{
                tokenRange.start.line + localRange.start.line,
                localRange.start.column,
                tokenRange.start.line + localRange.end.line,
                localRange.end.column};
            problem->setFinalLocation({m_file, todoRange});
            m_problems << problem;
        }
    }
    clang_disposeTokens(m_unit, tokens, nTokens);
}

QList< ProblemPointer > TodoExtractor::problems() const
{
    return m_problems;
}
