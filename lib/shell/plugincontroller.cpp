/* This file is part of the KDE project
Copyright (C) 2004, 2007 Alexander Dymo <adymo@kdevelop.org>
Copyright (C) 2006 Matt Rogers <mattr@kde.org
Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>

Based on code from Kopete
Copyright (c) 2002-2003 by Martijn Klingens <klingens@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/
#include "plugincontroller.h"

#include <QFile>
#include <QTimer>
#include <QApplication>
#include <kcmdlineargs.h>
#include <klibloader.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <klocale.h>
#include <kxmlguiwindow.h>
#include <kparts/componentfactory.h>
#include <assert.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kstandarddirs.h>
#include <kaction.h>
#include <kaction.h>
#include <kxmlguifactory.h>
#include <kstaticdeleter.h>
#include <kmenu.h>
#include <QAction>

#include "iplugin.h"
#include "profileengine.h"
#include "mainwindow.h"

#include "core.h"
/*#include "partselectwidget.h"*/
#include "shellextension.h"
#include <QtDesigner/QExtensionManager>

namespace KDevelop
{

struct PluginControllerPrivate
{
public:
    QList<KPluginInfo*> plugins;

    //map plugin infos to currently loaded plugins
    typedef QMap<KPluginInfo*, IPlugin*> InfoToPluginMap;
    InfoToPluginMap loadedPlugins;

    // The plugin manager's mode. The mode is StartingUp until loadAllPlugins()
    // has finished loading the plugins, after which it is set to Running.
    // ShuttingDown and DoneShutdown are used during shutdown by the
    // async unloading of plugins.
    enum CleanupMode { Running, CleaningUp, CleanupDone };
    CleanupMode cleanupMode;

