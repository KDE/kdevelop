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

#ifndef TODOEXTRACTOR_H
#define TODOEXTRACTOR_H

#include "duchainexport.h"

#include <language/duchain/problem.h>

#include <clang-c/Index.h>

class KDEVCLANGDUCHAIN_EXPORT TodoExtractor
{
public:
    TodoExtractor(CXTranslationUnit unit, const KDevelop::IndexedString& file);

    /**
     * Retrieve the list of to-do problems this instance found
     */
    QList<KDevelop::ProblemPointer> problems() const;

private:
    static CXChildVisitResult visitCursor(CXCursor cursor, CXCursor /*parent*/, CXClientData data);

    bool containsTodo(const QString& text) const;
    void extractTodos(CXComment comment, CXCursor cursor);

    CXTranslationUnit m_unit;
    KDevelop::IndexedString m_file;
    QStringList m_todoMarkerWords;

    QList<KDevelop::ProblemPointer> m_problems;
};

#endif // TODOEXTRACTOR_H
