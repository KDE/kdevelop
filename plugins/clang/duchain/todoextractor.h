/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TODOEXTRACTOR_H
#define TODOEXTRACTOR_H

#include "clangprivateexport.h"

#include <language/duchain/problem.h>

#include <clang-c/Index.h>

class KDEVCLANGPRIVATE_EXPORT TodoExtractor
{
public:
    TodoExtractor(CXTranslationUnit unit, CXFile file);

    /**
     * Retrieve the list of to-do problems this instance found
     */
    QList<KDevelop::ProblemPointer> problems() const;

private:
    void extractTodos();

    CXTranslationUnit m_unit;
    CXFile m_file;
    QStringList m_todoMarkerWords;

    QList<KDevelop::ProblemPointer> m_problems;
};

#endif // TODOEXTRACTOR_H
