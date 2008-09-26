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

#include "qtestoutputparsertest.h"
#include "../qtestoutputparser.h"
#include "../qtestcase.h"
#include "../qtestcommand.h"

#include "kdevtest.h"

#include <QBuffer>
#include <QMetaType>
#include <QFileInfo>
#include <QVariant>
#include <QModelIndex>
#include <QAbstractListModel>

using QTest::QTestOutputParser;
using QTest::QTestCase;
using QTest::QTestCommand;
using Veritas::TestResult;

using QTest::QTestOutputParserTest;

Q_DECLARE_METATYPE(QFileInfo)
Q_DECLARE_METATYPE(Veritas::TestState)
Q_DECLARE_METATYPE(QTest::QTestCase*)
Q_DECLARE_METATYPE(QModelIndex)

/*example xml output :
     "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>"
     "<TestCase name=\"TestTest\">"
     "<Environment>"
     "<QtVersion>4.4.0-rc1</QtVersion>"
     "<QTestVersion>4.4.0-rc1</QTestVersion>"
     "</Environment>"
     "<TestFunction name=\"initTestCase\">"
     "<Incident type=\"pass\" file=\"\" line=\"0\" />"
     "</TestFunction>"
     "<TestFunction name=\"simpleRoot\">"
     "<Incident type=\"pass\" file=\"\" line=\"0\" />"
     "</TestFunction>"
     "<TestFunction name=\"cleanupTestCase\">"
     "<Incident type=\"pass\" file=\"\" line=\"0\" />"
     "</TestFunction>"
     "</TestCase>)"*/

//////////////// HELPER MACROS ///////////////////////////////////////////////

#define QTEST_HEADER_XML \
    "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"\
    "<TestCase name=\"TestTest\">\n"\
    "<Environment>\n"\
        "<QtVersion>4.4.0-rc1</QtVersion>\n"\
        "<QTestVersion>4.4.0-rc1</QTestVersion>\n"\
    "</Environment>\n"

#define QTEST_INITTESTCASE_XML\
    "<TestFunction name=\"initTestCase\">\n"\
        "<Incident type=\"pass\" file=\"\" line=\"0\" />\n"\
    "</TestFunction>\n"

#define QTEST_CLEANUPTESTCASE_XML\
    "<TestFunction name=\"cleanupTestCase\">\n"\
        "<Incident type=\"pass\" file=\"\" line=\"0\" />\n"\
    "</TestFunction>\n"

#define QTEST_FOOTER_XML \
    "</TestCase>\n"

#define QTEST_SUCCESSFUNCTION_XML \
    "<TestFunction name=\"someCommand\">\n" \
        "<Incident type=\"pass\" file=\"\" line=\"0\" />\n" \
    "</TestFunction>\n"

#include <QSignalSpy>

//////////////// HELPER CLASSES //////////////////////////////////////////////

namespace
{
// sole purpose of this is to get a valid QModelIndex.
class FakeModel : public QAbstractListModel
{
    public:
        int rowCount(const QModelIndex&) const {
            return 1;
        }
        virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const {
            Q_UNUSED(index);
            Q_UNUSED(role);
            return QVariant("owk");
        }
        QModelIndex index(int row, int column = 0, const QModelIndex& parent = QModelIndex()) const {
            Q_UNUSED(parent);
            QModelIndex i = createIndex(row, column, test);
            test->setIndex(i);
            return i;
        }
        Veritas::Test* test;
};
}

namespace QTest
{
template<> inline char* toString(const TestResult& res)
{
    return qstrdup((QString::number(res.state()) + ' ' +
                    res.message()).toLatin1().constData());
}
}

////////////// FIXTURE ///////////////////////////////////////////////////////

void QTestOutputParserTest::initTestCase()
{
    qRegisterMetaType<QModelIndex>("QModelIndex");
}

QTestOutputParserTest::TestInfo::TestInfo()
    : test(0), started(0), finished(0), result(0)
{}

QTestOutputParserTest::TestInfo::~TestInfo()
{
    if (started) delete started;
    if (finished) delete finished;
    if (result) delete result;
}

void QTestOutputParserTest::TestInfo::reset()
{
    if (started) delete started;
    if (finished) delete finished;
    if (result) delete result;
    test = 0;
    started = 0;
    finished = 0;
    result = 0;
}


