/*
    SPDX-FileCopyrightText: 2026 Fernando Vilas <fvilas@iname.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CLANGDEPENDENCYSCANNER_H
#define CLANGDEPENDENCYSCANNER_H

#include "clangprivateexport.h"

#include <language/interfaces/idependencyscanner.h>
#include <serialization/indexedstring.h>

#include <QHash>
#include <QMutex>
#include <QStringList>

/**
 * C++ / Clang implementation of KDevelop::IDependencyScanner.
 *
 * Dependency scanning (clang-scan-deps) and BMI precompilation (clang++ --precompile)
 * execute exclusively inside independent ThreadWeaver jobs returned by
 * createDependencyScanJob(). Results are persisted in ClangModuleRepository (an
 * ItemRepository-backed store), so cachedCompileFlags(), cachedModuleArguments(), and
 * cachedProblems() are fast, thread-safe cache reads that never launch external processes.
 */
class KDEVCLANGPRIVATE_EXPORT ClangDependencyScanner : public KDevelop::IDependencyScanner
{
public:
    struct ScanConfig
    {
        QString buildDir;
        QStringList searchIncludePaths;
        QString parserOptions;
        QStringList compileArgs;
    };

    ClangDependencyScanner();
    ~ClangDependencyScanner() override;

    /**
     * Optional per-file configuration override (primarily for tests or callers
     * that already resolved project build directories and include paths).
     */
    void setScanConfig(const KDevelop::IndexedString& url, const ScanConfig& config);
    void clearScanConfigs();

    ThreadWeaver::JobPointer createDependencyScanJob(const KDevelop::IndexedString& url) override;

    QStringList cachedCompileFlags(const KDevelop::IndexedString& url) const override;
    QVector<QByteArray> cachedModuleArguments(const KDevelop::IndexedString& url) const override;
    QList<KDevelop::ProblemPointer> cachedProblems(const KDevelop::IndexedString& url) const override;

    void invalidate(const KDevelop::IndexedString& url) override;

    void setProblems(const KDevelop::IndexedString& url, const QList<KDevelop::ProblemPointer>& problems);

private:
    ScanConfig resolveConfig(const KDevelop::IndexedString& url) const;

    mutable QMutex m_mutex;
    QHash<KDevelop::IndexedString, ScanConfig> m_configs;
    QHash<KDevelop::IndexedString, QList<KDevelop::ProblemPointer>> m_problems;
};

#endif // CLANGDEPENDENCYSCANNER_H
