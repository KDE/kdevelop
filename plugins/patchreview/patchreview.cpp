/***************************************************************************
   Copyright 2006-2009 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "patchreview.h"

#include <kmimetype.h>
#include <kmimetypechooser.h>
#include <kmimetypetrader.h>
#include <krandom.h>
#include <QTabWidget>
#include <QTimer>
#include <QPersistentModelIndex>
#include <kfiledialog.h>
#include <interfaces/idocument.h>
#include <interfaces/icore.h>
#include <kde_terminal_interface.h>
#include <kparts/part.h>
#include <kparts/factory.h>

#include "libdiff2/komparemodellist.h"
#include "libdiff2/kompare.h"
#include <kmessagebox.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/markinterface.h>
#include <ktexteditor/movinginterface.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iuicontroller.h>

///Whether arbitrary exceptions that occurred while diff-parsing within the library should be caught
#define CATCHLIBDIFF

/* Exclude this file from doublequote_chars check as krazy doesn't understand
   std::string*/
//krazy:excludeall=doublequote_chars
#include <sublime/controller.h>
#include <sublime/mainwindow.h>
#include <sublime/area.h>
#include <sublime/document.h>
#include <sublime/view.h>
#include "diffsettings.h"
#include <interfaces/iplugincontroller.h>
#include <interfaces/ipatchexporter.h>
#include "patchhighlighter.h"
#include "patchreviewtoolview.h"
#include <vcs/models/vcsfilechangesmodel.h>
#include <shell/core.h>
#include <ktexteditor/modificationinterface.h>

using namespace KDevelop;

namespace
{
// Maximum number of files to open directly within a tab when the review is started
const int maximumFilesToOpenDirectly = 15;
}

Q_DECLARE_METATYPE( const Diff2::DiffModel* )

void PatchReviewPlugin::seekHunk( bool forwards, const KUrl& fileName ) {
    try {
        if ( !m_modelList.get() )
            throw "no model";

        for ( int a = 0; a < m_modelList->modelCount(); ++a ) {
            const Diff2::DiffModel* model = m_modelList->modelAt( a );
            if ( !model || !model->differences() )
                continue;

            KUrl file = urlForFileModel( model );

            if ( !fileName.isEmpty() && fileName != file )
                continue;

            IDocument* doc = ICore::self()->documentController()->documentForUrl( file );

            if ( doc && doc == ICore::self()->documentController()->activeDocument() && m_highlighters.contains( doc->url() ) && m_highlighters[doc->url()] ) {
                ICore::self()->documentController()->activateDocument( doc );
                if ( doc->textDocument() ) {
                    const QList<KTextEditor::MovingRange*> ranges = m_highlighters[doc->url()]->ranges();

                    KTextEditor::View * v = doc->textDocument()->activeView();
                    int bestLine = -1;
                    if ( v ) {
                        KTextEditor::Cursor c = v->cursorPosition();
                        for ( QList<KTextEditor::MovingRange*>::const_iterator it = ranges.begin(); it != ranges.end(); ++it ) {
                            int line;

                            line = ( *it )->start().line();

                            if ( forwards ) {
                                if ( line > c.line() && ( bestLine == -1 || line < bestLine ) )
                                    bestLine = line;
                            } else {
                                if ( line < c.line() && ( bestLine == -1 || line > bestLine ) )
                                    bestLine = line;
                            }
                        }
                        if ( bestLine != -1 ) {
                            v->setCursorPosition( KTextEditor::Cursor( bestLine, 0 ) );
                            return;
                        }
                    }
                }
            }
        }
    } catch ( const QString & str ) {
        kDebug() << "seekHunk():" << str;
    } catch ( const char * str ) {
        kDebug() << "seekHunk():" << str;
    }
    kDebug() << "no matching hunk found";
}

