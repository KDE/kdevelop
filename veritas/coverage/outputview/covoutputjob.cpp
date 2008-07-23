/* KDevelop xUnit plugin
 *
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

#include "covoutputjob.h"
#include "covoutputmodel.h"
#include "covoutputdelegate.h"

#include "../lcovinfoparser.h"
#include "../coveredfile.h"

#include <QDir>
#include <QTableWidget>

#include <KProcess>
#include <KLocale>
#include <KDebug>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <util/processlinemaker.h>
#include <shell/core.h>

using KDevelop::ICore;
using KDevelop::IOutputView;
using KDevelop::IToolViewFactory;
using KDevelop::ProcessLineMaker;

using Veritas::CoveredFile;
using Veritas::CovOutputJob;
using Veritas::CovOutputModel;
using Veritas::CovOutputDelegate;
using Veritas::LcovInfoParser;


namespace
{

class CoverageReportFactory : public IToolViewFactory
{
public:
    CoverageReportFactory(const QList<CoveredFile*>& files): m_files(files) {}

    virtual QWidget* create(QWidget *parent = 0) {
        Q_UNUSED(parent);
        QTableWidget* tbl = new QTableWidget(parent);
        tbl->setColumnCount(4);
        tbl->setRowCount(m_files.count());
        QStringList headers;
        headers << i18n("source file") 
                << i18n("line coverage %")
                << i18n("SLOC")
                << i18n("lines visitted");
        tbl->setHorizontalHeaderLabels(headers);
        int i=0;
        foreach(CoveredFile* f, m_files) {
            tbl->setItem(i, 0, new QTableWidgetItem(f->m_sourceLoc.pathOrUrl()));
            tbl->setItem(i, 2, new QTableWidgetItem(QString::number(f->m_nrofLines)));
            tbl->setItem(i, 3, new QTableWidgetItem(QString::number(f->m_nrofInstrumentedLines)));
            if (f->m_nrofLines != 0)
                tbl->setItem(i, 1, new QTableWidgetItem(QString::number(100*(double(f->m_nrofInstrumentedLines))/f->m_nrofLines)));
            i += 1;
        }
        return tbl;
    }

    virtual Qt::DockWidgetArea defaultPosition() {
        return Qt::RightDockWidgetArea;
    }

    virtual QString id() const {
        return "org.kdevelop.gcovreport";
    }

private:
    QList<CoveredFile*> m_files;
};

} // end anonymous namespace

CovOutputJob::CovOutputJob(CovOutputDelegate* parent, const KUrl& root)
        : OutputJob(parent), m_root(root)
{}

void CovOutputJob::start()
{
    setToolTitle(i18n("Cov verbose output"));
    setToolIcon(KIcon("edit-find"));
    setViewType(KDevelop::IOutputView::HistoryView);
    setStandardToolView(KDevelop::IOutputView::TestView);
    //setTitle("");
    setBehaviours(IOutputView::AutoScroll | IOutputView::AllowUserClose);
    setModel(new CovOutputModel(delegate()), IOutputView::TakeOwnership);
    setDelegate(delegate());

    startOutput();
    m_lcov = new KProcess;
    QStringList args;
    m_tmpPath = QDir::tempPath() + QDir::separator() + "kdevcoverage.tmp";
    args << "-o"
         << m_tmpPath
         << m_root.pathOrUrl();
    m_lcov->setProgram("lcov_geninfo", args);
    m_lcov->setOutputChannelMode(KProcess::SeparateChannels);
    m_lineMaker = new ProcessLineMaker(m_lcov);

    kDebug() << "Executing ``" << m_lcov->program().join(" ") << "''";
    connect(m_lineMaker, SIGNAL(receivedStdoutLines(QStringList)),
            model(), SLOT(appendOutputs(QStringList)));
    connect(m_lineMaker, SIGNAL(receivedStderrLines(QStringList)),
            model(), SLOT(appendErrors(QStringList)));
    connect(m_lcov, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(slotFinished()));
    connect(m_lcov, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(slotError(QProcess::ProcessError)));

    m_lcov->start();
}

void CovOutputJob::slotFinished()
{
    m_lineMaker->flushBuffers();
    model()->slotCompleted();
    emitResult();
    LcovInfoParser m_parser;
    m_parser.setSource(m_tmpPath);
    QList<CoveredFile*> files = m_parser.go();
    foreach(CoveredFile* f, files) {
        f->print();
    }
    CoverageReportFactory* fac = new CoverageReportFactory(files);
    ICore::self()->uiController()->addToolView(QString("Coverage Report"), fac);
}

void CovOutputJob::slotError(QProcess::ProcessError)
{
    m_lineMaker->flushBuffers();
    model()->slotCompleted();
    m_lcov->kill();
    setError(UserDefinedError);
    emitResult();
}

CovOutputDelegate* CovOutputJob::delegate() const
{
    return const_cast<CovOutputDelegate*>(static_cast<const CovOutputDelegate*>(parent()));
}

CovOutputModel* CovOutputJob::model() const
{
    return static_cast<CovOutputModel*>(OutputJob::model());
}

#include "covoutputjob.moc"
