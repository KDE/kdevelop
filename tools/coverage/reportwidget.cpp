/* KDevelop coverage plugin
 *    Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "reportwidget.h"
#include "annotationmanager.h"
#include "coveredfile.h"
#include "drilldownview.h"
#include "reportmodel.h"
#include "reportproxymodel.h"
#include "lcovinfoparser.h"
#include "lcovjob.h"
#include "covoutputdelegate.h"
#include "ui_reportwidget.h"

#include <QTimer>
#include <KLocale>
#include <KIcon>
#include <KProcess>
#include <KUrlNavigator>

#include <interfaces/icore.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <project/interfaces/ibuildsystemmanager.h>

using KDevelop::ICore;
using KDevelop::IDocument;
using KDevelop::IDocumentController;
using KDevelop::IProject;
using KDevelop::IProjectController;
using KDevelop::IBuildSystemManager;

using Veritas::AnnotationManager;
using Veritas::CoveredFile;
using Veritas::CovOutputDelegate;
using Veritas::DrillDownView;
using Veritas::LcovInfoParser;
using Veritas::LcovJob;
using Veritas::ReportWidget;
using Veritas::ReportDirData;
using Veritas::ReportModel;
using Veritas::ReportViewFactory;

//////////////////////////////// ReportWidget ///////////////////////////////

void ReportWidget::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    installEventFilter(this);
    switch(m_state) {
    case DirView: { table()->resizeDirStateColumns(); break; }
    case FileView: { table()->resizeFileStateColumns(); break; }
    default: { Q_ASSERT(0); }
    }
    removeEventFilter(this);
}

bool ReportWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type()==QEvent::Resize) {
        return true;
    }
    return QWidget::eventFilter(obj, event);
}

DrillDownView* ReportWidget::table() const
{
    return m_ui->table;
}

void ReportWidget::init()
{
    m_ui = new Ui::ReportWidget();
    m_ui->setupUi(this);

    //KUrlNavigator can't be set using a placeholder in Qt Designer as there is 
    //no KUrlNavigator(QWidget*) constructor
    int startButtonIndex = m_ui->buildPathFrameLayout->indexOf(m_ui->startButton);
    m_targetDirectory = new KUrlNavigator(0, KUrl(QDir::homePath()), this);
    m_ui->buildPathFrameLayout->insertWidget(startButtonIndex, m_targetDirectory);
    
    m_ui->startButton->setIcon(KIcon("arrow-right"));
    connect(m_ui->startButton, SIGNAL(clicked(bool)), SLOT(startLcovJob()));
    
    connect(table(), SIGNAL(doubleClicked(QModelIndex)),
            SLOT(dispatchDoubleClickedSignal(QModelIndex)));

    connect(table(), SIGNAL(completedSlideLeft()), SLOT(setDirViewState()));
    connect(table(), SIGNAL(completedSlideRight()), SLOT(setFileViewState()));

    m_timer->setInterval(50);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateTableView()));
}

void ReportWidget::updateTableView()
{
    if (m_timerTicks < 10) {
        table()->viewport()->update();
        m_timerTicks += 1;
        m_timer->start();
    }
}

ReportWidget::ReportWidget(QWidget* parent) : 
    QWidget(parent),
    m_state(ReportWidget::DirView),
    m_manager(0),
    m_proxy(0),
    m_model(0),
    m_timer(new QTimer(this)),
    m_targetDirectory(0),
    m_delegate(new CovOutputDelegate(this)),
    m_ui(0)
{
    setObjectName("Coverage Report");
    setWindowIcon(SmallIcon("system-file-manager"));
}

#define ILLEGAL_STATE \
    Q_ASSERT_X(0, "ReportWidget::signalDispatch", \
                  "Serious corruption, impossible m_state value.")

void ReportWidget::dispatchSelectionSignal(const QItemSelection& selected, const QItemSelection& deselected)
{
    switch(m_state) {
    case DirView: {
        setCoverageStatistics(selected, deselected); break;
    } case FileView: {
        /*jumpToSource(selected, deselected); */break;
    } default: {
        ILLEGAL_STATE;
    }}
}

