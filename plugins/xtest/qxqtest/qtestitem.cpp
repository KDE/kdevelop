/* KDevelop xUnit plugin
 *
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
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

#include "qtestitem.h"
#include "qtestcase.h"
#include "qtestcommand.h"
#include "qtestoutputparser.h"

#include <qxrunner/runnermodel.h>
#include "kdebug.h"

using QxQTest::QTestItem;
using QxQTest::QTestOutputParser;
using QxQTest::QTestCommand;
using QxQTest::QTestCase;
using QxRunner::RunnerItem;

QTestItem::QTestItem(const QList<QVariant>& data, RunnerItem* parent, QTestBase* test)
    : RunnerItem(data, parent), m_test(test)
{
}

QTestItem::~QTestItem()
{
}

void QTestItem::runCase(QTestCase* caze)
{
     // Should be moved to QTestCase by making it runneritems themselves
    KProcess* proc = new KProcess;
    QStringList argv;
    argv << "-xml";
    proc->setProgram(caze->executable().filePath(), argv);
    kDebug() << "executing " << proc->program();
    proc->setOutputChannelMode(KProcess::SeparateChannels);
    proc->start();
    proc->waitForFinished(-1);
    QTestOutputParser parser(proc);
    parser.goAsync(this);
    delete proc;
}

bool QTestItem::isRunnable()
{
    return (qobject_cast<QTestCase*>(m_test) != 0);
}

int QTestItem::run()
{
    kDebug() << "running m_test " << m_test->name();
    QTestCase* caze = qobject_cast<QTestCase*>(m_test);
    if (caze)
    {
        runCase(caze);
        return 1;
    }
    return 0;
/*    if (child(0))
        return QxRunner::NoResult; // Have nothing to do as a parent
    QTestCommand* cmd = qobject_cast<QTestCommand*>(m_test);
    if (cmd == 0)
        return QxRunner::NoResult; // Only run testcommands

    KProcess proc;
    startProcess(cmd, &proc);
    int i = parseOutput(&proc);
    signalCompleted();
    return i;*/
}

void QTestItem::startProcess(QTestCommand* cmd, KProcess* proc)
{
    QString cmdStr = cmd->command();
    QStringList splitted = cmdStr.split(" ");
    QStringList argv;
    argv << "-xml" << splitted[1]; // the testcommand name
    proc->setProgram(splitted[0], argv);
    kDebug() << proc->program().join(" ");
    proc->setOutputChannelMode(KProcess::SeparateChannels);
    proc->start(); // command name is argument to qtest exe
    proc->waitForFinished(-1); // blocks
}

int QTestItem::parseOutput(KProcess* proc)
{
    QTestOutputParser parser(proc);
    QTestResult res = parser.go();
    res.log();
    setData(2, res.message());
    setData(3, res.file().filePath());
    setData(4, res.line());
    setResult(res.state());
    return res.state();
}
