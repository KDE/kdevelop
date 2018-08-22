/* This file is part of KDevelop

   Copyright 2018 Anton Anikin <anton@anikin.xyz>

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

#include <KLocalizedString>

namespace Clazy
{

inline KDevelop::ProblemModelSet* problemModelSet()
{
    return KDevelop::ICore::self()->languageController()->problemModelSet();
}

inline QString problemModelId()
{
    return QStringLiteral("clazy");
}

ProblemModel::ProblemModel(Plugin* plugin)
    : KDevelop::ProblemModel(plugin)
    , m_plugin(plugin)
    , m_project(nullptr)
    , m_pathLocation(KDevelop::DocumentRange::invalid())
{
    setFeatures(CanDoFullUpdate |
                ScopeFilter |
                SeverityFilter |
                Grouping |
                CanByPassScopeFilter|
                ShowSource);

    reset();

    problemModelSet()->addModel(problemModelId(), i18n("Clazy"), this);
}

ProblemModel::~ProblemModel()
{
    problemModelSet()->removeModel(problemModelId());
}

KDevelop::IProject* ProblemModel::project() const
{
    return m_project;
}

void ProblemModel::setMessage(const QString& message)
{
    setPlaceholderText(message, m_pathLocation, i18n("Clazy"));
}

// The code is adapted version of cppcheck::ProblemModel::problemExists()
// TODO Add into KDevelop::ProblemModel class ?
bool ProblemModel::problemExists(KDevelop::IProblem::Ptr newProblem)
{
    for (const auto& problem : qAsConst(m_problems)) {
        if (newProblem->source() == problem->source() &&
            newProblem->sourceString() == problem->sourceString() &&
            newProblem->severity() == problem->severity() &&
            newProblem->finalLocation() == problem->finalLocation() &&
            newProblem->description() == problem->description() &&
            newProblem->explanation() == problem->explanation())
            return true;
    }

    return false;
}

// The code is adapted version of cppcheck::ProblemModel::addProblems()
// TODO Add into KDevelop::ProblemModel class ?
void ProblemModel::addProblems(const QVector<KDevelop::IProblem::Ptr>& problems)
{
    static int maxLength = 0;

    if (m_problems.isEmpty()) {
        maxLength = 0;
    }

    for (const auto& problem : problems) {
        if (problemExists(problem)) {
            continue;
        }

        m_problems.append(problem);
        addProblem(problem);

        // This performs adjusting of columns width in the ProblemsView
        if (maxLength < problem->description().length()) {
            maxLength = problem->description().length();
            setProblems(m_problems);
        }
    }
}

void ProblemModel::setProblems()
{
    if (m_problems.isEmpty()) {
        setMessage(i18n("Analysis completed, no problems detected."));
    } else {
        setProblems(m_problems);
    }
}

void ProblemModel::reset()
{
    reset(nullptr, QString());
}

void ProblemModel::reset(KDevelop::IProject* project, const QString& path)
{
    m_project = project;

    m_path = path;
    m_pathLocation.document = KDevelop::IndexedString(path);

    clearProblems();
    m_problems.clear();

    QString tooltip;
    if (m_project) {
        setMessage(i18n("Analysis started..."));
        tooltip = i18nc("@info:tooltip %1 is the path of the file", "Re-run last Clazy analysis (%1)", prettyPathName(m_path));
    } else {
        tooltip = i18nc("@info:tooltip", "Re-run last Clazy analysis");
    }

    setFullUpdateTooltip(tooltip);
}

void ProblemModel::show()
{
    problemModelSet()->showModel(problemModelId());
}

void ProblemModel::forceFullUpdate()
{
    if (m_project && !m_plugin->isRunning()) {
        m_plugin->runClazy(m_project, m_path);
    }
}

}
