#include "lcovinfoparsertest.h"
#include <QtTest/QTest>
#include <qtest_kde.h>
#include <veritas/tests/kasserts.h>
#include <veritas/coverage/lcovinfoparser.h>
#include <veritas/coverage/coveredfile.h>
#include "coveredfilefactory.h"

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
    KOMPARE(f1->instrumented(), f2->instrumented());
    KVERIFY(qAbs(f1->coverage() - f2->coverage()) < 0.01);
    KOMPARE(f1->callCountMap(), f2->callCountMap());
    KOMPARE(f1->coveredLines(), f2->coveredLines());
}

void LcovInfoParserTest::singleCoveredFile()
{
    // just a quick test, needs to be extended and cleaned.

    QByteArray input =
        "SF:/path/to/foo.h\n"
        "FN:10,_ZN11QStringListD1Ev\n"
        "DA:10,5\n"
        "DA:11,0\n"
        "LF:2\n"
        "LH:1\n"
        "end_of_record\n";
    QBuffer* buff = new QBuffer(&input);
    m_parser->setSource(buff);
    QList<CoveredFile*> files = m_parser->go();
    KOMPARE(1, files.count());
    CoveredFile* f = files[0];
    KVERIFY(f != 0);

    KOMPARE(KUrl("/path/to/foo.h"), f->url());
    KOMPARE(2, f->sloc());
    KOMPARE(1, f->instrumented());
    KOMPARE(50, f->coverage());
    KOMPARE(2, f->callCountMap().count());
    KVERIFY(f->callCountMap().contains(10));
    KVERIFY(f->callCountMap().contains(11));
    KOMPARE(5, f->callCountMap()[10]);
    KOMPARE(0, f->callCountMap()[11]);
}

void LcovInfoParserTest::multipleFiles()
{
    QByteArray input =
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

    QBuffer* buff = new QBuffer(&input);
    m_parser->setSource(buff);
    QList<CoveredFile*> files = m_parser->go();
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
}

QTEST_KDEMAIN( LcovInfoParserTest, NoGUI )
#include "lcovinfoparsertest.moc"
