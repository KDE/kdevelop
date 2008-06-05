/* KDevelop xUnit pluginQ
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
#include "qtestxmlparser.h"
#include "qtestcommand.h"

#include "kdebug.h"

using QxQTest::QTestItem;
using QxQTest::QTestXmlParser;
using QxQTest::QTestCommand;
using QxRunner::RunnerItem;

QTestItem::QTestItem(const QList<QVariant>& data, RunnerItem* parent, QTestBase* test)
    : RunnerItem(data, parent), m_test(test)
{
}

QTestItem::~QTestItem()
{
}

int QTestItem::run()
{
    if (child(0))
        return QxRunner::NoResult; // Have nothing to do as a parent
    QTestCommand* cmd = qobject_cast<QTestCommand*>(m_test);
    if (cmd == 0)
        return QxRunner::NoResult; // Only run testcommands

    KProcess proc;
    startProcess(cmd, &proc);
    return parseOutput(&proc);
}

void QTestItem::startProcess(QTestCommand* cmd, KProcess* proc)
{
    QString cmdStr = cmd->command();
    QStringList splitted = cmdStr.split(" ");
    QStringList argv;
    argv << "-xml" << splitted[1]; // the testcommand name
    proc->setProgram(splitted[0], argv);
    kDebug(9504) << proc->program().join(" ");
    proc->setOutputChannelMode(KProcess::SeparateChannels);
    proc->start(); // command name is argument to qtest exe
    proc->waitForFinished(-1); // blocks
}

int QTestItem::parseOutput(KProcess* proc)
{
    QTestXmlParser parser(proc);
    QTestResult res = parser.go();
    res.log();
    setData(2, res.message());
    setData(3, res.file().filePath());
    setData(4, res.line());
    setResult(res.state());
    return res.state();
}