void QTestOutputParserTest::init()
{
    m_buffer = 0;
    m_parser = new QTestOutputParser;
    m_caze = createTestCase(m_cazeInfo);
    m_assertType = QTestAssert;
}

void QTestOutputParserTest::cleanup()
{
    delete m_parser;
    delete m_buffer;
    delete m_caze;
    m_cazeInfo.reset();
    m_command1Info.reset();
    m_command2Info.reset();
}

// fixture setup helper
void QTestOutputParserTest::initParser(QByteArray& xml, QTestCase* caze)
{
    m_buffer = new QBuffer(&xml, 0);
    m_parser->setDevice(m_buffer);
    m_parser->setCase(caze);
    m_buffer->open(QIODevice::ReadOnly);
}



// creation method
QTestCase* QTestOutputParserTest::createTestCase(TestInfo& cInfo)
{
    QTestCase* caze = new QTestCase("TestTest", QFileInfo(), 0);
    cInfo.test = caze;
    cInfo.finished = new QSignalSpy(caze, SIGNAL(finished(QModelIndex)));
    cInfo.started  = new QSignalSpy(caze, SIGNAL(started(QModelIndex)));

    FakeModel* fm = new FakeModel;
    fm->test = caze;
    QModelIndex index = fm->index(0);
    delete fm;
    return caze;
}

// creation method
void QTestOutputParserTest::createTestCommand(TestInfo& cInfo, QTestCase* parent, QString name)
{
    cInfo.test = new QTestCommand(name, (QTestCase*)parent);
    parent->addChild(cInfo.test);
    cInfo.started  = new QSignalSpy(cInfo.test, SIGNAL(started(QModelIndex)));
    cInfo.finished = new QSignalSpy(cInfo.test, SIGNAL(finished(QModelIndex)));

    FakeModel* fm = new FakeModel;
    fm->test = cInfo.test;
    QModelIndex index = fm->index(0);
    delete fm;
}

// fixture setup helper
void QTestOutputParserTest::setExpectedSuccess(TestInfo& tInfo)
{
    setExpectedResult(tInfo, Veritas::RunSuccess, "", 0, "");
}

// fixture setup helper
void QTestOutputParserTest::setExpectedFailure(TestInfo& tInfo)
{
    setExpectedResult(tInfo, Veritas::RunError, "/path/to/file.cpp",
                             100, "failure message");
}

// fixture setup helper
void QTestOutputParserTest::setExpectedResult(
    TestInfo& tInfo,
    Veritas::TestState state,
    QString filePath,
    int lineNumber,
    QString failureMessage)
{
    TestResult* res = new TestResult;
    res->setState(state);
    res->setFile(filePath);
    res->setLine(lineNumber);
    res->setMessage(failureMessage);
    tInfo.result = res;
}

/////////////// DATA TEST ///////////////////////////////////////////////////////////

//test command
void QTestOutputParserTest::parse()
{
    // exercise
    QFETCH(QByteArray, xml);
    initParser(xml, m_caze);

    QFETCH(Veritas::TestState, state);
    QFETCH(QFileInfo, file);
    QFETCH(int, line);
    QFETCH(QString, message);
    createTestCommand(m_command1Info, m_caze, "someCommand");
    setExpectedResult(m_command1Info, state, file.filePath(), line, message);
    QSignalSpy spy(m_parser, SIGNAL(done()));

    m_parser->go();

    assertParsed(m_command1Info);
    checkResult(m_command1Info);
    KOMPARE(1, spy.count());
}

// test data implementation
void QTestOutputParserTest::parse_data()
{
    setupColumns();
    addSunnyDayData();
    addBasicFailureData();
}

// helper for parse_data
void QTestOutputParserTest::setupColumns()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<Veritas::TestState>("state");
    QTest::addColumn<QFileInfo>("file");
    QTest::addColumn<int>("line");
    QTest::addColumn<QString>("message");
    QTest::addColumn<QTestCase*>("case");
}

// test data
void QTestOutputParserTest::addSunnyDayData()
{
    // first row - sunny day test succes
    QByteArray input =
        QTEST_HEADER_XML
        QTEST_INITTESTCASE_XML
        "<TestFunction name=\"someCommand\">\n"
            "<Incident type=\"pass\" file=\"\" line=\"0\" />\n"
        "</TestFunction>\n"
        QTEST_CLEANUPTESTCASE_XML
        QTEST_FOOTER_XML;

    QTest::newRow("sunny day test succes")
    << input << Veritas::RunSuccess
    << QFileInfo("") << 0 << "" << m_caze;
}

