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
#include "qtestcommand.h"
#include "qtestoutputparser.h"
#include <QDir>
#include <KProcess>
#include <KDebug>

using QTest::QTestCase;
using QTest::QTestCommand;
using QTest::QTestSuite;
using QTest::QTestOutputParser;
using Veritas::Test;

QTestCase::QTestCase(const QString& name, const QFileInfo& exe, QTestSuite* parent)
    : Test(name, parent), m_exe(exe)
{}

QTestCase::~QTestCase()
{}

bool QTestCase::shouldRun() const
{
    return true;
}

QFileInfo QTestCase::executable()
{
    QFileInfo exe(m_exe);
    Test* suite = parent();
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

QTestCommand* QTestCase::child(int i) const
{
    return static_cast<QTestCommand*>(Test::child(i));
}

int QTestCase::run()
{
    QString dir = QDir::currentPath();
    KProcess* proc = new KProcess;
    QStringList argv;
    argv << "-xml";
    QDir::setCurrent(executable().dir().absolutePath());
    proc->setProgram("./" + executable().fileName(), argv);
    kDebug() << "executing " << proc->program();
    proc->setOutputChannelMode(KProcess::SeparateChannels);
    proc->start();
    proc->waitForFinished(-1);
    QTestOutputParser parser(proc);
    parser.go(this);
    QDir::setCurrent(dir);
    delete proc;
    return 1;
}

#include "qtestcase.moc"
