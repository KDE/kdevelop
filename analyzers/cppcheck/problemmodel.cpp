/* This file is part of KDevelop

   Copyright 2017 Anton Anikin <anton.anikin@htower.ru>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "problemmodel.h"

#include "plugin.h"
#include "utils.h"

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <shell/problemmodelset.h>

#include <klocalizedstring.h>

namespace cppcheck
{

inline KDevelop::ProblemModelSet* problemModelSet()
{
    return KDevelop::ICore::self()->languageController()->problemModelSet();
}

static const QString problemModelId = QStringLiteral("Cppcheck");

ProblemModel::ProblemModel(Plugin* plugin)
    : KDevelop::ProblemModel(plugin)
    , m_plugin(plugin)
    , m_project(nullptr)
{
    setFeatures(CanDoFullUpdate | ScopeFilter | SeverityFilter | Grouping | CanByPassScopeFilter);
    reset();
    problemModelSet()->addModel(problemModelId, i18n("Cppcheck"), this);
}

ProblemModel::~ProblemModel()
{
    problemModelSet()->removeModel(problemModelId);
}

KDevelop::IProject* ProblemModel::project() const
{
    return m_project;
}

void ProblemModel::addProblems(const QVector<KDevelop::IProblem::Ptr>& problems)
{
    static int maxLength = 0;

    if (m_problems.isEmpty()) {
        maxLength = 0;
    }

    m_problems.append(problems);
    for (auto p : problems) {
        addProblem(p);

        // This performs adjusting of columns width in the ProblemsView
        if (maxLength < p->description().length()) {
            maxLength = p->description().length();
            setProblems(m_problems);
            break;
        }
    }
}

void ProblemModel::setProblems()
{
    setProblems(m_problems);
}

void ProblemModel::reset()
{
    reset(nullptr, QString());
}

void ProblemModel::reset(KDevelop::IProject* project, const QString& path)
{
    m_project = project;
    m_path = path;

    clearProblems();
    m_problems.clear();

    QString tooltip = i18nc("@info:tooltip", "Re-run last Cppcheck analyze");
    if (m_project) {
        tooltip += QString(" (%1)").arg(prettyPathName(m_path));
    }
    setFullUpdateTooltip(tooltip);
}

void ProblemModel::show()
{
    problemModelSet()->showModel(problemModelId);
}

void ProblemModel::forceFullUpdate()
{
    if (m_project && !m_plugin->isRunning()) {
        m_plugin->runCppcheck(m_project, m_path);
    }
}

}
