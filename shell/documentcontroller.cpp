/* This file is part of the KDE project
Copyright 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
Copyright 2002 Bernd Gehrmann <bernd@kdevelop.org>
Copyright 2003 Roberto Raggi <roberto@kdevelop.org>
Copyright 2003-2008 Hamish Rodda <rodda@kde.org>
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
#include <QApplication>

#include <kio/netaccess.h>
#include <kfiledialog.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <krecentfilesaction.h>
#include <ktemporaryfile.h>
#include <kplugininfo.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include <sublime/area.h>
#include <sublime/view.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iprojectcontroller.h>

#include "core.h"
#include "mainwindow.h"
#include "textdocument.h"
#include "uicontroller.h"
#include "partcontroller.h"
#include "savedialog.h"


namespace KDevelop
{

struct DocumentControllerPrivate {
    DocumentControllerPrivate(DocumentController* c)
    : controller(c)
    {
    }

    ~DocumentControllerPrivate()
    {
        //delete temporary files so they are removed from disk
        foreach (KTemporaryFile *temp, tempFiles)
            delete temp;
    }

    QString presetEncoding;

    // used to map urls to open docs
    QHash< KUrl, IDocument* > documents;

    QHash< QString, IDocumentFactory* > factories;
    QList<KTemporaryFile*> tempFiles;

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
            kDebug() << "destroying document" << doc;
            documents.remove(url);
        }
    }
    void chooseDocument()
    {
        controller->openDocument(KUrl());
    }

    void changeDocumentUrl(KDevelop::IDocument* document)
    {
        QMutableHashIterator<KUrl, IDocument*> it = documents;
        while (it.hasNext()) {
            if (it.next().value() == document) {
                if (documents.contains(document->url())) {
                    // Weird situation (saving as a file that is aready open)
                    IDocument* origDoc = documents[document->url()];
                    if (origDoc->state() & IDocument::Modified) {
                        // given that the file has been saved, close the saved file as the other instance will become conflicted on disk
                        document->close();
                        controller->activateDocument( origDoc );
                        break;
                    }
                    // Otherwise close the original document
                    origDoc->close();
                } else {
                    // Remove the original document
                    it.remove();
                }

                documents.insert(document->url(), document);
                break;
            }
        }
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

/*    HistoryEntry createHistoryEntry();
    void addHistoryEntry();
    void jumpTo( const HistoryEntry & );*/
};


DocumentController::DocumentController( QObject *parent )
        : IDocumentController( parent )
{
    setObjectName("DocumentController");
    d = new DocumentControllerPrivate(this);
    QDBusConnection::sessionBus().registerObject( "/org/kdevelop/DocumentController",
        this, QDBusConnection::ExportScriptableSlots );

    connect(this, SIGNAL(documentUrlChanged(KDevelop::IDocument*)), this, SLOT(changeDocumentUrl(KDevelop::IDocument*)));

    if(!(Core::self()->setupFlags() & Core::NoUi)) setupActions();
}

void KDevelop::DocumentController::initialize()
{
}