void PatchReviewPlugin::addHighlighting( const KUrl& highlightFile, IDocument* document ) {
    try {
        if ( !modelList() )
            throw "no model";

        for ( int a = 0; a < modelList()->modelCount(); ++a ) {
            Diff2::DiffModel* model = modelList()->modelAt( a );
            if ( !model )
                continue;

            KUrl file = urlForFileModel( model );

            if ( file != highlightFile )
                continue;

            kDebug() << "highlighting" << file.prettyUrl();

            IDocument* doc = document;
            if( !doc )
                doc = ICore::self()->documentController()->documentForUrl( file );

            kDebug() << "highlighting file" << file << "with doc" << doc;

            if ( !doc || !doc->textDocument() )
                continue;

            removeHighlighting( file );

            m_highlighters[file] = new PatchHighlighter( model, doc, this );
        }
    } catch ( const QString & str ) {
        kDebug() << "highlightFile():" << str;
    } catch ( const char * str ) {
        kDebug() << "highlightFile():" << str;
    }
}

void PatchReviewPlugin::highlightPatch() {
    try {
        if ( !modelList() )
            throw "no model";

        for ( int a = 0; a < modelList()->modelCount(); ++a ) {
            const Diff2::DiffModel* model = modelList()->modelAt( a );
            if ( !model )
                continue;

            KUrl file = urlForFileModel( model );

            addHighlighting( file );
        }
    } catch ( const QString & str ) {
        kDebug() << "highlightFile():" << str;
    } catch ( const char * str ) {
        kDebug() << "highlightFile():" << str;
    }
}

void PatchReviewPlugin::removeHighlighting( const KUrl& file ) {
    if ( file.isEmpty() ) {
        ///Remove all highlighting
        qDeleteAll( m_highlighters );
        m_highlighters.clear();
    } else {
        HighlightMap::iterator it = m_highlighters.find( file );
        if ( it != m_highlighters.end() ) {
            delete *it;
            m_highlighters.erase( it );
        }
    }
}

void PatchReviewPlugin::notifyPatchChanged() {
    kDebug() << "notifying patch change: " << m_patch->file();
    m_updateKompareTimer->start( 500 );
}

void PatchReviewPlugin::showPatch() {
    startReview( m_patch, OpenAndRaise );
}

void PatchReviewPlugin::forceUpdate() {
    if( m_patch ) {
        m_patch->update();

        notifyPatchChanged();
    }
}

void PatchReviewPlugin::updateKompareModel() {
    if ( !m_patch ) {
        ///TODO: this method should be cleaned up, it can be called by the timer and
        ///      e.g. https://bugs.kde.org/show_bug.cgi?id=267187 shows how it could
        ///      lead to asserts before...
        return;
    }

    kDebug() << "updating model";
    try {
        removeHighlighting();
        m_modelList.reset( 0 );
        delete m_diffSettings;

        emit patchChanged();

        {
            IDocument* patchDoc = ICore::self()->documentController()->documentForUrl( m_patch->file() );
            if( patchDoc )
                patchDoc->reload();
        }

        m_diffSettings = new DiffSettings( 0 );
        m_kompareInfo.reset( new Kompare::Info() );
        m_kompareInfo->localDestination = m_patch->file().toLocalFile();
        m_kompareInfo->localSource = m_patch->baseDir().toLocalFile();
        m_kompareInfo->depth = m_patch->depth();
        m_kompareInfo->applied = m_patch->isAlreadyApplied();

        m_modelList.reset( new Diff2::KompareModelList( m_diffSettings.data(), new QWidget, this ) );
        m_modelList->slotKompareInfo( m_kompareInfo.get() );

        try {
            if ( !m_modelList->openDirAndDiff() )
            {
#if 0
                // Don't error out on empty files, as those are valid diffs too
                if( QFileInfo( m_patch->file().toLocalFile() ).size() != 0 )
                    throw "could not open diff " + m_patch->file().prettyUrl() + " on " + m_patch->baseDir().prettyUrl();
#endif
            }
        } catch ( const QString & str ) {
            throw;
        } catch ( ... ) {
            throw QString( "lib/libdiff2 crashed, memory may be corrupted. Please restart kdevelop." );
        }

        emit patchChanged();

        for( int i = 0; i < m_modelList->modelCount(); i++ ) {
            const Diff2::DiffModel* model = m_modelList->modelAt( i );
            for( int j = 0; j < model->differences()->count(); j++ ) {
                model->differences()->at( j )->apply( m_patch->isAlreadyApplied() );
            }
        }

        highlightPatch();

        return;
    } catch ( const QString & str ) {
        KMessageBox::error( 0, str, i18n( "Kompare Model Update" ) );
    } catch ( const char * str ) {
        KMessageBox::error( 0, str, i18n( "Kompare Model Update" ) );
    }
    removeHighlighting();
    m_modelList.reset( 0 );
    m_kompareInfo.reset( 0 );
    delete m_diffSettings;

    emit patchChanged();
}

