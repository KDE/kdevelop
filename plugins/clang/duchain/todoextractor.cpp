/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "todoextractor.h"

#include "../util/clangtypes.h"

#include <language/duchain/problem.h>
#include <language/duchain/stringhelpers.h>
#include <language/editor/documentrange.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/icompletionsettings.h>

#include <QStringList>
#include <QDir>

#include <algorithm>
#include <limits>

using namespace KDevelop;

namespace {

inline int findEndOfLineOrEnd(const QString& str, int from = 0)
{
    const int index = str.indexOf(QLatin1Char('\n'), from);
    return (index == -1 ? str.length() : index);
}

inline int findBeginningOfLineOrStart(const QString& str, int from = 0)
{
    const int index = str.lastIndexOf(QLatin1Char('\n'), from);
    return (index == -1 ? 0 : index+1);
}

inline int findEndOfCommentOrEnd(const QString& str, int from = 0)
{
    const int index = str.indexOf(QLatin1String("*/"), from);
    return (index == -1 ? str.length() : index);
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
        KTextEditor::Range localRange;
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
        for (const QString& todoMarker : m_todoMarkerWords) {
            int offset = m_str.indexOf(todoMarker, m_offset);
            if (offset != -1) {
                m_offset = offset;
                parseTodoMarker();
            }
        }
    }

    void skipUntilNewline()
    {
        m_offset = findEndOfLineOrEnd(m_str, m_offset);
    }

    void parseTodoMarker()
    {
        // okay, we've found something
        // m_offset points to the start of the to-do item
        const int lineStart = findBeginningOfLineOrStart(m_str, m_offset);
        const int lineEnd = findEndOfLineOrEnd(m_str, m_offset);
        Q_ASSERT(lineStart <= m_offset);
        Q_ASSERT(lineEnd > m_offset);

        QString text = m_str.mid(m_offset, lineEnd - m_offset);
        Q_ASSERT(!text.contains(QLatin1Char('\n')));

        // there's nothing to be stripped on the left side, hence ignore that
        text.chop(text.length() - findEndOfCommentOrEnd(text));
        text = text.trimmed(); // remove additional whitespace from the end

        // check at what line within the comment we are by just counting the newlines until now
        const int line = std::count(m_str.constBegin(), m_str.constBegin() + m_offset, QLatin1Char('\n'));
        KTextEditor::Cursor start = {line, m_offset - lineStart};
        KTextEditor::Cursor end = {line, start.column() + static_cast<int>(text.size())};
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

Q_DECLARE_TYPEINFO(CommentTodoParser::Result, Q_MOVABLE_TYPE);

TodoExtractor::TodoExtractor(CXTranslationUnit unit, CXFile file)
    : m_unit(unit)
    , m_file(file)
    , m_todoMarkerWords(KDevelop::ICore::self()->languageController()->completionSettings()->todoMarkerWords())
{
    extractTodos();
}

void TodoExtractor::extractTodos()
{
    using uintLimits = std::numeric_limits<uint>;

    auto start = clang_getLocation(m_unit, m_file, 1, 1);
    auto end = clang_getLocation(m_unit, m_file, uintLimits::max(), uintLimits::max());

    auto range = clang_getRange(start, end);

    IndexedString path(QDir(ClangString(clang_getFileName(m_file)).toString()).canonicalPath());

    if(clang_Range_isNull(range)){
        return;
    }

    const ClangTokens tokens(m_unit, range);
    for (CXToken token : tokens) {
        CXTokenKind tokenKind = clang_getTokenKind(token);
        if (tokenKind != CXToken_Comment) {
            continue;
        }

        CXString tokenSpelling = clang_getTokenSpelling(m_unit, token);
        auto tokenRange = ClangRange(clang_getTokenExtent(m_unit, token)).toRange();
        const QString text = ClangString(tokenSpelling).toString();

        CommentTodoParser parser(text, m_todoMarkerWords);
        const auto parserResults = parser.results();
        m_problems.reserve(m_problems.size() + parserResults.size());
        for (const CommentTodoParser::Result& result : parserResults) {
            ProblemPointer problem(new Problem);
            problem->setDescription(result.description);
            problem->setSeverity(IProblem::Hint);
            problem->setSource(IProblem::ToDo);

            // move the local range to the correct location
            // note: localRange is the range *within* the comment only
            auto localRange = result.localRange;
            KTextEditor::Range todoRange{
                tokenRange.start().line() + localRange.start().line(),
                localRange.start().column(),
                tokenRange.start().line() + localRange.end().line(),
                localRange.end().column()};
            problem->setFinalLocation({path, todoRange});
            m_problems << problem;
        }
    }
}

QList< ProblemPointer > TodoExtractor::problems() const
{
    return m_problems;
}
