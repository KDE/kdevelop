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

#include "patchreviewtoolview.h"
#include "localpatchsource.h"
#include "patchreview.h"
#include "standardpatchexport.h"
#include "libdiff2/diffmodellist.h"
#include "libdiff2/komparemodellist.h"
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <vcs/models/vcsfilechangesmodel.h>
#include <interfaces/iplugincontroller.h>
#include <KLineEdit>
#include <QMenu>

using namespace KDevelop;

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
            "%2 (1 hunk)", "%2 (%1 hunks)", hunksNum,
            item->data(VcsFileChangesModel::VcsStatusInfoRole).value<VcsStatusInfo>().url().pathOrUrl() );
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

    lpatch->setCommand(m_editPatch.command->text());
    lpatch->setFilename(m_editPatch.filename->url());
    lpatch->setBaseDir(m_editPatch.baseDir->url());
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

    m_editPatch.command->setText( lpatch->command());
    m_editPatch.filename->setUrl( lpatch->file() );
    m_editPatch.baseDir->setUrl( lpatch->baseDir() );
    m_editPatch.applied->setCheckState( lpatch->isAlreadyApplied() ? Qt::Checked : Qt::Unchecked );

    if ( lpatch->command().isEmpty() )
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
    connect( m_editPatch.filesList, SIGNAL( activated ( QModelIndex ) ), this, SLOT( fileDoubleClicked( QModelIndex ) ) );

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

void PatchReviewToolView::finishReview() {
    QList<KUrl> selectedUrls = m_fileModel->checkedUrls();
    kDebug() << "finishing review with" << selectedUrls;
    m_plugin->finishReview( selectedUrls );
}

void PatchReviewToolView::fileDoubleClicked( const QModelIndex& idx ) {
    QModelIndex i = idx.sibling(idx.row(), 0);
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