K_PLUGIN_FACTORY( KDevProblemReporterFactory, registerPlugin<PatchReviewPlugin>(); )
K_EXPORT_PLUGIN( KDevProblemReporterFactory( KAboutData( "kdevpatchreview", "kdevpatchreview", ki18n( "Patch Review" ), "0.1", ki18n( "Highlights code affected by a patch" ), KAboutData::License_GPL ) ) )

class PatchReviewToolViewFactory : public KDevelop::IToolViewFactory
{
public:
    PatchReviewToolViewFactory( PatchReviewPlugin *plugin ) : m_plugin( plugin ) {}

    virtual QWidget* create( QWidget *parent = 0 ) {
        return m_plugin->createToolView( parent );
    }

    virtual Qt::DockWidgetArea defaultPosition() {
        return Qt::BottomDockWidgetArea;
    }

    virtual QString id() const {
        return "org.kdevelop.PatchReview";
    }

private:
    PatchReviewPlugin *m_plugin;
};

PatchReviewPlugin::~PatchReviewPlugin() {
    removeHighlighting();
    delete m_patch;
}

void PatchReviewPlugin::registerPatch( IPatchSource::Ptr patch ) {
    if( !m_knownPatches.contains( patch ) ) {
        m_knownPatches << patch;
        connect( patch, SIGNAL( destroyed( QObject* ) ), SLOT( clearPatch( QObject* ) ) );
    }
}

void PatchReviewPlugin::clearPatch( QObject* _patch ) {
    kDebug() << "clearing patch" << _patch << "current:" << ( QObject* )m_patch;
    IPatchSource::Ptr patch( ( IPatchSource* )_patch );
    m_knownPatches.removeAll( patch );
    m_knownPatches.removeAll( 0 );

    if( patch == m_patch ) {
        kDebug() << "is current patch";
        if( !m_knownPatches.empty() )
            setPatch( m_knownPatches.first() );
        else
            setPatch( IPatchSource::Ptr( new LocalPatchSource ) );
    }
}

#if 0
#ifdef HAVE_KOMPARE
void showDiff( const KDevelop::VcsDiff& d ) {
    ICore::self()->uiController()->switchToArea( "review", KDevelop::IUiController::ThisWindow );
    foreach( const VcsLocation& l, d.leftTexts().keys() )
    {
        KUrl to;
        if( d.rightTexts().contains( l ) )
        {
            KTemporaryFile temp2;
            temp2.setSuffix( "2.patch" );
            //FIXME: don't leak
            temp2.setAutoRemove( false );
            temp2.open();
            QTextStream t2( &temp2 );
            t2 << d.rightTexts()[l];
            temp2.close();
            to = temp2.fileName();
        }
        else
            to = l.localUrl();

        KUrl fakeUrl( to );
        fakeUrl.setScheme( "kdevpatch" );

        IDocumentFactory* docf = ICore::self()->documentController()->factory( "text/x-patch" );
        IDocument* doc = docf->create( fakeUrl, ICore::self() );
        IPatchDocument* pdoc = dynamic_cast<IPatchDocument*>( doc );

        Q_ASSERT( pdoc );
        ICore::self()->documentController()->openDocument( doc );
        pdoc->setDiff( d.leftTexts()[l], to );
    }
}
#endif
#endif

