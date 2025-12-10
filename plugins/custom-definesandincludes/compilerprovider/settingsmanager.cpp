/*
    SPDX-FileCopyrightText: 2010 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "settingsmanager.h"

#include <QDataStream>
#include <QThread>
#include <QCoreApplication>
#include <QMimeDatabase>

#include <KConfig>
#include <KConfigGroup>

#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <project/projectmodel.h>

#include <algorithm>

#include "compilerprovider.h"

using namespace KDevelop;

namespace {
constexpr Utils::LanguageType configurableLanguageTypes[] =
    { Utils::C, Utils::Cpp, Utils::OpenCl, Utils::Cuda };

namespace ConfigConstants {
inline QString configKey() { return QStringLiteral("CustomDefinesAndIncludes"); }
inline QString definesKey() { return QStringLiteral("Defines"); }
inline QString includesKey() { return QStringLiteral("Includes"); }
inline QString projectPathPrefix() { return QStringLiteral("ProjectPath"); }
inline QString projectPathKey() { return QStringLiteral("Path"); }

inline QString customBuildSystemGroup() { return QStringLiteral("CustomBuildSystem"); }
inline QString definesAndIncludesGroup() { return QStringLiteral("Defines And Includes"); }

inline QString compilersGroup() { return QStringLiteral("Compilers"); }
inline QString compilerNameKey() { return QStringLiteral("Name"); }
inline QString compilerPathKey() { return QStringLiteral("Path"); }
inline QString compilerTypeKey() { return QStringLiteral("Type"); }

QString parserArgumentsKey(Utils::LanguageType languageType)
{
    switch (languageType) {
    case Utils::C:
        return QStringLiteral("parserArgumentsC");
    case Utils::Cpp:
        return QStringLiteral("parserArguments");
    case Utils::OpenCl:
        return QStringLiteral("parserArgumentsOpenCL");
    case Utils::Cuda:
        return QStringLiteral("parserArgumentsCuda");
    // TODO: is there a need for "parserArgumentsObjC[++]" and if so how/where
    // if not, merge the ObjC cases with the C/C++ cases.
    case Utils::ObjC:
        return QStringLiteral("parserArgumentsC");
    case Utils::ObjCpp:
        return QStringLiteral("parserArguments");
    case Utils::Other:
        break;
    }
    Q_UNREACHABLE();
}

QString parseAmbiguousAsCPP()
{
    return QStringLiteral("parseAmbiguousAsCPP");
}
}

// the grouplist is randomly sorted b/c it uses QSet internally
// we sort the keys here, as the structure is properly defined for us
QStringList sorted(QStringList list)
{
    std::sort(list.begin(), list.end());
    return list;
}

ParserArguments createDefaultArguments()
{
    ParserArguments arguments;
    arguments[Utils::C] = QStringLiteral(
        "-ferror-limit=100 -fspell-checking -Wdocumentation -Wunused-parameter -Wunreachable-code -Wall -std=c17");
    arguments[Utils::Cpp] = QStringLiteral(
        "-ferror-limit=100 -fspell-checking -Wdocumentation -Wunused-parameter -Wunreachable-code -Wall -std=c++20");
    arguments[Utils::OpenCl] = QStringLiteral("-ferror-limit=100 -fspell-checking -Wdocumentation -Wunused-parameter -Wunreachable-code -Wall -cl-std=CL1.1");
    arguments[Utils::Cuda] = arguments[Utils::Cpp];
    // For now, use the same arguments for ObjC(++) as for C(++). -Wall enables a number
    // of language-specific warnings, removing the need to add them explicitly.
    // (https://embeddedartistry.com/blog/2017/3/7/clang-weverything)
    arguments[Utils::ObjC] = arguments[Utils::C];
    arguments[Utils::ObjCpp] = arguments[Utils::Cpp];
    arguments.parseAmbiguousAsCPP = true;

    return arguments;
}

const ParserArguments& defaultArguments()
{
    static ParserArguments arguments = createDefaultArguments();
    return arguments;
}

CompilerPointer createCompilerFromConfig(KConfigGroup& cfg)
{
    auto grp = cfg.group(QStringLiteral("Compiler"));
    auto name = grp.readEntry( ConfigConstants::compilerNameKey(), QString() );
    if (name.isEmpty()) {
        return SettingsManager::globalInstance()->provider()->defaultCompiler();
    }

    const auto& compilers = SettingsManager::globalInstance()->provider()->compilers();
    for (auto& c : compilers) {
        if (c->name() == name) {
            return c;
        }
    }

    // Otherwise we have no such compiler registered (broken config file), return default one
    return SettingsManager::globalInstance()->provider()->defaultCompiler();
}

void writeCompilerToConfig(KConfigGroup& cfg, const CompilerPointer& compiler)
{
    Q_ASSERT(compiler);

    auto grp = cfg.group(QStringLiteral("Compiler"));
    // Store only compiler name, path and type retrieved from registered compilers
    grp.writeEntry(ConfigConstants::compilerNameKey(), compiler->name());
}

void doWriteSettings( KConfigGroup grp, const QVector<ConfigEntry>& paths )
{
    int pathIndex = 0;
    for ( const auto& path : paths ) {
        KConfigGroup pathgrp = grp.group(ConfigConstants::projectPathPrefix() + QString::number(pathIndex++));
        pathgrp.writeEntry(ConfigConstants::projectPathKey(), path.path);
        for (auto type : configurableLanguageTypes) {
            pathgrp.writeEntry(ConfigConstants::parserArgumentsKey(type), path.parserArguments[type]);
        }
        pathgrp.writeEntry(ConfigConstants::parseAmbiguousAsCPP(), path.parserArguments.parseAmbiguousAsCPP);

        {
            int index = 0;
            KConfigGroup includes(pathgrp.group(ConfigConstants::includesKey()));
            for (auto& include : path.includes) {
                includes.writeEntry(QString::number(++index), include);
            }

        }
        {
            KConfigGroup defines(pathgrp.group(ConfigConstants::definesKey()));
            for (auto it = path.defines.begin(); it != path.defines.end(); ++it) {
                defines.writeEntry(it.key(), it.value());
            }
        }
        writeCompilerToConfig(pathgrp, path.compiler);
    }
}

/// @param remove if true all read entries will be removed from the config file
QVector<ConfigEntry> doReadSettings( KConfigGroup grp, bool remove = false )
{
    QVector<ConfigEntry> paths;
    const auto& sortedGroupNames = sorted(grp.groupList());
    for (const QString& grpName : sortedGroupNames) {
        if (!grpName.startsWith(ConfigConstants::projectPathPrefix())) {
            continue;
        }
        KConfigGroup pathgrp = grp.group( grpName );

        ConfigEntry path;
        path.path = pathgrp.readEntry(ConfigConstants::projectPathKey(), "");
        for (auto type : configurableLanguageTypes) {
            path.parserArguments[type] = pathgrp.readEntry(ConfigConstants::parserArgumentsKey(type), defaultArguments()[type]);
        }
        path.parserArguments.parseAmbiguousAsCPP = pathgrp.readEntry(ConfigConstants::parseAmbiguousAsCPP(), defaultArguments().parseAmbiguousAsCPP);

        for (auto type : configurableLanguageTypes) {
            if (path.parserArguments[type].isEmpty()) {
                path.parserArguments[type] = defaultArguments()[type];
            }
        }

        { // defines
            // Backwards compatibility with old config style
            if(pathgrp.hasKey(ConfigConstants::definesKey())) {
                QByteArray tmp = pathgrp.readEntry(ConfigConstants::definesKey(), QByteArray() );
                QDataStream s( tmp );
                s.setVersion( QDataStream::Qt_4_5 );
                // backwards compatible reading
                QHash<QString, QVariant> defines;
                s >> defines;
                path.setDefines(defines);
            } else {
                KConfigGroup defines(pathgrp.group(ConfigConstants::definesKey()));
                QMap<QString, QString> defMap = defines.entryMap();
                path.defines.reserve(defMap.size());
                for(auto it = defMap.constBegin(); it != defMap.constEnd(); ++it) {
                    QString key = it.key();
                    if(key.isEmpty()) {
                        // Toss out the invalid key and value since a valid
                        // value needs a valid key
                        continue;
                    } else {
                            path.defines.insert(key, it.value());
                    }
                }
            }
        }

        { // includes
            // Backwards compatibility with old config style
            if(pathgrp.hasKey(ConfigConstants::includesKey())){
                QByteArray tmp = pathgrp.readEntry(ConfigConstants::includesKey(), QByteArray());
                QDataStream s( tmp );
                s.setVersion( QDataStream::Qt_4_5 );
                s >> path.includes;
            } else {
                KConfigGroup includes(pathgrp.group(ConfigConstants::includesKey()));
                const QMap<QString, QString> incMap = includes.entryMap();
                for (auto& value :incMap) {
                    if(value.isEmpty()){
                        continue;
                    }
                    path.includes += value;
                }
            }
        }

        path.compiler = createCompilerFromConfig(pathgrp);

        if ( remove ) {
            pathgrp.deleteGroup();
        }

        Q_ASSERT(!path.parserArguments.isAnyEmpty());
        paths << path;
    }

    return paths;
}

/**
 * Reads and converts paths from old (Custom Build System's) format to the current one.
 * @return all converted paths (if any)
 */