// test data
void QTestOutputParserTest::addBasicFailureData()
{
    // second row - test failure
    QByteArray input =
        QTEST_HEADER_XML
        QTEST_INITTESTCASE_XML
        "<TestFunction name=\"someCommand\">\n"
            "<Incident type=\"fail\" file=\"/path/to/file.cpp\" line=\"100\">\n"
            "<Description><![CDATA[some message]]></Description>\n"
            "</Incident>\n"
        "</TestFunction>\n"
        QTEST_CLEANUPTESTCASE_XML
        QTEST_FOOTER_XML;

    QTest::newRow("basic failure")
    << input << Veritas::RunError
    << QFileInfo("/path/to/file.cpp") << 100
    << "some message" << m_caze;
}

////////////// TEST COMMANDS ///////////////////////////////////////////////////////

// test command
void QTestOutputParserTest::initFailure()
{
    QByteArray input =
        QTEST_HEADER_XML
        "<TestFunction name=\"initTestCase\">\n"
            "<Incident type=\"fail\" file=\"/path/to/file.cpp\" line=\"100\">\n"
            "<Description><![CDATA[failure message]]></Description>\n"
            "</Incident>\n"
        "</TestFunction>\n"
        QTEST_SUCCESSFUNCTION_XML
        QTEST_CLEANUPTESTCASE_XML
        QTEST_FOOTER_XML;
    initParser(input, m_caze);
    setExpectedFailure(m_cazeInfo);

    m_parser->go();

    assertParsed(m_cazeInfo);
    checkResult(m_cazeInfo);
}

// test command
void QTestOutputParserTest::cleanupFailure()
{
    QByteArray input =
        QTEST_HEADER_XML
        QTEST_INITTESTCASE_XML
        QTEST_SUCCESSFUNCTION_XML
        "<TestFunction name=\"cleanupTestCase\">\n"
            "<Incident type=\"fail\" file=\"/path/to/file.cpp\" line=\"100\">\n"
            "<Description><![CDATA[failure message]]></Description>\n"
            "</Incident>\n"
        "</TestFunction>\n"
        QTEST_FOOTER_XML;
    initParser(input, m_caze);

    setExpectedFailure(m_cazeInfo);
    createTestCommand(m_command1Info, m_caze, "someCommand");
    setExpectedSuccess(m_command1Info);

    m_parser->go();

    assertParsed(m_command1Info);
    checkResult(m_command1Info);
    assertParsed(m_cazeInfo);
    checkResult(m_cazeInfo);
}

// command
void QTestOutputParserTest::doubleFailure()
{
    QByteArray input =
        QTEST_HEADER_XML
        QTEST_INITTESTCASE_XML
        "<TestFunction name=\"command1\">\n"
            "<Incident type=\"fail\" file=\"/path/to/file.cpp\" line=\"100\">\n"
            "<Description><![CDATA[failure message]]></Description>\n"
            "</Incident>\n"
        "</TestFunction>\n"
        "<TestFunction name=\"command2\">\n"
            "<Incident type=\"fail\" file=\"/path/to/another.cpp\" line=\"50\">\n"
            "<Description><![CDATA[another failure message]]></Description>\n"
            "</Incident>\n"
        "</TestFunction>\n"
        QTEST_CLEANUPTESTCASE_XML
        QTEST_FOOTER_XML;
    initParser(input, m_caze);

    createTestCommand(m_command1Info, m_caze, "command1");
    setExpectedResult(m_command1Info, Veritas::RunError, "/path/to/file.cpp",
                              100, "failure message");
    createTestCommand(m_command2Info, m_caze, "command2");
    setExpectedResult(m_command2Info, Veritas::RunError, "/path/to/another.cpp",
                               50, "another failure message");

    m_parser->go();

    assertParsed(m_command1Info);
    checkResult(m_command1Info);
    assertParsed(m_command2Info);
    checkResult(m_command2Info);
}

