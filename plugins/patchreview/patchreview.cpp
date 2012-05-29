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
#include <klineedit.h>
#include <kmimetypechooser.h>
#include <kmimetypetrader.h>
#include <krandom.h>
#include <QTabWidget>
#include <QMenu>
#include <QFile>
#include <QTimer>
#include <QPersistentModelIndex>
#include <kfiledialog.h>
#include <interfaces/idocument.h>
#include <QStandardItemModel>
#include <interfaces/icore.h>
#include <kde_terminal_interface.h>
#include <kparts/part.h>
#include <kparts/factory.h>
#include <kdialog.h>
#include <ktemporaryfile.h>

#include "libdiff2/komparemodellist.h"
#include "libdiff2/kompare.h"
#include <kmessagebox.h>
#include <QMetaType>
#include <QVariant>
#include <ktexteditor/cursor.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/markinterface.h>
#include <ktexteditor/movinginterface.h>
#include <interfaces/idocumentcontroller.h>
#include <kprocess.h>
#include <interfaces/iuicontroller.h>
#include <kaboutdata.h>

///Whether arbitrary exceptions that occurred while diff-parsing within the library should be caught
#define CATCHLIBDIFF

/* Exclude this file from doublequote_chars check as krazy doesn't understand
   std::string*/
//krazy:excludeall=doublequote_chars
#include <krun.h>
#include <kparts/mainwindow.h>
#include <qtextdocument.h>
#include <util/activetooltip.h>
#include <ktextbrowser.h>
#include <kiconeffect.h>
#include <kcolorutils.h>
#include <kcolorscheme.h>
#include <sublime/controller.h>
#include <sublime/mainwindow.h>
#include <sublime/area.h>
#include <sublime/document.h>
#include <sublime/view.h>
#include <interfaces/iprojectcontroller.h>
#include "diffsettings.h"
#include <interfaces/iplugincontroller.h>
#include <interfaces/ipatchexporter.h>
#include "standardpatchexport.h"
#include <language/highlighting/colorcache.h>
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

class PatchFilesModel : public VcsFileChangesModel
{
public:
    PatchFilesModel( QObject *parent, bool allowSelection ) : VcsFileChangesModel( parent, allowSelection ) { };
    enum ItemRoles { HunksNumberRole = VcsStatusInfoRole+1 };

public slots:
    void updateState( const KDevelop::VcsStatusInfo &status, unsigned hunksNum ) {
        int row = VcsFileChangesModel::updateState( invisibleRootItem(), status );
        if ( row == -1 )
            return;

        QStandardItem *item = invisibleRootItem()->child( row, 0 );
        setFileInfo( item, hunksNum );
        item->setData( QVariant( hunksNum ), HunksNumberRole );
    }

    void updateState( const KDevelop::VcsStatusInfo &status ) {
        int row = VcsFileChangesModel::updateState( invisibleRootItem(), status );
        if ( row == -1 )
            return;

        QStandardItem *item = invisibleRootItem()->child( row, 0 );
        setFileInfo( invisibleRootItem()->child( row, 0 ), item->data( HunksNumberRole ).toUInt() );
    }

private:
    void setFileInfo( QStandardItem *item, unsigned int hunksNum ) {
        QString newText = i18ncp( "%1: number of changed hunks, %2: file name",
            "%2 (1 hunk)", "%2 (%1 hunks)", hunksNum, item->text() );
        item->setText( newText );
    }
};

PatchReviewToolView::PatchReviewToolView( QWidget* parent, PatchReviewPlugin* plugin )
    : QWidget( parent ), m_reversed( false ), m_plugin( plugin ) {
    connect( plugin, SIGNAL( patchChanged() ), SLOT( patchChanged() ) );
    connect( ICore::self()->documentController(), SIGNAL( documentActivated( KDevelop::IDocument* ) ), this, SLOT( documentActivated( KDevelop::IDocument* ) ) );

    showEditDialog();
    patchChanged();
}

void PatchReviewToolView::patchChanged() {
    fillEditFromPatch();
    kompareModelChanged();
}

PatchReviewToolView::~PatchReviewToolView() {
}

LocalPatchSource* PatchReviewToolView::GetLocalPatchSource() {
    IPatchSource::Ptr ips = m_plugin->patch();

    if ( !ips )
        return 0;
    return dynamic_cast<LocalPatchSource*>( ips.data() );
}

void PatchReviewToolView::updatePatchFromEdit() {
    LocalPatchSource* lpatch = GetLocalPatchSource();
    if( !lpatch )
        return;

    lpatch->m_command = m_editPatch.command->text();
    lpatch->m_filename = m_editPatch.filename->url();
    lpatch->m_baseDir = m_editPatch.baseDir->url();
    lpatch->setAlreadyApplied( m_editPatch.applied->checkState() == Qt::Checked );

    m_plugin->notifyPatchChanged();
}

