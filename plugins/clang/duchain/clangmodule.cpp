/*
    SPDX-FileCopyrightText: 2026 Fernando Vilas <fvilas@iname.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "clangmodule.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUrl>

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <language/editor/documentrange.h>

#include <functional>

#include "clanghelpers.h"
#include "clangmodulegraph.h"
#include "clangmoduleprecompilejob.h"
#include "clangmodulerepository.h"
#include "clangmodulescanner.h"
#include <ThreadWeaver/Queue>
#include "clangsettings/clangsettingsmanager.h"
#include "util/clangdebug.h"
#include "util/clangtypes.h"

ClangModule::ClangModule(const QString& name, const QString& sourceFile,
                         const QString& workingDir, const QStringList& compileArgs,
                         const QStringList& dependencies)
    : m_name(name)
    , m_sourceFile(sourceFile)
    , m_workingDir(workingDir)
    , m_compileArgs(compileArgs)
    , m_dependencies(dependencies)
{
}

QString ClangModule::name() const
{
    return m_name;
}

QString ClangModule::sourceFile() const
{
    return m_sourceFile;
}

QString ClangModule::workingDir() const
{
    return m_workingDir;
}

QString ClangModule::pcmPath(const QString& pcmOutputDir) const
{
    return pcmOutputDir + QLatin1Char('/') + m_name + QStringLiteral(".pcm");
}

QStringList ClangModule::compileArgs() const
{
    return m_compileArgs;
}

QStringList ClangModule::dependencies() const
{
    return m_dependencies;
}

bool ClangModule::isUpToDate(const QString& pcmOutputDir) const
{
    const QString pcm = pcmPath(pcmOutputDir);
    const QFileInfo pcmInfo(pcm);
    const QFileInfo sourceInfo(m_sourceFile);
    return pcmInfo.exists() && pcmInfo.lastModified() >= sourceInfo.lastModified();
}

bool ClangModule::isCppModuleUnit(const QString& path, const QVector<UnsavedFile>& unsavedFiles)
{
    // Note: A lightweight preamble inspection is used instead of a libclang CXCursor
    // because this check must run before constructing compiler arguments for
    // clang_parseTranslationUnit (e.g. to suppress incompatible -include PCH flags),
    // and libclang's C API lacks a dedicated CXCursor kind for C++20 module declarations.
    QByteArray header;
    for (const auto& unsavedFile : unsavedFiles) {
        const CXUnsavedFile file = unsavedFile.toClangApi();
        if (QString::fromUtf8(file.Filename) == path) {
            header = QByteArray(file.Contents, static_cast<int>(std::min<unsigned long>(file.Length, 4096)));
            break;
        }
    }
    if (header.isEmpty()) {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            header = file.read(4096);
        }
    }
    if (header.isEmpty()) {
        return false;
    }

    // Match any leading whitespace, line comments (//...), or block comments (/*...*/),
    // followed by a C++20 module declaration: "module;", "export module", or "module <name>".
    static const QRegularExpression moduleDeclarationRegex(
        QStringLiteral(R"(^(?:\s+|//[^\n]*|/\*[\s\S]*?\*/)*(?:export\s+module\b|module\s*[:;]|module\s+[a-zA-Z_]))"));
    return moduleDeclarationRegex.match(QString::fromUtf8(header)).hasMatch();
}