QVector<ConfigEntry> convertedPaths( KConfig* cfg )
{
    KConfigGroup group = cfg->group(ConfigConstants::customBuildSystemGroup());
    if ( !group.isValid() )
        return {};

    QVector<ConfigEntry> paths;
    const auto sortedGroupNames = sorted(group.groupList());
    for (const QString& grpName : sortedGroupNames) {
        KConfigGroup subgroup = group.group( grpName );
        if ( !subgroup.isValid() )
            continue;

        paths += doReadSettings( subgroup, true );
    }

    return paths;
}

}

void ConfigEntry::setDefines(const QHash<QString, QVariant>& newDefines)
{
    defines.clear();
    defines.reserve(newDefines.size());
    for (auto it = newDefines.begin(); it != newDefines.end(); ++it) {
        defines[it.key()] = it.value().toString();
    }
}

SettingsManager::SettingsManager()
  : m_provider(this)
{}

SettingsManager::~SettingsManager()
{}

SettingsManager* SettingsManager::globalInstance()
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());
    static SettingsManager s_globalInstance;
    return &s_globalInstance;
}

CompilerProvider* SettingsManager::provider()
{
    return &m_provider;
}

const CompilerProvider* SettingsManager::provider() const
{
    return &m_provider;
}

void SettingsManager::writePaths( KConfig* cfg, const QVector< ConfigEntry >& paths )
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    KConfigGroup grp = cfg->group(ConfigConstants::configKey());
    if ( !grp.isValid() )
        return;

    grp.deleteGroup();

    doWriteSettings( grp, paths );
}

