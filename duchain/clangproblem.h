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


#ifndef CLANGPROBLEM_H
#define CLANGPROBLEM_H

#include "duchainexport.h"

#include <language/duchain/problem.h>
#include <language/editor/simplerange.h>
#include <interfaces/iassistant.h>

struct KDEVCLANGDUCHAIN_EXPORT ClangFixit
{
    QString replacementText;
    KDevelop::SimpleRange range;

    bool operator==(const ClangFixit& other) const
    {
        return replacementText == other.replacementText
            && range == other.range;
    }

    /**
     * Human-readable description of this object
     *
     * @example 'Insert " = 0" after col:1'
     */
    QString description() const;
};

using ClangFixits = QVector<ClangFixit>;

class KDEVCLANGDUCHAIN_EXPORT ClangProblem : public KDevelop::Problem
{
public:
    using Ptr = KSharedPtr<ClangProblem>;
    using ConstPtr = KSharedPtr<const ClangProblem>;
    using FixitMap = QHash<ClangProblem::ConstPtr, ClangFixits>;

    virtual KSharedPtr<KDevelop::IAssistant> solutionAssistant() const override;

    ClangFixits fixits() const;
    void setFixits(const ClangFixits& fixits);

    /**
     * Retrieve all fixits of this problem and its child diagnostics
     *
     * @return A mapping of problem pointers to the list of associated fixits
     */
    FixitMap allFixits() const;

private:
    ClangFixits m_fixits;
};

class KDEVCLANGDUCHAIN_EXPORT ClangFixitAssistant : public KDevelop::IAssistant
{
    Q_OBJECT

public:
    ClangFixitAssistant(const ClangProblem::FixitMap& fixitMap);

    virtual void createActions() override;

private:
    ClangProblem::FixitMap m_fixitMap;
};

class KDEVCLANGDUCHAIN_EXPORT ClangFixitAction : public KDevelop::IAssistantAction
{
    Q_OBJECT

public:
    ClangFixitAction(const ClangProblem::ConstPtr& problem, const ClangFixit& fixit);

    virtual QString description() const override;

public Q_SLOTS:
    virtual void execute() override;

private:
    ClangProblem::ConstPtr m_problem;
    ClangFixit m_fixit;
};

#endif // CLANGPROBLEM_H
