/*
 * This file is part of KDevelop
 *
 * Copyright 2018 Anton Anikin <anton@anikin.xyz>
 * Copyright 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "problemmodel.h"

// plugin
#include "utils.h"
#include "plugin.h"
// KDevPlatform
#include <interfaces/iproject.h>
#include <language/editor/documentrange.h>
// KF
#include <KLocalizedString>

namespace ClangTidy
{

ProblemModel::ProblemModel(Plugin* plugin, QObject* parent)
    : KDevelop::ProblemModel(parent)
    , m_plugin(plugin)
{
    setFeatures(CanDoFullUpdate |
                ScopeFilter |
                SeverityFilter |
                Grouping |
                CanByPassScopeFilter);
}

ProblemModel::~ProblemModel() = default;

void ProblemModel::forceFullUpdate()
{
    if (m_url.isValid() && !m_plugin->isRunning()) {
        m_plugin->runClangTidy(m_url, m_allFiles);
    }
}

void ProblemModel::reset(KDevelop::IProject* project, const QUrl& url, bool allFiles)
{
    m_url = url;
    m_allFiles = allFiles;
    const auto path = url.toLocalFile();

    clearProblems();
    m_problems.clear();

    QString tooltip;
    if (project) {
        setMessage(i18n("Analysis started..."));
        tooltip = i18nc("@info:tooltip %1 is the path of the file", "Re-run last Clang-Tidy analysis (%1)", Utils::prettyPathName(path));
    } else {
        tooltip = i18nc("@info:tooltip", "Re-run last Clang-Tidy analysis");
    }

    setFullUpdateTooltip(tooltip);
}

void ProblemModel::setMessage(const QString& message)
{
    KDevelop::DocumentRange pathLocation(KDevelop::DocumentRange::invalid());
    pathLocation.document = KDevelop::IndexedString(m_url.toLocalFile());
    setPlaceholderText(message, pathLocation, i18n("Clang-Tidy"));
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
    if (m_problems.isEmpty()) {
        m_maxProblemDescriptionLength = 0;
    }

    for (const auto& problem : problems) {
        if (problemExists(problem)) {
            continue;
        }

        m_problems.append(problem);
        addProblem(problem);

        // This performs adjusting of columns width in the ProblemsView
        if (m_maxProblemDescriptionLength < problem->description().length()) {
            m_maxProblemDescriptionLength = problem->description().length();
            setProblems(m_problems);
        }
    }
}

void ProblemModel::finishAddProblems()
{
    if (m_problems.isEmpty()) {
        setMessage(i18n("Analysis completed, no problems detected."));
    } else {
        setProblems(m_problems);
    }
}

}