QVector<ConfigEntry> SettingsManager::readPaths( KConfig* cfg ) const
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    auto converted = convertedPaths( cfg );
    if ( !converted.isEmpty() ) {
        const_cast<SettingsManager*>(this)->writePaths( cfg, converted );
        return converted;
    }

    KConfigGroup grp = cfg->group(ConfigConstants::configKey());
    if ( !grp.isValid() ) {
        return {};
    }

    return doReadSettings( grp );
}

bool SettingsManager::needToReparseCurrentProject( KConfig* cfg ) const
{
    auto grp = cfg->group(ConfigConstants::definesAndIncludesGroup());
    return grp.readEntry( "reparse", true );
}

void SettingsManager::writeUserDefinedCompilers(const QVector< CompilerPointer >& compilers)
{
    QVector< CompilerPointer > editableCompilers;
    for (const auto& compiler : compilers) {
        if (!compiler->editable()) {
            continue;
        }
        editableCompilers.append(compiler);
    }

    KConfigGroup config = KSharedConfig::openConfig()->group(ConfigConstants::compilersGroup());
    config.deleteGroup();
    config.writeEntry("number", editableCompilers.count());
    int i = 0;
    for (const auto& compiler : editableCompilers) {
        KConfigGroup grp = config.group(QString::number(i));
        ++i;

        grp.writeEntry(ConfigConstants::compilerNameKey(), compiler->name());
        grp.writeEntry(ConfigConstants::compilerPathKey(), compiler->path());
        grp.writeEntry(ConfigConstants::compilerTypeKey(), compiler->factoryName());
    }
    config.sync();
}