void PatchReviewToolView::fillEditFromPatch() {
    IPatchSource::Ptr ipatch = m_plugin->patch();
    if ( !ipatch )
        return;

    disconnect( m_editPatch.patchSelection, SIGNAL( currentIndexChanged( int ) ), this, SLOT( patchSelectionChanged( int ) ) );

    m_editPatch.patchSelection->clear();
    foreach( IPatchSource::Ptr patch, m_plugin->knownPatches() )
    {
        if( !patch )
            continue;
        m_editPatch.patchSelection->addItem( patch->icon(), patch->name() );
        if( patch == ipatch )
            m_editPatch.patchSelection->setCurrentIndex( m_editPatch.patchSelection->count()-1 );
    }

    connect( m_editPatch.patchSelection, SIGNAL( currentIndexChanged( int ) ), this, SLOT( patchSelectionChanged( int ) ) );

    m_editPatch.cancelReview->setVisible( ipatch->canCancel() );

    QString finishText = i18n( "Finish Review" );
    if( !ipatch->finishReviewCustomText().isEmpty() )
        finishText = ipatch->finishReviewCustomText();
    kDebug() << "finish-text: " << finishText;
    m_editPatch.finishReview->setText( finishText );
    m_fileModel->setIsCheckbable( m_plugin->patch()->canSelectFiles() );

    if( m_customWidget ) {
        kDebug() << "removing custom widget";
        m_customWidget->hide();
        m_editPatch.customWidgetsLayout->removeWidget( m_customWidget );
    }

    m_customWidget = ipatch->customWidget();
    if( m_customWidget ) {
        m_editPatch.customWidgetsLayout->insertWidget( 0, m_customWidget );
        m_customWidget->show();
        kDebug() << "got custom widget";
    }

    LocalPatchSource* lpatch = dynamic_cast<LocalPatchSource*>( ipatch.data() );
    m_editPatch.tabWidget->setVisible( lpatch );
    m_editPatch.baseDir->setVisible( lpatch );
    m_editPatch.label->setVisible( lpatch );
    m_editPatch.applied->setVisible( lpatch );
    if( !lpatch )
        return;

    m_editPatch.command->setText( lpatch->m_command );
    m_editPatch.filename->setUrl( lpatch->m_filename );
    m_editPatch.baseDir->setUrl( lpatch->m_baseDir );
    m_editPatch.applied->setCheckState( lpatch->isAlreadyApplied() ? Qt::Checked : Qt::Unchecked );

    if ( lpatch->m_command.isEmpty() )
        m_editPatch.tabWidget->setCurrentIndex( m_editPatch.tabWidget->indexOf( m_editPatch.fileTab ) );
    else
        m_editPatch.tabWidget->setCurrentIndex( m_editPatch.tabWidget->indexOf( m_editPatch.commandTab ) );
}

void PatchReviewToolView::patchSelectionChanged( int selection ) {
    m_fileModel->removeRows( 0, m_fileModel->rowCount() );
    if( selection >= 0 && selection < m_plugin->knownPatches().size() ) {
        m_plugin->setPatch( m_plugin->knownPatches()[selection] );
    }
}

void PatchReviewToolView::slotAppliedChanged( int newState ) {
    if ( LocalPatchSource* lpatch = GetLocalPatchSource() ) {
        lpatch->setAlreadyApplied( newState == Qt::Checked );
        m_plugin->notifyPatchChanged();
    }
}

void PatchReviewToolView::slotEditCommandChanged() {
//     m_editPatch.filename->lineEdit()->setText( "" );
    updatePatchFromEdit();
}

void PatchReviewToolView::slotEditFileNameChanged() {
//     m_editPatch.command->setText( "" );
    updatePatchFromEdit();
}

