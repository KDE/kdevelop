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
#include "checkoutputparser.h"

using QxCheck::TestSuite;
using QxCheck::TestBase;
using QxCheck::TestCase;
using QxCheck::CheckOutputParser;

TestSuite::TestSuite()
    : TestBase("", 0)
{
}

TestSuite::TestSuite(const QString& name, const QFileInfo& exe, TestBase* parent)
    : TestBase(name, parent), m_exe(exe)
{
}

TestSuite::~TestSuite()
{
}

TestCase* TestSuite::testAt(unsigned i)
{
    return qobject_cast<TestCase*>(childAt(i));
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
    QFile f(QFileInfo(m_exe.dir(), "checklog.xml").filePath());
    CheckOutputParser parser(&f);
    parser.go(this);
    return 0;
}

#include "testsuite.moc"