void PatchReviewPlugin::closeReview()
{
    if( m_patch ) {
        removeHighlighting();
        m_modelList.reset( 0 );

        emit patchChanged();

        if( !dynamic_cast<LocalPatchSource*>( m_patch.data() ) ) {
            // make sure "show" button still openes the file dialog to open a custom patch file
            setPatch( new LocalPatchSource );
        }

        Sublime::MainWindow* w = dynamic_cast<Sublime::MainWindow*>( ICore::self()->uiController()->activeMainWindow() );
        if( w->area()->objectName() == "review" ) {
            if( setUniqueEmptyWorkingSet() )
                ICore::self()->uiController()->switchToArea( "code", KDevelop::IUiController::ThisWindow );
        }
    }
}

void PatchReviewPlugin::cancelReview() {
    if( m_patch ) {
        m_patch->cancelReview();
        closeReview();
    }
}

void PatchReviewPlugin::finishReview( QList<KUrl> selection ) {
    if( m_patch && m_patch->finishReview( selection ) ) {
        closeReview();
    }
}

void PatchReviewPlugin::startReview( IPatchSource* patch, IPatchReview::ReviewMode mode ) {
    Q_UNUSED( mode );
    setPatch( patch );
    QMetaObject::invokeMethod( this, "updateReview", Qt::QueuedConnection );
}

void PatchReviewPlugin::switchAreaAndMakeWorkingSetUique() {
    Sublime::MainWindow* w = dynamic_cast<Sublime::MainWindow*>( ICore::self()->uiController()->activeMainWindow() );
    if ( w->area()->objectName() != "review" )
        ICore::self()->uiController()->switchToArea( "review", KDevelop::IUiController::ThisWindow );

    setUniqueEmptyWorkingSet();
}

bool PatchReviewPlugin::isWorkingSetUnique() const {
    Sublime::MainWindow* w = dynamic_cast<Sublime::MainWindow*>( ICore::self()->uiController()->activeMainWindow() );
    foreach( Sublime::Area* area, w->areas() )
        if( area != w->area() && area->workingSet() == w->area()->workingSet() )
            return false;
    return true;
}

bool PatchReviewPlugin::setUniqueEmptyWorkingSet() {
    Sublime::MainWindow* w = dynamic_cast<Sublime::MainWindow*>( ICore::self()->uiController()->activeMainWindow() );

    if( !ICore::self()->documentController()->saveAllDocumentsForWindow( ICore::self()->uiController()->activeMainWindow(), KDevelop::IDocument::Default, true ) )
        return false;

    if( !w->area()->workingSet().startsWith( "review" ) )
        w->area()->setWorkingSet( "review" );

    while( !isWorkingSetUnique() )
        w->area()->setWorkingSet( QString( "review_%1" ).arg( rand() % 10000 ) );

    // We've asked the user, so just clear silently
    w->area()->clearViews( true );

    return true;
}

