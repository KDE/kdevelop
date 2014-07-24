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
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/icompletionsettings.h>

#include <KDebug>

#include <QStringList>

using namespace KDevelop;

TodoExtractor::TodoExtractor(CXTranslationUnit unit, const KDevelop::IndexedString& file)
    : m_unit(unit)
    , m_file(file)
    , m_todoMarkerWords(KDevelop::ICore::self()->languageController()->completionSettings()->todoMarkerWords())
{
    CXCursor tuCursor = clang_getTranslationUnitCursor(unit);
    clang_visitChildren(tuCursor, &visitCursor, this);
}

QList< ProblemPointer > TodoExtractor::problems() const
{
    return m_problems;
}

bool TodoExtractor::containsTodo(const QString& text) const
{
    foreach (const QString& todoMarker, m_todoMarkerWords) {
        if (text.contains(todoMarker)) {
            return true;
        }
    }
    return false;
}

/**
 * Extract to-do problems from comment @p comment
 *
 * @param cursor Cursor this comment is attached to
 */
void TodoExtractor::extractTodos(CXComment comment, CXCursor cursor)
{
    auto kind = clang_Comment_getKind(comment);
    if (kind == CXComment_Text) {
        const QString text = ClangString(clang_TextComment_getText(comment)).toString();
        if (!containsTodo(text)) {
            return;
        }

        // FIXME: Find the correct range for the line the to-do appears in
        // This requires new API in libclang for accessing clang::comments::Comment::getSourceRange
        // For now, let's just use the declaration's range which is attached to this comment
        auto range = ClangRange(clang_Cursor_getSpellingNameRange(cursor, 0, 0)).toSimpleRange();
        const DocumentRange docRange(m_file, range);

        ProblemPointer problem(new Problem);
        problem->setDescription(text.trimmed());
        problem->setSeverity(ProblemData::Hint);
        problem->setSource(ProblemData::ToDo);
        problem->setFinalLocation(docRange);
        m_problems << problem;
    }

    // recursive call
    int numChildren = clang_Comment_getNumChildren(comment);
    for (int i = 0; i < numChildren; ++i) {
        extractTodos(clang_Comment_getChild(comment, i), cursor);
    }
}

CXChildVisitResult TodoExtractor::visitCursor(CXCursor cursor, CXCursor /*parent*/, CXClientData data)
{
    auto control = static_cast<TodoExtractor*>(data);
    Q_ASSERT(control);

    auto comment = clang_Cursor_getParsedComment(cursor);
    control->extractTodos(comment, cursor);

    return CXChildVisit_Recurse;
}