QString ClangModule::findStandardModuleSource(const QString& modName, const QStringList& searchIncludePaths)
{
    const bool isCompat = (modName == QLatin1String("std.compat"));
    const QString libstdcxxFile = isCompat ? QStringLiteral("bits/std.compat.cc") : QStringLiteral("bits/std.cc");
    const QString libcxxFile = isCompat ? QStringLiteral("std.compat.cppm") : QStringLiteral("std.cppm");

    QStringList candidatePaths = searchIncludePaths;
    for (const auto& sysPath : {QStringLiteral("/usr/include"), QStringLiteral("/usr/local/include")}) {
        if (!candidatePaths.contains(sysPath)) {
            candidatePaths << sysPath;
        }
    }

    for (const QString& incPath : candidatePaths) {
        if (incPath.isEmpty()) {
            continue;
        }

        // 1. Direct candidate checks (prefer Clang's libc++, fallback to GCC's libstdc++)
        const QString clangCandidate = incPath + QLatin1Char('/') + libcxxFile;
        if (QFile::exists(clangCandidate)) {
            return clangCandidate;
        }
        const QString gccCandidate = incPath + QLatin1Char('/') + libstdcxxFile;
        if (QFile::exists(gccCandidate)) {
            return gccCandidate;
        }

        // 2. Check c++ subdirectory if incPath is a parent include directory (e.g. /usr/include or custom prefix/include)
        const QDir incDir(incPath);
        if (incDir.exists(QStringLiteral("c++"))) {
            const QDir cppDir(incDir.filePath(QStringLiteral("c++")));
            const auto entries = cppDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::Reversed);
            for (const auto& entry : entries) {
                const QString nestedClang = cppDir.absoluteFilePath(entry + QLatin1Char('/') + libcxxFile);
                if (QFile::exists(nestedClang)) {
                    return nestedClang;
                }
                const QString nestedGcc = cppDir.absoluteFilePath(entry + QLatin1Char('/') + libstdcxxFile);
                if (QFile::exists(nestedGcc)) {
                    return nestedGcc;
                }
            }
        }
    }
    return {};
}

QString ClangModule::selectStdVersionFlag(const QString& candidateFlag, const Map& modules)
{
    QString bestFlag;
    auto bestRank = ClangHelpers::CppStdVersion::Cpp23;

    auto considerArgs = [&](const QString& args) {
        const QString flag = ClangHelpers::extractStdFlag(args);
        if (flag.isEmpty()) {
            return;
        }
        const auto rank = ClangHelpers::stdVersionRank(flag);
        if (rank > bestRank || (bestFlag.isEmpty() && rank == bestRank)) {
            bestRank = rank;
            bestFlag = flag;
        }
    };

    considerArgs(candidateFlag);
    for (const auto& mod : modules) {
        if (mod) {
            for (const QString& arg : mod->compileArgs()) {
                considerArgs(arg);
            }
        }
    }

    return bestFlag.isEmpty() ? QStringLiteral("-std=c++23") : bestFlag;
}

ClangModule::Ptr ClangModule::standardLibraryModule(const QString& modName,
                                                    const QStringList& searchIncludePaths,
                                                    const QString& stdVersionFlag)
{
    const QString source = findStandardModuleSource(modName, searchIncludePaths);
    if (source.isEmpty()) {
        return {};
    }
    QStringList compileArgs{stdVersionFlag, QStringLiteral("-fPIC")};
    const QFileInfo fi(source);
    const QDir parentDir = fi.dir();
    if (source.endsWith(QLatin1String(".cppm"))) {
        compileArgs << (QStringLiteral("-I") + parentDir.canonicalPath());
    } else {
        const QDir cppVersionDir(parentDir.filePath(QStringLiteral("..")));
        if (cppVersionDir.exists()) {
            compileArgs << (QStringLiteral("-I") + cppVersionDir.canonicalPath());
            const QString version = cppVersionDir.dirName();
            const QDir baseDir(cppVersionDir.filePath(QStringLiteral("../..")));
            const auto tripleEntries = baseDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            for (const auto& triple : tripleEntries) {
                const QString tripleCpp = baseDir.filePath(triple + QStringLiteral("/c++/") + version);
                if (QDir(tripleCpp).exists()) {
                    compileArgs << (QStringLiteral("-I") + tripleCpp);
                }
            }
        }
    }
    return Ptr::create(modName, source, QString(), compileArgs);
}

static QHash<QString, ClangModule::Ptr> s_moduleRegistry;
static QRecursiveMutex s_registryMutex;
static QHash<QString, QSharedPointer<QRecursiveMutex>> s_moduleMutexes;

void ClangModule::registerModule(const Ptr& module)
{
    if (!module) {
        return;
    }
    QMutexLocker locker(&s_registryMutex);
    s_moduleRegistry.insert(module->name(), module);

    qint64 mtime = 0;
    if (!module->sourceFile().isEmpty()) {
        const QFileInfo fi(module->sourceFile());
        if (fi.exists()) {
            mtime = fi.lastModified().toMSecsSinceEpoch();
        }
    }

    ClangModuleRepository::ModuleMetadata meta;
    meta.moduleName = KDevelop::IndexedString(module->name());
    meta.sourceFile = KDevelop::IndexedString(module->sourceFile());
    meta.workingDir = KDevelop::IndexedString(module->workingDir());
    meta.sourceModificationTime = mtime;
    meta.dependencies.reserve(module->dependencies().size());
    for (const QString& dep : module->dependencies()) {
        meta.dependencies.append(KDevelop::IndexedString(dep));
    }

    ClangModuleRepository::self().insert(meta);
}