// test command
void QTestOutputParserTest::skipSingle()
{
    // QSKIP with the SkipSingle flag
    QByteArray input =
        QTEST_HEADER_XML
        QTEST_INITTESTCASE_XML
        "<TestFunction name=\"command\">\n"
        "<Message type=\"skip\" file=\"/path/to/file.cpp\" line=\"8\">\n"
        "    <Description><![CDATA[skipCommand]]></Description>\n"
        "</Message>\n"
        "<Incident type=\"pass\" file=\"\" line=\"0\" />\n"
        "</TestFunction>\n"
        QTEST_CLEANUPTESTCASE_XML
        QTEST_FOOTER_XML;
    initParser(input, m_caze);
    createTestCommand(m_command1Info, m_caze, "command");
    setExpectedResult(m_command1Info, Veritas::RunInfo, "/path/to/file.cpp", 8, "skipCommand (skipped)");

    m_parser->go();

    assertParsed(m_command1Info);
    checkResult(m_command1Info);
}

// test command
void QTestOutputParserTest::skipAll()
{
    // QSKIP with the SkipAll flag. QTestLib refuses to print an <Incident type="pass" /> for those.
    QByteArray input =
        QTEST_HEADER_XML
        QTEST_INITTESTCASE_XML
        "<TestFunction name=\"command\">\n"
        "<Message type=\"skip\" file=\"/path/to/file.cpp\" line=\"8\">\n"
        "    <Description><![CDATA[skipCommand]]></Description>\n"
        "</Message>\n"
        "</TestFunction>\n"
        QTEST_CLEANUPTESTCASE_XML
        QTEST_FOOTER_XML;
    initParser(input, m_caze);

    createTestCommand(m_command1Info, m_caze, "command");
    setExpectedResult(m_command1Info, Veritas::RunInfo, "/path/to/file.cpp", 8, "skipCommand (skipped)");

    m_parser->go();

    assertParsed(m_command1Info);
    checkResult(m_command1Info);
}

// test command
void QTestOutputParserTest::qassert()
{
    QByteArray input =
        QTEST_HEADER_XML
        QTEST_INITTESTCASE_XML
        "<TestFunction name=\"command\">\n"
        "<Message type=\"qfatal\" file=\"\" line=\"0\">\n"
            "<Description><![CDATA[ASSERT: \"condition\" in file /path/to/file.cpp, line 66]]></Description>\n"
        "</Message>\n"
        "<Incident type=\"fail\" file=\"Unknown file\" line=\"0\">\n"
            "<Description><![CDATA[Received a fatal error.]]></Description>\n"
        "</Incident>\n"
        "</TestFunction>\n"
        QTEST_CLEANUPTESTCASE_XML
        QTEST_FOOTER_XML;
    initParser(input, m_caze);

    createTestCommand(m_command1Info, m_caze, "command");
    setExpectedResult(m_command1Info, Veritas::RunFatal, "/path/to/file.cpp", 66, "ASSERT: \"condition\"");

    m_parser->go();

    assertParsed(m_command1Info);
    checkResult(m_command1Info);
}

// test command
void QTestOutputParserTest::qassertx()
{
    QByteArray input =
        QTEST_HEADER_XML
        QTEST_INITTESTCASE_XML
        "<TestFunction name=\"command\">\n"
        "<Message type=\"qfatal\" file=\"\" line=\"0\">\n"
            "<Description><![CDATA[ASSERT failure in command: \"message in file\", file /path/to/file.cpp, line 66]]></Description>\n"
        "</Message>\n"
        "<Incident type=\"fail\" file=\"Unknown file\" line=\"0\">\n"
            "<Description><![CDATA[Received a fatal error.]]></Description>\n"
        "</Incident>\n"
        "</TestFunction>\n"
        QTEST_CLEANUPTESTCASE_XML
        QTEST_FOOTER_XML;
    initParser(input, m_caze);

    createTestCommand(m_command1Info, m_caze, "command");
    setExpectedResult(m_command1Info, Veritas::RunFatal, "/path/to/file.cpp", 66, "ASSERT failure in command: \"message in file\"");

    m_parser->go();

    assertParsed(m_command1Info);
    checkResult(m_command1Info);
}

/////////////// RANDOM TEST ///////////////////////////////////////////////////

#include <stdlib.h>
#include <time.h>
#include <QTextStream>

namespace {
QTextStream cout(stdout);
}

