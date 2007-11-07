/* This file is part of the KDE project
Copyright 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
Copyright 2002 Bernd Gehrmann <bernd@kdevelop.org>
Copyright 2003 Roberto Raggi <roberto@kdevelop.org>
Copyright 2003 Hamish Rodda <rodda@kde.org>
Copyright 2003 Harald Fernengel <harry@kdevelop.org>
Copyright 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>
Copyright 2005 Adam Treat <treat@kde.org>
Copyright 2004-2007 Alexander Dymo <adymo@kdevelop.org>
Copyright 2007 Andreas Pakulat <apaku@gmx.de>

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
#include "documentcontroller.h"

#include <QFileInfo>
#include <QtDBus/QtDBus>

#include <kio/netaccess.h>
#include <kfiledialog.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <krecentfilesaction.h>

#include <sublime/area.h>
#include <sublime/view.h>

#include "core.h"
#include "mainwindow.h"
#include "textdocument.h"
#include "uicontroller.h"
#include "partcontroller.h"
#include "iplugincontroller.h"

#include <kplugininfo.h>

/*

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <QMap>
#include <QFile>
#include <QDebug>
#include <QLabel>
#include <QLayout>
#include <QCheckBox>
#include <QRadioButton>

#include <krun.h>
#include <kmenu.h>
#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kmimetype.h>
#include <kmimetypetrader.h>
#include <klineedit.h>
#include <kshortcut.h>
#include <kdirwatch.h>
#include <khtml_part.h>
#include <kdeversion.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kxmlguiwindow.h>
#include <kdialog.h>
#include <kcompletion.h>
#include <kiconloader.h>
#include <kxmlguifactory.h>
#include <kservicetypetrader.h>
#include <ksqueezedtextlabel.h>
#include <kencodingfiledialog.h>
#include <krecentfilesaction.h>
#include <ktoolbarpopupaction.h>
#include <kstandardaction.h>

#include <kio/netaccess.h>

#include <kparts/part.h>
#include <kparts/factory.h>
#include <kparts/partmanager.h>
#include <kparts/browserextension.h>

#include <ktexteditor/view.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/document.h>

#include "kdevconfig.h"
#include "kdevcontext.h"
#include "kdevproject.h"
#include "kdevmainwindow.h"
#include "ui_mimewarningdialog.h"
#include "kdevpartcontroller.h"
#include "kdevlanguagesupport.h"
#include "kdevbackgroundparser.h"
*/

namespace KDevelop
{

struct DocumentControllerPrivate {
    DocumentControllerPrivate(DocumentController* c)
        : controller(c)
    {
    }

    QString presetEncoding;

    // used to map urls to open docs
    QHash< KUrl, IDocument* > documents;

    QHash< QString, IDocumentFactory* > factories;

    struct HistoryEntry
    {
        HistoryEntry() {}
        HistoryEntry( const KUrl & u, const KTextEditor::Cursor& cursor );

        KUrl url;
        KTextEditor::Cursor cursor;
        int id;
    };

    void removeDocument(Sublime::Document *doc)
    {
        QList<KUrl> urlsForDoc = documents.keys(dynamic_cast<KDevelop::IDocument*>(doc));
        foreach (const KUrl &url, urlsForDoc)
        {
            kDebug(9501) << "destroying document" << doc;
            documents.remove(url);
        }
    }
    void chooseDocument()
    {
        controller->openDocument(KUrl());
    }


    DocumentController* controller;

    QList<HistoryEntry> backHistory;
    QList<HistoryEntry> forwardHistory;
    bool isJumping;

