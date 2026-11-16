/*
    SPDX-FileCopyrightText: 2026 Fernando Vilas <fvilas@iname.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CLANGMODULESCANNER_H
#define CLANGMODULESCANNER_H

#include "clangprivateexport.h"

#include <functional>

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QStringList>

class KDEVCLANGPRIVATE_EXPORT ClangModuleScanner
{
public:
    struct ModuleDescription
    {
        QString name;
        QString sourceFile;
        bool isInterface = true;
        QStringList requiredModules;
    };

    struct ScanResult
    {
        bool success = false;
        QString errorString;
        QHash<QString, ModuleDescription> modules;
        QHash<QString, QStringList> fileRequirements;
    };

    /**
     * Parse P1689 standard JSON produced by clang-scan-deps.
     * @param json Raw JSON data from clang-scan-deps.
     * @param outputToSourceMap Optional mapping from primary-output path to source file path.
     */
    static ScanResult parseP1689Json(const QByteArray& json,
                                     const QHash<QString, QString>& outputToSourceMap = {});

    /**
     * Run clang-scan-deps for a single source file.
     * @param sourceFile Source file to scan.
     * @param compileArgs Compiler arguments for the file.
     * @param workingDirectory Working directory for invocation.
     * @param abortFunction Optional cancellation callback.
     */
    static ScanResult scanFile(const QString& sourceFile,
                               const QStringList& compileArgs,
                               const QString& workingDirectory = {},
                               const std::function<bool()>& abortFunction = {});
};

#endif // CLANGMODULESCANNER_H
