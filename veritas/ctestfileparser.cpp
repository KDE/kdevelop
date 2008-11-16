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

#include "ctestfileparser.h"
#include "testexecutableinfo.h"
#include <KDebug>
#include <QDir>

using Veritas::TestExecutableInfo;
using Veritas::FilesystemAccess;
using Veritas::CTestfileParser;


FilesystemAccess::FilesystemAccess()
{
}

FilesystemAccess::~FilesystemAccess()
{
}

bool FilesystemAccess::changeDir(const KUrl& dir)
{
    m_currentDir = dir;
    return true;
}

QStringList FilesystemAccess::listing()
{
    QDir dir(currentDirectory().path());
    return dir.entryList();
}

QIODevice* FilesystemAccess::file(const QString& name)
{
    return new QFile(KUrl(currentDirectory(),name).path());
}

KUrl FilesystemAccess::currentDirectory() const
{
    return m_currentDir;
}

CTestfileParser::CTestfileParser(FilesystemAccess* dirAccess) : m_dirAccess(dirAccess)
{
}

CTestfileParser::~CTestfileParser()
{
}

void CTestfileParser::parse(const KUrl& dir)
{
    m_dirAccess->changeDir(dir);
    if (m_dirAccess->listing().contains("CTestTestfile.cmake")) {
        KUrl::List subDirs;
        readCTestfile(subDirs, m_testExes);
        foreach(const KUrl& sub, subDirs) {
            parse(sub); // recurse down
        }
    }
}

// helper for parse
void CTestfileParser::readCTestfile(KUrl::List& subDirs, QList<TestExecutableInfo>& tests)
{
    QIODevice* file = m_dirAccess->file("CTestTestfile.cmake");
    Q_ASSERT(file);
    file->open(QIODevice::ReadOnly);
    char line[2048];
    while(1) {
        int nrofRead = file->readLine(line, 2048);
        if (nrofRead == -1 ) break;
        if (nrofRead == 0) break;
        QString line_(line);
        if (line_.startsWith("SUBDIRS(")) {
            KUrl subDir = processSubDirLine(line_);
            if (subDir.isValid()) {
                subDirs << subDir;
            }
        } else if (line_.startsWith("ADD_TEST(")) {
            TestExecutableInfo ti = processAddTestLine(line_);
            if (!ti.name().isEmpty()) {
                tests << ti;
            }
        }
    }
    file->close();
    delete file;
}

// helper for readCTestFile
KUrl CTestfileParser::processSubDirLine(const QString& line)
{
    if (!line.endsWith(")\n")) return KUrl();
    QString dir_ = line.mid(8, line.size()-2-8);
    if (dir_.isEmpty()) return KUrl();
    KUrl dir(m_dirAccess->currentDirectory(), dir_ + "/");
    return dir;
}

// helper for readCTestFile
TestExecutableInfo CTestfileParser::processAddTestLine(const QString& line)
{
// ADD_TEST(qtest-unit-qtestoutputparser "/home/nix/KdeDev/kdevelop/build/plugins/xtest/qtest/tests/qtest-unit-qtestoutputparser.shell")
// ADD_TEST(veritas-mem-runnermodel "/home/nix/KdeDev/kdevplatform/veritas/tests/runMemcheck.py" "/home/nix/KdeDev/kdevplatform/build/veritas/tests/veritas-unit-runnermodel" "/home/nix/KdeDev/kdevplatform/build")

    QString data = line.mid(9, line.size()-1-10);
    int i = data.indexOf("\"");
    if (i==-1) return TestExecutableInfo();
    QString testName = data.mid(0,i-1);
    QStringList args = data.mid(i,-1).split('\"');
    if (args.size() < 2) return TestExecutableInfo();

    TestExecutableInfo test;
    test.setName(testName);
    args.pop_front(); // single empty item
    test.setCommand(args[0]);
    args.pop_front(); // remove test-command url, the first item
    QMutableStringListIterator it(args);
    while (it.hasNext()) { // get rid of anymore empty items
        it.next();
        if (it.value() == " " || it.value().isEmpty()) {
            it.remove();
        }
    }
    test.setArguments(args);
    return test;
}

QList<Veritas::TestExecutableInfo> CTestfileParser::testExecutables() const
{
    return m_testExes;
}