void ReportWidget::dispatchDoubleClickedSignal(const QModelIndex& index)
{
    switch(m_state) {
    case DirView: {
        table()->slideRight(index); break;
    } case FileView: {
        jumpToSource(index); break;
    } default: {
        ILLEGAL_STATE;
    }}
}

void ReportWidget::jumpToSource(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(selected);
    Q_UNUSED(deselected);
    // TODO
}

void ReportWidget::jumpToSource(const QModelIndex& index)
{
    QStandardItem* item = getItemFromProxyIndex(index);
    if (!item || item->type() != ReportModel::File ) return;
    ReportFileItem* file = static_cast<ReportFileItem*>(item);
    Q_ASSERT(file);

    IDocumentController* dc = ICore::self()->documentController();
    kDebug() << "Open doc with coverage annotations " << file->url();
    KTextEditor::Cursor cursor;
    IDocument* doc;
    doc = dc->openDocument(file->url(), cursor, IDocumentController::DoNotCreateView);
    if (!doc || !doc->isTextDocument()) return;
    Q_ASSERT(m_manager);
    m_manager->watch(doc);
    dc->activateDocument(doc);
}

void ReportWidget::reset_()
{
    m_proxy->invalidate();
    filterBox()->clear();
    table()->viewport()->update();
    m_timerTicks = 0;
    m_timer->start(); // TODO figure out the correct signal instead of this kludge
}

void ReportWidget::setDirViewState()
{
    m_state = ReportWidget::DirView;
    m_proxy->setDirViewState();
    reset_();
    table()->resizeDirStateColumns();
    filterBox()->setReadOnly(false);
    filterBox()->setText(m_oldDirFilter);

    //When the view slides to the left, the previously selected directories are
    //selected again. However, until the slide is completed we are in File
    //state, so the statistics aren't updated.
    m_ui->sloc->setText("0");
    m_ui->nrofCoveredLines->setText("0");
    setCoverageStatistics(table()->selectionModel()->selection(), QItemSelection());
}

QLineEdit* ReportWidget::filterBox() const
{
    return m_ui->filterBox;
}

void ReportWidget::setFileViewState()
{
    m_state = ReportWidget::FileView;
    m_proxy->setFileViewState();
    m_oldDirFilter = filterBox()->text();
    reset_();
    table()->resizeFileStateColumns();
    filterBox()->setReadOnly(true);

    //Set the statistics for the current directory
    setCoverageStatistics(table()->rootIndex());
}

ReportWidget::~ReportWidget()
{
    delete m_ui;
}

QStandardItem* ReportWidget::getItemFromProxyIndex(const QModelIndex& index) const
{
    if (!index.isValid()) {
        kDebug() << "Invalid index";
        return 0;
    }
    QModelIndex sourceIndex = m_proxy->mapToSource(index);
    QStandardItem* item = m_model->itemFromIndex(sourceIndex);
    Q_ASSERT(item);
    return item;
}

const ReportDirData* ReportWidget::getReportDirDataFromProxyIndex(const QModelIndex& index) const
{
    QStandardItem* item = getItemFromProxyIndex(index);
    if (!item || item->type() != ReportModel::Dir) return 0;

    return &static_cast<ReportDirItem*>(item)->reportDirData();
}

void ReportWidget::setCoverageStatistics(const ReportDirData& data)
{
    //QLocale used as QString::number does not honor the user's locale setting
    m_ui->coverageRatio->setText(QLocale().toString(data.coverageRatio(), 'f', 1) + " %");
    m_ui->coverageRatio->update();
    m_ui->sloc->setText(QString::number(data.sloc()));
    m_ui->sloc->update();
    m_ui->nrofCoveredLines->setText(QString::number(data.nrofCoveredLines()));
    m_ui->nrofCoveredLines->update();
}

void ReportWidget::setCoverageStatistics(const QModelIndex& index)
{
    const ReportDirData* data = getReportDirDataFromProxyIndex(index);
    if (!data) {
        return;
    }

    setCoverageStatistics(*data);
}

