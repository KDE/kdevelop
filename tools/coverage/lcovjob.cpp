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
#include <KConfigGroup>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <util/processlinemaker.h>
#include <shell/core.h>

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
        : OutputJob(parent), m_lcov(0), m_root(root), m_parser(0)
{}

LcovJob::~LcovJob()
{}

void LcovJob::setDelegate(CovOutputDelegate* delegate)
{
    OutputJob::setDelegate(delegate);
}

void LcovJob::setParser(LcovInfoParser* parser)
{
    Q_ASSERT(parser); Q_ASSERT(m_parser == 0);
    m_parser = parser;
}

void LcovJob::setProcess(KProcess *proc)
{
    Q_ASSERT(proc);
    m_lcov = proc;
    m_lcov->setParent(this);
}

void LcovJob::initProcess()
{
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
    KConfigGroup grp = KGlobal::config()->group("Code Coverage");
    KUrl geninfo = grp.readEntry("geninfoPath", KUrl("/usr/bin/geninfo") );
    m_lcov->setProgram( geninfo.toLocalFile(), args);
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
    setToolTitle(i18n("Cov verbose output"));
    setViewType(KDevelop::IOutputView::HistoryView);
    setStandardToolView(KDevelop::IOutputView::TestView);
    setBehaviours(IOutputView::AutoScroll | IOutputView::AllowUserClose);
    setModel(new CovOutputModel(0), IOutputView::TakeOwnership);
}

void LcovJob::initParser()
{
    Q_ASSERT(m_parser);
    connect(m_lineMaker, SIGNAL(receivedStdoutLines(QStringList)),
            m_parser, SLOT(parseLines(QStringList)));
}


void LcovJob::start()
{
    initOutputView();
    startOutput();
    initProcess();
    initParser();
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

CovOutputModel* LcovJob::model() const
{
    return static_cast<CovOutputModel*>(OutputJob::model());
}

#include "lcovjob.moc"
