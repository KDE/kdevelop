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
#include <util/path.h>
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
K_EXPORT_PLUGIN(KDevOpenWithFactory(KAboutData("kdevopenwith","kdevopenwith", ki18n("Open With"), "0.1", ki18n("This plugin allows to open files with associated external applications."), KAboutData::License_GPL)))


OpenWithPlugin::OpenWithPlugin ( QObject* parent, const QVariantList& )
    : IPlugin ( KDevOpenWithFactory::componentData(), parent ),
    m_actionMap( 0 )
{
    KDEV_USE_EXTENSION_INTERFACE( IOpenWith )
}

OpenWithPlugin::~OpenWithPlugin()
{
}

KDevelop::ContextMenuExtension OpenWithPlugin::contextMenuExtension( KDevelop::Context* context )
{
    // do not recurse
    if (context->hasType(KDevelop::Context::OpenWithContext)) {
        return ContextMenuExtension();
    }

    m_urls.clear();
    m_actionMap.reset();
    m_services.clear();

    FileContext* filectx = dynamic_cast<FileContext*>( context );
    ProjectItemContext* projctx = dynamic_cast<ProjectItemContext*>( context );
    if ( filectx && filectx->urls().count() > 0 ) {
        m_urls = filectx->urls();
    } else if ( projctx && projctx->items().count() > 0 ) {
        // For now, let's handle *either* files only *or* directories only
        const int wantedType = projctx->items().first()->type();
        foreach( ProjectBaseItem* item, projctx->items() ) {
            if (wantedType == ProjectBaseItem::File && item->file()) {
                m_urls << item->file()->path().toUrl();
            } else if ((wantedType == ProjectBaseItem::Folder || wantedType == ProjectBaseItem::BuildFolder) && item->folder()) {
                m_urls << item->folder()->path().toUrl();
            }
        }
    }

    if (m_urls.isEmpty()) {
        return KDevelop::ContextMenuExtension();
    }

    m_actionMap.reset(new QSignalMapper( this ));
    connect( m_actionMap.data(), SIGNAL(mapped(QString)), SLOT(open(QString)) );

    // Ok, lets fetch the mimetype for the !!first!! url and the relevant services
    // TODO: Think about possible alternatives to using the mimetype of the first url.
    KMimeType::Ptr mimetype = KMimeType::findByUrl( m_urls.first() );
    m_mimeType = mimetype->name();

    QList<QAction*> partActions = actionsForServiceType("KParts/ReadOnlyPart");
    QList<QAction*> appActions = actionsForServiceType("Application");

    OpenWithContext subContext(m_urls, mimetype);
    QList<ContextMenuExtension> extensions = ICore::self()->pluginController()->queryPluginsForContextMenuExtensions( &subContext );
    foreach( const ContextMenuExtension& ext, extensions ) {
        appActions += ext.actions(ContextMenuExtension::OpenExternalGroup);
        partActions += ext.actions(ContextMenuExtension::OpenEmbeddedGroup);
    }

    // Now setup a menu with actions for each part and app
    KMenu* menu = new KMenu( i18n("Open With" ) );
    menu->setIcon( SmallIcon( "document-open" ) );

    if (!partActions.isEmpty()) {
        menu->addTitle(i18n("Embedded Editors"));
        menu->addActions( partActions );
    }
    if (!appActions.isEmpty()) {
        menu->addTitle(i18n("External Applications"));
        menu->addActions( appActions );
    }

    KAction* openAction = new KAction( i18n( "Open" ), this );
    openAction->setIcon( SmallIcon( "document-open" ) );
    connect( openAction, SIGNAL(triggered()), SLOT(openDefault()) );

    KDevelop::ContextMenuExtension ext;
    ext.addAction( KDevelop::ContextMenuExtension::FileGroup, openAction );
    if (!menu->isEmpty()) {
        ext.addAction(KDevelop::ContextMenuExtension::FileGroup, menu->menuAction());
    } else {
        delete menu;
    }
    return ext;
}

bool sortActions(QAction* left, QAction* right)
{
    return left->text() < right->text();
}

bool isTextEditor(const KService::Ptr& service)
{
    return service->serviceTypes().contains( "KTextEditor/Document" );
}

QString defaultForMimeType(const QString& mimeType)
{
    KConfigGroup config = KGlobal::config()->group("Open With Defaults");
    if (config.hasKey(mimeType)) {
        QString storageId = config.readEntry(mimeType, QString());
        if (!storageId.isEmpty() && KService::serviceByStorageId(storageId)) {
            return storageId;
        }
    }
    return QString();
}

QList<QAction*> OpenWithPlugin::actionsForServiceType( const QString& serviceType )
{
    KService::List list = KMimeTypeTrader::self()->query( m_mimeType, serviceType );
    KService::Ptr pref = KMimeTypeTrader::self()->preferredService( m_mimeType, serviceType );

    m_services += list;
    QList<QAction*> actions;
    QAction* standardAction = 0;
    const QString defaultId = defaultForMimeType(m_mimeType);
    foreach( KService::Ptr svc, list ) {
        KAction* act = new KAction( isTextEditor(svc) ? i18n("Default Editor") : svc->name(), this );
        act->setIcon( SmallIcon( svc->icon() ) );
        if (svc->storageId() == defaultId || (defaultId.isEmpty() && isTextEditor(svc))) {
            QFont font = act->font();
            font.setBold(true);
            act->setFont(font);
        }
        connect(act, SIGNAL(triggered()), m_actionMap.data(), SLOT(map()));
        m_actionMap->setMapping( act, svc->storageId() );
        actions << act;
        if ( isTextEditor(svc) ) {
            standardAction = act;
        } else if ( svc->storageId() == pref->storageId() ) {
            standardAction = act;
        }
    }
    qSort(actions.begin(), actions.end(), sortActions);
    if (standardAction) {
        actions.removeOne(standardAction);
        actions.prepend(standardAction);
    }
    return actions;
}

void OpenWithPlugin::openDefault()
{
    //  check preferred handler
    const QString defaultId = defaultForMimeType(m_mimeType);
    if (!defaultId.isEmpty()) {
        open(defaultId);
        return;
    }

    // default handlers
    if (m_mimeType == "inode/directory") {
        KService::Ptr service = KMimeTypeTrader::self()->preferredService(m_mimeType);
        KRun::run(*service, m_urls, ICore::self()->uiController()->activeMainWindow());
    } else {
        foreach( const KUrl& u, m_urls ) {
            ICore::self()->documentController()->openDocument( u );
        }
    }
}

void OpenWithPlugin::open( const QString& storageid )
{
    KService::Ptr svc = KService::serviceByStorageId( storageid );
    if( svc->isApplication() ) {
        KRun::run( *svc, m_urls, ICore::self()->uiController()->activeMainWindow() );
    } else {
        QString prefName = svc->desktopEntryName();
        if ( isTextEditor(svc) ) {
            // If the user chose a KTE part, lets make sure we're creating a TextDocument instead of 
            // a PartDocument by passing no preferredpart to the documentcontroller
            // TODO: Solve this rather inside DocumentController
            prefName = "";
        }
        foreach( const KUrl& u, m_urls ) {
            ICore::self()->documentController()->openDocument( u, prefName );
        }
    }

    KConfigGroup config = KGlobal::config()->group("Open With Defaults");
    if (storageid != config.readEntry(m_mimeType, QString())) {
        int setDefault = KMessageBox::questionYesNo(
            qApp->activeWindow(),
            i18nc("%1: mime type name, %2: app/part name", "Do you want to open all '%1' files by default with %2?",
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