ClangModule::Ptr ClangModule::registeredModule(const QString& name)
{
    QMutexLocker locker(&s_registryMutex);
    const KDevelop::IndexedString indexedName(name);

    // 1. In-memory cache validation: if already loaded in memory, verify that
    // the source file on disk has not been modified since registration.
    // If it was modified, evict the stale entry from the registry and return null.
    auto it = s_moduleRegistry.constFind(name);
    if (it != s_moduleRegistry.constEnd() && it.value()) {
        const auto& mod = it.value();
        const QFileInfo fi(mod->sourceFile());
        if (fi.exists()) {
            const auto meta = ClangModuleRepository::self().find(indexedName);
            if (meta && meta->sourceModificationTime != 0
                && meta->sourceModificationTime != fi.lastModified().toMSecsSinceEpoch()) {
                s_moduleRegistry.erase(it);
                return {};
            }
        }
        return mod;
    }

    // 2. Persistent disk-cache rehydration: if not found in memory, query the
    // persistent ClangModuleRepository. If a matching entry exists and its
    // source file modification time matches disk, reconstruct the module instance,
    // populate the in-memory cache, and return it.
    const auto meta = ClangModuleRepository::self().find(indexedName);
    if (meta) {
        const QString source = meta->sourceFile.str();
        const QFileInfo fi(source);
        if (fi.exists() && fi.lastModified().toMSecsSinceEpoch() == meta->sourceModificationTime) {
            QStringList deps;
            deps.reserve(meta->dependencies.size());
            for (const auto& d : meta->dependencies) {
                deps.append(d.str());
            }
            auto mod = Ptr::create(meta->moduleName.str(), source, meta->workingDir.str(),
                                   QStringList{}, deps);
            s_moduleRegistry.insert(name, mod);
            return mod;
        }
    }

    return {};
}

void ClangModule::clearModules()
{
    QMutexLocker locker(&s_registryMutex);
    s_moduleMutexes.clear();
    s_moduleRegistry.clear();
    ClangModuleRepository::self().clear();
}

ClangModule::Ptr ClangModule::scanAndRegisterModuleFile(const QString& sourceFile,
                                                       const QStringList& compileArgs,
                                                       const QString& workingDir,
                                                       const std::function<bool()>& abortFunction)
{
    const auto scanResult = ClangModuleScanner::scanFile(sourceFile, compileArgs, workingDir, abortFunction);
    if (!scanResult.success) {
        return {};
    }

    Ptr primary;
    for (auto it = scanResult.modules.constBegin(); it != scanResult.modules.constEnd(); ++it) {
        const auto& desc = it.value();
        auto mod = Ptr::create(desc.name, desc.sourceFile, workingDir, compileArgs, desc.requiredModules);
        registerModule(mod);
        if (!primary) {
            primary = mod;
        }
    }
    return primary;
}

QStringList ClangModule::transitiveDependencies(const QStringList& rootModules, const Map& allModules)
{
    QStringList result;
    QSet<QString> visited;
    QStringList worklist = rootModules;

    while (!worklist.isEmpty()) {
        const QString modName = worklist.takeFirst();
        if (visited.contains(modName)) {
            continue;
        }
        visited.insert(modName);
        result.append(modName);

        auto it = allModules.constFind(modName);
        if (it != allModules.constEnd() && it.value()) {
            for (const QString& dep : it.value()->dependencies()) {
                if (!visited.contains(dep)) {
                    worklist.append(dep);
                }
            }
        }
    }
    return result;
}

