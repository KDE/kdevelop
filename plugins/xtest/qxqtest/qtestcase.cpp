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

#include "qtestcase.h"
#include "qtestsuite.h"
#include "qtestoutputparser.h"
#include <QDir>
#include <kprocess.h>

using QxQTest::QTestCase;
using QxQTest::QTestBase;
using QxQTest::QTestCommand;
using QxQTest::QTestSuite;
using QxQTest::QTestOutputParser;

QTestCase::QTestCase()
    : QTestBase("", 0), m_exe(QFileInfo(""))
{
}

QTestCase::QTestCase(const QString& name, const QFileInfo& exe, QTestBase* parent)
    : QTestBase(name, parent), m_exe(exe)
{
}

QTestCase::~QTestCase()
{}

QFileInfo QTestCase::executable()
{
    QFileInfo exe(m_exe);
    QTestBase* suite = owner();
    if(suite != 0 && qobject_cast<QTestSuite*>(suite) != 0)
    {
        QDir path = QDir(qobject_cast<QTestSuite*>(suite)->path().filePath());
        exe.setFile(path, m_exe.filePath());
    }
    return exe;
}

void QTestCase::setExecutable(const QFileInfo& exe)
{
    m_exe = exe;
}

QTestCommand* QTestCase::testAt(unsigned i)
{
    return qobject_cast<QTestCommand*>(childAt(i));
}

int QTestCase::run()
{
    KProcess* proc = new KProcess;
    QStringList argv;
    argv << "-xml";
    proc->setProgram(executable().filePath(), argv);
    kDebug() << "executing " << proc->program();
    proc->setOutputChannelMode(KProcess::SeparateChannels);
    proc->start();
    proc->waitForFinished(-1);
    QTestOutputParser parser(proc);
    parser.go(this);
    delete proc;
    return 1;
}

QTestCommand* QTestCase::findTestNamed(const QString& name)
{
    QTestCommand* cmd;
    for (int i = 0; i < childCount(); i++)
    {
        cmd = testAt(i);
        if (name == cmd->name())
            return cmd;
    }
    return 0;
}

#include "qtestcase.moc"
