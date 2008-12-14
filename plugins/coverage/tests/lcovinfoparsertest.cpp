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

#include "lcovinfoparsertest.h"
#include "../lcovinfoparser.h"
#include "../coveredfile.h"
#include "coveredfilefactory.h"

#include "testutils.h"

using Veritas::CoveredFile;
using Veritas::LcovInfoParser;
using Veritas::LcovInfoParserTest;
using Veritas::Test::CoveredFileFactory;

void LcovInfoParserTest::init()
{
    m_parser = new LcovInfoParser;
}

void LcovInfoParserTest::cleanup()
{
    delete m_parser;
}

void LcovInfoParserTest::assertCoveredFilesEqual(CoveredFile* f1, CoveredFile* f2)
{
    KOMPARE(f1->url(), f2->url());
    KOMPARE(f1->sloc(), f2->sloc());
    KOMPARE(f1->nrofCoveredLines(), f2->nrofCoveredLines());
    KVERIFY(qAbs(f1->coverageRatio() - f2->coverageRatio()) < 0.01);
    KOMPARE(f1->callCountMap(), f2->callCountMap());
    KOMPARE(f1->coveredLines(), f2->coveredLines());
}

void LcovInfoParserTest::singleCoveredFile()
{
    // just a quick test, needs to be extended and cleaned.

    QString input =
        "SF:/path/to/foo.h\n"
        "FN:10,_ZN11QStringListD1Ev\n"
        "DA:10,5\n"
        "DA:11,0\n"
        "LF:2\n"
        "LH:1\n"
        "end_of_record\n";
    
    m_parser->parseLines( input.split( '\n' ) );
    QList<CoveredFile*> files = m_parser->fto_coveredFiles();
    KOMPARE(1, files.count());
    CoveredFile* f = files[0];
    KVERIFY(f != 0);

    KOMPARE(KUrl("/path/to/foo.h"), f->url());
    KOMPARE(2, f->sloc());
    KOMPARE(1, f->nrofCoveredLines());
    KOMPARE(50, f->coverageRatio());
    KOMPARE(2, f->callCountMap().count());
    KVERIFY(f->callCountMap().contains(10));
    KVERIFY(f->callCountMap().contains(11));
    KOMPARE(5, f->callCountMap()[10]);
    KOMPARE(0, f->callCountMap()[11]);
 }

void LcovInfoParserTest::multipleFiles()
{
    QString input =
        "SF:/path/to/foo.h\n"
        "FN:10,functionFoo()\n"
        "DA:10,5\n"
        "DA:11,0\n"
        "LF:2\n"
        "LH:1\n"
        "end_of_record\n"
        "SF:/path/to/bar.h\n"
        "FN:15,functionBar()\n"
        "DA:15,5\n"
        "DA:16,0\n"
        "DA:17,10\n"
        "LF:3\n"
        "LH:2\n"
        "end_of_record\n";

    m_parser->parseLines( input.split( '\n' ) );
    QList<CoveredFile*> files = m_parser->fto_coveredFiles();
    KOMPARE(2, files.count());

    CoveredFile* actualFile1 = files[0];
    KVERIFY(actualFile1 != 0);
    QMap<int, int> expCallCount; // expected number of calls on a source line. 
                                 // { source line x call count }
    expCallCount[10] = 5;
    expCallCount[11] = 0;
    KUrl url("/path/to/foo.h");
    CoveredFile* expFile1 = CoveredFileFactory::create(url, expCallCount);
    assertCoveredFilesEqual(expFile1, actualFile1);

    CoveredFile* actualFile2 = files[1];
    KVERIFY(actualFile2 != 0);
    url = KUrl("/path/to/bar.h");
    expCallCount.clear();
    expCallCount[15] = 5;
    expCallCount[16] = 0;
    expCallCount[17] = 10;
    CoveredFile* expFile2 = CoveredFileFactory::create(url, expCallCount);
    assertCoveredFilesEqual(expFile2, actualFile2);
    
    delete expFile1;
    delete expFile2;
}

QTEST_KDEMAIN( LcovInfoParserTest, NoGUI)
#include "lcovinfoparsertest.moc"