QStringList ClangModule::precompileArguments(const Map& allModules,
                                             const QStringList& searchIncludePaths,
                                             const QString& userStdFlag,
                                             const QString& pcmOutputDir) const
{
    QString stdArg;
    for (const QString& arg : m_compileArgs) {
        if (const QString flag = ClangHelpers::extractStdFlag(arg);
            !flag.isEmpty() && ClangHelpers::stdVersionRank(flag) >= ClangHelpers::CppStdVersion::Cpp20) {
            stdArg = flag;
            break;
        }
    }
    if (stdArg.isEmpty()) {
        if (const QString flag = ClangHelpers::extractStdFlag(userStdFlag);
            !flag.isEmpty() && ClangHelpers::stdVersionRank(flag) >= ClangHelpers::CppStdVersion::Cpp20) {
            stdArg = flag;
        }
    }
    if (stdArg.isEmpty()) {
        stdArg = QStringLiteral("-std=c++20");
    }

    QStringList procArgs;
    procArgs << stdArg
             << QStringLiteral("--precompile")
             << QStringLiteral("-x") << QStringLiteral("c++-module")
             << QStringLiteral("-Wno-reserved-module-identifier");

    for (int j = 0; j < m_compileArgs.size(); ++j) {
        const QString& arg = m_compileArgs.at(j);
        if (arg.startsWith(QLatin1String("-I"))
            || arg.startsWith(QLatin1String("-isystem"))
            || arg.startsWith(QLatin1String("-D"))
            || arg.startsWith(QLatin1String("-fPIC"), Qt::CaseInsensitive)
            || arg.startsWith(QLatin1String("-fPIE"), Qt::CaseInsensitive)
            || arg.startsWith(QLatin1String("-fvisibility"))
            || arg == QLatin1String("-fexceptions"))
        {
            procArgs << arg;
            if ((arg == QLatin1String("-I") || arg == QLatin1String("-isystem") || arg == QLatin1String("-D")) && j + 1 < m_compileArgs.size()) {
                procArgs << m_compileArgs.at(++j);
            }
        }
    }

    for (const QString& incPath : searchIncludePaths) {
        if (!incPath.isEmpty()) {
            const QString flag = QStringLiteral("-I") + incPath;
            if (!procArgs.contains(flag)) {
                procArgs << flag;
            }
        }
    }

    const QStringList allDeps = transitiveDependencies(m_dependencies, allModules);
    for (const QString& dep : allDeps) {
        auto depModule = allModules.value(dep);
        const QString depPcm = depModule ? depModule->pcmPath(pcmOutputDir)
                                         : (pcmOutputDir + QLatin1Char('/') + dep + QStringLiteral(".pcm"));
        if (QFile::exists(depPcm)) {
            procArgs << QStringLiteral("-fmodule-file=%1=%2").arg(dep, depPcm);
        }
    }

    return procArgs;
}

