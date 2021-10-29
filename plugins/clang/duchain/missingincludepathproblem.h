/*
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MISSINGINCLUDEPATHPROBLEM_H
#define MISSINGINCLUDEPATHPROBLEM_H

#include "clangproblem.h"

class MissingIncludePathProblem : public ClangProblem
{
public:
    using Ptr = QExplicitlySharedDataPointer<MissingIncludePathProblem>;

    MissingIncludePathProblem(CXDiagnostic diagnostic, CXTranslationUnit unit);

    KDevelop::IAssistant::Ptr solutionAssistant() const override;
};

#endif // MISSINGINCLUDEPATHPROBLEM_H
