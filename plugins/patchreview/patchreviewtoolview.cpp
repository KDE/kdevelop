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
#include <interfaces/itestcontroller.h>
#include <interfaces/itestsuite.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iuicontroller.h>
#include <util/projecttestjob.h>
#include <sublime/area.h>
#include <sublime/view.h>
#include <sublime/document.h>
#include <sublime/mainwindow.h>
#include <KLineEdit>
#include <KTextEditor/Document>
#include <QMenu>
#include <QFileInfo>

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
    : QWidget( parent ),
    m_resetCheckedUrls( true ),
    m_plugin( plugin )
{
    connect( m_plugin->finishReviewAction(), SIGNAL(triggered(bool)), this, SLOT( finishReview() ) );

    connect( plugin, SIGNAL( patchChanged() ), SLOT( patchChanged() ) );
    connect( plugin, SIGNAL( startingNewReview() ), SLOT( startingNewReview() ) );
    connect( ICore::self()->documentController(), SIGNAL( documentActivated( KDevelop::IDocument* ) ), this, SLOT( documentActivated( KDevelop::IDocument* ) ) );

    Sublime::MainWindow* w = dynamic_cast<Sublime::MainWindow*>( ICore::self()->uiController()->activeMainWindow() );
    connect(w, SIGNAL(areaChanged(Sublime::Area*)), m_plugin, SLOT(areaChanged(Sublime::Area*)));

    showEditDialog();
    patchChanged();
}