bool ClangModule::precompile(const QString& clangCompiler,
                             Map& allModules,
                             const QStringList& searchIncludePaths,
                             const QString& userStdFlag,
                             const QString& pcmOutputDir,
                             QSet<QString>& building,
                             const std::function<bool()>& abortFunction,
                             QList<KDevelop::ProblemPointer>* problems)
{
    QSharedPointer<QRecursiveMutex> moduleMutex;
    {
        QMutexLocker regLocker(&s_registryMutex);
        moduleMutex = s_moduleMutexes.value(m_name);
        if (!moduleMutex) {
            moduleMutex = QSharedPointer<QRecursiveMutex>::create();
            s_moduleMutexes.insert(m_name, moduleMutex);
        }
    }
    QMutexLocker locker(moduleMutex.data());

    if (abortFunction && abortFunction()) {
        return false;
    }

    if (building.contains(m_name) || clangCompiler.isEmpty()) {
        return false;
    }

    if (isUpToDate(pcmOutputDir)) {
        return true;
    }

    building.insert(m_name);

    for (const QString& dep : m_dependencies) {
        if (abortFunction && abortFunction()) {
            building.remove(m_name);
            return false;
        }
        auto depModule = allModules.value(dep);
        if (!depModule && (dep == QLatin1String("std") || dep == QLatin1String("std.compat"))) {
            const QString stdFlag = selectStdVersionFlag(userStdFlag, allModules);
            depModule = standardLibraryModule(dep, searchIncludePaths, stdFlag);
            if (depModule) {
                allModules.insert(dep, depModule);
            }
        }
        if (depModule) {
            depModule->precompile(clangCompiler, allModules, searchIncludePaths, userStdFlag, pcmOutputDir, building, abortFunction, problems);
        }
    }

    building.remove(m_name);

    if (abortFunction && abortFunction()) {
        return false;
    }

    const QString pcm = pcmPath(pcmOutputDir);
    const QString tmpPcmFile = pcm + QStringLiteral(".tmp");
    QStringList procArgs = precompileArguments(allModules, searchIncludePaths, userStdFlag, pcmOutputDir);
    procArgs << QStringLiteral("-o") << tmpPcmFile;
    procArgs << m_sourceFile;

    QProcess proc;
    proc.setWorkingDirectory(m_workingDir);
    proc.start(clangCompiler, procArgs);
    if (!proc.waitForStarted(5000)) {
        QFile::remove(tmpPcmFile);
        return false;
    }

    const int maxWaitMs = 30000;
    int waitedMs = 0;
    while (!proc.waitForFinished(50)) {
        if (abortFunction && abortFunction()) {
            proc.kill();
            proc.waitForFinished(500);
            QFile::remove(tmpPcmFile);
            return false;
        }
        waitedMs += 50;
        if (waitedMs >= maxWaitMs) {
            proc.kill();
            proc.waitForFinished(500);
            QFile::remove(tmpPcmFile);
            qCWarning(KDEV_CLANG) << "Timed out precompiling module" << m_name;
            return false;
        }
    }

    if (proc.exitCode() != 0) {
        const QString err = QString::fromUtf8(proc.readAllStandardError());
        QFile::remove(tmpPcmFile);
        qCWarning(KDEV_CLANG) << "Failed to precompile module" << m_name << err;
        if (problems) {
            KDevelop::ProblemPointer problem(new KDevelop::Problem);
            problem->setSeverity(KDevelop::IProblem::Error);
            problem->setSource(KDevelop::IProblem::Disk);
            problem->setDescription(QStringLiteral("Failed to precompile module '%1'").arg(m_name));
            problem->setExplanation(err);
            problem->setFinalLocation(KDevelop::DocumentRange(KDevelop::IndexedString(m_sourceFile), KTextEditor::Range(0, 0, 0, 0)));
            problems->append(problem);
        }
        return false;
    }

    QFile::remove(pcm);
    if (!QFile::rename(tmpPcmFile, pcm)) {
        QFile::remove(tmpPcmFile);
        return false;
    }

    return QFile::exists(pcm);
}

QByteArray ClangModule::moduleFileArg(const QString& pcmOutputDir) const
{
    const QString pcm = pcmPath(pcmOutputDir);
    if (QFile::exists(pcm)) {
        return QByteArrayLiteral("-fmodule-file=") + m_name.toUtf8() + '=' + pcm.toUtf8();
    }
    return {};
}