QList<QByteArray> chopInPieces(const QByteArray& input, int nrofPieces)
{
    QList<int> cutPositions;
    for (int i=0; i<nrofPieces-1; i++) {
        int pos = -1;
        do {
            pos = rand() % input.size();
        } while (cutPositions.contains(pos));
        cutPositions << pos;
    }
    qSort(cutPositions);
    QList<QByteArray> pieces;
    pieces << input.mid(0, (nrofPieces == 1) ? -1 : cutPositions[0]);
    for (int i=0; i<nrofPieces-1; i++) {
        int startPos = -1;
        int nrofChars = -1;
        if (i==nrofPieces-2) {
            startPos = cutPositions[i];
            nrofChars = -1;
        } else {
            startPos = cutPositions[i];
            nrofChars = cutPositions[i+1] - cutPositions[i];
        }
        pieces << input.mid(startPos, nrofChars);
    }
    return pieces;
}

void QTestOutputParserTest::runRandomCommand(const QList<QByteArray>& pieces, QTestCase* caze)
{
    m_parser = new QTestOutputParser;
    m_buffer = new QBuffer();
    m_parser->setDevice(m_buffer);
    m_parser->setCase(caze);
    m_buffer->open(QIODevice::ReadOnly | QIODevice::WriteOnly);

    foreach(const QByteArray& piece, pieces) {
        qint64 pos = m_buffer->pos();
        m_buffer->write(piece);
        m_buffer->seek(pos);
        m_parser->go();
    }

    delete m_parser;
    delete m_buffer;
}

void QTestOutputParserTest::verifyRandomResults(QList<TestInfo*>& expected)
{
    foreach(TestInfo* ti, expected) {
        assertParsed(*ti);
        checkResult(*ti);
    }
}

void QTestOutputParserTest::generateRandomInput(int maxCommands, QByteArray& parserInput, QList<TestInfo*>& expected, QTestCase*& caze)
{
    parserInput = QTEST_HEADER_XML
                     QTEST_INITTESTCASE_XML;

    TestInfo* cazeInfo = new TestInfo;
    caze = createTestCase(*cazeInfo);
    delete cazeInfo;
    int nrofCommands = (rand() % maxCommands) + 1;

    int nrofSuccess = 0, nrofFailures = 0, nrofSkipSingle = 0, nrofSkipAll = 0, nrofAsserts = 0;
    for (int i=0; i<nrofCommands; i++) {
        int type = rand() % 7;
        TestInfo* cmdInfo = new TestInfo;
        createTestCommand(*cmdInfo, caze, QString("command%1").arg(i));
        parserInput += "<TestFunction name=\"command" + QString::number(i).toLatin1() + "\">\n";
        switch(type) {
        case 0: case 1: // run-off-the-mill success
            setExpectedResult(*cmdInfo, Veritas::RunSuccess, "", 0, "");
            parserInput +=
                "<Incident type=\"pass\" file=\"\" line=\"0\" />\n";
            nrofSuccess++;
            break;
        case 2: case 3: // normal failure
            setExpectedResult(*cmdInfo, Veritas::RunError, "/path/to/file.cpp", 100, "some message");
            parserInput +=
                "<Incident type=\"fail\" file=\"/path/to/file.cpp\" line=\"100\">\n"
                    "<Description><![CDATA[some message]]></Description>\n"
                "</Incident>\n";
            nrofFailures++;
            break;
        case 4: // SkipSingle
            setExpectedResult(*cmdInfo, Veritas::RunInfo, "/path/to/skipfile.cpp", 8, "skipCommand (skipped)");
            parserInput +=
                "<Message type=\"skip\" file=\"/path/to/skipfile.cpp\" line=\"8\">\n"
                "    <Description><![CDATA[skipCommand]]></Description>\n"
                "</Message>\n"
                "<Incident type=\"pass\" file=\"\" line=\"0\" />\n";
            nrofSkipSingle++;
            break;
        case 5: // SkipAll ie without <Incident type='pass' />
            setExpectedResult(*cmdInfo, Veritas::RunInfo, "/path/to/skipfile.cpp", 9, "skipCommand (skipped)");
            parserInput +=
                "<Message type=\"skip\" file=\"/path/to/skipfile.cpp\" line=\"9\">\n"
                "    <Description><![CDATA[skipCommand]]></Description>\n"
                "</Message>\n";
            nrofSkipAll++;
            break;
        case 6: // Q_ASSERT
            setExpectedResult(*cmdInfo, Veritas::RunFatal, "/path/to/qassertfile.cpp", 66, "ASSERT: \"condition\"");
            parserInput +=
                "<Message type=\"qfatal\" file=\"\" line=\"0\">\n"
                    "<Description><![CDATA[ASSERT: \"condition\" in file /path/to/qassertfile.cpp, line 66]]></Description>\n"
                "</Message>\n"
                "<Incident type=\"fail\" file=\"Unknown file\" line=\"0\">\n"
                    "<Description><![CDATA[Received a fatal error.]]></Description>\n"
                "</Incident>\n";
            nrofAsserts++;
            break;
        default: ;
        }
        parserInput += "</TestFunction>\n";
        expected << cmdInfo;
    }

    parserInput +=
        QTEST_CLEANUPTESTCASE_XML
        QTEST_FOOTER_XML;

    m_randomTestType += "[success:" + QString::number(nrofSuccess) + ";failures:" + QString::number(nrofFailures) +
                        ";skipsingle:" + QString::number(nrofSkipSingle) + ";skipall:" + QString::number(nrofSkipAll) +
                        ";asserts:" + QString::number(nrofAsserts) + "]\n";
}

