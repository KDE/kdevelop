/*
    SPDX-FileCopyrightText: 2026 Fernando Vilas <fvilas@iname.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "clangmoduleprecompilejob.h"

#include <language/editor/documentrange.h>
#include <ThreadWeaver/DependencyPolicy>

#include "clangmodulegraph.h"
#include "util/clangdebug.h"

ClangModulePrecompileJob::ClangModulePrecompileJob(const QString& moduleName,
                                                 const QString& clangCompiler,
                                                 const ClangModule::Map& allModules,
                                                 const QStringList& searchIncludePaths,
                                                 const QString& parserOptions,
                                                 const QString& pcmOutputDir,
                                                 const std::function<bool()>& abortFunction)
    : m_moduleName(moduleName)
    , m_clangCompiler(clangCompiler)
    , m_allModules(allModules)
    , m_searchIncludePaths(searchIncludePaths)
    , m_parserOptions(parserOptions)
    , m_pcmOutputDir(pcmOutputDir)
    , m_abortFunction(abortFunction)
{
}

ClangModulePrecompileJob::~ClangModulePrecompileJob() = default;

QString ClangModulePrecompileJob::moduleName() const
{
    return m_moduleName;
}

bool ClangModulePrecompileJob::success() const
{
    return m_success;
}

QByteArray ClangModulePrecompileJob::moduleFileArg() const
{
    return m_moduleFileArg;
}

QList<KDevelop::ProblemPointer> ClangModulePrecompileJob::problems() const
{
    return m_problems;
}

void ClangModulePrecompileJob::run(ThreadWeaver::JobPointer /*self*/, ThreadWeaver::Thread* /*thread*/)
{
    if ((m_abortFunction && m_abortFunction()) || shouldAbort()) {
        return;
    }

    auto mod = m_allModules.value(m_moduleName);
    if (!mod) {
        return;
    }

    QSet<QString> building;
    m_success = mod->precompile(
        m_clangCompiler,
        m_allModules,
        m_searchIncludePaths,
        m_parserOptions,
        m_pcmOutputDir,
        building,
        [this] {
            return (m_abortFunction && m_abortFunction()) || shouldAbort();
        },
        &m_problems
    );

    if (m_success) {
        m_moduleFileArg = mod->moduleFileArg(m_pcmOutputDir);
    }
}

QList<QSharedPointer<ClangModulePrecompileJob>> ClangModulePrecompileJob::createJobs(
    const QStringList& requiredModules,
    const ClangModule::Map& modules,
    const QString& clangCompiler,
    const QStringList& searchIncludePaths,
    const QString& parserOptions,
    const QString& pcmOutputDir,
    const std::function<bool()>& abortFunction,
    QList<KDevelop::ProblemPointer>* problems)
{
    if (requiredModules.isEmpty() || (abortFunction && abortFunction())) {
        return {};
    }

    ClangModuleGraph graph;
    for (auto it = modules.constBegin(); it != modules.constEnd(); ++it) {
        if (it.value()) {
            QList<KDevelop::IndexedString> deps;
            deps.reserve(it.value()->dependencies().size());
            for (const QString& dep : it.value()->dependencies()) {
                deps.append(KDevelop::IndexedString(dep));
            }
            graph.addModule(KDevelop::IndexedString(it.key()), deps, KDevelop::IndexedString(it.value()->sourceFile()));
        }
    }

    QList<KDevelop::IndexedString> targetList;
    targetList.reserve(requiredModules.size());
    for (const QString& req : requiredModules) {
        targetList.append(KDevelop::IndexedString(req));
    }

    const auto sortRes = graph.dependenciesFor(targetList);
    if (!sortRes.success) {
        const QString cycleStr = ClangModuleGraph::formatCycleString(sortRes.cycle);
        qCWarning(KDEV_CLANG) << "Circular module dependency detected:" << cycleStr;
        if (problems) {
            KDevelop::ProblemPointer problem(new KDevelop::Problem);
            problem->setSeverity(KDevelop::IProblem::Error);
            problem->setSource(KDevelop::IProblem::Preprocessor);
            problem->setDescription(QStringLiteral("Circular module dependency detected: %1").arg(cycleStr));
            problem->setExplanation(QStringLiteral("Modules cannot depend on each other cyclically. Cycle chain: %1").arg(cycleStr));
            const QString firstFile = sortRes.cycle.isEmpty() ? QString() : graph.moduleNode(sortRes.cycle.first()).sourceFile.str();
            problem->setFinalLocation(KDevelop::DocumentRange(KDevelop::IndexedString(firstFile), KTextEditor::Range(0, 0, 0, 0)));
            problems->append(problem);
        }
        return {};
    }

    QMap<QString, QSharedPointer<ClangModulePrecompileJob>> jobMap;
    QList<QSharedPointer<ClangModulePrecompileJob>> jobList;

    for (const auto& indexedMod : sortRes.order) {
        const QString modName = indexedMod.str();
        auto it = modules.find(modName);
        if (it == modules.end() || !it.value()) {
            continue;
        }

        auto job = QSharedPointer<ClangModulePrecompileJob>::create(
            modName, clangCompiler, modules, searchIncludePaths, parserOptions, pcmOutputDir, abortFunction);
        job->assignQueuePolicy(&ThreadWeaver::DependencyPolicy::instance());
        jobMap.insert(modName, job);
        jobList.append(job);
    }

    for (auto it = jobMap.constBegin(); it != jobMap.constEnd(); ++it) {
        const QString modName = it.key();
        auto mod = modules.value(modName);
        if (!mod) {
            continue;
        }
        auto consumerJob = it.value();
        for (const auto& depName : mod->dependencies()) {
            if (jobMap.contains(depName)) {
                auto depJob = jobMap.value(depName);
                ThreadWeaver::DependencyPolicy::instance().addDependency(consumerJob, depJob);
            }
        }
    }

    return jobList;
}