QVector<QByteArray> ClangModule::precompileModules(const QStringList& requiredModules,
                                                   Map& modules,
                                                   const QStringList& searchIncludePaths,
                                                   const QString& parserOptions,
                                                   const QString& pcmOutputDir,
                                                   const std::function<bool()>& abortFunction,
                                                   QList<KDevelop::ProblemPointer>* problems)
{
    QVector<QByteArray> moduleArgs;
    if (requiredModules.isEmpty() || (abortFunction && abortFunction())) {
        return moduleArgs;
    }

    const QString clangCompiler = ClangHelpers::clangCompilerPath();
    if (clangCompiler.isEmpty()) {
        static bool warnedOnce = false;
        if (!warnedOnce) {
            warnedOnce = true;
            qCWarning(KDEV_CLANG) << "C++20 module support requires 'clang++' to generate module interfaces."
                                  << "Please install clang to enable module support.";
        }
        return moduleArgs;
    }

    const QString pcmDir = !pcmOutputDir.isEmpty()
        ? pcmOutputDir
        : ((KDevelop::ICore::self()
            ? KDevelop::ICore::self()->sessionTemporaryDirectoryPath()
            : (QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QStringLiteral("/kdev_pcm")))
            + QStringLiteral("/pcm"));
    QDir().mkpath(pcmDir);

    QStringList allIncludePaths = searchIncludePaths;
    const QString builtin = ClangHelpers::clangBuiltinIncludePath();
    if (!builtin.isEmpty() && !allIncludePaths.contains(builtin)) {
        allIncludePaths << builtin;
    }
    for (const auto& mod : modules) {
        if (!mod) {
            continue;
        }
        const auto compileArgs = mod->compileArgs();
        for (int j = 0; j < compileArgs.size(); ++j) {
            const QString& arg = compileArgs.at(j);
            if (arg.startsWith(QLatin1String("-isystem")) || arg.startsWith(QLatin1String("-I"))) {
                QString inc;
                if (arg == QLatin1String("-isystem") || arg == QLatin1String("-I")) {
                    if (j + 1 < compileArgs.size()) {
                        inc = compileArgs.at(++j);
                    }
                } else if (arg.startsWith(QLatin1String("-isystem"))) {
                    inc = arg.mid(QStringLiteral("-isystem").length());
                } else if (arg.startsWith(QLatin1String("-I"))) {
                    inc = arg.mid(QStringLiteral("-I").length());
                }
                if (!inc.isEmpty() && !allIncludePaths.contains(inc)) {
                    allIncludePaths << inc;
                }
            }
        }
    }

    // Ensure standard library modules (std, std.compat) are created if required
    auto ensureStdModule = [&](const QString& modName) {
        if ((modName == QLatin1String("std") || modName == QLatin1String("std.compat")) && !modules.contains(modName)) {
            const QString stdFlag = selectStdVersionFlag(parserOptions, modules);
            if (auto stdMod = standardLibraryModule(modName, allIncludePaths, stdFlag)) {
                modules.insert(modName, stdMod);
            }
        }
    };
    for (const QString& req : requiredModules) {
        ensureStdModule(req);
    }
    for (const auto& mod : std::as_const(modules)) {
        if (mod) {
            for (const QString& dep : mod->dependencies()) {
                ensureStdModule(dep);
            }
        }
    }

    const auto jobs = ClangModulePrecompileJob::createJobs(
        requiredModules, modules, clangCompiler, allIncludePaths, parserOptions, pcmDir, abortFunction, problems);
    if (jobs.isEmpty()) {
        return moduleArgs;
    }

    ThreadWeaver::Queue queue;
    for (const auto& job : jobs) {
        queue.enqueue(job);
    }
    queue.finish();

    for (const auto& job : jobs) {
        if (job->success()) {
            const QByteArray arg = job->moduleFileArg();
            if (!arg.isEmpty() && !moduleArgs.contains(arg)) {
                moduleArgs << arg;
            }
        } else {
            qCWarning(KDEV_CLANG) << "Failed to precompile module" << job->moduleName();
        }
        if (problems) {
            const auto jobProblems = job->problems();
            for (const auto& p : jobProblems) {
                if (!problems->contains(p)) {
                    problems->append(p);
                }
            }
        }
    }

    return moduleArgs;
}

