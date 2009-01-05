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

#ifndef QTEST_CTESTFILEPARSERTEST_H_INCLUDED
#define QTEST_CTESTFILEPARSERTEST_H_INCLUDED

#include <QtCore/QObject>
#include <KUrl>

class FilesystemAccessMock;
namespace Veritas
{
class CTestfileParser;

class CTestfileParserTest : public QObject
{
Q_OBJECT
private slots:
    void init();
    void cleanup();

    void emptyDirectory();
    void emptyCTestfile();

    void subdir();
    void multipleSubdir();
    void addtest();
    void multipleAddtest();
    void addTestMultipleArguments();
    void mixedAddtestSubdir();
    void addtestInSubdirs();

    void garbageLines();
    void illFormattedAddTest();
    void illFormattedSubdir();

    void testRealFileSystemAccess();

private:
    void initializeDirectoryContents(const KUrl& dir, QStringList files);
    void initializeFileContents(const QString& file, const QString& contents);

    void assertNoTestsFound(CTestfileParser* parser);
    void assertTestFound(CTestfileParser* parser, int num, QString name, QString exe, KUrl workingDirectory);
    void assertSingleCTestfileAccessed();
    void assertSubdirAccessed(KUrl::List dirs);
    void assertNumberOfTestsEquals(int num, CTestfileParser* parser);

private:
    CTestfileParser* m_parser;
    FilesystemAccessMock* m_fsAccess;
    KUrl m_someDir;
};
}

#endif // QTEST_CTESTFILEPARSERTEST_H_INCLUDED
