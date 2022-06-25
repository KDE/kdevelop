/*
    SPDX-FileCopyrightText: 2020 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "cmakefileapi.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVersionNumber>

#include <makefileresolver/makefileresolver.h>

#include "cmakeutils.h"
#include "cmakeprojectdata.h"

#include <debug.h>

using namespace KDevelop;

namespace {
Path toCanonical(const Path& path)
{
    QFileInfo info(path.toLocalFile());
    if (!info.exists())
        return path;
    const auto canonical = info.canonicalFilePath();
    if (info.filePath() != canonical) {
        return Path(canonical);
    } else {
        return path;
    }
}

QJsonObject parseFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCWarning(CMAKE) << "failed to read json file" << path << file.errorString();
        return {};
    }
    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error) {
        qCWarning(CMAKE) << "failed to parse json file" << path << error.errorString() << error.offset;
        return {};
    }
    return document.object();
}

bool isKDevelopClientResponse(const QJsonObject& indexObject)
{
    return indexObject.value(QLatin1String("reply")).toObject().contains(QLatin1String("client-kdevelop"));
}
}

namespace CMake {
namespace FileApi {
bool supported(const QString& cmakeExecutable)
{
    return QVersionNumber::fromString(cmakeExecutableVersion(cmakeExecutable)) >= QVersionNumber(3, 14);
}

void writeClientQueryFile(const QString& buildDirectory)
{
    const QDir queryDir(buildDirectory + QLatin1String("/.cmake/api/v1/query/client-kdevelop/"));
    if (!queryDir.exists() && !queryDir.mkpath(QStringLiteral("."))) {
        qCWarning(CMAKE) << "failed to create file API query dir:" << queryDir.absolutePath();
        return;
    }

    QFile queryFile(queryDir.absoluteFilePath(QStringLiteral("query.json")));
    if (!queryFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCWarning(CMAKE) << "failed to open query file for writing:" << queryFile.fileName() << queryFile.errorString();
        return;
    }

    queryFile.write(R"({"requests": [{"kind": "codemodel", "version": 2}, {"kind": "cmakeFiles", "version": 1}]})");
}

static QDir toReplyDir(const QString& buildDirectory)
{
    QDir replyDir(buildDirectory + QLatin1String("/.cmake/api/v1/reply/"));
    if (!replyDir.exists()) {
        qCWarning(CMAKE) << "cmake-file-api reply directory does not exist:" << replyDir.path();
    }
    return replyDir;
}

ReplyIndex findReplyIndexFile(const QString& buildDirectory)
{
    const auto replyDir = toReplyDir(buildDirectory);
    for (const auto& entry : replyDir.entryInfoList({QStringLiteral("index-*.json")}, QDir::Files, QDir::Name | QDir::Reversed)) {
        const auto object = parseFile(entry.absoluteFilePath());
        if (isKDevelopClientResponse(object)) {
            return {entry.lastModified(), object};
        }
    }
    qCWarning(CMAKE) << "no cmake-file-api reply index file found in" << replyDir.absolutePath();
    return {};
}

static CMakeTarget parseTarget(const QJsonObject& target, StringInterner& stringInterner,
                               PathInterner& sourcePathInterner, PathInterner& buildPathInterner,
                               CMakeFilesCompilationData& compilationData)
{
    CMakeTarget ret;
    ret.name = target.value(QLatin1String("name")).toString();
    ret.type = CMakeTarget::typeToEnum(target.value(QLatin1String("type")).toString());
    ret.folder = target.value(QLatin1String("folder")).toObject().value(QLatin1String("name")).toString();

    for (const auto& jsonArtifact : target.value(QLatin1String("artifacts")).toArray()) {
        const auto artifact = jsonArtifact.toObject();
        const auto buildPath = buildPathInterner.internPath(artifact.value(QLatin1String("path")).toString());
        if (buildPath.isValid()) {
            ret.artifacts.append(buildPath);
        }
    }

    for (const auto& jsonSource : target.value(QLatin1String("sources")).toArray()) {
        const auto source = jsonSource.toObject();
        const auto sourcePath = sourcePathInterner.internPath(source.value(QLatin1String("path")).toString());
        if (sourcePath.isValid()) {
            ret.sources.append(sourcePath);
        }
    }

    QVector<CMakeFile> compileGroups;
    for (const auto& jsonCompileGroup : target.value(QLatin1String("compileGroups")).toArray()) {
        CMakeFile cmakeFile;
        const auto compileGroup = jsonCompileGroup.toObject();

        cmakeFile.language = compileGroup.value(QLatin1String("language")).toString();

        for (const auto& jsonFragment : compileGroup.value(QLatin1String("compileCommandFragments")).toArray()) {
            const auto fragment = jsonFragment.toObject();
            cmakeFile.compileFlags += fragment.value(QLatin1String("fragment")).toString();
            cmakeFile.compileFlags += QLatin1Char(' ');
        }
        cmakeFile.compileFlags = stringInterner.internString(cmakeFile.compileFlags);

        for (const auto& jsonDefine : compileGroup.value(QLatin1String("defines")).toArray()) {
            const auto define = jsonDefine.toObject();
            cmakeFile.addDefine(define.value(QLatin1String("define")).toString());
        }
        cmakeFile.defines = MakeFileResolver::extractDefinesFromCompileFlags(cmakeFile.compileFlags, stringInterner, cmakeFile.defines);

        for (const auto& jsonInclude : compileGroup.value(QLatin1String("includes")).toArray()) {
            const auto include = jsonInclude.toObject();
            const auto path = sourcePathInterner.internPath(include.value(QLatin1String("path")).toString());
            if (path.isValid()) {
                cmakeFile.includes.append(path);
            }
        }

        compileGroups.append(cmakeFile);
    }

    for (const auto& jsonSource : target.value(QLatin1String("sources")).toArray()) {
        const auto source = jsonSource.toObject();
        const auto compileGroupIndex = source.value(QLatin1String("compileGroupIndex")).toInt(-1);
        if (compileGroupIndex < 0 || compileGroupIndex > compileGroups.size()) {
            continue;
        }
        const auto compileGroup = compileGroups.value(compileGroupIndex);
        const auto path = sourcePathInterner.internPath(source.value(QLatin1String("path")).toString());
        if (path.isValid()) {
            compilationData.files[toCanonical(path)] = compileGroup;
        }
    }
    return ret;
}

static CMakeProjectData parseCodeModel(const QJsonObject& codeModel, const QDir& replyDir,
                                       StringInterner&stringInterner, PathInterner& sourcePathInterner, PathInterner& buildPathInterner)
{
    CMakeProjectData ret;
    // for now, we only use the first available configuration and don't support multi configurations
    const auto configuration = codeModel.value(QLatin1String("configurations")).toArray().at(0).toObject();
    const auto targets = configuration.value(QLatin1String("targets")).toArray();
    const auto directories = configuration.value(QLatin1String("directories")).toArray();
    for (const auto& directoryValue : directories) {
        const auto directory = directoryValue.toObject();
        if (!directory.contains(QLatin1String("targetIndexes"))) {
            continue;
        }
        const auto dirSourcePath = sourcePathInterner.internPath(directory.value(QLatin1String("source")).toString());
        auto& dirTargets = ret.targets[dirSourcePath];
        for (const auto& targetIndex : directory.value(QLatin1String("targetIndexes")).toArray()) {
            const auto jsonTarget = targets.at(targetIndex.toInt(-1)).toObject();
            if (jsonTarget.isEmpty()) {
                continue;
            }
            const auto targetFile = jsonTarget.value(QLatin1String("jsonFile")).toString();
            const auto target = parseTarget(parseFile(replyDir.absoluteFilePath(targetFile)),
                                            stringInterner, sourcePathInterner, buildPathInterner,
                                            ret.compilationData);
            if (target.name.isEmpty()) {
                continue;
            }
            dirTargets.append(target);
        }
    }
    ret.compilationData.isValid = !codeModel.isEmpty();
    ret.compilationData.rebuildFileForFolderMapping();
    if (!ret.compilationData.isValid) {
        qCWarning(CMAKE) << "failed to parse code model" << codeModel;
    }
    return ret;
}

static QHash<Path, CMakeProjectData::CMakeFileFlags>
parseCMakeFiles(const QJsonObject& cmakeFiles, PathInterner& sourcePathInterner, QDateTime* lastModifiedCMakeFile)
{
    Q_ASSERT(lastModifiedCMakeFile);
    QHash<Path, CMakeProjectData::CMakeFileFlags> ret;
    for (const auto& jsonInput : cmakeFiles.value(QLatin1String("inputs")).toArray()) {
        const auto input = jsonInput.toObject();
        const auto path = sourcePathInterner.internPath(input.value(QLatin1String("path")).toString());
        CMakeProjectData::CMakeFileFlags flags;
        flags.isGenerated = input.value(QLatin1String("isGenerated")).toBool();
        flags.isExternal = input.value(QLatin1String("isExternal")).toBool();
        flags.isCMake = input.value(QLatin1String("isCMake")).toBool();
        ret[path] = flags;

        if (path.isLocalFile()) {
            const auto info = QFileInfo(path.toLocalFile());
            *lastModifiedCMakeFile = std::max(info.lastModified(), *lastModifiedCMakeFile);
        }
    }
    return ret;
}

CMakeProjectData parseReplyIndexFile(const ReplyIndex& replyIndex, const Path& sourceDirectory,
                                     const Path& buildDirectory)
{
    const auto reply = replyIndex.data.value(QLatin1String("reply")).toObject();
    const auto clientKDevelop = reply.value(QLatin1String("client-kdevelop")).toObject();
    const auto query = clientKDevelop.value(QLatin1String("query.json")).toObject();
    const auto responses = query.value(QLatin1String("responses")).toArray();
    const auto replyDir = toReplyDir(buildDirectory.toLocalFile());

    StringInterner stringInterner;
    PathInterner sourcePathInterner(toCanonical(sourceDirectory));
    PathInterner buildPathInterner(buildDirectory);

    CMakeProjectData codeModel;
    QHash<Path, CMakeProjectData::CMakeFileFlags> cmakeFiles;

    QDateTime lastModifiedCMakeFile;
    for (const auto& responseValue : responses) {
        const auto response = responseValue.toObject();
        const auto kind = response.value(QLatin1String("kind"));
        const auto jsonFile = response.value(QLatin1String("jsonFile")).toString();
        const auto jsonFilePath = replyDir.absoluteFilePath(jsonFile);
        if (kind == QLatin1String("codemodel")) {
            codeModel = parseCodeModel(parseFile(jsonFilePath), replyDir,
                                       stringInterner, sourcePathInterner, buildPathInterner);
        } else if (kind == QLatin1String("cmakeFiles")) {
            cmakeFiles = parseCMakeFiles(parseFile(jsonFilePath), sourcePathInterner, &lastModifiedCMakeFile);
        }
    }

    if (!codeModel.compilationData.isValid) {
        qCWarning(CMAKE) << "failed to find code model in reply index" << sourceDirectory << buildDirectory
                         << replyIndex.data;
        return {};
    }

    codeModel.lastModifiedCMakeFile = lastModifiedCMakeFile;
    codeModel.lastModifiedProjectData = replyIndex.lastModified;
    codeModel.cmakeFiles = cmakeFiles;
    return codeModel;
}
}
}