void PatchReviewToolView::showEditDialog() {
    m_editPatch.setupUi( this );

    m_fileModel = new PatchFilesModel( this, m_plugin->patch()->canSelectFiles() );
    m_editPatch.filesList->setModel( m_fileModel );
    m_editPatch.filesList->header()->hide();
    m_editPatch.filesList->setRootIsDecorated( false );

    m_editPatch.previousHunk->setIcon( KIcon( "arrow-up" ) );
    m_editPatch.nextHunk->setIcon( KIcon( "arrow-down" ) );
    m_editPatch.cancelReview->setIcon( KIcon( "dialog-cancel" ) );
    m_editPatch.finishReview->setIcon( KIcon( "dialog-ok" ) );

    QMenu* exportMenu = new QMenu( m_editPatch.exportReview );
    StandardPatchExport* stdactions = new StandardPatchExport( m_plugin, this );
    stdactions->addActions( exportMenu );
    connect( exportMenu, SIGNAL( triggered( QAction* ) ), m_plugin, SLOT( exporterSelected( QAction* ) ) );

    IPluginController* pluginManager = ICore::self()->pluginController();
    foreach( IPlugin* p, pluginManager->allPluginsForExtension( "org.kdevelop.IPatchExporter" ) )
    {
        KPluginInfo info = pluginManager->pluginInfo( p );
        QAction* action = exportMenu->addAction( KIcon( info.icon() ), info.name() );
        action->setData( qVariantFromValue<QObject*>( p ) );
    }

    m_editPatch.exportReview->setMenu( exportMenu );

    connect( m_editPatch.previousHunk, SIGNAL( clicked( bool ) ), this, SLOT( prevHunk() ) );
    connect( m_editPatch.nextHunk, SIGNAL( clicked( bool ) ), this, SLOT( nextHunk() ) );
    connect( m_editPatch.filesList, SIGNAL( doubleClicked( QModelIndex ) ), this, SLOT( fileDoubleClicked( QModelIndex ) ) );

    connect( m_editPatch.cancelReview, SIGNAL( clicked( bool ) ), m_plugin, SLOT( cancelReview() ) );
    connect( m_editPatch.finishReview, SIGNAL( clicked( bool ) ), this, SLOT( finishReview() ) );
    //connect( m_editPatch.cancelButton, SIGNAL(pressed()), this, SLOT(slotEditCancel()) );

    //connect( this, SIGNAL(finished(int)), this, SLOT(slotEditDialogFinished(int)) );

    connect( m_editPatch.applied, SIGNAL( stateChanged( int ) ), SLOT( slotAppliedChanged( int ) ) );
    connect( m_editPatch.filename, SIGNAL( textChanged( QString ) ), SLOT( slotEditFileNameChanged() ) );

    m_editPatch.baseDir->setMode( KFile::Directory );

    connect( m_editPatch.command, SIGNAL( textChanged( QString ) ), this, SLOT( slotEditCommandChanged() ) );
//   connect( m_editPatch.commandToFile, SIGNAL(clicked(bool)), this, SLOT(slotToFile()) );

    connect( m_editPatch.filename->lineEdit(), SIGNAL( returnPressed() ), this, SLOT( slotEditFileNameChanged() ) );
    connect( m_editPatch.filename->lineEdit(), SIGNAL( editingFinished() ), this, SLOT( slotEditFileNameChanged() ) );
    connect( m_editPatch.filename, SIGNAL( urlSelected( KUrl ) ), this, SLOT( slotEditFileNameChanged() ) );
    connect( m_editPatch.command, SIGNAL( textChanged( QString ) ), this, SLOT( slotEditCommandChanged() ) );
//     connect( m_editPatch.commandToFile, SIGNAL(clicked(bool)), m_plugin, SLOT(commandToFile()) );

    connect( m_editPatch.patchSelection, SIGNAL( currentIndexChanged( int ) ), this, SLOT( patchSelectionChanged( int ) ) );

    connect( m_editPatch.updateButton, SIGNAL( clicked( bool ) ), m_plugin, SLOT( forceUpdate() ) );

    connect( m_editPatch.showButton, SIGNAL( clicked( bool ) ), m_plugin, SLOT( showPatch() ) );
}

void PatchReviewToolView::nextHunk() {
//   updateKompareModel();
    m_plugin->seekHunk( true );
}

void PatchReviewToolView::prevHunk() {
//   updateKompareModel();
    m_plugin->seekHunk( false );
}

KUrl PatchReviewPlugin::diffFile() {
    return m_patch->file();
}

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

void PatchReviewToolView::finishReview() {
    QList<KUrl> selectedUrls = m_fileModel->checkedUrls();
    kDebug() << "finishing review with" << selectedUrls;
    m_plugin->finishReview( selectedUrls );
}

void PatchReviewToolView::fileDoubleClicked( const QModelIndex& i ) {
    KUrl file = m_fileModel->statusInfo( i ).url();

    kDebug() << "opening" << file.toLocalFile();

    ICore::self()->documentController()->openDocument( file, KTextEditor::Cursor() );

    m_plugin->seekHunk( true, file );
}

KUrl PatchReviewPlugin::urlForFileModel( const Diff2::DiffModel* model ) {
    KUrl file = m_patch->baseDir();

    file.addPath( model->destinationPath() );
    file.addPath( model->destinationFile() );

    return file;
}

void PatchReviewToolView::kompareModelChanged() {
    m_fileModel->clear();

    if ( !m_plugin->modelList() )
        return;

    QMap<KUrl, KDevelop::VcsStatusInfo::State> additionalUrls = m_plugin->patch()->additionalSelectableFiles();

    const Diff2::DiffModelList* models = m_plugin->modelList()->models();
    if( models )
    {
        Diff2::DiffModelList::const_iterator it = models->constBegin();
        for(; it != models->constEnd(); ++it ) {
            Diff2::DifferenceList * diffs = ( *it )->differences();
            int cnt = 0;
            if ( diffs )
                cnt = diffs->count();

            KUrl file = m_plugin->urlForFileModel( *it );
            if( !QFileInfo( file.toLocalFile() ).isReadable() )
                continue;

            VcsStatusInfo status;
            status.setUrl( file );
            status.setState( VcsStatusInfo::ItemModified );

            m_fileModel->updateState( status, cnt );
        }
    }

    for( QMap<KUrl, KDevelop::VcsStatusInfo::State>::const_iterator it = additionalUrls.constBegin(); it != additionalUrls.constEnd(); it++ ) {
        VcsStatusInfo status;
        status.setUrl( it.key() );
        status.setState( it.value() );
        m_fileModel->updateState( status );
    }

    m_editPatch.filesList->resizeColumnToContents( 0 );
}