void ClangModule::discoverProjectModules(const QString& sourceFile,
                                        const QStringList& searchIncludePaths,
                                        const QStringList& compileArgs,
                                        const QString& buildDir,
                                        const std::function<bool()>& abortFunction)
{
    if (abortFunction && abortFunction()) {
        return;
    }

    static const QStringList moduleExtensions = {
        QStringLiteral(".cppm"),
        QStringLiteral(".ixx"),
        QStringLiteral(".mxx"),
        QStringLiteral(".cxxm"),
    };

    QSet<QString> candidateFiles;

    // 1. If inside a KDevelop project, query project files
    if (auto core = KDevelop::ICore::self()) {
        if (auto pc = core->projectController()) {
            if (auto project = pc->findProjectForUrl(QUrl::fromLocalFile(sourceFile))) {
                const auto fileSet = project->fileSet();
                for (const auto& indexedPath : fileSet) {
                    const QString path = indexedPath.str();
                    if (path == sourceFile) {
                        continue;
                    }
                    const bool hasModuleExtension = std::any_of(
                        moduleExtensions.cbegin(), moduleExtensions.cend(), [&path](const QString& ext) {
                            return path.endsWith(ext, Qt::CaseInsensitive);
                        });
                    if (hasModuleExtension || isCppModuleUnit(path, {})) {
                        candidateFiles.insert(path);
                    }
                }
            }
        }
    }

    // 2. Also search directory of sourceFile and search include paths
    QStringList searchDirs;
    const QDir sourceDir = QFileInfo(sourceFile).dir();
    searchDirs << sourceDir.canonicalPath();
    for (const QString& inc : searchIncludePaths) {
        if (!inc.isEmpty() && !searchDirs.contains(inc)) {
            searchDirs << inc;
        }
    }

    QStringList nameFilters;
    nameFilters.reserve(moduleExtensions.size());
    for (const QString& ext : moduleExtensions) {
        nameFilters << QLatin1Char('*') + ext;
    }

    for (const QString& dirPath : searchDirs) {
        if (abortFunction && abortFunction()) {
            return;
        }
        const QDir dir(dirPath);
        if (!dir.exists()) {
            continue;
        }
        const auto entries = dir.entryInfoList(nameFilters, QDir::Files | QDir::Readable);
        for (const auto& fi : entries) {
            const QString absPath = fi.canonicalFilePath();
            if (absPath != sourceFile) {
                candidateFiles.insert(absPath);
            }
        }
    }

    // Build a map of cached source files to their recorded modification times
    QHash<QString, qint64> cachedFileMtimes;
    const auto allEntries = ClangModuleRepository::self().allModules();
    for (const auto& entryName : allEntries) {
        if (const auto meta = ClangModuleRepository::self().find(entryName)) {
            cachedFileMtimes.insert(meta->sourceFile.str(), meta->sourceModificationTime);
        }
    }

    // Scan each candidate file that isn't already up-to-date in ClangModuleRepository
    for (const QString& candidate : candidateFiles) {
        if (abortFunction && abortFunction()) {
            return;
        }
        const QFileInfo fi(candidate);
        if (cachedFileMtimes.value(candidate, -1) != fi.lastModified().toMSecsSinceEpoch()) {
            scanAndRegisterModuleFile(candidate, compileArgs, buildDir, abortFunction);
        }
    }
}

QVector<QByteArray> ClangModule::moduleArgumentsForFile(const QString& sourceFile,
                                                       const QString& buildDir,
                                                       const QStringList& searchIncludePaths,
                                                       const QString& parserOptions,
                                                       const QStringList& compileArgs,
                                                       const std::function<bool()>& abortFunction,
                                                       QList<KDevelop::ProblemPointer>* problems)
{
    if (!QFile::exists(sourceFile) || (abortFunction && abortFunction())) {
        return {};
    }

    QStringList args = compileArgs;
    const bool hasStdFlag = std::any_of(args.cbegin(), args.cend(), [](const QString& arg) {
        return !ClangHelpers::extractStdFlag(arg).isEmpty();
    });
    if (!hasStdFlag) {
        if (!parserOptions.isEmpty()) {
            args << parserOptions.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        } else {
            args << QStringLiteral("-std=c++20");
        }
    }
    for (const QString& inc : searchIncludePaths) {
        const QString incFlag = QStringLiteral("-I") + inc;
        if (!inc.isEmpty() && !args.contains(incFlag)) {
            args << incFlag;
        }
    }

    const auto scanRes = ClangModuleScanner::scanFile(sourceFile, args, buildDir, abortFunction);
    if (!scanRes.success || (abortFunction && abortFunction())) {
        return {};
    }

    // Register any module interface provided by this file
    for (auto it = scanRes.modules.constBegin(); it != scanRes.modules.constEnd(); ++it) {
        const auto& desc = it.value();
        registerModule(Ptr::create(desc.name, desc.sourceFile, buildDir, args, desc.requiredModules));
    }

    const QStringList requiredModules = scanRes.fileRequirements.value(sourceFile);
    if (requiredModules.isEmpty()) {
        return {};
    }

    // If any required module is not yet registered, search the project/include paths
    const bool needDiscovery = std::any_of(requiredModules.cbegin(), requiredModules.cend(), [](const QString& req) {
        return req != QLatin1String("std") && req != QLatin1String("std.compat") && !registeredModule(req);
    });
    if (needDiscovery) {
        discoverProjectModules(sourceFile, searchIncludePaths, args, buildDir, abortFunction);
    }

    Map modules;
    {
        QMutexLocker locker(&s_registryMutex);
        modules = s_moduleRegistry;
    }

    return precompileModules(requiredModules, modules, searchIncludePaths, parserOptions, {}, abortFunction, problems);
}
