/*
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>
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

#include "customdefinesandincludesmanager.h"

#include "settingsmanager.h"

#include <interfaces/iproject.h>
#include <project/projectmodel.h>

#include <QList>
#include <QStringList>
#include <QMutex>
#include <QMutexLocker>

#include <KPluginFactory>
#include <KAboutData>

namespace KDevelop
{

class ManagerPrivate
{
public:
    ///@return: The ConfigEntry, with includes/defines from @p paths for all parent folders of @p item.
    static ConfigEntry findConfigForItem( const QList<ConfigEntry>& paths, const ProjectBaseItem* item )
    {
        ConfigEntry ret;

        auto itemPath = item->path().toUrl();
        KUrl rootDirectory = item->project()->folder();

        for ( const ConfigEntry& entry : paths ) {
            KUrl targetDirectory = rootDirectory;
            // note: a dot represents the project root
            if ( entry.path != "." ) {
                targetDirectory.addPath( entry.path );
            }

            if ( targetDirectory.isParentOf( itemPath ) ) {
                ret.includes += entry.includes;

                for ( auto it = entry.defines.constBegin(); it != entry.defines.constEnd(); it++ ) {
                    if ( !ret.defines.contains( it.key() ) ) {
                        ret.defines[it.key()] = it.value();
                    }
                }
            }
        }
        ret.includes.removeDuplicates();
        return ret;
    }

    static Path::List includeDirectories( const ProjectBaseItem* item )
    {
        if ( !item ) {
            return {};
        }

        KConfig* cfg = item->project()->projectConfiguration().data();

        return KDevelop::toPathList( findConfigForItem( m_manager->readSettings( cfg ), item ).includes );
    }

    static QHash<QString, QString> defines( const ProjectBaseItem* item )
    {
        if ( !item ) {
            return {};
        }

        KConfig* cfg = item->project()->projectConfiguration().data();

        const auto result = findConfigForItem( m_manager->readSettings( cfg ), item ).defines;
        QHash<QString, QString> defines;
        for ( auto it = result.constBegin(); it != result.constEnd(); it++ ) {
            defines[it.key()] = it.value().toString();
        }
        return defines;
    }

    static CustomDefinesAndIncludesManager* m_manager;
    static QMutex m_locker;
};
CustomDefinesAndIncludesManager* ManagerPrivate::m_manager = nullptr;
QMutex ManagerPrivate::m_locker;

K_PLUGIN_FACTORY(CustomDefinesAndIncludesManagerFactory, registerPlugin<CustomDefinesAndIncludesManager>(); )
K_EXPORT_PLUGIN(CustomDefinesAndIncludesManagerFactory(KAboutData("kdevcustomdefinesandincludesmanager","kdevcustomdefinesandincludesmanager", ki18n("Custom Defines and Includes Manager"), "0.1", ki18n(""), KAboutData::License_GPL)))

CustomDefinesAndIncludesManager::CustomDefinesAndIncludesManager( QObject* parent, const QVariantList& )
    : IPlugin( CustomDefinesAndIncludesManagerFactory::componentData(), parent )
{
    KDEV_USE_EXTENSION_INTERFACE(IDefinesAndIncludesManager);
    ManagerPrivate::m_manager = this;
}

QHash<QString, QString> CustomDefinesAndIncludesManager::defines( const ProjectBaseItem* item ) const
{
    QMutexLocker lock( &ManagerPrivate::m_locker );
    return ManagerPrivate::defines( item );
}

Path::List CustomDefinesAndIncludesManager::includes( const ProjectBaseItem* item ) const
{
    QMutexLocker lock( &ManagerPrivate::m_locker );
    return ManagerPrivate::includeDirectories( item );
}

QList<ConfigEntry> CustomDefinesAndIncludesManager::readSettings( KConfig* cfg ) const
{
    return SettingsManager::readSettings( cfg );
}

void CustomDefinesAndIncludesManager::writeSettings( KConfig* cfg, const QList<ConfigEntry>& paths ) const
{
    SettingsManager::writeSettings( cfg, paths );
}
}