void PatchReviewToolView::documentActivated( IDocument* doc ) {
    if ( !m_plugin->modelList() )
        return;

    QStandardItem *fileItem = m_fileModel->fileItemForUrl( doc->url() );
    if ( fileItem ) {
        m_editPatch.filesList->setCurrentIndex( fileItem->index() );
    } else {
        m_editPatch.filesList->setCurrentIndex( QModelIndex() );
    }
}

void PatchHighlighter::aboutToDeleteMovingInterfaceContent( KTextEditor::Document* ) {
    kDebug() << "about to delete";
    clear();
}

QSize sizeHintForHtml( QString html, QSize maxSize ) {
    QTextDocument doc;
    doc.setHtml( html );

    QSize ret;
    if( doc.idealWidth() > maxSize.width() ) {
        doc.setPageSize( QSize( maxSize.width(), 30 ) );
        ret.setWidth( maxSize.width() );
    }else{
        ret.setWidth( doc.idealWidth() );
    }
    ret.setHeight( doc.size().height() );
    if( ret.height() > maxSize.height() )
        ret.setHeight( maxSize.height() );
    return ret;
}

namespace
{
QPointer<QWidget> currentTooltip;
KTextEditor::MovingRange* currentTooltipMark;
}

void PatchHighlighter::showToolTipForMark( QPoint pos, KTextEditor::MovingRange* markRange, QPair<int, int> highlightMark ) {
    if( currentTooltipMark == markRange && currentTooltip )
        return;
    delete currentTooltip;

    //Got the difference
    Diff2::Difference* diff = m_differencesForRanges[markRange];

    QString html;
#if 0
    if( diff->hasConflict() )
        html += i18n( "<b><span style=\"color:red\">Conflict</span></b><br/>" );
#endif

    Diff2::DifferenceStringList lines;

    if( m_plugin->patch()->isAlreadyApplied() && !diff->applied() )
        html += i18n( "<b>Reverted.</b><br/>" );
    else if( !m_plugin->patch()->isAlreadyApplied() && diff->applied() )
        html += i18n( "<b>Applied.</b><br/>" );

    if( diff->applied() ) {
        if( isInsertion( diff ) )
        {
            html += i18n( "<b>Insertion</b><br/>" );
        }else{
            if( isRemoval( diff ) )
                html += i18n( "<b>Removal</b><br/>" );
            html += i18n( "<b>Previous:</b><br/>" );
            lines = diff->sourceLines();
        }
    }else{
        if( isRemoval( diff ) ) {
            html += i18n( "<b>Removal</b><br/>" );
        }else{
            if( isInsertion( diff ) )
                html += i18n( "<b>Insertion</b><br/>" );

            html += i18n( "<b>Alternative:</b><br/>" );

            lines = diff->destinationLines();
        }
    }

    for( int a = 0; a < lines.size(); ++a ) {
        Diff2::DifferenceString* line = lines[a];
        uint currentPos = 0;
        QString string = line->string();

        Diff2::MarkerList markers = line->markerList();

        for( int b = 0; b < markers.size(); ++b ) {
            QString spanText = Qt::escape( string.mid( currentPos, markers[b]->offset() - currentPos ) );
            if( markers[b]->type() == Diff2::Marker::End && ( currentPos != 0 || markers[b]->offset() != static_cast<uint>( string.size() ) ) )
            {
                if( a == highlightMark.first && b == highlightMark.second )
                    html += "<b><span style=\"background:#FF5555\">" + spanText + "</span></b>";
                else
                    html += "<b><span style=\"background:#FFBBBB\">" + spanText + "</span></b>";
            }else{
                html += spanText;
            }
            currentPos = markers[b]->offset();
        }

        html += Qt::escape( string.mid( currentPos, string.length()-currentPos ) );
        html += "<br/>";
    }

    KTextBrowser* browser = new KTextBrowser;
    browser->setPalette( QApplication::palette() );
    browser->setHtml( html );

    int maxHeight = 500;

    browser->setMinimumSize( sizeHintForHtml( html, QSize( ( ICore::self()->uiController()->activeMainWindow()->width()*2 )/3, maxHeight ) ) );
    browser->setMaximumSize( browser->minimumSize() + QSize( 10, 10 ) );
    if( browser->minimumHeight() != maxHeight )
        browser->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setMargin( 0 );
    layout->addWidget( browser );

    KDevelop::ActiveToolTip* tooltip = new KDevelop::ActiveToolTip( ICore::self()->uiController()->activeMainWindow(), pos + QPoint( 5, -browser->sizeHint().height() - 30 ) );
    tooltip->setLayout( layout );
    tooltip->resize( tooltip->sizeHint() + QSize( 10, 10 ) );
    tooltip->move( pos - QPoint( 0, 20 + tooltip->height() ) );
    tooltip->addExtendRect( QRect( pos - QPoint( 15, 15 ), pos + QPoint( 15, 15 ) ) );

    currentTooltip = tooltip;
    currentTooltipMark = markRange;

    ActiveToolTip::showToolTip( tooltip );
}

