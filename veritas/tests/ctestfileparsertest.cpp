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


#include "ctestfileparsertest.h"
#include <QtTest/QTest>
#include "kasserts.h"
#include <KDebug>
#include "../ctestfileparser.h"

#include <veritas/testexecutableinfo.h>

using Veritas::TestExecutableInfo;
using Veritas::FilesystemAccess;
using Veritas::CTestfileParser;
using Veritas::CTestfileParserTest;

////////////// AUXILIARY CLASSES /////////////////////////////////////////////

class FilesystemAccessMock : public FilesystemAccess
{
public:
    FilesystemAccessMock() {
        m_currentDirectory = KUrl("/foo/bar/");
    }

    virtual bool changeDir(const KUrl& dir) {
         m_changedToDirs << dir;
         m_currentDirectory = dir;
         return true;
    }

    virtual QStringList listing() {
        return m_listing[m_currentDirectory];
    }

    virtual QIODevice* file(const QString& name) {
        m_filesAccessed << name;
        return m_files[name];
    }

    virtual KUrl currentDirectory() const {
        return m_currentDirectory;
    }

    QMap<KUrl, QStringList> m_listing;
    KUrl::List m_changedToDirs;
    QMap<QString, QIODevice*> m_files;
    QStringList m_filesAccessed;
    KUrl m_currentDirectory;
};

////////////// FIXTURE METHODS /////////////////////////////////////////////

void CTestfileParserTest::init()
{
    m_fsAccess = new FilesystemAccessMock;
    m_parser = new CTestfileParser(m_fsAccess);
    m_someDir = KUrl(m_fsAccess->currentDirectory(), "someDir/");
}

void CTestfileParserTest::cleanup()
{
    delete m_fsAccess;
    delete m_parser;
}

////////////// TEST COMMANDS /////////////////////////////////////////////

void CTestfileParserTest::emptyDirectory()
{
    initializeDirectoryContents(m_someDir, QStringList());
    m_parser->parse(m_someDir);
    assertNoTestsFound(m_parser);
}

void CTestfileParserTest::emptyCTestfile()
{
    initializeDirectoryContents(m_someDir, QStringList() << "CTestTestfile.cmake");
    initializeFileContents("CTestTestfile.cmake", "\n");

    m_parser->parse(m_someDir);

    assertNoTestsFound(m_parser);
    assertSingleCTestfileAccessed();
}

void CTestfileParserTest::subdir()
{
    initializeDirectoryContents(m_someDir, QStringList() << "CTestTestfile.cmake");
    initializeFileContents("CTestTestfile.cmake", "SUBDIRS(foobar)\n");

    m_parser->parse(m_someDir);

    assertNoTestsFound(m_parser);
    assertSingleCTestfileAccessed();
    KUrl test(m_someDir, "foobar/");
    KUrl test_(m_someDir);
    test_.addPath("foobar");
    assertSubdirAccessed(KUrl::List() << KUrl(m_someDir, "foobar/"));
}

void CTestfileParserTest::multipleSubdir()
{
    initializeDirectoryContents(m_someDir, QStringList() << "CTestTestfile.cmake");
    initializeFileContents("CTestTestfile.cmake", 
                           "SUBDIRS(foo)\n"
                           "SUBDIRS(bar)\n");

    m_parser->parse(m_someDir);

    assertNoTestsFound(m_parser);
    assertSingleCTestfileAccessed();
    assertSubdirAccessed(KUrl::List() << KUrl(m_someDir, "foo/") << KUrl(m_someDir, "bar/"));
}

void CTestfileParserTest::addtest()
{
    initializeDirectoryContents(m_someDir, QStringList() << "CTestTestfile.cmake");
    initializeFileContents("CTestTestfile.cmake",
                           "ADD_TEST(foo \"/path/to/foo.exe\")\n");

    m_parser->parse(m_someDir);

    assertSingleCTestfileAccessed();
    assertNumberOfTestsEquals(1, m_parser);
    assertTestFound(m_parser, 0, "foo", "/path/to/foo.exe");
}

void CTestfileParserTest::multipleAddtest()
{
    initializeDirectoryContents(m_someDir, QStringList() << "CTestTestfile.cmake");
    initializeFileContents("CTestTestfile.cmake",
                           "ADD_TEST(foo \"/path/to/foo.exe\")\n"
                           "ADD_TEST(bar \"bar.exe\")\n");

    m_parser->parse(m_someDir);

    assertSingleCTestfileAccessed();
    assertNumberOfTestsEquals(2, m_parser);
    assertTestFound(m_parser, 0, "foo", "/path/to/foo.exe");
    assertTestFound(m_parser, 1, "bar", "bar.exe");
}

void CTestfileParserTest::mixedAddtestSubdir()
{
    initializeDirectoryContents(m_someDir, QStringList() << "CTestTestfile.cmake");
    initializeFileContents("CTestTestfile.cmake",
                           "ADD_TEST(foo \"/path/to/foo.exe\")\n"
                           "SUBDIRS(foobar)\n"
                           "ADD_TEST(moo \"/path/to/moo.exe\")\n"
                           "SUBDIRS(zoo)\n");

    m_parser->parse(m_someDir);

    assertSingleCTestfileAccessed();
    assertNumberOfTestsEquals(2, m_parser);
    assertTestFound(m_parser, 0, "foo", "/path/to/foo.exe");
    assertTestFound(m_parser, 1, "moo", "/path/to/moo.exe");
    assertSubdirAccessed(KUrl::List() << KUrl(m_someDir, "foobar/") << KUrl(m_someDir, "zoo/"));
}