QVector< CompilerPointer > SettingsManager::userDefinedCompilers() const
{
    QVector< CompilerPointer > compilers;

    KConfigGroup config = KSharedConfig::openConfig()->group(ConfigConstants::compilersGroup());
    int count = config.readEntry("number", 0);
    for (int i = 0; i < count; i++) {
        KConfigGroup grp = config.group(QString::number(i));

        auto name = grp.readEntry(ConfigConstants::compilerNameKey(), QString());
        auto path = grp.readEntry(ConfigConstants::compilerPathKey(), QString());
        auto type = grp.readEntry(ConfigConstants::compilerTypeKey(), QString());

        const auto cf = m_provider.compilerFactories();
        for (auto& f : cf) {
            if (f->name() == type) {
                auto compiler = f->createCompiler(name, path);
                compilers.append(compiler);
            }
        }
    }
    return compilers;
}

ParserArguments SettingsManager::defaultParserArguments() const
{
    return defaultArguments();
}

ConfigEntry::ConfigEntry(const QString& path)
    : path(path)
    , compiler(SettingsManager::globalInstance()->provider()->defaultCompiler())
    , parserArguments(defaultArguments())
{}

namespace Utils {
LanguageType languageType(const QString& path, bool treatAmbiguousAsCPP)
{
    QMimeDatabase db;
    const auto mimeType = db.mimeTypeForFile(path).name();
    if (mimeType == QLatin1String("text/x-csrc") ||
        mimeType == QLatin1String("text/x-chdr") ) {
        if (treatAmbiguousAsCPP) {
            if (path.endsWith(QLatin1String(".h"), Qt::CaseInsensitive)) {
                return Cpp;
            }
        }

        // TODO: No proper mime type detection possible yet
        // cf. https://bugs.freedesktop.org/show_bug.cgi?id=26913
        if (path.endsWith(QLatin1String(".cl"), Qt::CaseInsensitive)) {
            return OpenCl;
        }

        // TODO: No proper mime type detection possible yet
        // cf. https://bugs.freedesktop.org/show_bug.cgi?id=23700
        if (path.endsWith(QLatin1String(".cu"), Qt::CaseInsensitive)) {
            return Cuda;
        }

        return C;
    }

    if (mimeType == QLatin1String("text/x-c++src") ||
        mimeType == QLatin1String("text/x-c++hdr") ) {
        return Cpp;
    }

    if (mimeType == QLatin1String("text/x-objc++src")) {
        return ObjCpp;
    }

    if (mimeType == QLatin1String("text/x-objcsrc")) {
        return ObjC;
    }

    if (mimeType == QLatin1String("text/x-opencl-src")) {
        return OpenCl;
    }

    return Other;
}
}

bool ParserArguments::isAnyEmpty() const
{
    return std::any_of(std::begin(arguments), std::end(arguments),
        [](const QString& args) { return args.isEmpty(); }
    );
}
