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

#include <QTabWidget>
#include <QTimer>
#include <QFileInfo>

#include <interfaces/idocument.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/ipatchexporter.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iuicontroller.h>

#include <kparts/part.h>
#include <kparts/factory.h>
#include <KLocalizedString>

#include <libkomparediff2/komparemodellist.h>
#include <libkomparediff2/kompare.h>
#include <libkomparediff2/diffsettings.h>

#include <kmessagebox.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

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
#include "patchhighlighter.h"
#include "patchreviewtoolview.h"
#include "localpatchsource.h"
#include "debug.h"
#include <ktexteditor/modificationinterface.h>
#include <ktexteditor/movingrange.h>
#include <KIO/NetAccess>
#include <KActionCollection>

Q_LOGGING_CATEGORY(PLUGIN_PATCHREVIEW, "kdevplatform.plugins.patchreview")

using namespace KDevelop;

namespace
{
// Maximum number of files to open directly within a tab when the review is started
const int maximumFilesToOpenDirectly = 15;
}

Q_DECLARE_METATYPE( const Diff2::DiffModel* )

void PatchReviewPlugin::seekHunk( bool forwards, const QUrl& fileName ) {
    try {
        qCDebug(PLUGIN_PATCHREVIEW) << forwards << fileName << fileName.isEmpty();
        if ( !m_modelList )
            throw "no model";

        for ( int a = 0; a < m_modelList->modelCount(); ++a ) {
            const Diff2::DiffModel* model = m_modelList->modelAt( a );
            if ( !model || !model->differences() )
                continue;

            QUrl file = urlForFileModel( model );

            if ( !fileName.isEmpty() && fileName != file )
                continue;

            IDocument* doc = ICore::self()->documentController()->documentForUrl( file );

            if ( doc && m_highlighters.contains( doc->url() ) && m_highlighters[doc->url()] ) {
                if ( doc->textDocument() ) {
                    const QList<KTextEditor::MovingRange*> ranges = m_highlighters[doc->url()]->ranges();

                    KTextEditor::View * v = doc->activeTextView();
                    int bestLine = -1;
                    if ( v ) {
                        KTextEditor::Cursor c = v->cursorPosition();
                        for ( QList<KTextEditor::MovingRange*>::const_iterator it = ranges.begin(); it != ranges.end(); ++it ) {
                            int line = ( *it )->start().line();

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
                        } else if(fileName.isEmpty()) {
                            int next = qBound(0, forwards ? a+1 : a-1, m_modelList->modelCount()-1);
                            ICore::self()->documentController()->openDocument(urlForFileModel(m_modelList->modelAt(next)));
                        }
                    }
                }
            }
        }
    } catch ( const QString & str ) {
        qCDebug(PLUGIN_PATCHREVIEW) << "seekHunk():" << str;
    } catch ( const char * str ) {
        qCDebug(PLUGIN_PATCHREVIEW) << "seekHunk():" << str;
    }
    qCDebug(PLUGIN_PATCHREVIEW) << "no matching hunk found";
}

void PatchReviewPlugin::addHighlighting( const QUrl& highlightFile, IDocument* document ) {
    try {
        if ( !modelList() )
            throw "no model";

        for ( int a = 0; a < modelList()->modelCount(); ++a ) {
            Diff2::DiffModel* model = modelList()->modelAt( a );
            if ( !model )
                continue;

            QUrl file = urlForFileModel( model );

            if ( file != highlightFile )
                continue;

            qCDebug(PLUGIN_PATCHREVIEW) << "highlighting" << file.toDisplayString();

            IDocument* doc = document;
            if( !doc )
                doc = ICore::self()->documentController()->documentForUrl( file );

            qCDebug(PLUGIN_PATCHREVIEW) << "highlighting file" << file << "with doc" << doc;

            if ( !doc || !doc->textDocument() )
                continue;

            removeHighlighting( file );

            m_highlighters[file] = new PatchHighlighter( model, doc, this );
        }
    } catch ( const QString & str ) {
        qCDebug(PLUGIN_PATCHREVIEW) << "highlightFile():" << str;
    } catch ( const char * str ) {
        qCDebug(PLUGIN_PATCHREVIEW) << "highlightFile():" << str;
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

            QUrl file = urlForFileModel( model );

            addHighlighting( file );
        }
    } catch ( const QString & str ) {
        qCDebug(PLUGIN_PATCHREVIEW) << "highlightFile():" << str;
    } catch ( const char * str ) {
        qCDebug(PLUGIN_PATCHREVIEW) << "highlightFile():" << str;
    }
}