    QString profile;
    ProfileEngine engine;
    Core *core;
};

PluginController::PluginController(Core *core)
    : IPluginController(), d(new PluginControllerPrivate)
{
    d->core = core;
    d->profile = ShellExtension::getInstance() ->defaultProfile();
    d->plugins = KPluginInfo::fromServices( KServiceTypeTrader::self()->query( QLatin1String( "KDevelop/Plugin" ),
        QLatin1String( "[X-KDevelop-Version] == 4" ) ) );
    d->cleanupMode = PluginControllerPrivate::Running;
    m_manager = new QExtensionManager();
}

PluginController::~PluginController()
{
    if ( d->cleanupMode != PluginControllerPrivate::CleanupDone )
        kWarning(9000) << k_funcinfo << "Destructing plugin controller without going through the shutdown process! Backtrace is: "
                       << endl << kBacktrace() << endl;

    // Quick cleanup of the remaining plugins, hope it helps
    // Note that deleting it.value() causes slotPluginDestroyed to be called, which
    // removes the plugin from the list of loaded plugins.
    while ( !d->loadedPlugins.empty() )
    {
        PluginControllerPrivate::InfoToPluginMap::ConstIterator it = d->loadedPlugins.begin();
        kWarning(9000) << k_funcinfo << "Deleting stale plugin '" << it.key()->pluginName()
                << "'" << endl;
        delete it.value();
    }
    delete m_manager;
    qDeleteAll(d->plugins);
    delete d;
}

QString PluginController::currentProfile() const
{
    return d->profile;
}

ProfileEngine& PluginController::engine() const
{
    return d->engine;
}

KPluginInfo* PluginController::pluginInfo( IPlugin* plugin ) const
{
    for ( PluginControllerPrivate::InfoToPluginMap::ConstIterator it = d->loadedPlugins.begin();
          it != d->loadedPlugins.end(); ++it )
    {
        if ( it.value() == plugin )
            return it.key();
    }
    return 0;
}

void PluginController::cleanup()
{
    if(d->cleanupMode != PluginControllerPrivate::Running)
    {
        kDebug(9000) << k_funcinfo << "called when not running. state = " << d->cleanupMode << endl;
        return;
    }

    d->cleanupMode = PluginControllerPrivate::CleaningUp;

    // Ask all plugins to unload
    for ( PluginControllerPrivate::InfoToPluginMap::ConstIterator it = d->loadedPlugins.begin();
          it != d->loadedPlugins.end(); /* EMPTY */ )
    {
        // Plugins could emit their ready for unload signal directly in response to this,
        // which would invalidate the current iterator. Therefore, we copy the iterator
        // and increment it beforehand.
        PluginControllerPrivate::InfoToPluginMap::ConstIterator current( it );
        ++it;

        //Let the plugin do some stuff before unloading
        IPlugin* plugin = current.value();
        plugin->unload();
        delete plugin;
    }

    d->cleanupMode = PluginControllerPrivate::CleanupDone;
}

IPlugin* PluginController::loadPlugin( const QString& pluginName )
{
    return loadPluginInternal( pluginName );
}

void PluginController::loadPlugins( PluginType type )
{
    KPluginInfo::List offers = d->engine.offers( d->profile, type );
    foreach( KPluginInfo* pi, offers )
    {
        loadPluginInternal( pi->pluginName() );
    }
}

void PluginController::unloadPlugins( PluginType type )
{
    //TODO see if this can be optimized so it's not something like O(n^2)
    KPluginInfo::List offers = d->engine.offers( d->profile, type );
    foreach( KPluginInfo* pi, offers )
    {
        foreach ( KPluginInfo* lpi, d->loadedPlugins.keys() )
        {
            if ( pi->pluginName() == lpi->pluginName() )
                unloadPlugin( pi->pluginName() );
        }
    }
}

QList<IPlugin *> PluginController::loadedPlugins() const
{
    return d->loadedPlugins.values();
}

void PluginController::unloadPlugin( const QString & pluginId )
{
    if( IPlugin *thePlugin = plugin( pluginId ) )
    {
        thePlugin->unload();
        delete thePlugin;
    }
}

KUrl::List PluginController::profileResources( const QString &nameFilter )
{
    return d->engine.resources( currentProfile(), nameFilter );
}

KUrl::List PluginController::profileResourcesRecursive( const QString &nameFilter )
{
    return d->engine.resourcesRecursive( currentProfile(), nameFilter );
}

QString PluginController::changeProfile( const QString &newProfile )
{
	Q_UNUSED( newProfile );
    /* FIXME disabled for now
    QStringList unload;
    KService::List coreLoad;
    KService::List globalLoad;
    d->engine.diffProfiles( ProfileEngine::Core,
                           currentProfile(),
                           newProfile,
                           unload,
                           coreLoad );
    d->engine.diffProfiles( ProfileEngine::Global,
                           currentProfile(),
                           newProfile,
                           unload,
                           globalLoad );

    QString oldProfile = d->profile;
    d->profile = newProfile;

    unloadPlugins( unload );
    loadPlugins( coreLoad );
    loadPlugins( globalLoad );

    return oldProfile;
    */
    return QString();
}

KPluginInfo * PluginController::infoForPluginId( const QString &pluginId ) const
{
    QList<KPluginInfo *>::ConstIterator it;
    for ( it = d->plugins.begin(); it != d->plugins.end(); ++it )
    {
        if ( ( *it )->pluginName() == pluginId )
            return *it;
    }

    return 0L;
}

IPlugin *PluginController::loadPluginInternal( const QString &pluginId )
{
    KPluginInfo *info = infoForPluginId( pluginId );
    if ( !info )
    {
        kWarning(9000) << k_funcinfo << "Unable to find a plugin named '" << pluginId << "'!" << endl;
        return 0L;
    }

    if ( d->loadedPlugins.contains( info ) )
        return d->loadedPlugins[ info ];

    kDebug(9000) << k_funcinfo << "Attempting to load '" << pluginId << "'" << endl;
    emit loadingPlugin( info->name() );
    int error = 0;
    IPlugin *plugin = 0;
    QStringList missingInterfaces;
    if ( checkForDependencies( info, missingInterfaces ) )
    {
        plugin = KServiceTypeTrader::createInstanceFromQuery<IPlugin>( QLatin1String( "KDevelop/Plugin" ),
                QString::fromLatin1( "[X-KDE-PluginInfo-Name]=='%1'" ).arg( pluginId ), d->core, QStringList(), &error );
        loadDependencies( info );
    }

    if ( plugin )
    {
        d->loadedPlugins.insert( info, plugin );
        plugin->registerExtensions();
        info->setPluginEnabled( true );

        connect( plugin, SIGNAL( destroyed( QObject * ) ),
                 this, SLOT( pluginDestroyed( QObject * ) ) );

        kDebug(9000) << k_funcinfo << "Successfully loaded plugin '" << pluginId << "'" << endl;

        emit pluginLoaded( plugin );
    }
    else
    {
        if( !error && !missingInterfaces.isEmpty() )
        {
            kDebug(9000) << k_funcinfo << "Can't load plugin '" << pluginId
                    << "' some of its required dependecies could not be fullfilled:" << endl
                    << missingInterfaces.join(",") << endl;
        }else
        {
            switch( error )
            {
                case KLibLoader::ErrNoServiceFound:
                    kDebug(9000) << k_funcinfo << "No service implementing the given mimetype "
                            << "and fullfilling the given constraint expression can be found." << endl;
                    break;

                case KLibLoader::ErrServiceProvidesNoLibrary:
                    kDebug(9000) << "the specified service provides no shared library." << endl;
                    break;

                case KLibLoader::ErrNoLibrary:
                    kDebug(9000) << "the specified library could not be loaded." << endl;
                    break;

                case KLibLoader::ErrNoFactory:
                    kDebug(9000) << "the library does not export a factory for creating components." << endl;
                    break;

                case KLibLoader::ErrNoComponent:
                    kDebug(9000) << "the factory does not support creating components of the specified type." << endl;
                    break;
                default:
                    kDebug(9000) << "An unknown error occurred" << endl;
                    break;
            }

            kDebug(9000) << k_funcinfo << "Loading plugin '" << pluginId
                    << "' failed, KLibLoader reported error: '" << endl <<
                    KLibLoader::self()->lastErrorMessage() << "'" << endl;
        }
    }

    return plugin;
}


IPlugin* PluginController::plugin( const QString& pluginId )
{
    KPluginInfo *info = infoForPluginId( pluginId );
    if ( !info )
        return 0L;

    if ( d->loadedPlugins.contains( info ) )
        return d->loadedPlugins[ info ];
    else
        return 0L;
}

void PluginController::pluginDestroyed( QObject* deletedPlugin )
{
    for ( PluginControllerPrivate::InfoToPluginMap::Iterator it = d->loadedPlugins.begin();
          it != d->loadedPlugins.end(); ++it )
    {
        if ( it.value() == deletedPlugin )
        {
            d->loadedPlugins.erase( it );
            break;
        }
    }
}

///@todo plugin load operation should be O(n)
bool PluginController::checkForDependencies( KPluginInfo* info, QStringList& missing ) const
{
    QVariant prop = info->property( "X-KDevelop-IRequired" );
    bool result = true;
    if( prop.canConvert<QStringList>() )
    {
        QStringList deps = prop.toStringList();
        foreach( QString iface, deps )
        {
            KPluginInfo::List l = queryPlugins( QString("'%1' in [X-KDevelop-Interfaces]").arg(iface) );
            if( l.isEmpty() )
            {
                result = false;
                missing << iface;
            }
        }
    }
    return result;
}

void PluginController::loadDependencies( KPluginInfo* info )
{
    QVariant prop = info->property( "X-KDevelop-IRequired" );
    if( prop.canConvert<QStringList>() )
    {
        QStringList deps = prop.toStringList();
        foreach( QString iface, deps )
        {
            KPluginInfo* info = queryPlugins( QString("'%1' in [X-KDevelop-Interfaces]").arg(iface) ).first();
            loadPluginInternal( info->pluginName() );
        }
    }
}

IPlugin* PluginController::pluginForExtension( const QString& extension, const QString& pluginname)
{
    QStringList constraints;
    if (!pluginname.isEmpty())
        constraints << QString("[X-KDE-PluginInfo-Name]=='%1'").arg( pluginname );

    return pluginForExtension(extension, constraints);
}

IPlugin *PluginController::pluginForExtension(const QString &extension, const QStringList &constraints)
{
    KPluginInfo::List infos = queryExtensionPlugins(extension, constraints);

    if( infos.isEmpty() )
        return 0;
    if( d->plugins.contains( infos.first() ) )
        return d->loadedPlugins[ infos.first() ];
    else
        return loadPluginInternal( infos.first()->pluginName() );
}

QList<IPlugin*> PluginController::allPluginsForExtension(const QString &extension, const QStringList &constraints)
{
    KPluginInfo::List infos = queryExtensionPlugins(extension, constraints);
    QList<IPlugin*> plugins;
    foreach (KPluginInfo *info, infos)
    {
        if( d->plugins.contains( info ) )
            plugins << d->loadedPlugins[ info ];
        else
            plugins << loadPluginInternal( info->pluginName() );
    }
    return plugins;
}

KPluginInfo::List PluginController::queryExtensionPlugins(const QString &extension, const QStringList &constraints)
{
    QStringList c = constraints;
    c << QString("'%1' in [X-KDevelop-Interfaces]").arg( extension );
    return queryPlugins( c.join(" and ") );
}

QExtensionManager* PluginController::extensionManager()
{
    return m_manager;
}

QStringList PluginController::allPluginNames()
{
    QStringList names;
    Q_FOREACH( KPluginInfo* info , d->plugins )
    {
        names << info->pluginName();
    }
    return names;
}

void PluginController::buildContextMenu( KDevelop::Context* context, KMenu* menu )
{
    Q_FOREACH( KPluginInfo* info, d->loadedPlugins.keys() )
    {
        IPlugin* plug = d->loadedPlugins[info];
        QPair<QString, QList<QAction*> > actions = plug->requestContextMenuActions( context );
        if( actions.first.isEmpty() || actions.second.isEmpty() )
            continue;
        if( actions.second.size() == 1 )
        {
            QAction* a = actions.second.first();
            a->setText( a->text() + i18n( " (%1)", actions.first ) );
            menu->addAction(a);
        }else
        {
            QMenu* submenu = menu->addMenu( actions.first );
            submenu->addActions( actions.second );
        }
    }
}

}
#include "plugincontroller.moc"

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
