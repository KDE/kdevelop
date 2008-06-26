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

#include "testsuite.h"
#include "outputparser.h"
#include <KDebug>
#include <QDir>

using Check::TestSuite;
using Check::OutputParser;
using Veritas::TestCase;
using Veritas::Test;

TestSuite::TestSuite(const QString& name, const QFileInfo& exe, Test* parent)
    : Test(name, parent), m_exe(exe)
{}

TestSuite::~TestSuite()
{}

bool TestSuite::shouldRun() const
{
    return true;
}

TestCase* TestSuite::child(int i) const
{
    Test* child = Test::child(i);
    TestCase* caze = qobject_cast<TestCase*>(child);
    kWarning(caze==0) << "cast failed? " << name() << " " 
                      << i << " " << ((child!=0) ? child->name() : "null");
    return caze;
}

int TestSuite::run()
{
    KProcess proc;
    QStringList argv;
    proc.setProgram(m_exe.filePath(), argv);
    kDebug() << "executing " << proc.program();
    proc.setOutputChannelMode(KProcess::SeparateChannels);
    proc.start();
    proc.waitForFinished(-1);
    QStringList spl = m_exe.filePath().split('/');
    QFile f(QFileInfo(QDir::currentPath(), "checklog.xml").filePath());
    kWarning(!f.exists()) << "Failure: testresult dump does not exist [" << f.fileName();
    OutputParser parser(&f);
    parser.go(this);
    return 0;
}

#include "testsuite.moc"
