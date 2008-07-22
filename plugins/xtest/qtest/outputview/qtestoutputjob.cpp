/* KDevelop xUnit plugin
 *    Copyright 1999-2001 Bernd Gehrmann and the KDevelop Team <bernd@kdevelop.org>
 *    Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *    Copyright 2008 by Hamish Rodda <rodda@kde.org>
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

#include "qtestoutputjob.h"
#include "qtestoutputmodel.h"
#include "qtestoutputdelegate.h"

#include "../qtestview.h"
#include "../qtestcase.h"

#include <KProcess>
#include <KLocale>

#include <kdevplatform/interfaces/icore.h>
#include <kdevplatform/util/processlinemaker.h>

using namespace KDevelop;
using QTest::QTestCase;

QTestOutputJob::QTestOutputJob(QTestOutputDelegate* parent, QTestCase* caze)
        : OutputJob(parent), m_caze(caze)
{}

void QTestOutputJob::start()
{
    setToolTitle(i18n("QTest verbose output"));
    setToolIcon(KIcon("edit-find"));
    setViewType(KDevelop::IOutputView::HistoryView);
    setStandardToolView(KDevelop::IOutputView::TestView);
    //setTitle("");
    setBehaviours(KDevelop::IOutputView::AutoScroll | KDevelop::IOutputView::AllowUserClose);
    setModel(new QTestOutputModel(delegate()), KDevelop::IOutputView::TakeOwnership);
    setDelegate(delegate());

    startOutput();
    m_cat = new KProcess;
    QStringList l;
    l << m_caze->textOutFilePath().absoluteFilePath();
    l << m_caze->stdErrFilePath().absoluteFilePath();
    m_cat->setProgram("cat", l);
    m_cat->setOutputChannelMode(KProcess::SeparateChannels);
    m_lineMaker = new KDevelop::ProcessLineMaker(m_cat);

    connect(m_lineMaker, SIGNAL(receivedStdoutLines(QStringList)),
            model(), SLOT(appendOutputs(QStringList)));
    connect(m_lineMaker, SIGNAL(receivedStderrLines(QStringList)),
            model(), SLOT(appendErrors(QStringList)));
    connect(m_cat, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(slotFinished()));
    connect(m_cat, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(slotError(QProcess::ProcessError)));

    m_cat->start();
}

void QTestOutputJob::slotFinished()
{
    m_lineMaker->flushBuffers();
    model()->slotCompleted();
    emitResult();
}

void QTestOutputJob::slotError(QProcess::ProcessError)
{
    m_lineMaker->flushBuffers();
    model()->slotCompleted();
    m_cat->kill();
    setError(UserDefinedError);
    emitResult();
}

QTestOutputDelegate* QTestOutputJob::delegate() const
{
    return const_cast<QTestOutputDelegate*>(static_cast<const QTestOutputDelegate*>(parent()));
}

QTestOutputModel* QTestOutputJob::model() const
{
    return static_cast<QTestOutputModel*>(OutputJob::model());
}

#include "qtestoutputjob.moc"
