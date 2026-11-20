/*
    SPDX-FileCopyrightText: 2026 Fernando Vilas <fvilas@iname.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CLANGMODULE_H
#define CLANGMODULE_H

#include <QString>
#include <QStringList>
#include <QHash>
#include <QSet>
#include <QSharedPointer>
#include <QVector>

#include <functional>

#include <language/duchain/problem.h>

#include "clangprivateexport.h"
#include "unsavedfile.h"
#include "clangsettings/clangsettingsmanager.h"

class KDEVCLANGPRIVATE_EXPORT ClangModule
{
public:
    using Ptr = QSharedPointer<ClangModule>;
    using Map = QHash<QString, Ptr>;

    ClangModule(const QString& name, const QString& sourceFile,
                const QString& workingDir, const QStringList& compileArgs,
                const QStringList& dependencies = {});
    ~ClangModule() = default;

    QString name() const;
    QString sourceFile() const;
    QString workingDir() const;
    QString pcmPath(const QString& pcmOutputDir) const;
    QStringList compileArgs() const;
    QStringList dependencies() const;

    bool isUpToDate(const QString& pcmOutputDir) const;

    bool precompile(const QString& clangCompiler,
                    Map& allModules,
                    const QStringList& searchIncludePaths,
                    const QString& userStdFlag,
                    const QString& pcmOutputDir,
                    QSet<QString>& building,
                    const std::function<bool()>& abortFunction = {},
                    QList<KDevelop::ProblemPointer>* problems = nullptr);

    QByteArray moduleFileArg(const QString& pcmOutputDir) const;

    // Static Utility Methods
    /**
     * Determines whether the given file or unsaved buffer is a C++20 module unit.
     */
    static bool isCppModuleUnit(const QString& path, const QVector<UnsavedFile>& unsavedFiles);

    static void registerModule(const Ptr& module);
    static Ptr registeredModule(const QString& name);
    static void clearModules();
    static Ptr scanAndRegisterModuleFile(const QString& sourceFile,
                                         const QStringList& compileArgs = {},
                                         const QString& workingDir = {},
                                         const std::function<bool()>& abortFunction = {});

    /**
     * Discovers and registers C++20 module interface files within the enclosing project
     * or search paths for @p sourceFile.
     */
    static void discoverProjectModules(const QString& sourceFile,
                                       const QStringList& searchIncludePaths,
                                       const QStringList& compileArgs = {},
                                       const QString& buildDir = {},
                                       const std::function<bool()>& abortFunction = {});

    /**
     * Precompiles the requested modules and their transitive dependencies,
     * returning the list of -fmodule-file=<name>=<path.pcm> arguments.
     */
    static QVector<QByteArray> precompileModules(const QStringList& requiredModules,
                                                 Map& modules,
                                                 const QStringList& searchIncludePaths,
                                                 const QString& parserOptions,
                                                 const QString& pcmOutputDir = {},
                                                 const std::function<bool()>& abortFunction = {},
                                                 QList<KDevelop::ProblemPointer>* problems = nullptr);

    /**
     * High-level helper: discovers module dependencies for sourceFile,
     * precompiles required modules, and returns -fmodule-file=... arguments.
     */
    static QVector<QByteArray> moduleArgumentsForFile(const QString& sourceFile,
                                                      const QString& buildDir,
                                                      const QStringList& searchIncludePaths,
                                                      const QString& parserOptions,
                                                      const QStringList& compileArgs = {},
                                                      const std::function<bool()>& abortFunction = {},
                                                      QList<KDevelop::ProblemPointer>* problems = nullptr);

private:
    QStringList precompileArguments(const Map& allModules,
                                    const QStringList& searchIncludePaths,
                                    const QString& userStdFlag,
                                    const QString& pcmOutputDir) const;
    static QStringList transitiveDependencies(const QStringList& rootModules, const Map& allModules);
    static QString findStandardModuleSource(const QString& modName, const QStringList& searchIncludePaths);
    static QString selectStdVersionFlag(const QString& candidateFlag, const Map& modules);
    static Ptr standardLibraryModule(const QString& modName, const QStringList& searchIncludePaths, const QString& stdVersionFlag);

    QString m_name;
    QString m_sourceFile;
    QString m_workingDir;
    QStringList m_compileArgs;
    QStringList m_dependencies;
};

#endif // CLANGMODULE_H