void PatchReviewPlugin::updateReview() {
    if( !m_patch )
        return;

    m_updateKompareTimer->stop();
    updateKompareModel();

    switchAreaAndMakeWorkingSetUique();

    if( !m_modelList.get() )
        return;

    // list of opened documents to prevent flicker
    QMap<KUrl, IDocument*> documents;
    foreach( IDocument* doc, ICore::self()->documentController()->openDocuments() ) {
        documents[doc->url()] = doc;
    }

    IDocument* futureActiveDoc = 0;
    //Open the diff itself
#ifdef HAVE_KOMPARE
    KUrl fakeUrl( m_patch->file() );
    fakeUrl.setScheme( "kdevpatch" );
    IDocumentFactory* docf = ICore::self()->documentController()->factory( "text/x-patch" );
    IDocument* doc = docf->create( fakeUrl, ICore::self() );
    IPatchDocument* pdoc = dynamic_cast<IPatchDocument*>( doc );

    Q_ASSERT( pdoc );
    futureActiveDoc = ICore::self()->documentController()->openDocument( doc );
    //TODO: close kompare doc if available
#else
    if ( !documents.contains( m_patch->file() ) ) {
        futureActiveDoc = ICore::self()->documentController()->openDocument( m_patch->file() );
    } else {
        futureActiveDoc = documents.take( m_patch->file() );
    }
#endif
    if ( !futureActiveDoc || !futureActiveDoc->textDocument() ) {
        // might happen if e.g. openDocument dialog was cancelled by user
        // or under the theoretic possibility of a non-text document getting opened
        return;
    }
    futureActiveDoc->textDocument()->setReadWrite( false );
    futureActiveDoc->setPrettyName( i18n( "Overview" ) );
    IDocument* doc = ICore::self()->documentController()->documentForUrl( m_patch->file() );
    KTextEditor::ModificationInterface* modif = dynamic_cast<KTextEditor::ModificationInterface*>( doc->textDocument() );
    modif->setModifiedOnDiskWarning( false );

    if( m_modelList->modelCount() < maximumFilesToOpenDirectly ) {
        //Open all relates files
        for( int a = 0; a < m_modelList->modelCount(); ++a ) {
            KUrl absoluteUrl( m_patch->baseDir(),  m_modelList->modelAt( a )->destination() );

            if( QFileInfo( absoluteUrl.path() ).exists() && absoluteUrl.path() != "/dev/null" )
            {
                ICore::self()->documentController()->openDocument( absoluteUrl );
                documents.remove( absoluteUrl );

                seekHunk( true, absoluteUrl ); //Jump to the first changed position
            }else{
                // Maybe the file was deleted
                kDebug() << "could not open" << absoluteUrl << "because it doesn't exist";
            }
        }
    }

    Sublime::MainWindow* w = dynamic_cast<Sublime::MainWindow*>( ICore::self()->uiController()->activeMainWindow() );
    // Close views for documents that were loaded from the working set, but are not in the patch
    QList<IDocument*> documentsList = documents.values();
    foreach( Sublime::View* view, w->area()->views() ) {
        IDocument* doc = dynamic_cast<IDocument*>( view->document() );
        if( doc && documentsList.contains( doc ) ) {
            w->area()->closeView( view );
        }
    }

    Q_ASSERT( futureActiveDoc );
    ICore::self()->documentController()->activateDocument( futureActiveDoc );

    bool b = ICore::self()->uiController()->findToolView( i18n( "Patch Review" ), m_factory );
    Q_ASSERT( b );
    Q_UNUSED( b );
}

void PatchReviewPlugin::setPatch( IPatchSource* patch ) {
    if ( patch == m_patch ) {
        return;
    }

    if( m_patch ) {
        disconnect( m_patch, SIGNAL( patchChanged() ), this, SLOT( notifyPatchChanged() ) );
        if ( qobject_cast<LocalPatchSource*>( m_patch ) ) {
            // make sure we don't leak this
            // TODO: what about other patch sources?
            delete m_patch;
        }
    }
    m_patch = patch;

    if( m_patch ) {
        kDebug() << "setting new patch" << patch->name() << "with file" << patch->file();
        registerPatch( patch );

        connect( m_patch, SIGNAL( patchChanged() ), this, SLOT( notifyPatchChanged() ) );
    }

    notifyPatchChanged();
}