void ReportWidget::setCoverageStatistics(const QItemSelection& selected, const QItemSelection& deselected)
{
    ReportDirData fullData;
    if (m_ui->sloc->text() != "-" && m_ui->nrofCoveredLines->text() != "-") {
        fullData.setSloc(m_ui->sloc->text().toInt());
        fullData.setNrofCoveredLines(m_ui->nrofCoveredLines->text().toInt());
    }

    bool statisticsChanged = false;

    foreach (const QModelIndex& index, selected.indexes()) {
        const ReportDirData* data = getReportDirDataFromProxyIndex(index);
        if (data) {
            statisticsChanged = true;
            fullData.setSloc(fullData.sloc() + data->sloc());
            fullData.setNrofCoveredLines(fullData.nrofCoveredLines() + data->nrofCoveredLines());
        }
    }

    foreach (const QModelIndex& index, deselected.indexes()) {
        const ReportDirData* data = getReportDirDataFromProxyIndex(index);
        if (data) {
            statisticsChanged = true;
            fullData.setSloc(fullData.sloc() - data->sloc());
            fullData.setNrofCoveredLines(fullData.nrofCoveredLines() - data->nrofCoveredLines());
        }
    }

    if (statisticsChanged) {
        setCoverageStatistics(fullData);
    }
}

void ReportWidget::startLcovJob()
{
    Q_ASSERT(m_delegate); Q_ASSERT(m_targetDirectory); Q_ASSERT(m_ui->startButton->isEnabled());
    m_ui->startButton->setEnabled(false);

    if (m_state == FileView) {
        filterBox()->setReadOnly(false);
        filterBox()->setText(m_oldDirFilter);
    } else {
        m_oldDirFilter = filterBox()->text();
    }

    m_ui->coverageRatio->setText("-");
    m_ui->nrofCoveredLines->setText("-");
    m_ui->sloc->setText("-");

    m_state = DirView;
    
    if (m_model) delete m_model;
    if (m_manager) delete m_manager;
    if (m_proxy) delete m_proxy;
    
    m_model = new ReportModel(this);
    m_model->setRootDirectory( m_targetDirectory->url() );
    m_manager = new AnnotationManager(this);
    m_proxy = new ReportProxyModel(this);
    table()->setModel(m_proxy);
    m_proxy->setSourceModel(m_model);
    m_proxy->setFilterWildcard(m_oldDirFilter);

    //Since Qt 4.1 (or maybe 4.0, but not documented), setting the model
    //replaces the previous selection model, so it must be connected again
    //http://doc.trolltech.com/4.1/qabstractitemview.html#setModel
    connect(table()->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            SLOT(dispatchSelectionSignal(QItemSelection,QItemSelection)));

    LcovJob* job = new LcovJob(m_targetDirectory->url(), m_delegate);
    LcovInfoParser* parser = new LcovInfoParser(job);

    connect(parser, SIGNAL(parsedCoverageData(CoveredFile*)), m_model, SLOT(addCoverageData(CoveredFile*)));
    connect(parser, SIGNAL(parsedCoverageData(CoveredFile*)), m_manager, SLOT(addCoverageData(CoveredFile*)));
    connect(m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(updateColumns()));
    connect(filterBox(), SIGNAL(textChanged(QString)), m_proxy, SLOT(setFilterWildcard(QString)));

    job->setDelegate(m_delegate);
    job->setProcess(new KProcess);
    job->setParser(parser);

    ICore::self()->runController()->registerJob(job);
    connect(job, SIGNAL(finished(KJob*)), SLOT(jobFinished()));
}

void ReportWidget::updateColumns()
{
    switch(m_state) {
    case DirView: { table()->resizeDirStateColumns(); break; }
    case FileView: { table()->resizeFileStateColumns(); break; }
    default: { Q_ASSERT(0); }
    }
}

void ReportWidget::jobFinished()
{
    Q_ASSERT(!m_ui->startButton->isEnabled());
    m_ui->startButton->setEnabled(true);
}

#include "reportwidget.moc"