void PatchReviewToolView::resizeEvent(QResizeEvent* ev)
{
    bool vertical = (width() < height());
    m_editPatch.buttonsLayout->setDirection(vertical ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight);
    m_editPatch.contentLayout->setDirection(vertical ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight);
    m_editPatch.buttonsSpacer->changeSize(vertical ? 0 : 40, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    QWidget::resizeEvent(ev);
    if(m_customWidget) {
        m_editPatch.contentLayout->removeWidget( m_customWidget );
        m_editPatch.contentLayout->insertWidget(0, m_customWidget );
    }
}

void PatchReviewToolView::startingNewReview()
{
    m_resetCheckedUrls = true;
}

void PatchReviewToolView::patchChanged() {
    fillEditFromPatch();
    kompareModelChanged();
}

PatchReviewToolView::~PatchReviewToolView()
{
}

LocalPatchSource* PatchReviewToolView::GetLocalPatchSource() {
    IPatchSource::Ptr ips = m_plugin->patch();

    if ( !ips )
        return 0;
    return dynamic_cast<LocalPatchSource*>( ips.data() );
}

void PatchReviewToolView::fillEditFromPatch() {
    IPatchSource::Ptr ipatch = m_plugin->patch();
    if ( !ipatch )
        return;

    m_editPatch.cancelReview->setVisible( ipatch->canCancel() );

    m_fileModel->setIsCheckbable( m_plugin->patch()->canSelectFiles() );

    if( m_customWidget ) {
        kDebug() << "removing custom widget";
        m_customWidget->hide();
        m_editPatch.contentLayout->removeWidget( m_customWidget );
    }

    m_customWidget = ipatch->customWidget();
    if( m_customWidget ) {
        m_editPatch.contentLayout->insertWidget( 0, m_customWidget );
        m_customWidget->show();
        kDebug() << "got custom widget";
    }

    bool showTests = false;
    IProject* project = 0;
    QMap<KUrl, VcsStatusInfo::State> files = ipatch->additionalSelectableFiles();
    QMap<KUrl, VcsStatusInfo::State>::const_iterator it = files.constBegin();

    for (; it != files.constEnd(); ++it) {
        project = ICore::self()->projectController()->findProjectForUrl(it.key());
        if (project && !ICore::self()->testController()->testSuitesForProject(project).isEmpty()) {
            showTests = true;
            break;
        }
    }

    m_editPatch.testsButton->setVisible(showTests);
    m_editPatch.testProgressBar->hide();
}

void PatchReviewToolView::slotAppliedChanged( int newState ) {
    if ( LocalPatchSource* lpatch = GetLocalPatchSource() ) {
        lpatch->setAlreadyApplied( newState == Qt::Checked );
        m_plugin->notifyPatchChanged();
    }
}

void PatchReviewToolView::showEditDialog() {
    m_editPatch.setupUi( this );

    bool allowSelection = m_plugin->patch() && m_plugin->patch()->canSelectFiles();
    m_fileModel = new PatchFilesModel( this, allowSelection );
    m_editPatch.filesList->setModel( m_fileModel );
    m_editPatch.filesList->header()->hide();
    m_editPatch.filesList->setRootIsDecorated( false );
    m_editPatch.filesList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_editPatch.filesList, SIGNAL(customContextMenuRequested(QPoint)), SLOT(customContextMenuRequested(QPoint)));
    connect(m_fileModel, SIGNAL(itemChanged(QStandardItem*)), SLOT(fileItemChanged(QStandardItem*)));

    m_editPatch.previousFile->setIcon( KIcon( "arrow-left" ) );
    m_editPatch.previousHunk->setIcon( KIcon( "arrow-up" ) );
    m_editPatch.nextHunk->setIcon( KIcon( "arrow-down" ) );
    m_editPatch.nextFile->setIcon( KIcon( "arrow-right" ) );
    m_editPatch.cancelReview->setIcon( KIcon( "dialog-cancel" ) );
    m_editPatch.updateButton->setIcon( KIcon( "view-refresh" ) );
    m_editPatch.testsButton->setIcon( KIcon( "preflight-verifier" ) );
    m_editPatch.finishReview->setDefaultAction(m_plugin->finishReviewAction());

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
    connect( m_editPatch.previousFile, SIGNAL( clicked( bool ) ), this, SLOT( prevFile() ) );
    connect( m_editPatch.nextFile, SIGNAL( clicked( bool ) ), this, SLOT( nextFile() ) );
    connect( m_editPatch.filesList, SIGNAL( activated ( QModelIndex ) ), this, SLOT( fileDoubleClicked( QModelIndex ) ) );

    connect( m_editPatch.cancelReview, SIGNAL( clicked( bool ) ), m_plugin, SLOT( cancelReview() ) );
    //connect( m_editPatch.cancelButton, SIGNAL(pressed()), this, SLOT(slotEditCancel()) );

    //connect( this, SIGNAL(finished(int)), this, SLOT(slotEditDialogFinished(int)) );

    connect( m_editPatch.updateButton, SIGNAL( clicked( bool ) ), m_plugin, SLOT( forceUpdate() ) );

    connect( m_editPatch.testsButton, SIGNAL( clicked( bool ) ), this, SLOT( runTests() ) );
    
    m_selectAllAction = new QAction(KIcon("edit-select-all"), i18n("Select All"), this );
    connect( m_selectAllAction, SIGNAL(triggered(bool)), SLOT(selectAll()) );
    m_deselectAllAction = new QAction( i18n("Deselect All"), this );
    connect( m_deselectAllAction, SIGNAL(triggered(bool)), SLOT(deselectAll()) );
}

void PatchReviewToolView::customContextMenuRequested(const QPoint& )
{
    KUrl::List urls;
    QModelIndexList selectionIdxs = m_editPatch.filesList->selectionModel()->selectedIndexes();
    foreach(const QModelIndex& idx, selectionIdxs) {
        urls += idx.sibling(idx.row(), 0).data(KDevelop::VcsFileChangesModel::VcsStatusInfoRole).value<VcsStatusInfo>().url();
    }
    
    QPointer<QMenu> menu = new QMenu(m_editPatch.filesList);
    QList<ContextMenuExtension> extensions;
    if(!urls.isEmpty()) {
        KDevelop::FileContext context(urls);
        extensions = ICore::self()->pluginController()->queryPluginsForContextMenuExtensions( &context );
    }

    QList<QAction*> vcsActions;
    foreach( const ContextMenuExtension& ext, extensions )
    {
        vcsActions += ext.actions(ContextMenuExtension::VcsGroup);
    }

    menu->addAction(m_selectAllAction);
    menu->addAction(m_deselectAllAction);
    menu->addActions(vcsActions);
    if ( !menu->isEmpty() ) {
        menu->exec(QCursor::pos());
    }
    
    delete menu;
}