void PatchHighlighter::markClicked( KTextEditor::Document* doc, KTextEditor::Mark mark, bool& handled ) {
    m_applying = true;
    if( handled )
        return;

    handled = true;

    if( doc->activeView() ) ///This is a workaround, if the cursor is somewhere else, the editor will always jump there when a mark was clicked
        doc->activeView()->setCursorPosition( KTextEditor::Cursor( mark.line, 0 ) );

    KTextEditor::MovingRange* range = rangeForMark( mark );

    if( range ) {
        QString currentText = doc->text( range->toRange() );
        Diff2::Difference* diff = m_differencesForRanges[range];

        removeLineMarker( range );

        QString sourceText;
        QString targetText;

        for( int a = 0; a < diff->sourceLineCount(); ++a ) {
            sourceText += diff->sourceLineAt( a )->string();
            if( !sourceText.endsWith( "\n" ) )
                sourceText += "\n";
        }

        for( int a = 0; a < diff->destinationLineCount(); ++a ) {
            targetText += diff->destinationLineAt( a )->string();
            if( !targetText.endsWith( "\n" ) )
                targetText += "\n";
        }

        QString replace;
        QString replaceWith;

        if( !diff->applied() ) {
            replace = sourceText;
            replaceWith = targetText;
        }else {
            replace = targetText;
            replaceWith = sourceText;
        }

        if( currentText.simplified() != replace.simplified() ) {
            KMessageBox::error( ICore::self()->uiController()->activeMainWindow(), i18n( "Could not apply the change: Text should be \"%1\", but is \"%2\".", replace, currentText ) );
            return;
        }

        diff->apply( !diff->applied() );

        KTextEditor::Cursor start = range->start().toCursor();
        range->document()->replaceText( range->toRange(), replaceWith );
        KTextEditor::Range newRange( start, start );

        uint replaceWithLines = replaceWith.count( '\n' );
        newRange.end().setLine( newRange.end().line() +  replaceWithLines );
        range->setRange( newRange );

        addLineMarker( range, diff );
    }

    {
        // After applying the change, show the tooltip again, mainly to update an old tooltip
        delete currentTooltip;
        bool h = false;
        markToolTipRequested( doc, mark, QCursor::pos(), h );
    }
    m_applying = false;
}

KTextEditor::MovingRange* PatchHighlighter::rangeForMark( KTextEditor::Mark mark ) {
    for( QMap<KTextEditor::MovingRange*, Diff2::Difference*>::const_iterator it = m_differencesForRanges.constBegin(); it != m_differencesForRanges.constEnd(); ++it ) {
        if( it.key()->start().line() == mark.line )
        {
            return it.key();
        }
    }

    return 0;
}

void PatchHighlighter::markToolTipRequested( KTextEditor::Document*, KTextEditor::Mark mark, QPoint pos, bool& handled ) {
    if( handled )
        return;

    handled = true;

    int myMarksPattern = KTextEditor::MarkInterface::markType22 | KTextEditor::MarkInterface::markType23 | KTextEditor::MarkInterface::markType24 | KTextEditor::MarkInterface::markType25 | KTextEditor::MarkInterface::markType26 | KTextEditor::MarkInterface::markType27;
    if( mark.type & myMarksPattern ) {
        //There is a mark in this line. Show the old text.
        KTextEditor::MovingRange* range = rangeForMark( mark );
        if( range )
            showToolTipForMark( pos, range );
    }
}

bool PatchHighlighter::isInsertion( Diff2::Difference* diff ) {
    return diff->sourceLineCount() == 0;
}

bool PatchHighlighter::isRemoval( Diff2::Difference* diff ) {
    return diff->destinationLineCount() == 0;
}

QStringList PatchHighlighter::splitAndAddNewlines( const QString& text ) const {
    QStringList result = text.split( '\n', QString::KeepEmptyParts );
    for( QStringList::iterator iter = result.begin(); iter != result.end(); ++iter ) {
        iter->append( '\n' );
    }
    if ( !result.isEmpty() ) {
        QString & last = result.last();
        last.remove( last.size() - 1, 1 );
    }
    return result;
}

void PatchHighlighter::performContentChange( KTextEditor::Document* doc, const QStringList& oldLines, const QStringList& newLines, int editLineNumber ) {
    QPair<QList<Diff2::Difference*>, QList<Diff2::Difference*> > diffChange = m_model->linesChanged( oldLines, newLines, editLineNumber );
    QList<Diff2::Difference*> inserted = diffChange.first;
    QList<Diff2::Difference*> removed = diffChange.second;

    // Remove all ranges that are in the same line (the line markers)
    foreach( KTextEditor::MovingRange* r, m_differencesForRanges.keys() ) {
        Diff2::Difference* diff = m_differencesForRanges[r];
        if ( removed.contains( diff ) ) {
            removeLineMarker( r );
            m_ranges.remove( r );
            m_differencesForRanges.remove( r );
            delete r;
            delete diff;
        }
    }

    KTextEditor::MovingInterface* moving = dynamic_cast<KTextEditor::MovingInterface*>( doc );
    if ( !moving )
        return;

    foreach( Diff2::Difference* diff, inserted ) {
        int lineStart = diff->destinationLineNumber();
        if ( lineStart > 0 ) {
            --lineStart;
        }
        int lineEnd = diff->destinationLineEnd();
        if ( lineEnd > 0 ) {
            --lineEnd;
        }
        KTextEditor::Range newRange( lineStart, 0, lineEnd, 0 );
        KTextEditor::MovingRange * r = moving->newMovingRange( newRange );

        m_differencesForRanges[r] = diff;
        m_ranges.insert( r );
        addLineMarker( r, diff );
    }
}