void CTestfileParserTest::garbageLines()
{
    initializeDirectoryContents(m_someDir, QStringList() << "CTestTestfile.cmake");
    initializeFileContents("CTestTestfile.cmake",
                           "// comment here\n"
                           "ADD_TEST(foo \"/path/to/foo.exe\")\n"
                           "just some garbage\n"
                           "SUBDIRS(foobar)\n"
                           "ADD_TEST(moo \"/path/to/moo.exe\")\n"
                           "more junk\n"
                           "SUBDIRS(zoo)\n");

    m_parser->parse(m_someDir);

    assertSingleCTestfileAccessed();
    assertNumberOfTestsEquals(2, m_parser);
    assertTestFound(m_parser, 0, "foo", "/path/to/foo.exe");
    assertTestFound(m_parser, 1, "moo", "/path/to/moo.exe");
    assertSubdirAccessed(KUrl::List() << KUrl(m_someDir, "foobar/") << KUrl(m_someDir, "zoo/"));
}

void CTestfileParserTest::addTestMultipleArguments()
{
    initializeDirectoryContents(m_someDir, QStringList() << "CTestTestfile.cmake");
    initializeFileContents("CTestTestfile.cmake",
                           "ADD_TEST(zoo \"/path/to/zoo\" \"arg1\" \"arg2\")\n");

    m_parser->parse(m_someDir);

    assertSingleCTestfileAccessed();
    assertNumberOfTestsEquals(1, m_parser);
    assertTestFound(m_parser, 0, "zoo", "/path/to/zoo");
    TestExecutableInfo test = m_parser->testExecutables()[0];
    kDebug() << test.arguments().size();
    KOMPARE_MSG(QStringList() << "arg1" << "arg2", test.arguments(), test.arguments().join("<>"));
}

void CTestfileParserTest::illFormattedAddTest()
{
    initializeDirectoryContents(m_someDir, QStringList() << "CTestTestfile.cmake");
    initializeFileContents("CTestTestfile.cmake",
                           "ADD_TEST(foobar)\n" // No test exe location
                           "ADD_TEST()\n"
                           "ADD_TES(moo \"/path/to/moo.exe\")\n" // missing T in ADD_TES
                           "ADD_TEST(zoo /path/to/moo.exe)\n"); // no "" around exe

    m_parser->parse(m_someDir);

    assertSingleCTestfileAccessed();
    assertNumberOfTestsEquals(0, m_parser);
    assertSubdirAccessed(KUrl::List());
}

void CTestfileParserTest::illFormattedSubdir()
{
    initializeDirectoryContents(m_someDir, QStringList() << "CTestTestfile.cmake");
    initializeFileContents("CTestTestfile.cmake",
                           "SUBDIRS()\n" // No dir name
                           "SUBDIR(moo)\n" // missing S in SUBDIRS
                           "SUBDIRS(zoo\n"); // no ending ')'

    m_parser->parse(m_someDir);

    assertSingleCTestfileAccessed();
    assertNumberOfTestsEquals(0, m_parser);
    assertSubdirAccessed(KUrl::List());
}

////////////////// HELPER METHODS ////////////////////////////////////////////

void CTestfileParserTest::initializeDirectoryContents(const KUrl& someDir, QStringList files)
{
    m_fsAccess->m_listing[someDir] = files;
}

void CTestfileParserTest::assertNoTestsFound(CTestfileParser* parser)
{
    assertNumberOfTestsEquals(0, parser);
}

void CTestfileParserTest::assertTestFound(CTestfileParser* parser, int num, QString name, QString exe)
{
    QList<TestExecutableInfo> tests = parser->testExecutables();
    KVERIFY(num < tests.size());
    TestExecutableInfo test = tests[num];
    KOMPARE(name, test.name());
    KOMPARE(exe, test.command());
}

void CTestfileParserTest::assertSubdirAccessed(KUrl::List dirs)
{
    KOMPARE(dirs.size(), m_fsAccess->m_changedToDirs.size()-1);
    for(int i=1; i<m_fsAccess->m_changedToDirs.size(); i++) {
        KOMPARE(dirs[i-1], m_fsAccess->m_changedToDirs[i]);
    }
}

void CTestfileParserTest::initializeFileContents(const QString& file, const QString& contents)
{
    QBuffer* buff = new QBuffer();
    buff->open(QIODevice::ReadWrite);
    buff->write(contents.toLatin1());
    buff->close();
    m_fsAccess->m_files[file] = buff;
}

void CTestfileParserTest::assertSingleCTestfileAccessed()
{
    KOMPARE(1, m_fsAccess->m_filesAccessed.size());
    KOMPARE("CTestTestfile.cmake", m_fsAccess->m_filesAccessed[0]);
}

void CTestfileParserTest::assertNumberOfTestsEquals(int num, CTestfileParser* parser)
{
    KOMPARE(num, parser->testExecutables().size());
}

QTEST_MAIN( CTestfileParserTest )
#include "ctestfileparsertest.moc"
