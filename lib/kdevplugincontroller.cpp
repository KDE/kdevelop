/* This file is part of the KDE project
  Copyright (C) 2004 Alexander Dymo <adymo@kdevelop.org>

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
#include "kdevplugincontroller.h"

#include <QFile>

#include <kcmdlineargs.h>
#include <kapplication.h>
#include <klibloader.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kparts/componentfactory.h>
#include <assert.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kcmdlineargs.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kaction.h>
#include <kaction.h>
#include <kxmlguifactory.h>

#include "kdevcore.h"
#include "kdevplugin.h"
#include "kdevmakefrontend.h"
#include "kdevappfrontend.h"
#include "kdevdifffrontend.h"
#include "kdevcreatefile.h"
#include "kdevmainwindow.h"

#include "kdevcore.h"
/*#include "partselectwidget.h"*/
#include "shellextension.h"


KDevPluginController::KDevPluginController()
{
    /*  m_defaultProfile = QLatin1String( "FullIDE" );
    m_defaultProfilePath = kapp->dirs()->localkdedir() + "/" +
    KStandardDirs::kde_default( "data" ) +
    QLatin1String("/kdevelop/profiles/FullIDE");*/

    KCmdLineArgs * args = KCmdLineArgs::parsedArgs();
    if ( args->isSet( "profile" ) )
    {
        m_profile = QString::fromLocal8Bit( args->getOption( "profile" ) );
    }
    else
    {
        m_profile = ShellExtension::getInstance() ->defaultProfile();
    }

}

KDevPluginController::~KDevPluginController()
{
    unloadPlugins();
}

void KDevPluginController::cleanUp()
{
    unloadPlugins();
}

KService::List KDevPluginController::query( const QString &serviceType,
        const QString &constraint )
{
    return KServiceTypeTrader::self() ->query( serviceType,
            QString( "%1 and [X-KDevelop-Version] == %2" ).arg( constraint ).arg( KDEVELOP_PLUGIN_VERSION ) );
}

KService::List KDevPluginController::queryPlugins( const QString &constraint )
{
    return query( "KDevelop/Plugin", constraint );
}

void KDevPluginController::loadPlugins( ProfileEngine::OfferType offer,
                                        const QStringList & ignorePlugins )
{
    KService::List offers = m_engine.offers( m_profile, offer );
    loadPlugins( offers, ignorePlugins );
}

void KDevPluginController::unloadPlugins( ProfileEngine::OfferType offer )
{
    KService::List offers = m_engine.offers( m_profile, offer );
    KService::List::ConstIterator it = offers.begin();
    for ( ; it != offers.end(); ++it )
    {
        QString name = ( *it ) ->desktopEntryName();

        if ( KDevPlugin * plugin = m_parts.value( name ) )
        {
            removeAndForgetPart( name, plugin );
            delete plugin;
        }
    }
}

void KDevPluginController::loadPlugins( KService::List offers, const QStringList & ignorePlugins )
{
    for ( KService::List::ConstIterator it = offers.begin(); it != offers.end(); ++it )
    {
        QString name = ( *it ) ->desktopEntryName();

        // Check if it is already loaded or shouldn't be
        if ( m_parts.value( name ) != 0 || ignorePlugins.contains( name ) )
            continue;

        emit loadingPlugin( i18n( "Loading: %1", ( *it ) ->genericName() ) );

        KDevPlugin *plugin = loadPlugin( *it );
        if ( plugin )
        {
            m_parts[ name ] = plugin;
            integratePart( plugin );
        }
    }
}

bool KDevPluginController::unloadPlugins()
{
    QHash<QString, KDevPlugin *>::iterator it = m_parts.begin();
    while ( it != m_parts.end() )
    {
        KDevPlugin * part = it.value();
        removePart( part );
        delete part;
        it = m_parts.erase( it );
    }
    return true;
}

void KDevPluginController::unloadPlugins( QStringList const & unloadParts )
{
    QStringList::ConstIterator it = unloadParts.begin();
    while ( it != unloadParts.end() )
    {
        KDevPlugin * part = m_parts.value( *it );
        if ( part )
        {
            removePart( part );
            m_parts.remove( *it );
            delete part;
        }
        ++it;
    }
}

