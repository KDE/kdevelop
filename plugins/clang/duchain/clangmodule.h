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

#include <clang-c/CXCompilationDatabase.h>

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
                const QStringList& dependencies = {}, const QString& existingPcm = {});
    ~ClangModule() = default;

    QString name() const;
    QString sourceFile() const;
    QString workingDir() const;
    QString existingPcm() const;
    QString pcmPath(const QString& pcmOutputDir) const;
    QStringList compileArgs() const;
    QStringList dependencies() const;

    bool isUpToDate(const QString& pcmOutputDir) const;

    bool precompile(const QString& clangCompiler,
                    Map& allModules,
                    const QStringList& searchIncludePaths,
                    const QString& userStdFlag,
                    const QString& pcmOutputDir,
                    QSet<QString>& building);

    QByteArray moduleFileArg(const QString& pcmOutputDir) const;

    // Static Utility Methods
    static int stdVersionRank(const QString& flag);
    static bool isCpp20OrLater(const ParserSettings& parserSettings);
    static bool isCppModuleUnit(const QString& path, const QVector<UnsavedFile>& unsavedFiles);
    static QString findStandardModuleSource(const QString& modName, const QStringList& searchIncludePaths);
    static QString selectStdVersionFlag(const QString& candidateFlag, const Map& modules);
    static Map scanGccModules(CXCompilationDatabase db, const QString& buildDir);
    static Ptr standardLibraryModule(const QString& modName, const QStringList& searchIncludePaths, const QString& stdVersionFlag);
    static QVector<QByteArray> processGccModmap(const QString& gccModmapPath,
                                                CXCompilationDatabase db,
                                                const QString& buildDir,
                                                bool isModule,
                                                const QStringList& searchIncludePaths,
                                                const QString& parserOptions);

private:
    QString m_name;
    QString m_sourceFile;
    QString m_workingDir;
    QString m_existingPcm;
    QStringList m_compileArgs;
    QStringList m_dependencies;
};

#endif // CLANGMODULE_H
