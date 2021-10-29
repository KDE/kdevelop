/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CLANGDIAGNOSTICEVALUATOR_H
#define CLANGDIAGNOSTICEVALUATOR_H

#include "clangprivateexport.h"

#include <clang-c/Index.h>

class ClangProblem;

namespace ClangDiagnosticEvaluator {
KDEVCLANGPRIVATE_EXPORT ClangProblem* createProblem(CXDiagnostic diagnostic, CXTranslationUnit unit);

enum DiagnosticType {
    Unknown,
    UnknownDeclarationProblem,
    IncludeFileNotFoundProblem,
    ReplaceWithDotProblem,
    ReplaceWithArrowProblem
};

/**
 * @return Type of @p diagnostic
 * @sa DiagnosticType
 */
KDEVCLANGPRIVATE_EXPORT DiagnosticType diagnosticType(CXDiagnostic diagnostic);
}

#endif // CLANGDIAGNOSTICEVALUATOR_H