void PatchReviewToolView::nextHunk()
{
    IDocument* current = ICore::self()->documentController()->activeDocument();
    if(current && current->textDocument())
        m_plugin->seekHunk( true, current->textDocument()->url() );
}

void PatchReviewToolView::prevHunk()
{
    IDocument* current = ICore::self()->documentController()->activeDocument();
    if(current && current->textDocument())
        m_plugin->seekHunk( false, current->textDocument()->url() );
}

void PatchReviewToolView::seekFile(bool forwards)
{
    if(!m_plugin->patch())
        return;
    QList<KUrl> checkedUrls = m_fileModel->checkedUrls();
    QList<KUrl> allUrls = m_fileModel->urls();
    IDocument* current = ICore::self()->documentController()->activeDocument();
    if(!current || checkedUrls.empty())
        return;
    kDebug() << "seeking direction" << forwards;
    int currentIndex = allUrls.indexOf(current->url());
    KUrl newUrl;
    if((forwards && current->url() == checkedUrls.back()) ||
            (!forwards && current->url() == checkedUrls[0]))
    {
        newUrl = m_plugin->patch()->file();
        kDebug() << "jumping to patch";
    }
    else if(current->url() == m_plugin->patch()->file() || currentIndex == -1)
    {
        if(forwards)
            newUrl = checkedUrls[0];
        else
            newUrl = checkedUrls.back();
        kDebug() << "jumping from patch";
    }
    else
    {
        QSet<KUrl> checkedUrlsSet( checkedUrls.toSet() );
        for(int offset = 1; offset < allUrls.size(); ++offset)
        {
            int pos;
            if(forwards) {
                pos = (currentIndex + offset) % allUrls.size();
            }else{
                pos = currentIndex - offset;
                if(pos < 0)
                    pos += allUrls.size();
            }
            if(checkedUrlsSet.contains(allUrls[pos]))
            {
                newUrl = allUrls[pos];
                break;
            }
        }
    }

    if(newUrl.isValid())
    {
        activate( newUrl, forwards ? current : 0 );
    }else{
        kDebug() << "found no valid target url";
    }
}

void PatchReviewToolView::activate( const KUrl& url, IDocument* buddy ) const
{
    kDebug() << "activating url" << url;
    // If the document is already open in this area, just re-activate it
    if(KDevelop::IDocument* doc = ICore::self()->documentController()->documentForUrl(url)) {
        foreach(Sublime::View* view, ICore::self()->uiController()->activeArea()->views())
        {
            if(view->document() == dynamic_cast<Sublime::Document*>(doc))
            {
                ICore::self()->documentController()->activateDocument(doc);
                return;
            }
        }
    }
    
    // If the document is not open yet, open it in the correct order
    IDocument* newDoc = ICore::self()->documentController()->openDocument(url, KTextEditor::Range(), IDocumentController::DefaultMode, "", buddy);
    if(newDoc && newDoc->textDocument() && newDoc->textDocument()->activeView() && newDoc->textDocument()->activeView()->cursorPosition().line() == 0)
        m_plugin->seekHunk( true, url );
}

void PatchReviewToolView::fileItemChanged( QStandardItem* item )
{
    KUrl url = m_fileModel->statusInfo(item).url();
    if(item->checkState() != Qt::Checked)
    {
        // Eventually close the document
        if(KDevelop::IDocument* doc = ICore::self()->documentController()->documentForUrl(url)) {
            if(doc->state() == IDocument::Clean)
            {
                foreach(Sublime::View* view, ICore::self()->uiController()->activeArea()->views())
                {
                    if(view->document() == dynamic_cast<Sublime::Document*>(doc))
                    {
                        ICore::self()->uiController()->activeArea()->closeView(view);
                        return;
                    }
                }
            }
        }
    } else {
        ICore::self()->documentController()->openDocument(url, KTextEditor::Range::invalid(), IDocumentController::DoNotActivate);
    }
}

void PatchReviewToolView::nextFile()
{
    seekFile(true);
}

void PatchReviewToolView::prevFile()
{
    seekFile(false);
}

void PatchReviewToolView::deselectAll()
{
    m_fileModel->setAllChecked(false);
}

void PatchReviewToolView::selectAll()
{
    m_fileModel->setAllChecked(true);
}

