/*
    SPDX-FileCopyrightText: 2018 Anton Anikin <anton@anikin.xyz>
    SPDX-FileCopyrightText: 2018, 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "compileanalyzeproblemmodel.h"

// KDevPlatform
#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <language/editor/documentrange.h>
// KF
#include <KLocalizedString>

namespace KDevelop
{

CompileAnalyzeProblemModel::CompileAnalyzeProblemModel(const QString& toolName, QObject* parent)
    : KDevelop::ProblemModel(parent)
    , m_toolName(toolName)
    , m_pathLocation(KDevelop::DocumentRange::invalid())
{
}

CompileAnalyzeProblemModel::~CompileAnalyzeProblemModel() = default;

KDevelop::IProject* CompileAnalyzeProblemModel::project() const
{
    return m_project;
}

void CompileAnalyzeProblemModel::setMessage(const QString& message)
{
    setPlaceholderText(message, m_pathLocation, m_toolName);
}

// The code is adapted version of cppcheck::ProblemModel::problemExists()
// TODO Add into KDevelop::ProblemModel class ?
bool CompileAnalyzeProblemModel::problemExists(KDevelop::IProblem::Ptr newProblem)
{
    for (const auto& problem : std::as_const(m_problems)) {
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
void CompileAnalyzeProblemModel::addProblems(const QVector<KDevelop::IProblem::Ptr>& problems)
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

void CompileAnalyzeProblemModel::finishAddProblems(bool jobSucceeded)
{
    if (m_problems.isEmpty()) {
        if (jobSucceeded) {
            setMessage(i18n("Analysis completed, no problems detected."));
        } else {
            // canceled or an error => show nothing rather than a wrong message
            // TODO: show an error message in case of an error?
            setMessage(QString{});
        }
    } else {
        setProblems(m_problems);
    }
}

void CompileAnalyzeProblemModel::reset()
{
    reset(nullptr, QUrl(), false);
}

void CompileAnalyzeProblemModel::reset(KDevelop::IProject* project, const QUrl& path, bool allFiles)
{
    m_project = project;
    m_path = path;
    m_allFiles = allFiles;
    m_pathLocation.document = KDevelop::IndexedString(path.toLocalFile());

    clearProblems();
    m_problems.clear();

    QString tooltip;
    if (m_project) {
        setMessage(i18n("Analysis started..."));

        const QString prettyPathName = KDevelop::ICore::self()->projectController()->prettyFileName(        path, KDevelop::IProjectController::FormatPlain);
        tooltip = i18nc("@info:tooltip %2 is the path of the file", "Re-run last %1 analysis (%2)", m_toolName, prettyPathName);
    } else {
        tooltip = i18nc("@info:tooltip", "Re-run last %1 analysis", m_toolName);
    }

    setFullUpdateTooltip(tooltip);
}

void CompileAnalyzeProblemModel::forceFullUpdate()
{
    if (m_path.isValid()) {
        emit rerunRequested(m_path, m_allFiles);
    }
}

}

#include "moc_compileanalyzeproblemmodel.cpp"
