/*
    SPDX-FileCopyrightText: 2014 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "clangparsingenvironment.h"

using namespace KDevelop;

int ClangParsingEnvironment::type() const
{
    return CppParsingEnvironment;
}

void ClangParsingEnvironment::setProjectPaths(const Path::List& projectPaths)
{
    m_projectPaths = projectPaths;
}

Path::List ClangParsingEnvironment::projectPaths() const
{
    return m_projectPaths;
}

void ClangParsingEnvironment::addIncludes(const Path::List& includes)
{
    m_includes += includes;
}

void ClangParsingEnvironment::addFrameworkDirectories(const KDevelop::Path::List& frameworkDirectories)
{
    m_frameworkDirectories += frameworkDirectories;
}

template <typename PathType>
static PathType appendPaths(const KDevelop::Path::List &paths, const KDevelop::Path::List &projectPaths)
{
    PathType ret;
    ret.project.reserve(paths.size());
    ret.system.reserve(paths.size());
    for (const auto& path : paths) {
        bool inProject = false;
        for (const auto& project : projectPaths) {
            if (project.isParentOf(path) || project == path) {
                inProject = true;
                break;
            }
        }
        if (inProject) {
            ret.project.append(path);
        } else {
            ret.system.append(path);
        }
    }
    return ret;
}

ClangParsingEnvironment::IncludePaths ClangParsingEnvironment::includes() const
{
    return appendPaths<IncludePaths>(m_includes, m_projectPaths);
}

ClangParsingEnvironment::FrameworkDirectories ClangParsingEnvironment::frameworkDirectories() const
{
    return appendPaths<FrameworkDirectories>(m_frameworkDirectories, m_projectPaths);
}

void ClangParsingEnvironment::addDefines(const QHash<QString, QString>& defines)
{
    for (auto it = defines.constBegin(); it != defines.constEnd(); ++it) {
        m_defines[it.key()] = it.value();
    }
}

QMap<QString, QString> ClangParsingEnvironment::defines() const
{
    return m_defines;
}

void ClangParsingEnvironment::setPchInclude(const Path& path)
{
    m_pchInclude = path;
}

Path ClangParsingEnvironment::pchInclude() const
{
    return m_pchInclude;
}

void ClangParsingEnvironment::setWorkingDirectory(const Path& path)
{
    m_workingDirectory = path;
}

Path ClangParsingEnvironment::workingDirectory() const
{
    return m_workingDirectory;
}

void ClangParsingEnvironment::setTranslationUnitUrl(const IndexedString& url)
{
    m_tuUrl = url;
}

IndexedString ClangParsingEnvironment::translationUnitUrl() const
{
    return m_tuUrl;
}

void ClangParsingEnvironment::setQuality(Quality quality)
{
    m_quality = quality;
}

ClangParsingEnvironment::Quality ClangParsingEnvironment::quality() const
{
    return m_quality;
}

size_t ClangParsingEnvironment::hash() const
{
    KDevHash hash;
    hash << m_defines.size();

    for (auto it = m_defines.constBegin(); it != m_defines.constEnd(); ++it) {
        hash << qHash(it.key()) << qHash(it.value());
    }

    hash << m_includes.size();
    for (const auto& include : m_includes) {
        hash << qHash(include);
    }

    hash << m_frameworkDirectories.size();
    for (const auto& fwDir : m_frameworkDirectories) {
        hash << qHash(fwDir);
    }

    hash << qHash(m_pchInclude);
    hash << qHash(m_parserSettings.parserOptions);
    return hash;
}

bool ClangParsingEnvironment::operator==(const ClangParsingEnvironment& other) const
{
    return m_defines == other.m_defines
        && m_includes == other.m_includes
        && m_frameworkDirectories == other.m_frameworkDirectories
        && m_pchInclude == other.m_pchInclude
        && m_quality == other.m_quality
        && m_tuUrl == other.m_tuUrl
        && m_parserSettings == other.m_parserSettings;
}

void ClangParsingEnvironment::setParserSettings(const ParserSettings& parserSettings)
{
    m_parserSettings = parserSettings;
}

ParserSettings ClangParsingEnvironment::parserSettings() const
{
    return m_parserSettings;
}

void ClangParsingEnvironment::addParserArguments(const QString& parserArguments)
{
    m_parserSettings.parserOptions += QLatin1Char(' ') + parserArguments;
}

