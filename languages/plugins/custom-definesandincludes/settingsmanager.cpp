/*
 * This file is part of KDevelop
 *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>
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

#include "settingsmanager.h"

#include <KConfig>
#include <KConfigGroup>

#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>

#include "compilerprovider/icompilerprovider.h"

using KDevelop::ConfigEntry;

namespace ConfigConstants
{
const QString configKey = QLatin1String( "CustomDefinesAndIncludes" );
const QString definesKey = QLatin1String( "Defines" );
const QString includesKey = QLatin1String( "Includes" );
const QString projectPathPrefix = QLatin1String( "ProjectPath" );
const QString projectPathKey = QLatin1String( "Path" );

const QString customBuildSystemGroup = QLatin1String( "CustomBuildSystem" );
const QString definesAndIncludesGroup = QLatin1String( "Defines And Includes" );

const QString compilersGroup = QLatin1String( "Compilers" );
const QString compilerNameKey = QLatin1String( "Name" );
const QString compilerPathKey = QLatin1String( "Path" );
const QString compilerTypeKey = QLatin1String( "Type" );
}

SettingsManager::SettingsManager()
    : m_provider(nullptr)
{
}

void SettingsManager::setProvider(const ICompilerProvider* provider)
{
    m_provider = provider;
}

namespace
{
void doWriteSettings( KConfigGroup grp, const QList<ConfigEntry>& paths )
{
    int pathIndex = 0;
    for ( const auto& path : paths ) {
        KConfigGroup pathgrp = grp.group( ConfigConstants::projectPathPrefix + QString::number( pathIndex++ ) );
        pathgrp.writeEntry( ConfigConstants::projectPathKey, path.path );
        {
            QByteArray tmp;
            QDataStream s( &tmp, QIODevice::WriteOnly );
            s.setVersion( QDataStream::Qt_4_5 );
            s << path.includes;
            pathgrp.writeEntry( ConfigConstants::includesKey, tmp );
        }
        {
            QByteArray tmp;
            QDataStream s( &tmp, QIODevice::WriteOnly );
            s.setVersion( QDataStream::Qt_4_5 );
            s << path.defines;
            pathgrp.writeEntry( ConfigConstants::definesKey, tmp );
        }
    }
}

/// @param remove if true all read entries will be removed from the config file
QList<ConfigEntry> doReadSettings( KConfigGroup grp, bool remove = false )
{
    QList<ConfigEntry> paths;
    for( const QString &grpName : grp.groupList() ) {
        if ( grpName.startsWith( ConfigConstants::projectPathPrefix ) ) {
            KConfigGroup pathgrp = grp.group( grpName );

            ConfigEntry path;
            path.path = pathgrp.readEntry( ConfigConstants::projectPathKey, "" );

            {
                QByteArray tmp = pathgrp.readEntry( ConfigConstants::definesKey, QByteArray() );
                QDataStream s( tmp );
                s.setVersion( QDataStream::Qt_4_5 );
                s >> path.defines;
            }

            {
                QByteArray tmp = pathgrp.readEntry( ConfigConstants::includesKey, QByteArray() );
                QDataStream s( tmp );
                s.setVersion( QDataStream::Qt_4_5 );
                s >> path.includes;
            }
            if ( remove ) {
                pathgrp.deleteGroup();
            }
            paths << path;
        }
    }

    return paths;
}

/**
 * Reads and converts paths from old (Custom Build System's) format to the current one.
 * @return all converted paths (if any)
 */
QList<ConfigEntry> convertedPaths( KConfig* cfg )
{
    KConfigGroup group = cfg->group( ConfigConstants::customBuildSystemGroup );
    if ( !group.isValid() )
        return {};

    QList<ConfigEntry> paths;
    foreach( const QString &grpName, group.groupList() ) {
        KConfigGroup subgroup = group.group( grpName );
        if ( !subgroup.isValid() )
            continue;

        paths += doReadSettings( subgroup, true );
    }

    return paths;
}

}