void DocumentController::cleanup()
{
    d->fileOpenRecent->saveEntries( KConfigGroup(KGlobal::config(), "Recent Files" ) );

    // Close all documents without checking if they should be saved.
    // This is because the user gets a chance to save them during MainWindow::queryClose.
    foreach (Sublime::MainWindow* mw, Core::self()->uiControllerInternal()->mainWindows())
        foreach (IDocument* doc, documentsInWindow(dynamic_cast<KDevelop::MainWindow*>(mw)))
            doc->close(IDocument::Discard);
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
    action->setIcon(KIcon("document-open"));
    action->setShortcut( Qt::CTRL + Qt::Key_O );
    action->setText(i18n( "&Open File..." ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( chooseDocument() ) );
    action->setToolTip( i18n( "Open file" ) );
    action->setWhatsThis( i18n( "<b>Open file</b><p>Opens a file for editing.</p>" ) );

    d->fileOpenRecent = KStandardAction::openRecent(this,
                    SLOT(slotOpenDocument(const KUrl&)), ac);
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

IDocument* DocumentController::openDocumentFromText( const QString& data )
{
    KTemporaryFile *temp = new KTemporaryFile();
    temp->setSuffix("kdevtmp");
    temp->open();
    temp->write(data.toUtf8());
    temp->flush();
    d->tempFiles << temp;
    return openDocument(temp->fileName());
}

IDocument* DocumentController::openDocument( const KUrl & inputUrl,
        const KTextEditor::Range& range,
        DocumentActivationParams activationParams)
{
    IDocument* previousActiveDocument = activeDocument();
    KTextEditor::Cursor previousActivePosition;
    if(previousActiveDocument && previousActiveDocument->textDocument() && previousActiveDocument->textDocument()->activeView())
        previousActivePosition = previousActiveDocument->textDocument()->activeView()->cursorPosition();
    

    UiController *uiController = Core::self()->uiControllerInternal();
    Sublime::Area *area = uiController->activeArea();

    KUrl url = inputUrl;

    if ( url.isEmpty() && (!activationParams.testFlag(IDocumentController::DoNotCreateView)) )
    {
        KUrl dir;
        if( activeDocument() )
        {
            dir = KUrl( activeDocument()->url().directory() );
        }else
        {
            dir = Core::self()->projectController()->projectsBaseDirectory();
        }

        url = KFileDialog::getOpenUrl( dir, i18n( "*.*|Text File\n" ),
                                       Core::self()->uiControllerInternal()->defaultMainWindow(),
                                       i18n( "Open File" ) );
    }
    if ( url.isEmpty() )
        //still no url
        return 0;

    bool emitOpened = false;

    //get a part document
    if (!d->documents.contains(url))
    {
        //make sure the URL exists
        if ( !url.isValid() || !KIO::NetAccess::exists( url, KIO::NetAccess::DestinationSide, 0 ) )
        {
            kDebug() << "cannot find URL:" << url.url();
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
            kDebug() << "cannot open directory:" << url.url();
            return 0;
        }

        // Try to find a plugin that handles this mimetype
        QString constraint = QString("'%1' in [X-KDevelop-SupportedMimeTypes]").arg(mimeType->name());
        KPluginInfo::List plugins = IPluginController::queryPlugins( constraint );

        if( !plugins.isEmpty() )
        {
            KPluginInfo info = plugins.first();
            kDebug() << "loading" << info.pluginName();
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
        if ( !d->documents.contains(url) && Core::self()->partControllerInternal()->isTextType(mimeType))
            d->documents[url] = new TextDocument(url, Core::self());
        else if( !d->documents.contains(url) )
            d->documents[url] = new PartDocument(url, Core::self());
        emitOpened = d->documents.contains(url);
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

    if (!activationParams.testFlag(IDocumentController::DoNotCreateView))
    {
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
        bool addView = false, applyRange = true;
        if (!partView)
        {
            //no view currently shown for this url
            partView = sdoc->createView();
            addView = true;
        }
        
        KDevelop::TextView* textView = dynamic_cast<KDevelop::TextView*>(partView);
        if(textView && textView->textView()) {
            applyRange = false;
            if (range.isEmpty())
                textView->textView()->setCursorPosition( range.start() );
            else
                textView->textView()->setSelection( range );
        }else if(textView) {
            textView->setInitialRange(range);
        }
        
        if(addView) {
            //add view to the area
            area->addView(partView, uiController->activeSublimeWindow()->activeView());
        }
        
        if (!activationParams.testFlag(IDocumentController::DoNotActivate))
        {
            uiController->activeSublimeWindow()->activateView(partView);
        }
        d->fileOpenRecent->addUrl( url );

        if( applyRange && range.isValid() )
        {
            if (range.isEmpty())
                doc->setCursorPosition( range.start() );
            else
                doc->setTextSelection( range );
        }
    }

    // Deferred signals, wait until it's all ready first
    if( emitOpened ) {
        emit documentOpened( d->documents[url] );
    }

    if (!activationParams.testFlag(IDocumentController::DoNotActivate))
        emit documentActivated( doc );

    d->saveAll->setEnabled(true);
    d->revertAll->setEnabled(true);
    d->close->setEnabled(true);
    d->closeAll->setEnabled(true);
    d->closeAllOthers->setEnabled(true);

    if(doc) {
        KTextEditor::Cursor activePosition;
        if(doc->textDocument() && doc->textDocument()->activeView())
            activePosition = doc->textDocument()->activeView()->cursorPosition();
        
        emit documentJumpPerformed(doc, range.start(), previousActiveDocument, previousActivePosition);
    }
    
    return doc;
}

void DocumentController::fileClose()
{
    IDocument *activeDoc = activeDocument();
    if (activeDoc)
    {
        UiController *uiController = Core::self()->uiControllerInternal();
        Sublime::View *activeView = uiController->activeSublimeWindow()->activeView();
        if (activeView->document()->views().count() > 1)
        {
            //close only one active view
            Sublime::View *deletedView = uiController->activeArea()->removeView(activeView);
            deletedView->deleteLater();
        }
        else
        {
            //close the document instead
            activeDoc->close();
        }
    }
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
    foreach (IDocument *doc, d->documents)
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

void DocumentController::activateDocument( IDocument * document, const KTextEditor::Range& range )
{
    // TODO avoid some code in openDocument?
    openDocument(document->url(), range);
}

void DocumentController::slotSaveAllDocuments()
{
    saveAllDocuments(IDocument::Silent);
}

bool DocumentController::saveAllDocuments(IDocument::DocumentSaveMode mode)
{
    return saveSomeDocuments(openDocuments(), mode);
}

bool KDevelop::DocumentController::saveSomeDocuments(const QList< IDocument * > & list, IDocument::DocumentSaveMode mode)
{
    if (mode & IDocument::Silent) {
        foreach (IDocument* doc, modifiedDocuments(list)) {
            bool ret = doc->save(mode);
            Q_ASSERT(ret);
            // TODO if (!ret) showErrorDialog() ?
        }

    } else {
        // Ask the user which documents to save
        QList<IDocument*> checkSave = modifiedDocuments(list);

        if (!checkSave.isEmpty()) {
            KSaveSelectDialog dialog(checkSave, qApp->activeWindow());
            if (dialog.exec() == QDialog::Rejected)
                return false;
        }
    }

    return true;
}

QList< IDocument * > KDevelop::DocumentController::documentsInWindow(MainWindow * mw) const
{
    // Gather a list of all documents which do have a view in the given main window
    QList<IDocument*> list;
    foreach (IDocument* doc, openDocuments()) {
        if (Sublime::Document* sdoc = dynamic_cast<Sublime::Document*>(doc)) {
            foreach (Sublime::View* view, sdoc->views()) {
                if (view->hasWidget() && view->widget()->window() == mw) {
                    list.append(doc);
                    break;
                }
            }
        }
    }
    return list;
}

QList< IDocument * > KDevelop::DocumentController::documentsExclusivelyInWindow(MainWindow * mw) const
{
    // Gather a list of all documents which have views only in the given main window
    QList<IDocument*> checkSave;
    foreach (IDocument* doc, openDocuments()) {
        if (Sublime::Document* sdoc = dynamic_cast<Sublime::Document*>(doc)) {
            bool inOtherWindow = false;

            foreach (Sublime::View* view, sdoc->views()) {
                if (view->hasWidget() && view->widget()->window() != mw) {
                    inOtherWindow = true;
                    break;
                }
            }

            if (!inOtherWindow)
                checkSave.append(doc);
        }
    }
    return checkSave;
}

QList< IDocument * > KDevelop::DocumentController::modifiedDocuments(const QList< IDocument * > & list) const
{
    QList< IDocument * > ret;
    foreach (IDocument* doc, list)
        if (doc->state() == IDocument::Modified || doc->state() == IDocument::DirtyAndModified)
            ret.append(doc);
    return ret;
}

bool DocumentController::saveAllDocumentsForWindow(MainWindow* mw, IDocument::DocumentSaveMode mode)
{
    QList<IDocument*> checkSave = documentsExclusivelyInWindow(mw);

    return saveSomeDocuments(checkSave, mode);
}

void DocumentController::reloadAllDocuments()
{
    if (Sublime::MainWindow* mw = Core::self()->uiControllerInternal()->activeSublimeWindow()) {
        QList<IDocument*> views = documentsInWindow(dynamic_cast<KDevelop::MainWindow*>(mw));

        if (!saveSomeDocuments(views, IDocument::Default))
            // User cancelled or other error
            return;

        foreach (IDocument* doc, views)
            doc->reload();
    }
}

void DocumentController::closeAllDocuments()
{
    if (Sublime::MainWindow* mw = Core::self()->uiControllerInternal()->activeSublimeWindow()) {
        QList<IDocument*> views = documentsInWindow(dynamic_cast<KDevelop::MainWindow*>(mw));

        if (!saveSomeDocuments(views, IDocument::Default))
            // User cancelled or other error
            return;

        foreach (IDocument* doc, views)
            doc->close(IDocument::Discard);
    }
}

void DocumentController::closeAllOtherDocuments()
{
    if (Sublime::MainWindow* mw = Core::self()->uiControllerInternal()->activeSublimeWindow()) {
        Sublime::View* activeView = mw->activeView();

        if (!activeView) {
            kWarning() << "Shouldn't there always be an active view when this function is called?";
            return;
        }

        // Deal with saving unsaved solo views
        QList<IDocument*> soloViews = documentsExclusivelyInWindow(dynamic_cast<KDevelop::MainWindow*>(mw));
        soloViews.removeAll(dynamic_cast<IDocument*>(activeView->document()));

        if (!saveSomeDocuments(soloViews, IDocument::Default))
            // User cancelled or other error
            return;

        foreach (Sublime::View* view, mw->area()->views()) {
            if (view != activeView) {
                if (view->document()->views().count() > 1)
                {
                    //close only the view in question
                    mw->area()->removeView(view);
                    view->deleteLater();
                }
                else
                {
                    //close the document instead as no views remain
                    IDocument* doc = dynamic_cast<IDocument*>(view->document());
                    if (doc) {
                        doc->close(IDocument::Discard);

                    } else {
                        // Fallback, ick
                        kWarning() << "Tried to close non-IDocument sublime document";
                        mw->area()->removeView(view);
                        view->deleteLater();
                    }
                }
            }
        }
    }
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

QStringList DocumentController::documentTypes() const
{
    return QStringList() << "Text";
}

}

#include "documentcontroller.moc"
