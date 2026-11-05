/*
    SPDX-FileCopyrightText: 2026 Fernando Vilas <fvilas@iname.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "clangmodule.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTextStream>

#include <interfaces/icore.h>

#include <clang-c/Index.h>

#include <functional>

#include "clanghelpers.h"
#include "clangsettings/clangsettingsmanager.h"
#include "util/clangdebug.h"
#include "util/clangtypes.h"

ClangModule::ClangModule(const QString& name, const QString& sourceFile,
                         const QString& workingDir, const QStringList& compileArgs,
                         const QStringList& dependencies, const QString& existingPcm)
    : m_name(name)
    , m_sourceFile(sourceFile)
    , m_workingDir(workingDir)
    , m_existingPcm(existingPcm)
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

QString ClangModule::existingPcm() const
{
    return m_existingPcm;
}

QString ClangModule::pcmPath(const QString& pcmOutputDir) const
{
    if (!m_existingPcm.isEmpty()) {
        return m_existingPcm;
    }
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
    if (!m_existingPcm.isEmpty() && QFile::exists(m_existingPcm)) {
        return true;
    }
    const QString pcm = pcmPath(pcmOutputDir);
    const QFileInfo pcmInfo(pcm);
    const QFileInfo sourceInfo(m_sourceFile);
    return pcmInfo.exists() && pcmInfo.lastModified() >= sourceInfo.lastModified();
}

int ClangModule::stdVersionRank(const QString& flag)
{
    static const QRegularExpression stdRegex(QStringLiteral("-std=(?:c|gnu)\\+\\+([0-9a-z]+)"));
    const auto match = stdRegex.match(flag);
    if (!match.hasMatch()) {
        return 0;
    }
    const QString ver = match.captured(1).toLower();
    if (ver == QLatin1String("latest")) {
        return 99;
    }
    if (ver == QLatin1String("2a")) {
        return 20;
    }
    if (ver == QLatin1String("2b")) {
        return 23;
    }
    if (ver == QLatin1String("2c")) {
        return 26;
    }
    bool ok = false;
    const int val = ver.toInt(&ok);
    return (ok && val >= 11 && val < 90) ? val : 0;
}

bool ClangModule::isCpp20OrLater(const ParserSettings& parserSettings)
{
    return parserSettings.isCpp() && stdVersionRank(parserSettings.parserOptions) >= 20;
}

bool ClangModule::isCppModuleUnit(const QString& path, const QVector<UnsavedFile>& unsavedFiles)
{
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

        // 1. Direct candidate checks (e.g. <sys-include>/c++/<version>/bits/std.cc or libc++ include path)
        const QString gccCandidate = incPath + QLatin1Char('/') + libstdcxxFile;
        if (QFile::exists(gccCandidate)) {
            return gccCandidate;
        }
        const QString clangCandidate = incPath + QLatin1Char('/') + libcxxFile;
        if (QFile::exists(clangCandidate)) {
            return clangCandidate;
        }

        // 2. Check c++ subdirectory if incPath is a parent include directory (e.g. /usr/include or custom prefix/include)
        const QDir incDir(incPath);
        if (incDir.exists(QStringLiteral("c++"))) {
            const QDir cppDir(incDir.filePath(QStringLiteral("c++")));
            const auto entries = cppDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::Reversed);
            for (const auto& entry : entries) {
                const QString nestedGcc = cppDir.absoluteFilePath(entry + QLatin1Char('/') + libstdcxxFile);
                if (QFile::exists(nestedGcc)) {
                    return nestedGcc;
                }
                const QString nestedClang = cppDir.absoluteFilePath(entry + QLatin1Char('/') + libcxxFile);
                if (QFile::exists(nestedClang)) {
                    return nestedClang;
                }
            }
        }
    }
    return {};
}

QString ClangModule::selectStdVersionFlag(const QString& candidateFlag, const Map& modules)
{
    QString bestFlag = QStringLiteral("-std=c++23");
    int bestRank = 23;

    auto considerFlag = [&](const QString& flag) {
        const int rank = stdVersionRank(flag);
        if (rank > bestRank) {
            bestRank = rank;
            bestFlag = flag;
        } else if (rank == bestRank && bestFlag == QStringLiteral("-std=c++23") && flag.startsWith(QLatin1String("-std="))) {
            bestFlag = flag;
        }
    };

    if (!candidateFlag.isEmpty()) {
        static const QRegularExpression stdRegex(QStringLiteral("-std=(?:c|gnu)\\+\\+[0-9a-z]+"));
        const auto match = stdRegex.match(candidateFlag);
        if (match.hasMatch()) {
            considerFlag(match.captured(0));
        }
    }

    for (const auto& mod : modules) {
        if (!mod) {
            continue;
        }
        for (const QString& arg : mod->compileArgs()) {
            if (arg.startsWith(QLatin1String("-std=c++")) || arg.startsWith(QLatin1String("-std=gnu++"))) {
                considerFlag(arg);
            }
        }
    }

    return bestFlag;
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
    return Ptr::create(modName, source, QString(), compileArgs);
}

ClangModule::Map ClangModule::scanGccModules(CXCompilationDatabase db, const QString& buildDir)
{
    Map targets;
    CXCompileCommands allCmds = clang_CompilationDatabase_getAllCompileCommands(db);
    if (!allCmds) {
        return targets;
    }
    const unsigned numCmds = clang_CompileCommands_getSize(allCmds);

    for (unsigned i = 0; i < numCmds; ++i) {
        CXCompileCommand cmd = clang_CompileCommands_getCommand(allCmds, i);
        const unsigned numArgs = clang_CompileCommand_getNumArgs(cmd);

        QString modmapPath;
        QStringList allArgs;
        allArgs.reserve(numArgs);

        for (unsigned j = 0; j < numArgs; ++j) {
            ClangString argStr(clang_CompileCommand_getArg(cmd, j));
            const QString arg = QString::fromUtf8(argStr.c_str());
            allArgs << arg;
            if (arg.startsWith(QLatin1String("-fmodule-mapper="))) {
                modmapPath = arg.mid(QStringLiteral("-fmodule-mapper=").length());
            } else if (arg == QLatin1String("-fmodule-mapper") && j + 1 < numArgs) {
                ClangString nextArgStr(clang_CompileCommand_getArg(cmd, j + 1));
                modmapPath = QString::fromUtf8(nextArgStr.c_str());
            }
        }

        if (modmapPath.isEmpty()) {
            continue;
        }

        modmapPath.remove(QLatin1Char('\"')).remove(QLatin1Char('\''));

        ClangString dirStr(clang_CompileCommand_getDirectory(cmd));
        const QString cmdDir = QString::fromUtf8(dirStr.c_str());
        QDir workingDir(cmdDir.isEmpty() ? buildDir : cmdDir);
        if (QDir::isRelativePath(modmapPath)) {
            modmapPath = workingDir.absoluteFilePath(modmapPath);
        }

        QFile modmapFile(modmapPath);
        if (!modmapFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            continue;
        }

        ClangString fileStr(clang_CompileCommand_getFilename(cmd));
        const QString sourceFile = QString::fromUtf8(fileStr.c_str());

        if (!isCppModuleUnit(sourceFile, {})) {
            continue;
        }

        QString primaryModule;
        QString existingPcm;
        QStringList deps;

        QTextStream stream(&modmapFile);
        while (!stream.atEnd()) {
            const QString line = stream.readLine().trimmed();
            if (line.isEmpty() || line.startsWith(QLatin1Char('$'))) {
                continue;
            }
            const QStringList parts = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
            if (parts.isEmpty()) {
                continue;
            }
            if (primaryModule.isEmpty()) {
                primaryModule = parts.first();
                if (parts.size() > 1 && parts.at(1).endsWith(QLatin1String(".pcm"))) {
                    existingPcm = workingDir.absoluteFilePath(parts.at(1));
                }
            } else {
                deps << parts.first();
            }
        }

        if (!primaryModule.isEmpty()) {
            targets.insert(primaryModule, Ptr::create(primaryModule, sourceFile,
                                                      workingDir.absolutePath(), allArgs,
                                                      deps, existingPcm));
        }
    }
    clang_CompileCommands_dispose(allCmds);
    return targets;
}

bool ClangModule::precompile(const QString& clangCompiler,
                             Map& allModules,
                             const QStringList& searchIncludePaths,
                             const QString& userStdFlag,
                             const QString& pcmOutputDir,
                             QSet<QString>& building)
{
    static QRecursiveMutex s_pcmMutex;
    QMutexLocker locker(&s_pcmMutex);

    if (building.contains(m_name)) {
        return false;
    }

    if (isUpToDate(pcmOutputDir)) {
        return true;
    }

    building.insert(m_name);

    for (const QString& dep : m_dependencies) {
        auto depModule = allModules.value(dep);
        if (!depModule && (dep == QLatin1String("std") || dep == QLatin1String("std.compat"))) {
            const QString stdFlag = selectStdVersionFlag(userStdFlag, allModules);
            depModule = standardLibraryModule(dep, searchIncludePaths, stdFlag);
            if (depModule) {
                allModules.insert(dep, depModule);
            }
        }
        if (depModule) {
            depModule->precompile(clangCompiler, allModules, searchIncludePaths, userStdFlag, pcmOutputDir, building);
        }
    }

    building.remove(m_name);

    if (clangCompiler.isEmpty()) {
        return false;
    }

    QStringList procArgs;
    QString stdArg;
    for (const QString& arg : m_compileArgs) {
        if ((arg.startsWith(QLatin1String("-std=c++")) || arg.startsWith(QLatin1String("-std=gnu++")))
            && stdVersionRank(arg) >= 20) {
            stdArg = arg;
            break;
        }
    }
    if (stdArg.isEmpty()) {
        static const QRegularExpression stdRegex(QStringLiteral("-std=(?:c|gnu)\\+\\+[0-9a-z]+"));
        const auto match = stdRegex.match(userStdFlag);
        if (match.hasMatch() && stdVersionRank(match.captured(0)) >= 20) {
            stdArg = match.captured(0);
        }
    }
    if (stdArg.isEmpty()) {
        stdArg = QStringLiteral("-std=c++20");
    }
    procArgs << stdArg;
    procArgs << QStringLiteral("--precompile");
    procArgs << QStringLiteral("-x") << QStringLiteral("c++-module");
    procArgs << QStringLiteral("-Wno-reserved-module-identifier");

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
                ++j;
                procArgs << m_compileArgs.at(j);
            }
        }
    }

    QStringList allDeps;
    QSet<QString> visitedDeps;
    std::function<void(const QString&)> collectTransitiveDeps = [&](const QString& modName) {
        if (visitedDeps.contains(modName)) {
            return;
        }
        visitedDeps.insert(modName);
        auto mod = allModules.value(modName);
        if (mod) {
            for (const QString& d : mod->dependencies()) {
                collectTransitiveDeps(d);
            }
        }
        allDeps << modName;
    };

    for (const QString& dep : m_dependencies) {
        collectTransitiveDeps(dep);
    }

    for (const QString& dep : allDeps) {
        auto depModule = allModules.value(dep);
        const QString depPcm = depModule ? depModule->pcmPath(pcmOutputDir)
                                         : (pcmOutputDir + QLatin1Char('/') + dep + QStringLiteral(".pcm"));
        if (QFile::exists(depPcm)) {
            procArgs << QStringLiteral("-fmodule-file=%1=%2").arg(dep, depPcm);
        }
    }

    const QString pcm = pcmPath(pcmOutputDir);
    const QString tmpPcmFile = pcm + QStringLiteral(".tmp");
    procArgs << QStringLiteral("-o") << tmpPcmFile;
    procArgs << m_sourceFile;

    QProcess proc;
    proc.setWorkingDirectory(m_workingDir);
    proc.start(clangCompiler, procArgs);
    if (!proc.waitForFinished(30000) || proc.exitCode() != 0) {
        QFile::remove(tmpPcmFile);
        qCWarning(KDEV_CLANG) << "Failed to precompile module" << m_name << proc.readAllStandardError();
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

QVector<QByteArray> ClangModule::processGccModmap(const QString& gccModmapPath,
                                                  CXCompilationDatabase db,
                                                  const QString& buildDir,
                                                  bool isModule,
                                                  const QStringList& searchIncludePaths,
                                                  const QString& parserOptions)
{
    QVector<QByteArray> moduleArgs;
    QFile modmapFile(gccModmapPath);
    if (!modmapFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return moduleArgs;
    }

    QStringList neededModules;
    QTextStream stream(&modmapFile);
    while (!stream.atEnd()) {
        const QString line = stream.readLine().trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('$'))) {
            continue;
        }
        const QStringList parts = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (!parts.isEmpty()) {
            neededModules << parts.first();
        }
    }

    QStringList importedModules;
    for (int i = 0; i < neededModules.size(); ++i) {
        if (i == 0 && isModule) {
            continue;
        }
        importedModules << neededModules.at(i);
    }

    if (importedModules.isEmpty()) {
        return moduleArgs;
    }

    const QString clangCompiler = ClangHelpers::clangCompilerPath();
    if (clangCompiler.isEmpty()) {
        static bool warnedOnce = false;
        if (!warnedOnce) {
            warnedOnce = true;
            qCWarning(KDEV_CLANG) << "C++20 module support for GCC projects requires 'clang++' to generate module interfaces."
                                  << "Please install clang to enable module support.";
        }
        return moduleArgs;
    }

    const QString pcmDir = (KDevelop::ICore::self()
        ? KDevelop::ICore::self()->sessionTemporaryDirectoryPath()
        : (QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QStringLiteral("/kdev_pcm")))
        + QStringLiteral("/pcm");
    QDir().mkpath(pcmDir);
    auto modules = scanGccModules(db, buildDir);

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

    QStringList allModules;
    QSet<QString> visited;
    std::function<void(const QString&)> collectTransitive = [&](const QString& mod) {
        if (visited.contains(mod)) {
            return;
        }
        visited.insert(mod);
        auto it = modules.constFind(mod);
        if (it != modules.constEnd() && it.value()) {
            for (const QString& dep : it.value()->dependencies()) {
                collectTransitive(dep);
            }
        }
        allModules << mod;
    };

    for (const QString& modName : importedModules) {
        collectTransitive(modName);
    }

    QSet<QString> building;
    QSet<QString> addedModules;
    for (const QString& modName : allModules) {
        if (addedModules.contains(modName)) {
            continue;
        }
        addedModules.insert(modName);

        auto it = modules.constFind(modName);
        ClangModule::Ptr mod;
        if (it != modules.constEnd()) {
            mod = it.value();
        } else if (modName == QLatin1String("std") || modName == QLatin1String("std.compat")) {
            const QString stdFlag = selectStdVersionFlag(parserOptions, modules);
            mod = standardLibraryModule(modName, allIncludePaths, stdFlag);
            if (mod) {
                modules.insert(modName, mod);
            }
        }

        if (mod && mod->precompile(clangCompiler, modules, allIncludePaths, parserOptions, pcmDir, building)) {
            const QByteArray arg = mod->moduleFileArg(pcmDir);
            if (!arg.isEmpty()) {
                moduleArgs << arg;
            }
        }
    }

    return moduleArgs;
}