void SettingsManager::writePaths( KConfig* cfg, const QList< ConfigEntry >& paths )
{
    KConfigGroup grp = cfg->group( ConfigConstants::configKey );
    if ( !grp.isValid() )
        return;

    grp.deleteGroup();

    doWriteSettings( grp, paths );
}

QList<ConfigEntry> SettingsManager::readPaths( KConfig* cfg ) const
{
    auto converted = convertedPaths( cfg );
    if ( !converted.isEmpty() ) {
        const_cast<SettingsManager*>(this)->writePaths( cfg, converted );
        return converted;
    }

    KConfigGroup grp = cfg->group( ConfigConstants::configKey );
    if ( !grp.isValid() ) {
        return {};
    }

    return doReadSettings( grp );
}

CompilerPointer SettingsManager::currentCompiler( KConfig* cfg, const CompilerPointer& defaultCompiler ) const
{
    Q_ASSERT(m_provider);

    auto grp = cfg->group( ConfigConstants::definesAndIncludesGroup ).group("Compiler");
    auto name = grp.readEntry( ConfigConstants::compilerNameKey, QString() );
    if (name.isEmpty()) {
        return {};
    }

    for (auto c : m_provider->compilers()) {
        if (c->name() == name) {
            return c;
        }
    }

    auto path = grp.readEntry( ConfigConstants::compilerPathKey, QString() );
    auto type = grp.readEntry( ConfigConstants::compilerTypeKey, QString() );

    auto cf = m_provider->compilerFactories();
    for (auto f : cf) {
        if (f->name() == type) {
            return f->createCompiler(name, path, true);
        }
    }

    return defaultCompiler;
}

bool SettingsManager::needToReparseCurrentProject( KConfig* cfg ) const
{
    auto grp = cfg->group( ConfigConstants::definesAndIncludesGroup );
    return grp.readEntry( "reparse", true );
}

void SettingsManager::writeCurrentCompiler(KConfig* cfg, const CompilerPointer& compiler)
{
    auto grp = cfg->group(ConfigConstants::definesAndIncludesGroup).group("Compiler");
    grp.writeEntry(ConfigConstants::compilerNameKey, compiler->name());
    grp.writeEntry(ConfigConstants::compilerPathKey, compiler->path());
    grp.writeEntry(ConfigConstants::compilerTypeKey, compiler->factoryName());
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

    KConfigGroup config = KSharedConfig::openConfig()->group(ConfigConstants::compilersGroup);
    config.deleteGroup();
    config.writeEntry("number", editableCompilers.count());
    int i = 0;
    for (const auto& compiler : editableCompilers) {
        KConfigGroup grp = config.group(QString::number(i));
        ++i;

        grp.writeEntry(ConfigConstants::compilerNameKey, compiler->name());
        grp.writeEntry(ConfigConstants::compilerPathKey, compiler->path());
        grp.writeEntry(ConfigConstants::compilerTypeKey, compiler->factoryName());
    }
    config.sync();
}

QVector< CompilerPointer > SettingsManager::userDefinedCompilers() const
{
    Q_ASSERT(m_provider);

    QVector< CompilerPointer > compilers;

    KConfigGroup config = KSharedConfig::openConfig()->group(ConfigConstants::compilersGroup);
    int count = config.readEntry("number", 0);
    for (int i = 0; i < count; i++) {
        KConfigGroup grp = config.group(QString::number(i));

        auto name = grp.readEntry(ConfigConstants::compilerNameKey, QString());
        auto path = grp.readEntry(ConfigConstants::compilerPathKey, QString());
        auto type = grp.readEntry(ConfigConstants::compilerTypeKey, QString());

        auto cf = m_provider->compilerFactories();
        for (auto f : cf) {
            if (f->name() == type) {
                compilers.append(f->createCompiler(name, path));
            }
        }
    }
    return compilers;
}