KDevPlugin *KDevPluginController::loadPlugin( const KService::Ptr &service )
{
    int err = 0;
    KDevPlugin * pl = KService::createInstance<KDevPlugin>( service, 0,
                      argumentsFromService( service ), &err );
    return pl;
}

QStringList KDevPluginController::argumentsFromService( const KService::Ptr &service )
{
    QStringList args;
    if ( !service )
        // service is a reference to a pointer, so a check whether it is 0 is still required
        return args;
    QVariant prop = service->property( "X-KDevelop-Args" );
    if ( prop.isValid() )
        args = prop.toString().split( " " );
    return args;
}

void KDevPluginController::integratePart( KXMLGUIClient *part )
{
    Q_ASSERT( KDevCore::mainWindow() );
    if ( ! part || !KDevCore::mainWindow() ->guiFactory() )
        return ;

    KDevCore::mainWindow() ->guiFactory() ->addClient( part );
}

void KDevPluginController::integrateAndRememberPart( const QString &name, KDevPlugin *part )
{
    m_parts.insert( name, part );
    integratePart( part );
}

void KDevPluginController::removePart( KXMLGUIClient *part )
{
    Q_ASSERT( KDevCore::mainWindow() );
    if ( ! part || !KDevCore::mainWindow() ->guiFactory() )
        return ;
    KDevCore::mainWindow() ->guiFactory() ->removeClient( part );
}

void KDevPluginController::removeAndForgetPart( const QString &name, KDevPlugin *part )
{
    m_parts.remove( name );
    removePart( part );
}

const QList<KDevPlugin *> KDevPluginController::loadedPlugins( const QString& )
{
    return m_parts.values();
}

KDevPlugin * KDevPluginController::extension( const QString & serviceType, const QString & constraint )
{
    KService::List offers = KDevPluginController::query( serviceType, constraint );
    for ( KService::List::const_iterator it = offers.constBegin(); it != offers.constEnd(); ++it )
    {
        KDevPlugin *ext = m_parts.value( ( *it ) ->desktopEntryName() );
        if ( ext )
            return ext;
    }
    return 0;
}

KDevPlugin * KDevPluginController::loadPlugin( const QString & serviceType, const QString & constraint )
{
    KService::List offers = KDevPluginController::query( serviceType, constraint );
    if ( !offers.size() == 1 )
        return 0;

    KService::List::const_iterator it = offers.constBegin();
    QString name = ( *it ) ->desktopEntryName();

    KDevPlugin * plugin = 0;
    plugin = m_parts.value( name );
    if ( plugin )
    {
        return plugin;
    }

    plugin = loadPlugin( *it );
    if ( plugin )
    {
        m_parts.insert( name, plugin );
        integratePart( plugin );
    }

    return plugin;
}

void KDevPluginController::unloadPlugin( const QString & plugin )
{
    QStringList pluginList;
    pluginList << plugin;
    unloadPlugins( pluginList );
}

KUrl::List KDevPluginController::profileResources( const QString &nameFilter )
{
    return m_engine.resources( currentProfile(), nameFilter );
}

KUrl::List KDevPluginController::profileResourcesRecursive( const QString &nameFilter )
{
    return m_engine.resourcesRecursive( currentProfile(), nameFilter );
}

QString KDevPluginController::changeProfile( const QString &newProfile )
{
    QStringList unload;
    KService::List coreLoad;
    KService::List globalLoad;
    m_engine.diffProfiles( ProfileEngine::Core,
                           currentProfile(),
                           newProfile,
                           unload,
                           coreLoad );
    m_engine.diffProfiles( ProfileEngine::Global,
                           currentProfile(),
                           newProfile,
                           unload,
                           globalLoad );

    QString oldProfile = m_profile;
    m_profile = newProfile;

    unloadPlugins( unload );
    loadPlugins( coreLoad );
    loadPlugins( globalLoad );

    return oldProfile;
}

#include "kdevplugincontroller.moc"
