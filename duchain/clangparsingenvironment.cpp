/*
 * This file is part of KDevelop
 *
 * Copyright 2014 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

ClangParsingEnvironment::IncludePaths ClangParsingEnvironment::includes() const
{
    IncludePaths ret;
    ret.project.reserve(m_includes.size());
    ret.system.reserve(m_includes.size());
    foreach (const auto& path, m_includes) {
        bool inProject = false;
        foreach (const auto& project, m_projectPaths) {
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

uint ClangParsingEnvironment::hash() const
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

    hash << qHash(m_pchInclude);
    hash << qHash(m_languageStandard);
    return hash;
}

bool ClangParsingEnvironment::operator==(const ClangParsingEnvironment& other) const
{
    return m_defines == other.m_defines
        && m_includes == other.m_includes
        && m_pchInclude == other.m_pchInclude
        && m_quality == other.m_quality
        && m_tuUrl == other.m_tuUrl
        && m_languageStandard == other.m_languageStandard;
}

void ClangParsingEnvironment::setLanguageStandard(const QString& standard)
{
    m_languageStandard = standard;
}

QString ClangParsingEnvironment::languageStandard() const
{
    return m_languageStandard;
}