void PatchHighlighter::textRemoved( KTextEditor::Document* doc, const KTextEditor::Range& range, const QString& oldText ) {
    if ( m_applying ) { // Do not interfere with patch application
        return;
    }
    kDebug() << "removal range" << range;
    kDebug() << "removed text" << oldText;
    QStringList removedLines = splitAndAddNewlines( oldText );
    int startLine = range.start().line();
    QString remainingLine = doc->line( startLine );
    remainingLine += '\n';
    QString prefix = remainingLine.mid( 0, range.start().column() );
    QString suffix = remainingLine.mid( range.start().column() );
    if ( !removedLines.empty() ) {
        removedLines.first() = prefix + removedLines.first();
        removedLines.last() = removedLines.last() + suffix;
    }
    performContentChange( doc, removedLines, QStringList() << remainingLine, startLine + 1 );
}

void PatchHighlighter::textInserted( KTextEditor::Document* doc, KTextEditor::Range range ) {
    if( range == doc->documentRange() )
    {
        kDebug() << "re-doing";
        //The document was loaded / reloaded
        if ( !m_model->differences() )
            return;
        KTextEditor::MovingInterface* moving = dynamic_cast<KTextEditor::MovingInterface*>( doc );
        if ( !moving )
            return;

        KTextEditor::MarkInterface* markIface = dynamic_cast<KTextEditor::MarkInterface*>( doc );
        if( !markIface )
            return;

        clear();

        KColorScheme scheme( QPalette::Active );

        QImage tintedInsertion = KIcon( "insert-text" ).pixmap( 16, 16 ).toImage();
        KIconEffect::colorize( tintedInsertion, scheme.foreground( KColorScheme::NegativeText ).color(), 1.0 );
        QImage tintedRemoval = KIcon( "edit-delete" ).pixmap( 16, 16 ).toImage();
        KIconEffect::colorize( tintedRemoval, scheme.foreground( KColorScheme::NegativeText ).color(), 1.0 );
        QImage tintedChange = KIcon( "text-field" ).pixmap( 16, 16 ).toImage();
        KIconEffect::colorize( tintedChange, scheme.foreground( KColorScheme::NegativeText ).color(), 1.0 );

        markIface->setMarkDescription( KTextEditor::MarkInterface::markType22, i18n( "Insertion" ) );
        markIface->setMarkPixmap( KTextEditor::MarkInterface::markType22, QPixmap::fromImage( tintedInsertion ) );
        markIface->setMarkDescription( KTextEditor::MarkInterface::markType23, i18n( "Removal" ) );
        markIface->setMarkPixmap( KTextEditor::MarkInterface::markType23, QPixmap::fromImage( tintedRemoval ) );
        markIface->setMarkDescription( KTextEditor::MarkInterface::markType24, i18n( "Change" ) );
        markIface->setMarkPixmap( KTextEditor::MarkInterface::markType24, QPixmap::fromImage( tintedChange ) );

        markIface->setMarkDescription( KTextEditor::MarkInterface::markType25, i18n( "Insertion" ) );
        markIface->setMarkPixmap( KTextEditor::MarkInterface::markType25, KIcon( "insert-text" ).pixmap( 16, 16 ) );
        markIface->setMarkDescription( KTextEditor::MarkInterface::markType26, i18n( "Removal" ) );
        markIface->setMarkPixmap( KTextEditor::MarkInterface::markType26, KIcon( "edit-delete" ).pixmap( 16, 16 ) );
        markIface->setMarkDescription( KTextEditor::MarkInterface::markType27, i18n( "Change" ) );
        markIface->setMarkPixmap( KTextEditor::MarkInterface::markType27, KIcon( "text-field" ).pixmap( 16, 16 ) );

        for ( Diff2::DifferenceList::const_iterator it = m_model->differences()->constBegin(); it != m_model->differences()->constEnd(); ++it ) {
            Diff2::Difference* diff = *it;
            int line, lineCount;
            Diff2::DifferenceStringList lines;

            if( diff->applied() ) {
                line = diff->destinationLineNumber();
                lineCount = diff->destinationLineCount();
                lines = diff->destinationLines();
            } else {
                line = diff->sourceLineNumber();
                lineCount = diff->sourceLineCount();
                lines = diff->sourceLines();
            }

            if ( line > 0 )
                line -= 1;

            KTextEditor::Cursor c( line, 0 );
            KTextEditor::Cursor endC( line + lineCount, 0 );
            if ( doc->lines() <= c.line() )
                c.setLine( doc->lines() - 1 );
            if ( doc->lines() <= endC.line() )
                endC.setLine( doc->lines() );

            if ( endC.isValid() && c.isValid() ) {
                KTextEditor::MovingRange * r = moving->newMovingRange( KTextEditor::Range( c, endC ) );
                m_ranges << r;

                m_differencesForRanges[r] = *it;

                addLineMarker( r, diff );
            }
        }
    } else {
        if ( m_applying ) { // Do not interfere with patch application
            return;
        }
        kDebug() << "insertion range" << range;
        QString text = doc->text( range );
        kDebug() << "inserted text" << text;
        QStringList insertedLines = splitAndAddNewlines( text );
        int startLine = range.start().line();
        int endLine = range.end().line();
        QString prefix = doc->line( startLine ).mid( 0, range.start().column() );
        QString suffix = doc->line( endLine ).mid( range.end().column() );
        suffix += '\n';
        QString removedLine = prefix + suffix;
        if ( !insertedLines.empty() ) {
            insertedLines.first() = prefix + insertedLines.first();
            insertedLines.last() = insertedLines.last() + suffix;
        }
        performContentChange( doc, QStringList() << removedLine, insertedLines, startLine + 1 );
    }
}

