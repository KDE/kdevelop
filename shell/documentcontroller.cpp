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
#include <ktexteditor/document.h>

#include <sublime/area.h>
#include <sublime/view.h>

#include "core.h"
#include "mainwindow.h"
#include "textdocument.h"
#include "uicontroller.h"
#include "partcontroller.h"
#include "iplugincontroller.h"

#include <kplugininfo.h>

namespace KDevelop
{

struct DocumentControllerPrivate {
    DocumentControllerPrivate(DocumentController* c)
    : controller(c), cleaningUp(false)
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
    QPointer<KAction> close;
    QPointer<KAction> closeAll;
    QPointer<KAction> closeAllOthers;
    KRecentFilesAction* fileOpenRecent;
    bool cleaningUp;

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

void DocumentController::deinitialize()
{
    d->fileOpenRecent->saveEntries( KConfigGroup(KGlobal::config(), "Recent Files" ) );
}

DocumentController::~DocumentController()
{
    delete d;
}

void DocumentController::setupActions()
{
    KActionCollection * ac =
        Core::self()->uiControllerInternal()->defaultMainWindow()->actionCollection();

    KAction *action;

    action = ac->addAction( "file_open" );
    action->setIcon(KIcon("file_open"));
    action->setText(i18n( "&Open File..." ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( chooseDocument() ) );
    action->setToolTip( i18n( "Open file" ) );
    action->setWhatsThis( i18n( "<b>Open file</b><p>Opens a file for editing.</p>" ) );

    d->fileOpenRecent = KStandardAction::openRecent(this, SLOT(slotOpenDocument(const KUrl&)), this);
    ac->addAction(d->fileOpenRecent->objectName(), d->fileOpenRecent);
    d->fileOpenRecent->setWhatsThis(i18n("This lists files which you have opened recently, and allows you to easily open them again."));
    d->fileOpenRecent->loadEntries( KConfigGroup(KGlobal::config(), "Recent Files" ) );

    action = d->saveAll = ac->addAction( "file_save_all" );
    action->setIcon(KIcon("document-save"));
    action->setText(i18n( "Save Al&l" ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( slotSaveAllDocuments() ) );
    action->setToolTip( i18n( "Save all open documents" ) );
    action->setWhatsThis( i18n( "<b>Save all documents</b><p>Save all open documents, prompting for additional information when necessary.</p>" ) );
    action->setEnabled(false);

    action = d->revertAll = ac->addAction( "file_revert_all" );
    action->setIcon(KIcon("document-revert"));
    action->setText(i18n( "Rever&t All" ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( reloadAllDocuments() ) );
    action->setToolTip( i18n( "Revert all open documents" ) );
    action->setWhatsThis( i18n( "<b>Revert all documents</b><p>Revert all open documents, returning to the previously saved state.</p>" ) );
    action->setEnabled(false);

    action = d->close = ac->addAction( "file_close" );
    action->setIcon(KIcon("window-close"));
    action->setShortcut( Qt::CTRL + Qt::Key_W );
    action->setText( i18n( "&Close File" ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( fileClose() ) );
    action->setToolTip( i18n( "Close File" ) );
    action->setWhatsThis( i18n( "<b>Close File</b><p>Closes current file.</p>" ) );
    action->setEnabled(false);

    action = d->closeAll = ac->addAction( "file_close_all" );
    action->setIcon(KIcon("window-close"));
    action->setText(i18n( "Clos&e All" ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( closeAllDocuments() ) );
    action->setToolTip( i18n( "Close all open documents" ) );
    action->setWhatsThis( i18n( "<b>Close all documents</b><p>Close all open documents, prompting for additional information when necessary.</p>" ) );
    action->setEnabled(false);

    action = d->closeAllOthers = ac->addAction( "file_closeother" );
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

void DocumentController::openDocumentFromText( const QString& data )
{
    TextDocument *doc = new TextDocument(KUrl(), Core::self());
    Sublime::View *view = doc->createView();
    Core::self()->uiControllerInternal()->activeArea()->addView(view);
    Core::self()->uiControllerInternal()->activeSublimeWindow()->activateView(view);
    doc->textDocument()->setText( data );

    /* Not calling saveDocumentList here, since it's a bit tricky
       to restore a document having no URL.  */
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
    if( emitLoaded ) {
        emit documentLoadedPrepare( d->documents[url] );
        emit documentLoaded( d->documents[url] );
    }

    if (activate == IDocumentController::ActivateOnOpen)
        emit documentActivated( doc );

    d->saveAll->setEnabled(true);
    d->revertAll->setEnabled(true);
    d->close->setEnabled(true);
    d->closeAll->setEnabled(true);
    d->closeAllOthers->setEnabled(true);

    return doc;
}

void DocumentController::fileClose()
{
    if (activeDocument())
        activeDocument()->close();
}

void DocumentController::closeDocument( const KUrl &url )
{
    if( !d->documents.contains(url) )
        return;

    //this will remove all views and after the last view is removed, the
    //document will be self-destructed and removeDocument() slot will catch that
    //and clean up internal data structures
    d->documents[url]->close();
}

void DocumentController::notifyDocumentClosed(IDocument* doc)
{
    d->documents.remove(doc->url());

    if (d->documents.isEmpty()) {
        if (d->saveAll)
            d->saveAll->setEnabled(false);
        if (d->revertAll)
            d->revertAll->setEnabled(false);
        if (d->close)
            d->close->setEnabled(false);
        if (d->closeAll)
            d->closeAll->setEnabled(false);
        if (d->closeAllOthers)
            d->closeAllOthers->setEnabled(false);
    }

    emit documentClosed(doc);
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

void DocumentController::saveDocumentList()
{
    if (!d || d->cleaningUp)
        return;

    KConfigGroup cg = KGlobal::config()->group("Documents");
    QList<QString> urls;
    QHash< KUrl, IDocument* >::iterator i, e;
    for (i = d->documents.begin(), e = d->documents.end(); i != e; ++i)
        /* prettyUrl strips passwords from URL, which is probably
           good idea, assuming we don't want passwords to be
           stored in plain text in random config files. */
        urls.push_back(i.key().prettyUrl());
    cg.writeEntry("urls", urls);
    cg.sync();
}

void DocumentController::restoreDocumentList()
{
    KConfigGroup cg = KGlobal::config()->group("Documents");
    QList<QString> urls = cg.readEntry("urls", QList<QString>());
    /* Presently, if an URL was shown in several Sublime::Area
       instances, we won't properly restore that. The
       openDocument and Area should coordinate such details
       in future -- this function only cares about having
       the right list of URLs opened, not where they are shown. */
    foreach (const QString& s, urls)
    {
        openDocument(KUrl(s));
    }
}

void DocumentController::cleanup()
{
    /* From now on, we don't save document list, so that
       we don't make it empty as documents are closed
       during shut-down.  */
    d->cleaningUp = true;
}

QStringList DocumentController::documentTypes() const
{
    return QStringList() << "Text";
}

Sublime::Document* DocumentController::createDocument(const QString & type, const QString & specifier)
{
    if (type == "Text")
        return new TextDocument(KUrl(specifier), Core::self());

    return 0;
}

void DocumentController::saveArea(Sublime::Area * area, KConfigGroup & group)
{
    saveArea(area->rootIndex(), group);
}

void DocumentController::saveArea(Sublime::AreaIndex * area, KConfigGroup & group)
{
    if (area->isSplitted()) {
        group.writeEntry("Orientation", area->orientation() == Qt::Horizontal ? "Horizontal" : "Vertical");

        if (area->first()) {
            KConfigGroup subgroup(&group, "0");
            subgroup.deleteGroup();
            saveArea(area->first(), subgroup);
        }

        if (area->second()) {
            KConfigGroup subgroup(&group, "1");
            subgroup.deleteGroup();
            saveArea(area->second(), subgroup);
        }
    } else {
        group.writeEntry("View Count", area->viewCount());

        int index = 0;
        foreach (Sublime::View* view, area->views()) {
            group.writeEntry(QString("View %1 Type").arg(index), view->document()->documentType());
            group.writeEntry(QString("View %1").arg(index), view->document()->documentSpecifier());
            QString state = view->viewState();
            if (!state.isEmpty())
                group.writeEntry(QString("View %1 State").arg(index), state);

            ++index;
        }
    }
}

void DocumentController::loadArea(Sublime::Area * area, const KConfigGroup & group)
{
    loadArea(area->rootIndex(), group);
}

void DocumentController::loadArea(Sublime::AreaIndex * area, const KConfigGroup & group)
{
    if (group.hasKey("Orientation")) {
        QStringList subgroups = group.groupList();

        if (subgroups.contains("0")) {
            if (!area->isSplitted())
                area->split(group.readEntry("Orientation", "Horizontal") == "Vertical" ? Qt::Vertical : Qt::Horizontal);

            KConfigGroup subgroup(&group, "0");
            loadArea(area->first(), subgroup);

            if (subgroups.contains("1")) {
                Q_ASSERT(area->isSplitted());
                KConfigGroup subgroup(&group, "1");
                loadArea(area->second(), subgroup);
            }
        }

    } else {
        while (area->isSplitted()) {
            area = area->first();
            Q_ASSERT(area);// Split area index did not contain a first child area index if this fails
        }

        int viewCount = group.readEntry("View Count", 0);
        for (int i = 0; i < viewCount; ++i) {
            QString type = group.readEntry(QString("View %1 Type").arg(i), "");
            QString specifier = group.readEntry(QString("View %1").arg(i), "");

            bool viewExists = false;
            foreach (Sublime::View* view, area->views()) {
                if (view->document()->documentSpecifier() == specifier) {
                    viewExists = true;
                    break;
                }
            }

            if (viewExists)
                continue;

            Sublime::Document* document = createDocument(type, specifier);
            if (document) {
                Sublime::View* view = document->createView();

                QString state = group.readEntry(QString("View %1 State").arg(i), "");
                if (!state.isEmpty())
                    view->setState(state);

                area->add(view);

            } else {
                kWarning() << "Unable to create view of type " << type;
            }
        }
    }
}

}

#include "documentcontroller.moc"
