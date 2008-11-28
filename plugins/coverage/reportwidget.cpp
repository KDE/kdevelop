/* KDevelop xUnit plugin
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

#include <QTimer>
#include <QLabel>
#include <QLineEdit>
#include <QGridLayout>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QPushButton>
#include <KIconLoader>
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
using Veritas::ReportModel;
using Veritas::ReportViewFactory;

namespace {
class FilterBox : public QLineEdit
{
public:
    FilterBox(QWidget* parent) : QLineEdit(parent) {}
    QSize sizeHint() const { return QSize(500, 20); }
};
}

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
    return m_table;
}

void ReportWidget::init()
{
    QVBoxLayout *l = new QVBoxLayout();
    l->setMargin(5);
    l->setSpacing(5);

    QFrame* sel = new QFrame(this); // uppermost frame. contains a breadcrumb selector widget + 'go' button
    sel->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    l->addWidget(sel);
    QHBoxLayout* selLay = new QHBoxLayout;
    sel->setLayout(selLay);
    QLabel* buildDir = new QLabel(i18n("Build Path: "), this);
    selLay->addWidget(buildDir);
    m_targetDirectory = new KUrlNavigator(0, KUrl(QDir::homePath()), this);
    selLay->addWidget(m_targetDirectory);
    m_startButton = new QPushButton(KIcon("arrow-right"), QString(), this);
    selLay->addWidget(m_startButton);
    connect(m_startButton, SIGNAL(clicked(bool)), SLOT(startLcovJob()));
    
    QFrame* f = new QFrame(this); // frame that shows filter box + Line Coverage; Instrumented lines & sloc metrics
    f->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    l->addWidget(f);

    QGridLayout* gl = new QGridLayout;
    f->setLayout(gl);

    QLabel* covTextLabel = new QLabel(i18n("Line Coverage:"), this);
    QLabel* slocTextLabel = new QLabel(i18n("Total SLOC:"), this);
    QLabel* instrumentedLabel = new QLabel(i18n("Instrumented Lines:"), this);

    m_coverage = new QLabel("-", this);
    m_sloc = new QLabel("-", this);
    m_instrumented = new QLabel("-", this);
    QSpacerItem* s1 = new QSpacerItem(300,5, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QSpacerItem* s2 = new QSpacerItem(300,5, QSizePolicy::Expanding, QSizePolicy::Minimum);

    // gl | <-s1-> lineedit     <-s2->|
    //    |        label1: val1       |
    //    |        label2: val2       |
    //    |        label3: val3       |


    gl->addItem(s1, 0, 0, 4, 1); // rowspan 4    
    m_filterBox = new FilterBox(this);
    m_filterBox->resize(250, m_filterBox->height());
    gl->addWidget(m_filterBox, 0, 1, 1, 2);

    const int FIRST_COL=1;
    gl->addWidget(covTextLabel, 1, FIRST_COL); // row, col, rowspan, colspan
    gl->addWidget(m_coverage, 1, FIRST_COL+1);
    gl->addWidget(instrumentedLabel, 2, FIRST_COL);
    gl->addWidget(m_instrumented, 2, FIRST_COL+1);
    gl->addWidget(slocTextLabel, 3, FIRST_COL);
    gl->addWidget(m_sloc, 3, FIRST_COL+1);
    gl->addItem(s2, 0, 3, 4, 1);

    l->addWidget(table());
    setLayout(l);

    connect(table()->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            SLOT(dispatchSelectionSignal(QItemSelection,QItemSelection)));

    connect(table(),  SIGNAL(clicked(QModelIndex)),
            SLOT(dispatchClickedSignal(QModelIndex)));
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
    m_table(new  DrillDownView(this)),
    m_state(ReportWidget::DirView),
    m_manager(0),
    m_proxy(0),
    m_model(0),
    m_timer(new QTimer),
    m_delegate(new CovOutputDelegate)
{
    setObjectName("Coverage Report");
    setWindowIcon(SmallIcon("system-file-manager"));
    setWindowTitle(i18n("Coverage Report"));
    setWhatsThis(i18n("GCOV code coverage statistics."));
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

void ReportWidget::dispatchClickedSignal(const QModelIndex& index)
{
    switch(m_state) {
    case DirView: {
        setCoverageStatistics(index); break;
    } case FileView: {
        /*jumpToSource(index); */break;
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
    table()->selectionModel()->clearSelection();
    table()->selectionModel()->clear();
    table()->selectionModel()->reset();
    table()->viewport()->update();
    m_timerTicks = 0;
    m_timer->start(); // TODO figure out the correct signal instead of this kludge
}

void ReportWidget::setDirViewState()
{
    kDebug() << "switching to dir view state";
    m_state = ReportWidget::DirView;
    m_proxy->setDirViewState();
    reset_();
    table()->resizeDirStateColumns();
    filterBox()->setReadOnly(false);
    filterBox()->setText(m_oldDirFilter);
}

QLineEdit* ReportWidget::filterBox() const
{
    return m_filterBox;
}

void ReportWidget::setFileViewState()
{
    kDebug() << "switching to file view state";
    m_state = ReportWidget::FileView;
    m_proxy->setFileViewState();
    m_oldDirFilter = filterBox()->text();
    reset_();
    table()->resizeFileStateColumns();
    filterBox()->setReadOnly(true);
}

ReportWidget::~ReportWidget()
{}

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

void ReportWidget::setCoverageStatistics(const QModelIndex& index)
{
    QStandardItem* item = getItemFromProxyIndex(index);
    if (!item || item->type() != ReportModel::Dir) return;
    ReportDirItem* dir = static_cast<ReportDirItem*>(item);
    Q_ASSERT(dir);

    m_coverage->setText(QString::number(dir->coverage(), 'f', 1) + " %");
    m_coverage->update();
    m_sloc->setText(QString::number(dir->sloc()));
    m_sloc->update();
    m_instrumented->setText(QString::number(dir->instrumented()));
    m_instrumented->update();
}


void ReportWidget::setCoverageStatistics(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(deselected);
    QModelIndexList indexes = selected.indexes();
    if (indexes.count() < 1) {
        kDebug() << "Nothing selected";
        return; // NO-OP if nothing selected
    }
    setCoverageStatistics(indexes.first());
}

void ReportWidget::startLcovJob()
{
    Q_ASSERT(m_delegate); Q_ASSERT(m_targetDirectory); Q_ASSERT(m_startButton->isEnabled());
    m_startButton->setEnabled(false);
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

    LcovJob* job = new LcovJob(m_targetDirectory->url(), m_delegate);
    LcovInfoParser* parser = new LcovInfoParser(job);

    connect(parser, SIGNAL(parsedCoverageData(CoveredFile*)), m_model, SLOT(addCoverageData(CoveredFile*)));
    connect(parser, SIGNAL(parsedCoverageData(CoveredFile*)), m_manager, SLOT(addCoverageData(CoveredFile*)));
    connect(m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(updateColumns()));
    connect(m_filterBox, SIGNAL(textChanged(QString)), m_proxy, SLOT(setFilterWildcard(QString)));

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
    Q_ASSERT(!m_startButton->isEnabled());
    m_startButton->setEnabled(true);
}

#include "reportwidget.moc"