PatchHighlighter::PatchHighlighter( Diff2::DiffModel* model, IDocument* kdoc, PatchReviewPlugin* plugin ) throw( QString )
    : m_doc( kdoc ), m_plugin( plugin ), m_model( model ), m_applying( false ) {
//     connect( kdoc, SIGNAL(destroyed(QObject*)), this, SLOT(documentDestroyed()) );
    connect( kdoc->textDocument(), SIGNAL( textInserted( KTextEditor::Document*, KTextEditor::Range ) ), this, SLOT( textInserted( KTextEditor::Document*, KTextEditor::Range ) ) );
    connect( kdoc->textDocument(), SIGNAL( textChanged( KTextEditor::Document*, KTextEditor::Range, KTextEditor::Range ) ), this, SLOT( textChanged( KTextEditor::Document*, KTextEditor::Range, KTextEditor::Range ) ) );
    connect( kdoc->textDocument(), SIGNAL( textRemoved( KTextEditor::Document*, KTextEditor::Range, QString ) ), this, SLOT( textRemoved( KTextEditor::Document*, KTextEditor::Range, QString ) ) );
    connect( kdoc->textDocument(), SIGNAL( destroyed( QObject* ) ), this, SLOT( documentDestroyed() ) );

    KTextEditor::Document* doc = kdoc->textDocument();
    if ( doc->lines() == 0 )
        return;

    connect( doc, SIGNAL( markToolTipRequested( KTextEditor::Document*, KTextEditor::Mark, QPoint, bool & ) ), this, SLOT( markToolTipRequested( KTextEditor::Document*, KTextEditor::Mark, QPoint, bool & ) ) );
    connect( doc, SIGNAL( markClicked( KTextEditor::Document*, KTextEditor::Mark, bool & ) ), this, SLOT( markClicked( KTextEditor::Document*, KTextEditor::Mark, bool & ) ) );
    connect( doc, SIGNAL( aboutToDeleteMovingInterfaceContent( KTextEditor::Document* ) ), this, SLOT( aboutToDeleteMovingInterfaceContent( KTextEditor::Document* ) ) );

    textInserted( kdoc->textDocument(), kdoc->textDocument()->documentRange() );
}

void PatchHighlighter::removeLineMarker( KTextEditor::MovingRange* range ) {
    KTextEditor::MovingInterface* moving = dynamic_cast<KTextEditor::MovingInterface*>( range->document() );
    if ( !moving )
        return;

    KTextEditor::MarkInterface* markIface = dynamic_cast<KTextEditor::MarkInterface*>( range->document() );
    if( !markIface )
        return;

    markIface->removeMark( range->start().line(), KTextEditor::MarkInterface::markType22 );
    markIface->removeMark( range->start().line(), KTextEditor::MarkInterface::markType23 );
    markIface->removeMark( range->start().line(), KTextEditor::MarkInterface::markType24 );
    markIface->removeMark( range->start().line(), KTextEditor::MarkInterface::markType25 );
    markIface->removeMark( range->start().line(), KTextEditor::MarkInterface::markType26 );
    markIface->removeMark( range->start().line(), KTextEditor::MarkInterface::markType27 );

    // Remove all ranges that are in the same line (the line markers)
    foreach( KTextEditor::MovingRange* r, m_ranges )
    {
        if( r != range && range->contains( r->toRange() ) )
        {
            delete r;
            m_ranges.remove( r );
            m_differencesForRanges.remove( r );
        }
    }
}

