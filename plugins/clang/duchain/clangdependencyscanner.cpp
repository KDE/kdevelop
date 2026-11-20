/*
    SPDX-FileCopyrightText: 2026 Fernando Vilas <fvilas@iname.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "clangdependencyscanner.h"

#include <QFileInfo>
#include <QMutexLocker>
#include <QUrl>

#include <ThreadWeaver/Job>

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>

#include "clanghelpers.h"
#include "clangmodule.h"
#include "clangmodulerepository.h"
#include "clangsettings/clangsettingsmanager.h"
#include "custom-definesandincludes/idefinesandincludesmanager.h"

using namespace KDevelop;

namespace {

class ClangDependencyScanJob : public ThreadWeaver::Job
{
public:
    ClangDependencyScanJob(ClangDependencyScanner* scanner,
                           const IndexedString& url,
                           ClangDependencyScanner::ScanConfig config)
        : m_scanner(scanner)
        , m_url(url)
        , m_config(std::move(config))
    {
    }

protected:
    void run(ThreadWeaver::JobPointer /*self*/, ThreadWeaver::Thread* /*thread*/) override
    {
        if (shouldAbort()) {
            return;
        }

        const QString localPath = m_url.toUrl().isLocalFile() ? m_url.toUrl().toLocalFile() : m_url.str();
        const QFileInfo fi(localPath);
        if (!fi.exists()) {
            return;
        }

        QList<ProblemPointer> problems;
        const auto moduleArgs = ClangModule::moduleArgumentsForFile(
            localPath,
            m_config.buildDir,
            m_config.searchIncludePaths,
            m_config.parserOptions,
            m_config.compileArgs,
            [this] { return shouldAbort(); },
            &problems);

        if (shouldAbort()) {
            return;
        }

        ClangModuleRepository::FileScanMetadata meta;
        meta.sourceFile = m_url;
        meta.sourceModificationTime = fi.lastModified().toMSecsSinceEpoch();

        for (const QString& arg : m_config.compileArgs) {
            meta.compileFlags.append(IndexedString(arg));
        }
        if (!m_config.parserOptions.isEmpty()) {
            const auto splitOpts = m_config.parserOptions.split(QLatin1Char(' '), Qt::SkipEmptyParts);
            for (const QString& opt : splitOpts) {
                const IndexedString indexedOpt(opt);
                if (!meta.compileFlags.contains(indexedOpt)) {
                    meta.compileFlags.append(indexedOpt);
                }
            }
        }
        for (const QByteArray& arg : moduleArgs) {
            meta.moduleArguments.append(IndexedString(QString::fromUtf8(arg)));
        }

        ClangModuleRepository::self().insertFileScan(meta);
        if (m_scanner) {
            m_scanner->setProblems(m_url, problems);
        }
    }

private:
    ClangDependencyScanner* m_scanner;
    IndexedString m_url;
    ClangDependencyScanner::ScanConfig m_config;
};

} // namespace

ClangDependencyScanner::ClangDependencyScanner() = default;

ClangDependencyScanner::~ClangDependencyScanner() = default;

void ClangDependencyScanner::setScanConfig(const IndexedString& url, const ScanConfig& config)
{
    QMutexLocker lock(&m_mutex);
    m_configs.insert(url, config);
}

void ClangDependencyScanner::clearScanConfigs()
{
    QMutexLocker lock(&m_mutex);
    m_configs.clear();
    m_problems.clear();
}

