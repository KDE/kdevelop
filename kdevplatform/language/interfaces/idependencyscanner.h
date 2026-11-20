/*
    SPDX-FileCopyrightText: 2026 Fernando Vilas <fvilas@iname.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_IDEPENDENCYSCANNER_H
#define KDEVPLATFORM_IDEPENDENCYSCANNER_H

#include <QByteArray>
#include <QList>
#include <QStringList>
#include <QVector>

#include <ThreadWeaver/JobPointer>

#include <language/duchain/problem.h>
#include <language/languageexport.h>

namespace KDevelop {

class IndexedString;

/**
 * Interface for language- or build-system-specific dependency scanning and
 * per-source-file build metadata (such as C++20 module dependencies and
 * source-file-level compiler flags).
 *
 * Implementations run expensive scanning or precompilation in independent
 * background jobs (scheduled separately from ParseJobs) and persist results
 * in an ItemRepository-backed cache so ParseJobs can query cached metadata
 * synchronously without launching external processes.
 */
class KDEVPLATFORMLANGUAGE_EXPORT IDependencyScanner
{
public:
    IDependencyScanner();
    virtual ~IDependencyScanner();

    /**
     * Create an independent background job to scan dependencies (and e.g.
     * precompile module interfaces) for @p url.
     *
     * Returns a null JobPointer if no scan is needed or the cached entry is
     * already up to date.
     */
    virtual ThreadWeaver::JobPointer createDependencyScanJob(const IndexedString& url) = 0;

    /**
     * Query cached per-source-file compiler/parser flags for @p url.
     * Implementations must NOT launch external processes from this method.
     */
    virtual QStringList cachedCompileFlags(const IndexedString& url) const = 0;

    /**
     * Query cached module-import compiler arguments (e.g. -fmodule-file=...)
     * for @p url. Implementations must NOT launch external processes from this
     * method.
     */
    virtual QVector<QByteArray> cachedModuleArguments(const IndexedString& url) const = 0;

    /**
     * Query any cached diagnostics/problems from the last dependency scan
     * for @p url.
     */
    virtual QList<ProblemPointer> cachedProblems(const IndexedString& url) const = 0;

    /**
     * Mark @p url as needing a rescan (e.g. when saved to disk or when build
     * system configuration changes).
     */
    virtual void invalidate(const IndexedString& url) = 0;
};

}

#endif // KDEVPLATFORM_IDEPENDENCYSCANNER_H
