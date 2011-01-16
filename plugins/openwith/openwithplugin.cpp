/*
 * This file is part of KDevelop
 * Copyright 2009  Andreas Pakulat <apaku@gmx.de>
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

#include "openwithplugin.h"
#include <QVariantList>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kaction.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/context.h>
#include <project/projectmodel.h>
#include <kmimetype.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kmimetypetrader.h>
#include <QSignalMapper>
#include <kmenu.h>
#include <krun.h>
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <kparts/mainwindow.h>
#include <KMessageBox>
#include <QApplication>
#include <KConfigGroup>
using namespace KDevelop;

K_PLUGIN_FACTORY(KDevOpenWithFactory, registerPlugin<OpenWithPlugin>(); )
K_EXPORT_PLUGIN(KDevOpenWithFactory(KAboutData("kdevopenwith","kdevopenwith", ki18n("Open With"), "0.1", ki18n("Open files with external applications."), KAboutData::License_GPL)))


OpenWithPlugin::OpenWithPlugin ( QObject* parent, const QVariantList& ) 
    : IPlugin ( KDevOpenWithFactory::componentData(), parent ),
    m_actionMap( 0 )
{
//    setXMLFile( "kdevopenwithui.rc" );
    KDEV_USE_EXTENSION_INTERFACE( IOpenWith )
}

OpenWithPlugin::~OpenWithPlugin()
{
}

KDevelop::ContextMenuExtension OpenWithPlugin::contextMenuExtension ( KDevelop::Context* context )
{
    if( m_actionMap )
    {
        delete m_actionMap;
        m_actionMap = 0;
    }
    m_urls = QList<KUrl>();
    FileContext* filectx = dynamic_cast<FileContext*>( context );
    ProjectItemContext* projctx = dynamic_cast<ProjectItemContext*>( context );
    if( filectx && filectx->urls().count() > 0 )
    {
        m_urls = filectx->urls();
    } else if ( projctx && projctx->items().count() > 0 )
    {
        foreach( ProjectBaseItem* item, projctx->items() )
        {
            if( item->file() )
            {
                m_urls << item->file()->url();
            }
        }
    }
    if( !m_urls.isEmpty() )
    {
        m_actionMap = new QSignalMapper( this );
        connect( m_actionMap, SIGNAL(mapped(const QString&)), SLOT(open(const QString&)) );

        // Ok, lets fetch the mimetype for the !!first!! url and the relevant services
        // TODO: Think about possible alternatives to using the mimetype of the first url.
        KMimeType::Ptr mimetype = KMimeType::findByUrl( m_urls.first() );
        if(!mimetype->is("inode/directory")){
            m_mimeType = mimetype->name();
            KService::List apps = KMimeTypeTrader::self()->query( m_mimeType );
            KService::Ptr preferredapp = KMimeTypeTrader::self()->preferredService( m_mimeType );
            KService::List parts = KMimeTypeTrader::self()->query( m_mimeType, "KParts/ReadOnlyPart" );
            KService::Ptr preferredpart = KMimeTypeTrader::self()->preferredService( m_mimeType,
                                                                                     "KParts/ReadOnlyPart" );

            // Now setup a menu with actions for each part and app
            KMenu* menu = new KMenu( i18n("Open With" ) );
            menu->setIcon( SmallIcon( "document-open" ) );

            menu->addActions( actionsForServices( parts, preferredpart ) );
            menu->addActions( actionsForServices( apps, preferredapp ) );

            KAction* openAction = new KAction( i18n( "Open" ), this );
            openAction->setIcon( SmallIcon( "document-open" ) );
            connect( openAction, SIGNAL( triggered() ), SLOT( openDefault() ) );

            KDevelop::ContextMenuExtension ext;
            ext.addAction( KDevelop::ContextMenuExtension::FileGroup, openAction );
            ext.addAction( KDevelop::ContextMenuExtension::FileGroup, menu->menuAction() );
            return ext;
        }
    }
    return KDevelop::IPlugin::contextMenuExtension ( context );
}


QList< QAction* > OpenWithPlugin::actionsForServices ( const KService::List& list, KService::Ptr pref )
{
    QList<QAction*> openactions;
    foreach( KService::Ptr svc, list )
    {
        KAction* act = new KAction( svc->name(), this );
        act->setIcon( SmallIcon( svc->icon() ) );
        connect(act, SIGNAL(triggered()), m_actionMap, SLOT(map()));
        m_actionMap->setMapping( act, svc->storageId() );
        if( svc->storageId() == pref->storageId() )
        {
            openactions.prepend( act );
        } else
        {
            openactions.append( act );
        }
    }
    return openactions;
}

void OpenWithPlugin::openDefault()
{
    KConfigGroup config = KGlobal::config()->group("Open With Defaults");
    if (config.hasKey(m_mimeType)) {
        QString storageId = config.readEntry(m_mimeType, QString());
        if (!storageId.isEmpty() && KService::serviceByStorageId(storageId)) {
            open(storageId);
            return;
        }
    }
    foreach( const KUrl& u, m_urls ) {
        ICore::self()->documentController()->openDocument( u );
    }
}

void OpenWithPlugin::open ( const QString& storageid )
{
    KService::Ptr svc = KService::serviceByStorageId( storageid );
    if( svc->isApplication() )
    {
        KRun::run( *svc, m_urls, ICore::self()->uiController()->activeMainWindow() );
    } else 
    {
        QString prefName = svc->desktopEntryName();
        if( svc->serviceTypes().contains( "KTextEditor/Document" ) )
        {
            // If the user chose a KTE part, lets make sure we're creating a TextDocument instead of 
            // a PartDocument by passing no preferredpart to the documentcontroller
            // TODO: Solve this rather inside DocumentController
            prefName = "";
        }
        foreach( const KUrl& u, m_urls )
        {
            ICore::self()->documentController()->openDocument( u, prefName );
        }
    }

    KConfigGroup config = KGlobal::config()->group("Open With Defaults");
    if (storageid != config.readEntry(m_mimeType, QString())) {
        int setDefault = KMessageBox::questionYesNo(
            qApp->activeWindow(),
            i18n("Do you want to open %1 files by default with %2?",
                 m_mimeType, svc->name() ),
            i18n("Set as default?"),
            KStandardGuiItem::yes(), KStandardGuiItem::no(),
            QString("OpenWith-%1").arg(m_mimeType)
        );
        if (setDefault == KMessageBox::Yes) {
            config.writeEntry(m_mimeType, storageid);
        }
    }
}

void OpenWithPlugin::openFilesInternal( const KUrl::List& files )
{
    if (files.isEmpty()) {
        return;
    }

    m_urls = files;
    m_mimeType = KMimeType::findByUrl( m_urls.first() )->name();
    openDefault();
}
