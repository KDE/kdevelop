/*
    SPDX-FileCopyrightText: 2006-2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "patchreviewtoolview.h"
#include "localpatchsource.h"
#include "patchreview.h"
#include "debug.h"

#include <KompareDiff2/DiffModelList>
#include <KompareDiff2/ModelList>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <vcs/models/vcsfilechangesmodel.h>
#include <interfaces/ipatchsource.h>
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
#include <sublime/message.h>

#include <QFileInfo>
#include <QMenu>
#include <QJsonObject>
#include <QJsonArray>

#include <KLocalizedString>
#include <KTextEditor/Document>
#include <KTextEditor/View>

#ifdef WITH_PURPOSE
#include <Purpose/AlternativesModel>
#include <purpose_version.h>
#include <Purpose/Menu>
#endif

using namespace KDevelop;

class PatchFilesModel : public VcsFileChangesModel
{
    Q_OBJECT
public:
    PatchFilesModel( QObject *parent, bool allowSelection ) : VcsFileChangesModel( parent, allowSelection ) { };
    enum ItemRoles { HunksNumberRole = LastItemRole+1 };

public Q_SLOTS:
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
        const auto url = item->index().data(VcsFileChangesModel::UrlRole).toUrl();
        const QString path = ICore::self()->projectController()->prettyFileName(url, KDevelop::IProjectController::FormatPlain);
        const QString newText = i18ncp( "%1: number of changed hunks, %2: file name",
            "%2 (1 hunk)", "%2 (%1 hunks)", hunksNum,
            path);
        item->setText( newText );
    }
};

PatchReviewToolView::PatchReviewToolView( QWidget* parent, PatchReviewPlugin* plugin )
    : QWidget( parent ),
    m_resetCheckedUrls( true ),
    m_plugin( plugin )
{
    setWindowIcon(QIcon::fromTheme(QStringLiteral("text-x-patch"), windowIcon()));

    connect( m_plugin->finishReviewAction(), &QAction::triggered, this, &PatchReviewToolView::finishReview );

    connect( plugin, &PatchReviewPlugin::patchChanged, this, &PatchReviewToolView::patchChanged );
    connect( plugin, &PatchReviewPlugin::startingNewReview, this, &PatchReviewToolView::startingNewReview );
    connect( ICore::self()->documentController(), &IDocumentController::documentActivated, this, &PatchReviewToolView::documentActivated );

    auto* w = qobject_cast<Sublime::MainWindow*>(ICore::self()->uiController()->activeMainWindow());
    connect(w, &Sublime::MainWindow::areaChanged, m_plugin, &PatchReviewPlugin::areaChanged);

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

#ifdef WITH_PURPOSE
    IPatchSource::Ptr p = m_plugin->patch();
    if (p) {
        m_exportMenu->model()->setInputData(QJsonObject {
            { QStringLiteral("urls"), QJsonArray { p->file().toString() } },
            { QStringLiteral("mimeType"), { QStringLiteral("text/x-patch") } },
            { QStringLiteral("localBaseDir"), { p->baseDir().toString() } },
            { QStringLiteral("updateComment"), { QStringLiteral("Patch updated through KDevelop's Patch Review plugin") } }
        });
    }
#endif
}

PatchReviewToolView::~PatchReviewToolView()
{
}

LocalPatchSource* PatchReviewToolView::GetLocalPatchSource() {
    IPatchSource::Ptr ips = m_plugin->patch();

    if ( !ips )
        return nullptr;
    return qobject_cast<LocalPatchSource*>(ips.data());
}

void PatchReviewToolView::fillEditFromPatch() {
    IPatchSource::Ptr ipatch = m_plugin->patch();
    if ( !ipatch )
        return;

    m_editPatch.cancelReview->setVisible( ipatch->canCancel() );

    m_fileModel->setIsCheckbable( m_plugin->patch()->canSelectFiles() );

    if( m_customWidget ) {
        qCDebug(PLUGIN_PATCHREVIEW) << "removing custom widget";
        m_customWidget->hide();
        m_editPatch.contentLayout->removeWidget( m_customWidget );
    }

    m_customWidget = ipatch->customWidget();
    if( m_customWidget ) {
        m_editPatch.contentLayout->insertWidget( 0, m_customWidget );
        m_customWidget->show();
        qCDebug(PLUGIN_PATCHREVIEW) << "got custom widget";
    }

    bool showTests = false;
    QMap<QUrl, VcsStatusInfo::State> files = ipatch->additionalSelectableFiles();
    QMap<QUrl, VcsStatusInfo::State>::const_iterator it = files.constBegin();

    for (; it != files.constEnd(); ++it) {
        auto project = ICore::self()->projectController()->findProjectForUrl(it.key());
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
    m_fileSortProxyModel = new VcsFileChangesSortProxyModel(this);
    m_fileSortProxyModel->setSourceModel(m_fileModel);
    m_fileSortProxyModel->sort(1);
    m_fileSortProxyModel->setDynamicSortFilter(true);
    m_editPatch.filesList->setModel( m_fileSortProxyModel );
    m_editPatch.filesList->header()->hide();
    m_editPatch.filesList->setRootIsDecorated( false );
    m_editPatch.filesList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_editPatch.filesList, &QTreeView::customContextMenuRequested, this, &PatchReviewToolView::customContextMenuRequested);
    connect(m_fileModel, &PatchFilesModel::itemChanged, this, &PatchReviewToolView::fileItemChanged);
    m_editPatch.finishReview->setDefaultAction(m_plugin->finishReviewAction());

#ifdef WITH_PURPOSE
    m_exportMenu = new Purpose::Menu(this);
    connect(m_exportMenu, &Purpose::Menu::finished, this, [](const QJsonObject &output, int error, const QString &errorMessage) {
        Sublime::Message* message;
        if (error==0) {
            const QString messageText = i18n("<qt>You can find the new request at:<br /><a href='%1'>%1</a> </qt>", output[QLatin1String("url")].toString());
            message = new Sublime::Message(messageText, Sublime::Message::Information);
        } else {
            const QString messageText = i18n("Couldn't export the patch.\n%1", errorMessage);
            message = new Sublime::Message(messageText, Sublime::Message::Error);
        }
        ICore::self()->uiController()->postMessage(message);
    });
    // set the model input parameters to avoid terminal warnings
    m_exportMenu->model()->setInputData(QJsonObject {
        { QStringLiteral("urls"), QJsonArray { QString() } },
        { QStringLiteral("mimeType"), { QStringLiteral("text/x-patch") } }
    });
    m_exportMenu->model()->setPluginType(QStringLiteral("Export"));
    m_editPatch.exportReview->setMenu( m_exportMenu );
#else
    m_editPatch.exportReview->setEnabled(false);
#endif

    connect( m_editPatch.previousHunk, &QToolButton::clicked, this, &PatchReviewToolView::prevHunk );
    connect( m_editPatch.nextHunk, &QToolButton::clicked, this, &PatchReviewToolView::nextHunk );
    connect( m_editPatch.previousFile, &QToolButton::clicked, this, &PatchReviewToolView::prevFile );
    connect( m_editPatch.nextFile, &QToolButton::clicked, this, &PatchReviewToolView::nextFile );
    connect( m_editPatch.filesList, &QTreeView::activated , this, &PatchReviewToolView::fileDoubleClicked );

    connect( m_editPatch.cancelReview, &QToolButton::clicked, m_plugin, &PatchReviewPlugin::cancelReview );
    //connect( m_editPatch.cancelButton, SIGNAL(pressed()), this, SLOT(slotEditCancel()) );

    //connect( this, SIGNAL(finished(int)), this, SLOT(slotEditDialogFinished(int)) );

    connect( m_editPatch.updateButton, &QToolButton::clicked, m_plugin, &PatchReviewPlugin::forceUpdate );

    connect( m_editPatch.testsButton, &QToolButton::clicked, this, &PatchReviewToolView::runTests );

    m_selectAllAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-select-all")), i18nc("@action", "Select All"), this );
    connect( m_selectAllAction, &QAction::triggered, this, &PatchReviewToolView::selectAll );
    m_deselectAllAction = new QAction( i18nc("@action", "Deselect All"), this );
    connect( m_deselectAllAction, &QAction::triggered, this, &PatchReviewToolView::deselectAll );
}

void PatchReviewToolView::customContextMenuRequested(const QPoint& pos)
{
    QList<QUrl> urls;
    const QModelIndexList selectionIdxs = m_editPatch.filesList->selectionModel()->selectedIndexes();
    urls.reserve(selectionIdxs.size());
    for (const QModelIndex& idx : selectionIdxs) {
        urls += idx.data(KDevelop::VcsFileChangesModel::UrlRole).toUrl();
    }

    QPointer<QMenu> menu = new QMenu(m_editPatch.filesList);
    QList<ContextMenuExtension> extensions;
    if(!urls.isEmpty()) {
        KDevelop::FileContext context(urls);
        extensions = ICore::self()->pluginController()->queryPluginsForContextMenuExtensions(&context, menu);
    }

    QList<QAction*> vcsActions;
    for (const ContextMenuExtension& ext : std::as_const(extensions)) {
        vcsActions += ext.actions(ContextMenuExtension::VcsGroup);
    }

    menu->addAction(m_selectAllAction);
    menu->addAction(m_deselectAllAction);
    menu->addActions(vcsActions);
    menu->exec(m_editPatch.filesList->viewport()->mapToGlobal(pos));

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
    const QList<QUrl> checkedUrls = m_fileModel->checkedUrls();
    QList<QUrl> allUrls = m_fileModel->urls();
    IDocument* current = ICore::self()->documentController()->activeDocument();
    if(!current || checkedUrls.empty())
        return;
    qCDebug(PLUGIN_PATCHREVIEW) << "seeking direction" << forwards;
    int currentIndex = allUrls.indexOf(current->url());
    QUrl newUrl;
    if((forwards && current->url() == checkedUrls.back()) ||
            (!forwards && current->url() == checkedUrls[0]))
    {
        newUrl = m_plugin->patch()->file();
        qCDebug(PLUGIN_PATCHREVIEW) << "jumping to patch";
    }
    else if(current->url() == m_plugin->patch()->file() || currentIndex == -1)
    {
        if(forwards)
            newUrl = checkedUrls[0];
        else
            newUrl = checkedUrls.back();
        qCDebug(PLUGIN_PATCHREVIEW) << "jumping from patch";
    } else {
        QSet<QUrl> checkedUrlsSet(checkedUrls.begin(), checkedUrls.end());
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
        open( newUrl, true );
    }else{
        qCDebug(PLUGIN_PATCHREVIEW) << "found no valid target url";
    }
}

void PatchReviewToolView::open( const QUrl& url, bool activate ) const
{
    qCDebug(PLUGIN_PATCHREVIEW) << "activating url" << url;
    // If the document is already open in this area, just re-activate it
    if(KDevelop::IDocument* doc = ICore::self()->documentController()->documentForUrl(url)) {
        const auto views = ICore::self()->uiController()->activeArea()->views();
        for (Sublime::View* view : views) {
            if(view->document() == dynamic_cast<Sublime::Document*>(doc))
            {
                if (activate) {
                    // use openDocument() for the activation so that the document is added to File/Open Recent.
                    ICore::self()->documentController()->openDocument(doc->url(), KTextEditor::Range::invalid());
                }
                return;
            }
        }
    }

    QStandardItem* item = m_fileModel->itemForUrl( url );

    IDocument* buddyDoc = nullptr;

    if (m_plugin->patch() && item) {
        for (int preRow = item->row() - 1; preRow >= 0; --preRow) {
            QStandardItem* preItem = m_fileModel->item(preRow);
            if (!m_fileModel->isCheckable() || preItem->checkState() == Qt::Checked) {
                // found valid predecessor, take it as buddy
                buddyDoc = ICore::self()->documentController()->documentForUrl(preItem->index().data(VcsFileChangesModel::UrlRole).toUrl());
                if (buddyDoc) {
                    break;
                }
            }
        }
        if (!buddyDoc) {
            buddyDoc = ICore::self()->documentController()->documentForUrl(m_plugin->patch()->file());
        }
    }

    // we simplify and assume that documents to be opened without activating them also need not be
    // added to the Files/Open Recent menu.
    IDocument* newDoc = ICore::self()->documentController()->openDocument(url, KTextEditor::Range::invalid(),
        activate ? IDocumentController::DefaultMode : IDocumentController::DoNotActivate|IDocumentController::DoNotAddToRecentOpen,
        QString(), buddyDoc);

    KTextEditor::View* view = nullptr;
    if(newDoc)
        view = newDoc->activeTextView();

    if(view && view->cursorPosition().line() == 0)
        m_plugin->seekHunk( true, url );
}

void PatchReviewToolView::fileItemChanged( QStandardItem* item )
{
    if (item->column() != 0 || !m_plugin->patch())
        return;

    QUrl url = item->index().data(VcsFileChangesModel::UrlRole).toUrl();
    if (url.isEmpty())
        return;

    KDevelop::IDocument* doc = ICore::self()->documentController()->documentForUrl(url);
    if(m_fileModel->isCheckable() && item->checkState() != Qt::Checked)
    {   // The file was deselected, so eventually close it
        if(doc && doc->state() == IDocument::Clean)
        {
            const auto views = ICore::self()->uiController()->activeArea()->views();
            for (Sublime::View* view : views) {
                if(view->document() == dynamic_cast<Sublime::Document*>(doc))
                {
                    ICore::self()->uiController()->activeArea()->closeView(view);
                    return;
                }
            }
        }
    } else if (!doc) {
        // Maybe the file was unchecked before, or it  was just loaded.
        open( url, false );
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
    QList<QUrl> selectedUrls = m_fileModel->checkedUrls();
    qCDebug(PLUGIN_PATCHREVIEW) << "finishing review with" << selectedUrls;
    m_plugin->finishReview( selectedUrls );
}

void PatchReviewToolView::fileDoubleClicked( const QModelIndex& idx )
{
    const QUrl file = idx.data(VcsFileChangesModel::UrlRole).toUrl();
    open( file, true );
}

void PatchReviewToolView::kompareModelChanged() {

    QList<QUrl> oldCheckedUrls = m_fileModel->checkedUrls();

    m_fileModel->clear();

    if ( !m_plugin->modelList() )
        return;

    QMap<QUrl, KDevelop::VcsStatusInfo::State> additionalUrls = m_plugin->patch()->additionalSelectableFiles();

    const auto* const models = m_plugin->modelList()->models();
    if( models )
    {
        for (auto* model : *models) {
            const auto* const diffs = model->differences();
            int cnt = 0;
            if ( diffs )
                cnt = diffs->count();

            const QUrl file = m_plugin->urlForFileModel(model);
            if( file.isLocalFile() && !QFileInfo( file.toLocalFile() ).isReadable() )
                continue;

            VcsStatusInfo status;
            status.setUrl( file );
            status.setState( cnt>0 ? VcsStatusInfo::ItemModified : VcsStatusInfo::ItemUpToDate );

            m_fileModel->updateState( status, cnt );
        }
    }

    for( QMap<QUrl, KDevelop::VcsStatusInfo::State>::const_iterator it = additionalUrls.constBegin(); it != additionalUrls.constEnd(); ++it ) {
        VcsStatusInfo status;
        status.setUrl( it.key() );
        status.setState( it.value() );
        m_fileModel->updateState( status );
    }

    if(!m_resetCheckedUrls)
        m_fileModel->setCheckedUrls(oldCheckedUrls);
    else
        m_resetCheckedUrls = false;

    for (auto c = 0, columnCount = m_fileSortProxyModel->columnCount(); c != columnCount; ++c) {
        m_editPatch.filesList->resizeColumnToContents(c);
    }

    // Eventually select the active document
    documentActivated( ICore::self()->documentController()->activeDocument() );
}

void PatchReviewToolView::documentActivated( IDocument* doc ) {
    if( !doc )
        return;

    if ( !m_plugin->modelList() )
        return;

    const auto index = VcsFileChangesModel::statusIndexForUrl(*m_fileSortProxyModel, QModelIndex{}, doc->url());
    m_editPatch.filesList->setCurrentIndex(index);
}

void PatchReviewToolView::runTests()
{
    IPatchSource::Ptr ipatch = m_plugin->patch();
    if ( !ipatch ) {
        return;
    }

    IProject* project = nullptr;
    QMap<QUrl, VcsStatusInfo::State> files = ipatch->additionalSelectableFiles();
    QMap<QUrl, VcsStatusInfo::State>::const_iterator it = files.constBegin();

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

    auto* job = new ProjectTestJob(project, this);
    connect(job, &ProjectTestJob::finished, this, &PatchReviewToolView::testJobResult);
    connect(job, &KJob::percentChanged, this, &PatchReviewToolView::testJobPercent);
    ICore::self()->runController()->registerJob(job);
}

void PatchReviewToolView::testJobPercent(KJob* job, unsigned long percent)
{
    Q_UNUSED(job);
    m_editPatch.testProgressBar->setValue(percent);
}

void PatchReviewToolView::testJobResult(KJob* job)
{
    auto* testJob = qobject_cast<ProjectTestJob*>(job);
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

#include "patchreviewtoolview.moc"
#include "moc_patchreviewtoolview.cpp"