namespace {

QByteArray randomWord(int length)
{
    static QByteArray letters = "abcdefghijklmnopqrstuvwxyz1234567890abcdefghijklmnopqrstuvwxyz1234567890!@#$%^&*()-+=__[]{}'\"|,./<><><><><><><><><><><><><><><><><><><><><><><><><><><>\"\"\"\"\"\"?";
    static int nrof = letters.size();

    QByteArray result;
    result.reserve(length);
    for (int i=0; i<length; i++) {
        result.append(letters[rand() % nrof]);
    }
    return result;
}

void insertGarbage(QList<QByteArray>& input)
{
    QMutableListIterator<QByteArray> it(input);
    while(it.hasNext()) {
        int i = rand() % 3;
        if (i==2) {
            int length = (rand() % 20) +1;
            QByteArray garbage = randomWord(length);
            it.insert(garbage);
        }
        it.next();
    }
}
}

void QTestOutputParserTest::setQuiet(bool quiet)
{
    m_quiet = quiet;
}

void QTestOutputParserTest::randomValidXML()
{
    cout << "------------- OutputParser Random Stress Test  -------------" << endl;
    srand(time(0));

    int count = 0;
    m_assertType = QAssertAssert;
    for (int nrofPieces=1; nrofPieces<31; nrofPieces++) {
        for (int i=0; i<10000; i++) {
            m_randomTestType = "[pieces:" + QString::number(nrofPieces) + ";cycle:" + QString::number(i) + "]\n";
            QByteArray input;
            QTestCase* caze;
            QList<TestInfo*> expected;

            generateRandomInput(30, input, expected, caze);
            if (input.size() / 2 < nrofPieces) continue;
            m_pieces = chopInPieces(input, nrofPieces);
            runRandomCommand(m_pieces, caze);
            verifyRandomResults(expected);
            if (!m_quiet) {
                cout << ".";
                cout.flush();
            }
            count++;

            delete caze;
            qDeleteAll(expected);
        }
    }

    cout << "\n" << count << " tests succesful!" << endl;
    cout << "------------- OutputParser Random Stress Test  -------------" << endl;
    QCoreApplication::quit();
}

void QTestOutputParserTest::randomGarbageXML()
{
    cout << "------------- OutputParser Random Garbage Test -------------" << endl;
    srand(time(0));

    int count = 0;
    m_assertType = QAssertAssert;

    for (int nrofPieces=1; nrofPieces<31; nrofPieces++) {
        for (int i=0; i<10000; i++) {
            m_randomTestType = "[pieces:" + QString::number(nrofPieces) + ";cycle:" + QString::number(i) + "]\n";
            QByteArray input;
            QTestCase* caze;
            QList<TestInfo*> expected;

            generateRandomInput(30, input, expected, caze);
            if (input.size() / 2 < nrofPieces) continue;
            m_pieces = chopInPieces(input, nrofPieces);
            insertGarbage(m_pieces);
            runRandomCommand(m_pieces, caze);
            if (!m_quiet) {
                cout << ".";
                cout.flush();
            }
            count++;

            delete caze;
            qDeleteAll(expected);
        }
    }

    cout << "\n" << count << " tests succesful!" << endl;
    cout << "------------- OutputParser Random Garbage Test -------------" << endl;
    QCoreApplication::quit();
}

/////////////// CUSTOM ASSERTIONS ////////////////////////////////////////////

#include <veritas/test.h>
using Veritas::Test;

