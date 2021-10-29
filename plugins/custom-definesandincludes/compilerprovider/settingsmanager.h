/*
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