    QPointer<KAction> saveAll;
    QPointer<KAction> revertAll;
    QPointer<KAction> closeAll;
    QPointer<KAction> closeAllOthers;
    KRecentFilesAction* fileOpenRecent;
    
/*    HistoryEntry createHistoryEntry();
    void addHistoryEntry();
    void jumpTo( const HistoryEntry & );*/
};


DocumentController::DocumentController( QObject *parent )
        : IDocumentController( parent )
{
    d = new DocumentControllerPrivate(this);
    QDBusConnection::sessionBus().registerObject( "/org/kdevelop/DocumentController",
        this, QDBusConnection::ExportScriptableSlots );

    setupActions();
}

DocumentController::~DocumentController()
{
    d->fileOpenRecent->saveEntries( KConfigGroup(KGlobal::config(), "Recent Files" ) );
    delete d;
}

void DocumentController::setupActions()
{
    KActionCollection * ac =
        Core::self()->uiControllerInternal()->defaultMainWindow()->actionCollection();

    KAction *action;

    action = static_cast<KAction*>(ac->addAction( "file_open" ));
    action->setIcon(KIcon("file_open"));
    action->setText(i18n( "&Open File..." ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( chooseDocument() ) );
    action->setToolTip( i18n( "Open file" ) );
    action->setWhatsThis( i18n( "<b>Open file</b><p>Opens a file for editing.</p>" ) );

    d->fileOpenRecent = KStandardAction::openRecent(this, SLOT(slotOpenDocument(const KUrl&)), this);
    ac->addAction(d->fileOpenRecent->objectName(), d->fileOpenRecent);
    d->fileOpenRecent->setWhatsThis(i18n("This lists files which you have opened recently, and allows you to easily open them again."));
    d->fileOpenRecent->loadEntries( KConfigGroup(KGlobal::config(), "Recent Files" ) );

    action = d->saveAll = static_cast<KAction*>(ac->addAction( "file_save_all" ));
    action->setIcon(KIcon("document-save"));
    action->setText(i18n( "Save Al&l" ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( slotSaveAllDocuments() ) );
    action->setToolTip( i18n( "Save all open documents" ) );
    action->setWhatsThis( i18n( "<b>Save all documents</b><p>Save all open documents, prompting for additional information when necessary.</p>" ) );
    action->setEnabled(false);

    action = d->revertAll = static_cast<KAction*>(ac->addAction( "file_revert_all" ));
    action->setIcon(KIcon("document-revert"));
    action->setText(i18n( "Rever&t All" ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( reloadAllDocuments() ) );
    action->setToolTip( i18n( "Revert all open documents" ) );
    action->setWhatsThis( i18n( "<b>Revert all documents</b><p>Revert all open documents, returning to the previously saved state.</p>" ) );
    action->setEnabled(false);

    action = d->closeAll = static_cast<KAction*>(ac->addAction( "file_close_all" ));
    action->setIcon(KIcon("window-close"));
    action->setText(i18n( "Clos&e All" ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( closeAllDocuments() ) );
    action->setToolTip( i18n( "Close all open documents" ) );
    action->setWhatsThis( i18n( "<b>Close all documents</b><p>Close all open documents, prompting for additional information when necessary.</p>" ) );
    action->setEnabled(false);

    action = d->closeAllOthers = static_cast<KAction*>(ac->addAction( "file_closeother" ));
    action->setIcon(KIcon("window-close"));
    action->setText(i18n( "Close All Ot&hers" ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( closeAllOtherDocuments() ) );
    action->setToolTip( i18n( "Close all other documents" ) );
    action->setWhatsThis( i18n( "<b>Close all other documents</b><p>Close all open documents, with the exception of the currently active document.</p>" ) );
    action->setEnabled(false);
}

void DocumentController::setEncoding( const QString &encoding )
{
    d->presetEncoding = encoding;
}

QString KDevelop::DocumentController::encoding() const
{
    return d->presetEncoding;
}

void DocumentController::slotOpenDocument(const KUrl &url)
{
    openDocument(url);
}

IDocument* DocumentController::openDocument( const KUrl & inputUrl,
        const KTextEditor::Cursor& cursor,
        DocumentActivation activate )
{
    UiController *uiController = Core::self()->uiControllerInternal();
    Sublime::Area *area = uiController->activeArea();
    if (!area)
        return 0;

    KUrl url = inputUrl;

    if ( url.isEmpty() )
    {
        KSharedConfig * config = KGlobal::config().data();
        KConfigGroup group = config->group( "General Options" );
        QString dir = group.readEntry( "DefaultProjectsDirectory",
                                             QDir::homePath() );

        url = KFileDialog::getOpenUrl( dir, i18n( "*.*|Text File\n" ),
                                       Core::self()->uiControllerInternal()->defaultMainWindow(),
                                       i18n( "Open File" ) );
    }
    
    
    bool emitLoaded = false;

    //get a part document
    if (!d->documents.contains(url))
    {
        //make sure the URL exists
        if ( !url.isValid() || !KIO::NetAccess::exists( url, KIO::NetAccess::DestinationSide, 0 ) )
        {
            kDebug( 9000 ) << "cannot find URL:" << url.url();
            return 0;
        }

        // clean it and resolve possible symlink
        url.cleanPath( KUrl::SimplifyDirSeparators );
        if ( url.isLocalFile() )
        {
            QString path = QFileInfo( url.toLocalFile() ).canonicalFilePath();
            if ( !path.isEmpty() )
                url.setPath( path );
        }

        KMimeType::Ptr mimeType = KMimeType::findByUrl( url );

        // is the URL pointing to a directory?
        if ( mimeType->is( "inode/directory" ) )
        {
            kDebug( 9000 ) << "cannot open directory:" << url.url();
            return 0;
        }

        // Try to find a plugin that handles this mimetype
        QString constraint = QString("'%1' in [X-KDevelop-SupportedMimeTypes]").arg(mimeType->name());
        KPluginInfo::List plugins = IPluginController::queryPlugins( constraint );

        if( !plugins.isEmpty() )
        {
            KPluginInfo info = plugins.first();
            kDebug(9501) << "loading" << info.pluginName();
            Core::self()->pluginController()->loadPlugin( info.pluginName() );
            if( d->factories.contains( mimeType->name() ) )
            {
                IDocument* idoc = d->factories[mimeType->name()]->create(url, Core::self());
                if( idoc )
                {
                     d->documents[url] = idoc;
                }
            }
        }
        if ( !d->documents.contains(url) && Core::self()->partManagerInternal()->isTextType(mimeType))
            d->documents[url] = new TextDocument(url, Core::self());
        else if( !d->documents.contains(url) )
            d->documents[url] = new PartDocument(url, Core::self());
        emitLoaded = d->documents.contains(url);
    }
    IDocument *doc = d->documents[url];

    Sublime::Document *sdoc = dynamic_cast<Sublime::Document*>(doc);
    if( !sdoc )
    {
        d->documents.remove(url);
        delete doc;
        return 0;
    }
    //react on document deletion - we need to cleanup controller structures
    connect(sdoc, SIGNAL(aboutToDelete(Sublime::Document*)), this, SLOT(removeDocument(Sublime::Document*)));

    //find a view if there's one already opened in this area
    Sublime::View *partView = 0;
    foreach (Sublime::View *view, sdoc->views())
    {
        if (area->views().contains(view))
        {
            partView = view;
            break;
        }
    }
    if (!partView)
    {
        //no view currently shown for this url
        partView = sdoc->createView();

        //add view to the area
        area->addView(partView, uiController->activeSublimeWindow()->activeView());
    }
    if (activate == IDocumentController::ActivateOnOpen)
    {
        uiController->activeSublimeWindow()->activateView(partView);
    }
    if( cursor.isValid() )
    {
        doc->setCursorPosition( cursor );
    }

    d->fileOpenRecent->addUrl( url );

    // Deferred signals, wait until it's all ready first
    if( emitLoaded )
        emit documentLoaded( d->documents[url] );

    if (activate == IDocumentController::ActivateOnOpen)
        emit documentActivated( doc );

    d->saveAll->setEnabled(true);
    d->revertAll->setEnabled(true);
    d->closeAll->setEnabled(true);
    d->closeAllOthers->setEnabled(true);

    return doc;
}

void DocumentController::closeDocument( const KUrl &url )
{
    if( !d->documents.contains(url) )
        return;

    //this will remove all views and after the last view is removed, the
    //document will be self-destructed and removeDocument() slot will catch that
    //and clean up internal data structures
    d->documents[url]->close();

    if (d->documents.isEmpty()) {
        if (d->saveAll)
            d->saveAll->setEnabled(false);
        if (d->revertAll)
            d->revertAll->setEnabled(false);
        if (d->closeAll)
            d->closeAll->setEnabled(false);
        if (d->closeAllOthers)
            d->closeAllOthers->setEnabled(false);
    }
}

IDocument * DocumentController::documentForUrl( const KUrl & url ) const
{
    if ( d->documents.contains( url ) )
        return d->documents.value( url );

    return 0;
}

QList<IDocument*> DocumentController::openDocuments() const
{
    QList<IDocument*> opened;
    foreach (IDocument *doc, d->documents.values())
    {
        Sublime::Document *sdoc = dynamic_cast<Sublime::Document*>(doc);
        if( !sdoc )
        {
            continue;
        }
        if (!sdoc->views().isEmpty())
            opened << doc;
    }
    return opened;
}

void DocumentController::activateDocument( IDocument * document )
{
    openDocument(document->url());
}

void DocumentController::slotSaveAllDocuments()
{
    saveAllDocuments();
}

void DocumentController::saveAllDocuments(IDocument::DocumentSaveMode mode)
{
    foreach (IDocument* doc, d->documents)
        doc->save(mode);
}

void DocumentController::reloadAllDocuments()
{
    foreach (IDocument* doc, d->documents)
        doc->reload();
}

void DocumentController::closeAllDocuments()
{
    foreach (IDocument* doc, d->documents)
        doc->close();
}

void DocumentController::closeAllOtherDocuments()
{
    foreach (IDocument* doc, d->documents)
        if (doc != activeDocument())
            doc->close();
}

IDocument* DocumentController::activeDocument() const
{
    UiController *uiController = Core::self()->uiControllerInternal();
    if( !uiController->activeSublimeWindow() || !uiController->activeSublimeWindow()->activeView() ) return 0;
    return dynamic_cast<IDocument*>(uiController->activeSublimeWindow()->activeView()->document());
}

void DocumentController::registerDocumentForMimetype( const QString& mimetype,
                                        KDevelop::IDocumentFactory* factory )
{
    if( !d->factories.contains( mimetype ) )
        d->factories[mimetype] = factory;
}

}

#include "documentcontroller.moc"

