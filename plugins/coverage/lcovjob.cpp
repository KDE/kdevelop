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

#include "lcovjob.h"
#include "covoutputmodel.h"
#include "covoutputdelegate.h"

#include "lcovinfoparser.h"
#include "coveredfile.h"
#include "annotationmodel.h"
#include "annotationmanager.h"
#include "reportwidget.h"

#include <QDir>
#include <QTableWidget>

#include <KProcess>
#include <KLocale>
#include <KDebug>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iuicontroller.h>
#include <util/processlinemaker.h>
#include <shell/core.h>
#include <shell/uicontroller.h>
#include <sublime/view.h>
#include <sublime/area.h>
#include <sublime/tooldocument.h>

#include <ktexteditor/annotationinterface.h>
#include <ktexteditor/cursor.h>
#include <ktexteditor/document.h>

using KDevelop::ICore;
using KDevelop::Core;
using KDevelop::IDocument;
using KDevelop::IDocumentController;
using KDevelop::IOutputView;
using KDevelop::IToolViewFactory;
using KDevelop::ProcessLineMaker;
using KDevelop::UiController;

using KTextEditor::AnnotationModel;
using KTextEditor::AnnotationInterface;
using KTextEditor::AnnotationViewInterface;
using KTextEditor::Document;

using Veritas::CoveredFile;
using Veritas::LcovJob;
using Veritas::CovOutputModel;
using Veritas::CovOutputDelegate;
using Veritas::LcovInfoParser;
using Veritas::AnnotationManager;
using Veritas::ReportViewFactory;

LcovJob::LcovJob(const KUrl& root, QObject* parent)
        : OutputJob(parent), m_lcov(0), m_root(root), m_delegate(0)
{}

LcovJob::~LcovJob()
{}

void LcovJob::setProcess(KProcess *proc)
{
    Q_ASSERT(proc);
    m_lcov = proc;
    m_lcov->setParent(this);
}

void LcovJob::setDelegate(CovOutputDelegate* delegate)
{
    Q_ASSERT(delegate);
    m_delegate = delegate;
}

void LcovJob::initProcess()
{
    kDebug() << "";

    Q_ASSERT_X(m_lcov, "LcovJob::initProcess()", "illegal usage, set process first.");
    QStringList args;
    //m_tmpPath = QDir::tempPath() + QDir::separator() + "kdevcoverage.tmp";
    QString root = m_root.pathOrUrl();
    if (root.endsWith("./")) root.chop(3);
    args << "-o"
         //<< m_tmpPath
         << "-"
         << "--wcwd"
         << m_root.pathOrUrl()
         << m_root.pathOrUrl();
    m_lcov->setProgram("lcov_geninfo", args);
    m_lcov->setOutputChannelMode(KProcess::SeparateChannels);
    m_lineMaker = new ProcessLineMaker(m_lcov);

    //connect(m_lineMaker, SIGNAL(receivedStdoutLines(QStringList)),
    //        model(), SLOT(appendOutputs(QStringList)));
    connect(m_lineMaker, SIGNAL(receivedStderrLines(QStringList)),
            model(), SLOT(appendErrors(QStringList)));
    connect(m_lcov, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(slotFinished()));
    connect(m_lcov, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(slotError(QProcess::ProcessError)));
}

void LcovJob::initOutputView()
{
    kDebug() << "";

    setToolTitle(i18n("Cov verbose output"));
    setViewType(KDevelop::IOutputView::HistoryView);
    setStandardToolView(KDevelop::IOutputView::TestView);
    setBehaviours(IOutputView::AutoScroll | IOutputView::AllowUserClose);
    setModel(new CovOutputModel(delegate()), IOutputView::TakeOwnership);
    setDelegate(delegate());
}

void LcovJob::initParser()
{
    kDebug() << "";

    m_parser = new LcovInfoParser;
    connect(m_lineMaker, SIGNAL(receivedStdoutLines(QStringList)),
            m_parser, SLOT(parseLines(QStringList)));
}

namespace
{
class UiToolViewFactory: public Sublime::ToolFactory
{
public:
    UiToolViewFactory(IToolViewFactory *factory): m_factory(factory) {}
    ~UiToolViewFactory() { delete m_factory; }
    virtual QWidget* create(Sublime::ToolDocument *doc, QWidget *parent = 0) {
        Q_UNUSED(doc);
        return m_factory->create(parent);
    }
    QList<QAction*> toolBarActions(QWidget* viewWidget) const {
        return m_factory->toolBarActions(viewWidget);
    }
    QString id() const { return m_factory->id(); }

private:
    IToolViewFactory *m_factory;
};
}


void LcovJob::spawnCoverageTool()
{
    kDebug() << "";

    UiController* uic = Core::self()->uiControllerInternal();
    IToolViewFactory* fac = new ReportViewFactory(m_parser, m_root);
    Sublime::Area* area = uic->activeArea();
    Sublime::ToolDocument *doc = new Sublime::ToolDocument(QString("Coverage"), uic, new UiToolViewFactory(fac));
    Sublime::View* view = doc->createView();
    Sublime::Position pos = Sublime::dockAreaToPosition(fac->defaultPosition());
    area->addToolView(view, pos);
    connect(view, SIGNAL(raise(Sublime::View*)),
            uic, SLOT(raiseToolView(Sublime::View*)));
    uic->raiseToolView(view);
    fac->viewCreated(view);
}

void LcovJob::start()
{
    kDebug() << "";
    initOutputView();
    startOutput();
    initProcess();
    initParser();
    spawnCoverageTool();

    kDebug() << "Executing " << m_lcov->program().join(" ");
    m_lcov->start();
}

void LcovJob::slotFinished()
{
    m_lineMaker->flushBuffers();
    model()->slotCompleted();
    emitResult();
}

void LcovJob::slotError(QProcess::ProcessError)
{
    m_lineMaker->flushBuffers();
    model()->slotCompleted();
    m_lcov->kill();
    setError(UserDefinedError);
    emitResult();
}

CovOutputDelegate* LcovJob::delegate() const
{
    Q_ASSERT(m_delegate);
    return m_delegate;
}

CovOutputModel* LcovJob::model() const
{
    return static_cast<CovOutputModel*>(OutputJob::model());
}

#include "lcovjob.moc"