PatchReviewPlugin::PatchReviewPlugin( QObject *parent, const QVariantList & )
    : KDevelop::IPlugin( KDevProblemReporterFactory::componentData(), parent ),
    m_patch( 0 ), m_factory( new PatchReviewToolViewFactory( this ) ) {
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IPatchReview )
    qRegisterMetaType<const Diff2::DiffModel*>( "const Diff2::DiffModel*" );

    core()->uiController()->addToolView( i18n( "Patch Review" ), m_factory );
    setXMLFile( "kdevpatchreview.rc" );

    connect( ICore::self()->documentController(), SIGNAL( documentClosed( KDevelop::IDocument* ) ), this, SLOT( documentClosed( KDevelop::IDocument* ) ) );
    connect( ICore::self()->documentController(), SIGNAL( textDocumentCreated( KDevelop::IDocument* ) ), this, SLOT( textDocumentCreated( KDevelop::IDocument* ) ) );
    connect( ICore::self()->documentController(), SIGNAL( documentSaved( KDevelop::IDocument* ) ), this, SLOT( documentSaved( KDevelop::IDocument* ) ) );

    m_updateKompareTimer = new QTimer( this );
    m_updateKompareTimer->setSingleShot( true );
    connect( m_updateKompareTimer, SIGNAL( timeout() ), this, SLOT( updateKompareModel() ) );

    setPatch( IPatchSource::Ptr( new LocalPatchSource ) );
}

void PatchReviewPlugin::documentClosed( IDocument* doc ) {
    removeHighlighting( doc->url() );
}

void PatchReviewPlugin::documentSaved( IDocument* doc ) {
    // Only update if the url is not the patch-file, because our call to
    // the reload() KTextEditor function also causes this signal,
    // which would lead to an endless update loop.
    if( m_patch && doc->url() != m_patch->file() )
        forceUpdate();
}

void PatchReviewPlugin::textDocumentCreated( IDocument* doc ) {
    addHighlighting( doc->url(), doc );
}

void PatchReviewPlugin::unload() {
    core()->uiController()->removeToolView( m_factory );

    KDevelop::IPlugin::unload();
}

QWidget* PatchReviewPlugin::createToolView( QWidget* parent ) {
    return new PatchReviewToolView( parent, this );
}

void PatchReviewPlugin::exporterSelected( QAction* action ) {
    IPlugin* exporter = qobject_cast<IPlugin*>( action->data().value<QObject*>() );

    if( exporter ) {
        qDebug() << "exporting patch" << exporter << action->text();
        exporter->extension<IPatchExporter>()->exportPatch( patch() );
    }
}

#if 0
void PatchReviewPlugin::determineState() {
    LocalPatchSourcePointer lpatch = m_patch;
    if ( !lpatch ) {
        kDebug() <<"determineState(..) could not lock patch";
    }
    try {
        if ( lpatch->filename.isEmpty() )
            throw "state can only be determined for file-patches";

        KUrl fileUrl = lpatch->filename;

        {
            K3Process proc;
            ///Try to apply, if it works, the patch is not applied
            QString cmd =  "patch --dry-run -s -f -i " + fileUrl.toLocalFile();
            proc << splitArgs( cmd );

            kDebug() << "calling " << cmd;

            if ( !proc.start( K3Process::Block ) )
                throw "could not start process";

            if ( !proc.normalExit() )
                throw "process did not exit normally";

            kDebug() << "exit-status:" << proc.exitStatus();

            if ( proc.exitStatus() == 0 ) {
//                 lpatch->state = LocalPatchSource::NotApplied;
                return;
            }
        }

        {
            ///Try to revert, of it works, the patch is applied
            K3Process proc;
            QString cmd =  "patch --dry-run -s -f -i --reverse " + fileUrl.toLocalFile();
            proc << splitArgs( cmd );

            kDebug() << "calling " << cmd;

            if ( !proc.start( K3Process::Block ) )
                throw "could not start process";

            if ( !proc.normalExit() )
                throw "process did not exit normally";

            kDebug() << "exit-status:" << proc.exitStatus();

            if ( proc.exitStatus() == 0 ) {
//                 lpatch->state = LocalPatchSource::Applied;
                return;
            }
        }
    } catch ( const QString& str ) {
        kWarning() <<"Error:" << str;
    } catch ( const char* str ) {
        kWarning() << "Error:" << str;
    }

//     lpatch->state = LocalPatchSource::Unknown;
}
#endif
#include "patchreview.moc"

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