// custom assertion
void QTestOutputParserTest::checkResult(TestInfo& testInfo)
{
    QModelIndex i = testInfo.finished->first().value(0).value<QModelIndex>();
    Veritas::Test* test = static_cast<Veritas::Test*>(i.internalPointer());
    TestResult* actual = test->result();
    TestResult* expected = testInfo.result;
    assertResult(expected, actual, test->name());
}

namespace {
QByteArray join(const QList<QByteArray>& list)
{
    QByteArray joined;
    foreach(const QByteArray& item, list) {
        joined = joined.append("-------------------\n").append(item).append('\n');
    }
    return joined;
}
}

#define Q_ASSERT_GOT_SIGNAL(spy, test) \
if (1 != spy->count()) {\
    Q_ASSERT_X(1 == spy->count(), "assertParsed()", \
               QString("Parser did not emit " #spy " signal for %1.\n%3 input:\n%2").\
               arg(test->name()).arg(join(m_pieces).constData()).arg(m_randomTestType).toLatin1().constData()); \
} else (void)(0)

#define Q_ASSERT_EQUALS(expected, actual, item)\
if (expected != actual) {\
    Q_ASSERT_X(expected == actual, "assertResult", \
                QString("Wrong test " #item ". Expected %1 got %2.\nTest: %5\n%4\ninput:\n%3").\
                arg(expected).arg(actual).arg(join(m_pieces).data()).arg(m_randomTestType).arg(testName).toLatin1().constData());\
} else (void)(0)


// helper
void QTestOutputParserTest::assertParsed(TestInfo& testInfo)
{
    QSignalSpy* started = testInfo.started;
    QSignalSpy* completed = testInfo.finished;
    Veritas::Test* test = testInfo.test;

    if (m_assertType == QTestAssert) {
        KOMPARE_MSG(1, started->count(),
            QString("OutputParser did not emit started signal for ") + test->name());
        KOMPARE_MSG(1, completed->count(),
            QString("OutputParser did not emit finished signal for ") + test->name());
    } else if (m_assertType == QAssertAssert) {
        Q_ASSERT_GOT_SIGNAL(started, test);
        Q_ASSERT_GOT_SIGNAL(completed, test);
    } else { Q_ASSERT(0); }

    QModelIndex i1 = completed->first().value(0).value<QModelIndex>();
    QModelIndex i2 = started->first().value(0).value<QModelIndex>();

    if (m_assertType == QTestAssert) {
        KOMPARE(test, static_cast<Veritas::Test*>(i1.internalPointer()));
        KOMPARE(test, static_cast<Veritas::Test*>(i2.internalPointer()));
    } else if (m_assertType == QAssertAssert) {
        Test* startedTest = static_cast<Veritas::Test*>(i1.internalPointer());
        Test* complTest = static_cast<Veritas::Test*>(i2.internalPointer());
        QString testName = test->name();
        Q_ASSERT(startedTest); Q_ASSERT(complTest);
        Q_ASSERT_EQUALS(test->name(), startedTest->name(), started name);
        Q_ASSERT_EQUALS(test->name(), complTest->name(), completed name);
    }
}

void QTestOutputParserTest::assertResult(TestResult* expected, TestResult* actual, const QString& name)
{
    if (m_assertType == QTestAssert) {
        assertResult(*expected, *actual);
    } else if (m_assertType == QAssertAssert) {
        qassertResult(expected, actual, name);
    }
}

// helper
void QTestOutputParserTest::assertResult(const TestResult& expected, const TestResult& actual)
{
    KOMPARE_MSG(expected.state(), actual.state(),
                "Expected " + QString::number(expected.state()) +
                " got " + QString::number(actual.state()));
    KOMPARE(expected.file(), actual.file());
    KOMPARE(expected.line(), actual.line());
    KOMPARE_MSG(expected.message(), actual.message(),
                "Expected " + expected.message() + " got " + actual.message());
}

// helper
void QTestOutputParserTest::qassertResult(TestResult* expected, TestResult* actual, const QString& testName)
{
    Q_ASSERT_EQUALS(expected->state(), actual->state(), state);
    Q_ASSERT_EQUALS(expected->file().path(), actual->file().path(), file);
    Q_ASSERT_EQUALS(expected->line(), actual->line(), line);
    Q_ASSERT_EQUALS(expected->message(), actual->message(), message);
}

#include "qtestoutputparsertest.moc"
// QTEST_KDEMAIN(QTestOutputParserTest, NoGUI)