void PatchReviewToolView::finishReview() {
    QList<KUrl> selectedUrls = m_fileModel->checkedUrls();
    kDebug() << "finishing review with" << selectedUrls;
    m_plugin->finishReview( selectedUrls );
}

void PatchReviewToolView::fileDoubleClicked( const QModelIndex& idx ) {
    QModelIndex i = idx.sibling(idx.row(), 0);
    KUrl file = m_fileModel->statusInfo( i ).url();

    activate( file );
}

KUrl PatchReviewPlugin::urlForFileModel( const Diff2::DiffModel* model ) {
    KUrl file = m_patch->baseDir();

    file.addPath( model->destinationPath() );
    file.addPath( model->destinationFile() );

    return file;
}

void PatchReviewToolView::kompareModelChanged() {
    
    QList<KUrl> oldCheckedUrls = m_fileModel->checkedUrls();
    
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
            status.setState( cnt>0 ? VcsStatusInfo::ItemModified : VcsStatusInfo::ItemUpToDate );

            m_fileModel->updateState( status, cnt );
        }
    }

    for( QMap<KUrl, KDevelop::VcsStatusInfo::State>::const_iterator it = additionalUrls.constBegin(); it != additionalUrls.constEnd(); it++ ) {
        VcsStatusInfo status;
        status.setUrl( it.key() );
        status.setState( it.value() );
        m_fileModel->updateState( status );
    }
    
    if(!m_resetCheckedUrls)
        m_fileModel->setCheckedUrls(oldCheckedUrls);
    else
        m_resetCheckedUrls = false;

    m_editPatch.filesList->resizeColumnToContents( 0 );
    
    // Eventually select the active document
    documentActivated( ICore::self()->documentController()->activeDocument() );
}

void PatchReviewToolView::documentActivated( IDocument* doc ) {
    if( !doc )
        return;

    if ( !m_plugin->modelList() )
        return;

    QStandardItem *fileItem = m_fileModel->fileItemForUrl( doc->url() );
    if ( fileItem ) {
        m_editPatch.filesList->setCurrentIndex( fileItem->index() );
    } else {
        m_editPatch.filesList->setCurrentIndex( QModelIndex() );
    }
}

void PatchReviewToolView::runTests()
{
    IPatchSource::Ptr ipatch = m_plugin->patch();
    if ( !ipatch ) {
        return;
    }

    IProject* project = 0;
    QMap<KUrl, VcsStatusInfo::State> files = ipatch->additionalSelectableFiles();
    QMap<KUrl, VcsStatusInfo::State>::const_iterator it = files.constBegin();

    for (; it != files.constEnd(); ++it) {
        project = ICore::self()->projectController()->findProjectForUrl(it.key());
        if (project) {
           break;
        }
    }

    if (!project) {
        return;
    }

    m_editPatch.testProgressBar->setFormat(i18n("Running tests: %p%"));
    m_editPatch.testProgressBar->setValue(0);
    m_editPatch.testProgressBar->show();

    ProjectTestJob* job = new ProjectTestJob(project, this);
    connect (job, SIGNAL(finished(KJob*)), SLOT(testJobResult(KJob*)));
    connect (job, SIGNAL(percent(KJob*,ulong)), SLOT(testJobPercent(KJob*,ulong)));
    ICore::self()->runController()->registerJob(job);
}

void PatchReviewToolView::testJobPercent(KJob* job, ulong percent)
{
    Q_UNUSED(job);
    m_editPatch.testProgressBar->setValue(percent);
}

void PatchReviewToolView::testJobResult(KJob* job)
{
    ProjectTestJob* testJob = qobject_cast<ProjectTestJob*>(job);
    if (!testJob) {
        return;
    }

    ProjectTestResult result = testJob->testResult();

    QString format;
    if (result.passed > 0 && result.failed == 0 && result.error == 0)
    {
        format = i18np("Test passed", "All %1 tests passed", result.passed);
    }
    else
    {
        format = i18n("Test results: %1 passed, %2 failed, %3 errors", result.passed, result.failed, result.error);
    }
    m_editPatch.testProgressBar->setFormat(format);

    // Needed because some test jobs may raise their own output views
    ICore::self()->uiController()->raiseToolView(this);
}