void PatchHighlighter::addLineMarker( KTextEditor::MovingRange* range, Diff2::Difference* diff ) {
    KTextEditor::MovingInterface* moving = dynamic_cast<KTextEditor::MovingInterface*>( range->document() );
    if ( !moving )
        return;

    KTextEditor::MarkInterface* markIface = dynamic_cast<KTextEditor::MarkInterface*>( range->document() );
    if( !markIface )
        return;

    KSharedPtr<KTextEditor::Attribute> t( new KTextEditor::Attribute() );

    bool isOriginalState = diff->applied() == m_plugin->patch()->isAlreadyApplied();

    if( isOriginalState ) {
        t->setProperty( QTextFormat::BackgroundBrush, QBrush( ColorCache::self()->blendBackground( QColor( 0, 255, 255 ), 20 ) ) );
    }else{
        t->setProperty( QTextFormat::BackgroundBrush, QBrush( ColorCache::self()->blendBackground( QColor( 255, 0, 255 ), 20 ) ) );
    }
    range->setAttribute( t );
    range->setZDepth( -500 );

    KTextEditor::MarkInterface::MarkTypes mark;

    if( isOriginalState ) {
        mark = KTextEditor::MarkInterface::markType27;

        if( isInsertion( diff ) )
            mark = KTextEditor::MarkInterface::markType25;
        if( isRemoval( diff ) )
            mark = KTextEditor::MarkInterface::markType26;
    }else{
        mark = KTextEditor::MarkInterface::markType24;

        if( isInsertion( diff ) )
            mark = KTextEditor::MarkInterface::markType22;
        if( isRemoval( diff ) )
            mark = KTextEditor::MarkInterface::markType23;
    }

    markIface->addMark( range->start().line(), mark );

    Diff2::DifferenceStringList lines;
    if( diff->applied() )
        lines = diff->destinationLines();
    else
        lines = diff->sourceLines();

    for( int a = 0; a < lines.size(); ++a ) {
        Diff2::DifferenceString* line = lines[a];
        int currentPos = 0;
        QString string = line->string();

        Diff2::MarkerList markers = line->markerList();

        for( int b = 0; b < markers.size(); ++b ) {
            if( markers[b]->type() == Diff2::Marker::End )
            {
                if( currentPos != 0 || markers[b]->offset() != static_cast<uint>( string.size() ) )
                {
                    KTextEditor::MovingRange * r2 = moving->newMovingRange( KTextEditor::Range( KTextEditor::Cursor( a + range->start().line(), currentPos ), KTextEditor::Cursor( a + range->start().line(), markers[b]->offset() ) ) );
                    m_ranges << r2;

                    KSharedPtr<KTextEditor::Attribute> t( new KTextEditor::Attribute() );

                    t->setProperty( QTextFormat::BackgroundBrush, QBrush( ColorCache::self()->blendBackground( QColor( 255, 0, 0 ), 70 ) ) );
                    r2->setAttribute( t );
                    r2->setZDepth( -600 );
                }
            }
            currentPos = markers[b]->offset();
        }
    }
}

void PatchHighlighter::clear() {
    if( m_ranges.empty() )
        return;

    KTextEditor::MovingInterface* moving = dynamic_cast<KTextEditor::MovingInterface*>( m_doc->textDocument() );
    if ( !moving )
        return;

    KTextEditor::MarkInterface* markIface = dynamic_cast<KTextEditor::MarkInterface*>( m_doc->textDocument() );
    if( !markIface )
        return;

    QHash<int, KTextEditor::Mark*> marks = markIface->marks();
    foreach( int line, marks.keys() ) {
        markIface->removeMark( line, KTextEditor::MarkInterface::markType22 );
        markIface->removeMark( line, KTextEditor::MarkInterface::markType23 );
        markIface->removeMark( line, KTextEditor::MarkInterface::markType24 );
        markIface->removeMark( line, KTextEditor::MarkInterface::markType25 );
        markIface->removeMark( line, KTextEditor::MarkInterface::markType26 );
        markIface->removeMark( line, KTextEditor::MarkInterface::markType27 );
    }

    qDeleteAll( m_ranges );
    m_ranges.clear();
    m_differencesForRanges.clear();
}

PatchHighlighter::~PatchHighlighter() {
    clear();
}

IDocument* PatchHighlighter::doc() {
    return m_doc;
}

void PatchHighlighter::documentDestroyed() {
    kDebug() << "document destroyed";
    m_ranges.clear();
    m_differencesForRanges.clear();
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
        Q_ASSERT( m_patch );
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

void PatchReviewPlugin::cancelReview() {
    if( m_patch ) {
        removeHighlighting();
        m_modelList.reset( 0 );
        m_patch->cancelReview();

        emit patchChanged();

        if( !dynamic_cast<LocalPatchSource*>( m_patch.data() ) ) {
            delete m_patch;
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

void PatchReviewPlugin::finishReview( QList<KUrl> selection ) {
    if( m_patch ) {
        if( !m_patch->finishReview( selection ) )
            return;
        removeHighlighting();
        m_modelList.reset( 0 );

        emit patchChanged();

        if( !dynamic_cast<LocalPatchSource*>( m_patch.data() ) ) {
            delete m_patch;
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
            KUrl absoluteUrl = m_patch->baseDir();
            KUrl url( m_modelList->modelAt( a )->destination() );

            if( url.isRelative() )
                absoluteUrl.addPath( url.path() );
            else
                absoluteUrl = url;

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