ClangDependencyScanner::ScanConfig ClangDependencyScanner::resolveConfig(const IndexedString& url) const
{
    {
        QMutexLocker lock(&m_mutex);
        auto it = m_configs.constFind(url);
        if (it != m_configs.constEnd()) {
            return it.value();
        }
    }

    ScanConfig config;
    const QString localPath = url.toUrl().isLocalFile() ? url.toUrl().toLocalFile() : url.str();

    if (auto* mgr = IDefinesAndIncludesManager::manager()) {
        const auto includes = mgr->includesInBackground(localPath);
        for (const auto& inc : includes) {
            if (!inc.isEmpty()) {
                config.searchIncludePaths << inc.toLocalFile();
            }
        }
        const QString bgArgs = mgr->parserArgumentsInBackground(localPath);
        if (!bgArgs.isEmpty()) {
            config.compileArgs = bgArgs.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        }
        const auto defines = mgr->definesInBackground(localPath);
        for (auto it = defines.constBegin(); it != defines.constEnd(); ++it) {
            config.compileArgs << (it.value().isEmpty()
                                       ? QStringLiteral("-D") + it.key()
                                       : QStringLiteral("-D%1=%2").arg(it.key(), it.value()));
        }
    }

    if (auto* settingsMgr = ClangSettingsManager::self()) {
        const auto settings = settingsMgr->parserSettings(localPath);
        config.parserOptions = settings.parserOptions;
    }

    if (ICore::self() && ICore::self()->projectController()) {
        const QUrl fileUrl = QUrl::fromLocalFile(localPath);
        if (auto* project = ICore::self()->projectController()->findProjectForUrl(fileUrl)) {
            if (auto* bsm = project->buildSystemManager()) {
                if (auto* item = project->projectItem()) {
                    config.buildDir = bsm->buildDirectory(item).toLocalFile();
                }
            }
        }
    }

    return config;
}

ThreadWeaver::JobPointer ClangDependencyScanner::createDependencyScanJob(const IndexedString& url)
{
    if (url.isEmpty()) {
        return {};
    }

    const QString localPath = url.toUrl().isLocalFile() ? url.toUrl().toLocalFile() : url.str();
    const QFileInfo fi(localPath);
    if (!fi.exists()) {
        return {};
    }

    const qint64 diskMtime = fi.lastModified().toMSecsSinceEpoch();
    if (const auto cached = ClangModuleRepository::self().findFileScan(url)) {
        if (cached->sourceModificationTime == diskMtime) {
            return {};
        }
    }

    ScanConfig config = resolveConfig(url);
    ParserSettings ps;
    ps.parserOptions = config.parserOptions;
    if (!config.parserOptions.isEmpty() && !ClangHelpers::isCpp20OrLater(ps)) {
        return {};
    }

    return ThreadWeaver::JobPointer(new ClangDependencyScanJob(this, url, std::move(config)));
}

QStringList ClangDependencyScanner::cachedCompileFlags(const IndexedString& url) const
{
    const auto cached = ClangModuleRepository::self().findFileScan(url);
    if (!cached) {
        return {};
    }
    QStringList flags;
    flags.reserve(cached->compileFlags.size());
    for (const auto& flag : cached->compileFlags) {
        flags << flag.str();
    }
    return flags;
}

QVector<QByteArray> ClangDependencyScanner::cachedModuleArguments(const IndexedString& url) const
{
    const auto cached = ClangModuleRepository::self().findFileScan(url);
    if (!cached) {
        return {};
    }
    QVector<QByteArray> args;
    args.reserve(cached->moduleArguments.size());
    for (const auto& arg : cached->moduleArguments) {
        args << arg.byteArray();
    }
    return args;
}

QList<ProblemPointer> ClangDependencyScanner::cachedProblems(const IndexedString& url) const
{
    QMutexLocker lock(&m_mutex);
    return m_problems.value(url);
}

void ClangDependencyScanner::invalidate(const IndexedString& url)
{
    ClangModuleRepository::self().removeFileScan(url);
    QMutexLocker lock(&m_mutex);
    m_problems.remove(url);
}

void ClangDependencyScanner::setProblems(const IndexedString& url, const QList<ProblemPointer>& problems)
{
    QMutexLocker lock(&m_mutex);
    if (problems.isEmpty()) {
        m_problems.remove(url);
    } else {
        m_problems.insert(url, problems);
    }
}
