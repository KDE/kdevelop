/*
 * This file is part of KDevelop
 *
 * Copyright 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include "../idefinesandincludesmanager.h"

#include "compilerprovider.h"
#include "icompiler.h"

#include <QVector>

class KConfig;

namespace KDevelop {
class IProject;
class ProjectBaseItem;
}

struct ParserArguments
{
public:
    const QString& operator[](Utils::LanguageType languageType) const
    {
        Q_ASSERT(languageType >= Utils::C && languageType < Utils::Other);
        return arguments[languageType];
    }

    QString& operator[](Utils::LanguageType languageType)
    {
        Q_ASSERT(languageType >= Utils::C && languageType < Utils::Other);
        return arguments[languageType];
    }

    /// Is any of the arguments empty?
    bool isAnyEmpty() const;

private:
    QString arguments[Utils::Other];

public:
    bool parseAmbiguousAsCPP;
};

Q_DECLARE_METATYPE(ParserArguments)
Q_DECLARE_TYPEINFO(ParserArguments, Q_MOVABLE_TYPE);

struct ConfigEntry
{
    QString path;
    QStringList includes;
    KDevelop::Defines defines;
    CompilerPointer compiler;
    ParserArguments parserArguments;

    explicit ConfigEntry( const QString& path = QString() );

    // FIXME: get rid of this but stay backwards compatible
    void setDefines(const QHash<QString, QVariant>& defines);
};
Q_DECLARE_TYPEINFO(ConfigEntry, Q_MOVABLE_TYPE);

namespace Utils
{
LanguageType languageType(const QString& path, bool treatAmbiguousAsCPP = true);
}

class SettingsManager
{
public:
    ~SettingsManager();

    QVector<ConfigEntry> readPaths(KConfig* cfg) const;
    void writePaths(KConfig* cfg, const QVector<ConfigEntry>& paths);

    QVector<CompilerPointer> userDefinedCompilers() const;
    void writeUserDefinedCompilers(const QVector<CompilerPointer>& compilers);

    bool needToReparseCurrentProject( KConfig* cfg ) const;

    ParserArguments defaultParserArguments() const;

    CompilerProvider* provider();
    const CompilerProvider* provider() const;

    static SettingsManager* globalInstance();

private:
    SettingsManager();
    CompilerProvider m_provider;
};

#endif // SETTINGSMANAGER_H
