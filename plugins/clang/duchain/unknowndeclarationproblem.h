/*
    SPDX-FileCopyrightText: 2014 Jørgen Kvalsvik <lycantrophe@lavabit.com>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef UNKNOWNDECLARATION_H
#define UNKNOWNDECLARATION_H

#include "clangprivateexport.h"
#include "clangproblem.h"

#include <language/duchain/identifier.h>

class KDEVCLANGPRIVATE_EXPORT UnknownDeclarationProblem : public ClangProblem
{
public:
    using Ptr = QExplicitlySharedDataPointer<UnknownDeclarationProblem>;
    using ConstPtr = QExplicitlySharedDataPointer<const UnknownDeclarationProblem>;

    UnknownDeclarationProblem(CXDiagnostic diagnostic, CXTranslationUnit unit);

    void setSymbol(const KDevelop::QualifiedIdentifier& identifier);

    KDevelop::IAssistant::Ptr solutionAssistant() const override;

    /**
     * @param declarations declarations to find matching include files for
     * @return list of paths of include files usable for the given declarations
     */
    static QStringList findMatchingIncludeFiles(const QVector<KDevelop::Declaration*>& declarations);

private:
    KDevelop::QualifiedIdentifier m_identifier;
};

#endif // UNKNOWNDECLARATION_H
