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


#ifndef UNKNOWNDECLARATION_H
#define UNKNOWNDECLARATION_H

#include "duchainexport.h"

#include <language/duchain/problem.h>
#include <language/duchain/identifier.h>
#include <language/editor/simplerange.h>
#include <interfaces/iassistant.h>
#include <project/path.h>

struct KDEVCLANGDUCHAIN_EXPORT UnknownDeclarationFixit
{
    QString replacementText;
    KDevelop::SimpleRange range;

    bool operator==(const UnknownDeclarationFixit& other) const
    {
        return replacementText == other.replacementText
            && range == other.range;
    }

    /**
     * Human-readable description of this object
     *
     * @example 'Insert \"#include <vector>\" at line 10'
     */
    QString description() const;
};

using UnknownDeclarationFixits = QVector<UnknownDeclarationFixit>;

class KDEVCLANGDUCHAIN_EXPORT UnknownDeclarationProblem : public KDevelop::Problem
{
public:
    using Ptr = KSharedPtr<UnknownDeclarationProblem>;
    using ConstPtr = KSharedPtr<const UnknownDeclarationProblem>;

    UnknownDeclarationProblem() = default;
    UnknownDeclarationProblem( KDevelop::QualifiedIdentifier, KDevelop::Path );

    virtual KSharedPtr<KDevelop::IAssistant> solutionAssistant() const override;

    UnknownDeclarationProblem& setSymbol( KDevelop::QualifiedIdentifier );
    UnknownDeclarationProblem& setFile( KDevelop::Path arg );

private:
    KDevelop::QualifiedIdentifier m_identifier;
    KDevelop::Path m_file;
};

class KDEVCLANGDUCHAIN_EXPORT UnknownDeclarationFixitAssistant : public KDevelop::IAssistant
{
    Q_OBJECT

public:
    UnknownDeclarationFixitAssistant( UnknownDeclarationProblem::ConstPtr problem, const UnknownDeclarationFixits fixits);

    virtual void createActions() override;

private:
    UnknownDeclarationProblem::ConstPtr m_problem;
    UnknownDeclarationFixits m_fixits;
};

class KDEVCLANGDUCHAIN_EXPORT UnknownDeclarationFixitAction : public KDevelop::IAssistantAction
{
    Q_OBJECT

public:
    UnknownDeclarationFixitAction(const UnknownDeclarationProblem::ConstPtr& problem, const UnknownDeclarationFixit& fixit)
    : m_problem( problem ), m_fixit( fixit ) {}

    virtual QString description() const override;

public Q_SLOTS:
    virtual void execute() override;

private:
    UnknownDeclarationProblem::ConstPtr m_problem;
    UnknownDeclarationFixit m_fixit;
};

#endif // UNKNOWNDECLARATION_H