void PatchReviewPlugin::removeHighlighting( const QUrl& file ) {
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
    qCDebug(PLUGIN_PATCHREVIEW) << "notifying patch change: " << m_patch->file();
    m_updateKompareTimer->start( 500 );
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

    qCDebug(PLUGIN_PATCHREVIEW) << "updating model";
    try {
        removeHighlighting();
        m_modelList.reset( 0 );
        delete m_diffSettings;

        {
            IDocument* patchDoc = ICore::self()->documentController()->documentForUrl( m_patch->file() );
            if( patchDoc )
                patchDoc->reload();
        }
        QString patchFile;
        if( m_patch->file().isLocalFile() )
            patchFile = m_patch->file().toLocalFile();
        else if( m_patch->file().isValid() && !m_patch->file().isEmpty() ) {
            bool ret = KIO::NetAccess::download( m_patch->file(), patchFile, ICore::self()->uiController()->activeMainWindow() );
            if( !ret )
                qWarning() << "Problem while downloading: " << m_patch->file();
        }

        m_diffSettings = new DiffSettings( 0 );
        m_kompareInfo.reset( new Kompare::Info() );
        m_kompareInfo->localDestination = patchFile;
        m_kompareInfo->localSource = m_patch->baseDir().toLocalFile();
        m_kompareInfo->depth = m_patch->depth();
        m_kompareInfo->applied = m_patch->isAlreadyApplied();

        m_modelList.reset( new Diff2::KompareModelList( m_diffSettings.data(), new QWidget, this ) );
        m_modelList->slotKompareInfo( m_kompareInfo.data() );

        try {
            m_modelList->openDirAndDiff();
        } catch ( const QString & str ) {
            throw;
        } catch ( ... ) {
            throw QStringLiteral( "lib/libdiff2 crashed, memory may be corrupted. Please restart kdevelop." );
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

K_PLUGIN_FACTORY_WITH_JSON(KDevProblemReporterFactory, "kdevpatchreview.json", registerPlugin<PatchReviewPlugin>();)

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
    // Tweak to work around a crash on OS X; see https://bugs.kde.org/show_bug.cgi?id=338829
    // and http://qt-project.org/forums/viewthread/38406/#162801
    m_patch->deleteLater();
}

void PatchReviewPlugin::clearPatch( QObject* _patch ) {
    qCDebug(PLUGIN_PATCHREVIEW) << "clearing patch" << _patch << "current:" << ( QObject* )m_patch;
    IPatchSource::Ptr patch( ( IPatchSource* )_patch );

    if( patch == m_patch ) {
        qCDebug(PLUGIN_PATCHREVIEW) << "is current patch";
        setPatch( IPatchSource::Ptr( new LocalPatchSource ) );
    }
}

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

void PatchReviewPlugin::finishReview( QList<QUrl> selection ) {
    if( m_patch && m_patch->finishReview( selection ) ) {
        closeReview();
    }
}

void PatchReviewPlugin::startReview( IPatchSource* patch, IPatchReview::ReviewMode mode ) {
    Q_UNUSED( mode );
    emit startingNewReview();
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
        w->area()->setWorkingSet( QStringLiteral( "review_%1" ).arg( rand() % 10000 ) );

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

    if( !m_modelList )
        return;

    // list of opened documents to prevent flicker
    QMap<QUrl, IDocument*> documents;
    foreach( IDocument* doc, ICore::self()->documentController()->openDocuments() ) {
        documents[doc->url()] = doc;
    }

    IDocument* futureActiveDoc = 0;
    //Open the diff itself
    if ( !documents.contains( m_patch->file() ) ) {
        futureActiveDoc = ICore::self()->documentController()->openDocument( m_patch->file() );
    } else {
        futureActiveDoc = documents.take( m_patch->file() );
    }
    if ( !futureActiveDoc || !futureActiveDoc->textDocument() ) {
        // might happen if e.g. openDocument dialog was cancelled by user
        // or under the theoretic possibility of a non-text document getting opened
        return;
    }
    futureActiveDoc->textDocument()->setReadWrite( false );
    futureActiveDoc->setPrettyName( i18n( "Overview" ) );

    IDocument* buddyDoc = futureActiveDoc;

    KTextEditor::ModificationInterface* modif = dynamic_cast<KTextEditor::ModificationInterface*>( futureActiveDoc->textDocument() );
    modif->setModifiedOnDiskWarning( false );

    if( m_modelList->modelCount() < maximumFilesToOpenDirectly ) {
        //Open all relates files
        for( int a = 0; a < m_modelList->modelCount(); ++a ) {
            QUrl absoluteUrl = urlForFileModel( m_modelList->modelAt( a ) );

            if( QFileInfo( absoluteUrl.path() ).exists() && absoluteUrl.path() != "/dev/null" )
            {
                buddyDoc = ICore::self()->documentController()->openDocument( absoluteUrl, KTextEditor::Range::invalid(), IDocumentController::DoNotActivate, "", buddyDoc );
                documents.remove( absoluteUrl );

                seekHunk( true, absoluteUrl ); //Jump to the first changed position
            }else{
                // Maybe the file was deleted
                qCDebug(PLUGIN_PATCHREVIEW) << "could not open" << absoluteUrl << "because it doesn't exist";
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
        disconnect( m_patch.data(), &IPatchSource::patchChanged, this, &PatchReviewPlugin::notifyPatchChanged );
        if ( qobject_cast<LocalPatchSource*>( m_patch ) ) {
            // make sure we don't leak this
            // TODO: what about other patch sources?
            delete m_patch;
        }
    }
    m_patch = patch;

    if( m_patch ) {
        qCDebug(PLUGIN_PATCHREVIEW) << "setting new patch" << patch->name() << "with file" << patch->file() << "basedir" << patch->baseDir();

        connect( m_patch.data(), &IPatchSource::patchChanged, this, &PatchReviewPlugin::notifyPatchChanged );
    }
    QString finishText = i18n( "Finish Review" );
    if( m_patch && !m_patch->finishReviewCustomText().isEmpty() )
      finishText = m_patch->finishReviewCustomText();
    m_finishReview->setText( finishText );

    notifyPatchChanged();
}

PatchReviewPlugin::PatchReviewPlugin( QObject *parent, const QVariantList & )
    : KDevelop::IPlugin( "kdevpatchreview", parent ),
    m_patch( 0 ), m_factory( new PatchReviewToolViewFactory( this ) ) {
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IPatchReview )
    qRegisterMetaType<const Diff2::DiffModel*>( "const Diff2::DiffModel*" );

    core()->uiController()->addToolView( i18n( "Patch Review" ), m_factory );
    setXMLFile( "kdevpatchreview.rc" );

    connect( ICore::self()->documentController(), &IDocumentController::documentClosed, this, &PatchReviewPlugin::documentClosed );
    connect( ICore::self()->documentController(), &IDocumentController::textDocumentCreated, this, &PatchReviewPlugin::textDocumentCreated );
    connect( ICore::self()->documentController(), &IDocumentController::documentSaved, this, &PatchReviewPlugin::documentSaved );

    m_updateKompareTimer = new QTimer( this );
    m_updateKompareTimer->setSingleShot( true );
    connect( m_updateKompareTimer, &QTimer::timeout, this, &PatchReviewPlugin::updateKompareModel );

    m_finishReview = new QAction(this);
    m_finishReview->setIcon( QIcon::fromTheme( "dialog-ok" ) );
    actionCollection()->setDefaultShortcut( m_finishReview, Qt::CTRL|Qt::Key_Return );
    actionCollection()->addAction("commit_or_finish_review", m_finishReview);
    ICore::self()->uiController()->activeArea()->addAction(m_finishReview);

    areaChanged(ICore::self()->uiController()->activeArea());
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
    if (m_patch) {
        addHighlighting( doc->url(), doc );
    }
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
        qCDebug(PLUGIN_PATCHREVIEW) << "exporting patch" << exporter << action->text();
        // for git projects, m_patch will be a VCSDiffPatchSource instance
        exporter->extension<IPatchExporter>()->exportPatch( patch() );
    }
}

void PatchReviewPlugin::areaChanged(Sublime::Area* area)
{
    bool reviewing = area->objectName() == "review";
    m_finishReview->setEnabled(reviewing);
    if(!reviewing) {
        closeReview();
    }
}

#include "patchreview.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
