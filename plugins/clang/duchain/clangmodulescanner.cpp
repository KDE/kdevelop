/*
    SPDX-FileCopyrightText: 2026 Fernando Vilas <fvilas@iname.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "clangmodulescanner.h"

#include <functional>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>

#include "clanghelpers.h"
#include "util/clangdebug.h"

ClangModuleScanner::ScanResult ClangModuleScanner::parseP1689Json(
    const QByteArray& json,
    const QHash<QString, QString>& outputToSourceMap)
{
    ScanResult result;
    if (json.isEmpty()) {
        result.errorString = QStringLiteral("Empty P1689 JSON input");
        return result;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        result.errorString = QStringLiteral("Failed to parse P1689 JSON: %1").arg(parseError.errorString());
        return result;
    }

    const QJsonObject rootObj = doc.object();
    const QJsonArray rules = rootObj.value(QLatin1String("rules")).toArray();

    for (const QJsonValue& ruleVal : rules) {
        if (!ruleVal.isObject()) {
            continue;
        }
        const QJsonObject rule = ruleVal.toObject();
        const QString primaryOutput = rule.value(QLatin1String("primary-output")).toString();
        QString sourceFile = outputToSourceMap.value(primaryOutput);

        QStringList requiredModules;
        const QJsonArray requiresArr = rule.value(QLatin1String("requires")).toArray();
        for (const QJsonValue& reqVal : requiresArr) {
            if (reqVal.isObject()) {
                const QString reqName = reqVal.toObject().value(QLatin1String("logical-name")).toString();
                if (!reqName.isEmpty() && !requiredModules.contains(reqName)) {
                    requiredModules << reqName;
                }
            }
        }

        const QJsonArray providesArr = rule.value(QLatin1String("provides")).toArray();
        if (providesArr.isEmpty()) {
            if (!sourceFile.isEmpty()) {
                result.fileRequirements.insert(sourceFile, requiredModules);
            }
        } else {
            for (const QJsonValue& provVal : providesArr) {
                if (!provVal.isObject()) {
                    continue;
                }
                const QJsonObject prov = provVal.toObject();
                const QString modName = prov.value(QLatin1String("logical-name")).toString();
                if (modName.isEmpty()) {
                    continue;
                }
                const QString provSource = prov.value(QLatin1String("source-path")).toString();
                const QString actualSource = !provSource.isEmpty() ? provSource : sourceFile;
                const bool isInterface = prov.value(QLatin1String("is-interface")).toBool(true);

                ModuleDescription desc;
                desc.name = modName;
                desc.sourceFile = actualSource;
                desc.isInterface = isInterface;
                desc.requiredModules = requiredModules;

                result.modules.insert(modName, desc);
                if (!actualSource.isEmpty()) {
                    result.fileRequirements.insert(actualSource, requiredModules);
                }
            }
        }
    }

    result.success = true;
    return result;
}

ClangModuleScanner::ScanResult ClangModuleScanner::scanFile(
    const QString& sourceFile,
    const QStringList& compileArgs,
    const QString& workingDirectory,
    const std::function<bool()>& abortFunction)
{
    ScanResult result;
    if (abortFunction && abortFunction()) {
        result.errorString = QStringLiteral("Operation aborted");
        return result;
    }

    const QString scanDeps = ClangHelpers::clangScanDepsPath();
    if (scanDeps.isEmpty()) {
        result.errorString = QStringLiteral("clang-scan-deps binary not found");
        return result;
    }

    const QString clangCompiler = ClangHelpers::clangCompilerPath();
    if (clangCompiler.isEmpty()) {
        result.errorString = QStringLiteral("clang++ binary not found");
        return result;
    }

    QStringList sanitizedArgs = compileArgs;
    ClangHelpers::removeHarmfulModuleArguments(sanitizedArgs);

    const QString dummyOutput = sourceFile + QStringLiteral(".o");

    QStringList procArgs;
    procArgs << QStringLiteral("-format=p1689");
    procArgs << QStringLiteral("--");
    procArgs << clangCompiler;
    procArgs << sanitizedArgs;
    procArgs << QStringLiteral("-c") << sourceFile;
    procArgs << QStringLiteral("-o") << dummyOutput;

    QProcess proc;
    if (!workingDirectory.isEmpty()) {
        proc.setWorkingDirectory(workingDirectory);
    } else {
        proc.setWorkingDirectory(QFileInfo(sourceFile).absolutePath());
    }

    proc.start(scanDeps, procArgs);
    if (!proc.waitForStarted(5000)) {
        result.errorString = QStringLiteral("Failed to start clang-scan-deps");
        return result;
    }

    const int maxWaitMs = 30000;
    int waitedMs = 0;
    while (!proc.waitForFinished(50)) {
        if (abortFunction && abortFunction()) {
            proc.kill();
            proc.waitForFinished(500);
            result.errorString = QStringLiteral("Operation aborted");
            return result;
        }
        waitedMs += 50;
        if (waitedMs >= maxWaitMs) {
            proc.kill();
            proc.waitForFinished(500);
            result.errorString = QStringLiteral("clang-scan-deps timed out");
            return result;
        }
    }

    if (proc.exitCode() != 0) {
        result.errorString = QStringLiteral("clang-scan-deps failed: %1").arg(QString::fromUtf8(proc.readAllStandardError()));
        return result;
    }

    QHash<QString, QString> outputMap;
    outputMap.insert(dummyOutput, sourceFile);
    return parseP1689Json(proc.readAllStandardOutput(), outputMap);
}
