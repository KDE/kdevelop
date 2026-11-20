/*
    SPDX-FileCopyrightText: 2026 Fernando Vilas <fvilas@iname.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CLANGMODULEPRECOMPILEJOB_H
#define CLANGMODULEPRECOMPILEJOB_H

#include <ThreadWeaver/Job>
#include <interfaces/iproblem.h>
#include "clangmodule.h"

#include <functional>

class KDEVCLANGPRIVATE_EXPORT ClangModulePrecompileJob : public ThreadWeaver::Job
{
public:
    using Ptr = QSharedPointer<ClangModulePrecompileJob>;

    explicit ClangModulePrecompileJob(const QString& moduleName,
                                     const QString& clangCompiler,
                                     const ClangModule::Map& allModules,
                                     const QStringList& searchIncludePaths,
                                     const QString& parserOptions,
                                     const QString& pcmOutputDir = {},
                                     const std::function<bool()>& abortFunction = {});

    ~ClangModulePrecompileJob() override;

    QString moduleName() const;
    bool success() const override;
    QByteArray moduleFileArg() const;
    QList<KDevelop::ProblemPointer> problems() const;

    /**
     * Creates a list of jobs for the required modules and their dependencies,
     * configured with ThreadWeaver::DependencyPolicy so that dependencies run before
     * consumers. If circular dependencies are detected, returns an empty list and records
     * a problem in @p problems.
     */
    static QList<QSharedPointer<ClangModulePrecompileJob>> createJobs(
        const QStringList& requiredModules,
        const ClangModule::Map& modules,
        const QString& clangCompiler,
        const QStringList& searchIncludePaths,
        const QString& parserOptions,
        const QString& pcmOutputDir = {},
        const std::function<bool()>& abortFunction = {},
        QList<KDevelop::ProblemPointer>* problems = nullptr);

protected:
    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread) override;

private:
    QString m_moduleName;
    QString m_clangCompiler;
    ClangModule::Map m_allModules;
    QStringList m_searchIncludePaths;
    QString m_parserOptions;
    QString m_pcmOutputDir;
    std::function<bool()> m_abortFunction;

    bool m_success = false;
    QByteArray m_moduleFileArg;
    QList<KDevelop::ProblemPointer> m_problems;
};

#endif // CLANGMODULEPRECOMPILEJOB_H
