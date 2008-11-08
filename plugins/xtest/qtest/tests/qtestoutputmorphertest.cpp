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

#include "qtestoutputmorphertest.h"
#include "plugins/xtest/qtest/qtestoutputmorpher.h"
#include <kasserts.h>
#include <qtest_kde.h>

using QTest::Test::OutputMorpherTest;
using QTest::OutputMorpher;

// data-command. parse_data() member declared under the actual data.
void OutputMorpherTest::parse()
{
    QFETCH(QByteArray, input);
    QFETCH(QByteArray, output);

    QBuffer inputBuff;
    inputBuff.setData(input);
    OutputMorpher om;
    QBuffer outputBuff;
    om.setSource(&inputBuff);
    om.setTarget(&outputBuff);
    om.xmlToText();
    KOMPARE_MSG(output, outputBuff.data(),
                QString("\nExpected:\n") + QString(output) +
                "<-------------------------------------->\n" +
                "Got:\n" + QString(outputBuff.data()));
}

#define QTEST_XML_HEADER \
    "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n" \
    "<TestCase name=\"FooTest\">\n" \
    "<Environment>\n" \
    "<QtVersion>4.4.0</QtVersion>\n" \
    "<QTestVersion>4.4.0</QTestVersion>\n" \
    "</Environment>\n"

#define QTEST_XML_FOOTER \
    "</TestCase>\n";

#define QTEST_XML_INITTESTCASE \
    "<TestFunction name=\"initTestCase\">\n" \
    "<Incident type=\"pass\" file=\"\" line=\"0\" />\n" \
    "</TestFunction>\n"

#define QTEST_XML_CLEANUPTESTCASE \
    "<TestFunction name=\"cleanupTestCase\">\n" \
    "<Incident type=\"pass\" file=\"\" line=\"0\" />\n" \
    "</TestFunction>\n"

QByteArray emptyCaseInput =
    QTEST_XML_HEADER
    QTEST_XML_FOOTER

QByteArray emptyCaseOutput =
    "********* Started testing of FooTest *********\n"
    "********* Finished testing of FooTest *********\n";

QByteArray singleCommandInput =
    QTEST_XML_HEADER
    QTEST_XML_INITTESTCASE
    "<TestFunction name=\"foo\">\n"
    "<Incident type=\"pass\" file=\"\" line=\"0\" />\n"
    "</TestFunction>\n"
    QTEST_XML_CLEANUPTESTCASE
    QTEST_XML_FOOTER
QByteArray singleCommandOutput =
    "********* Started testing of FooTest *********\n"
    "PASS   : FooTest::initTestCase()\n"
    "PASS   : FooTest::foo()\n"
    "PASS   : FooTest::cleanupTestCase()\n"
    "********* Finished testing of FooTest *********\n";

QByteArray failingCommandInput =
    QTEST_XML_HEADER
    QTEST_XML_INITTESTCASE
    "<TestFunction name=\"foo\">\n"
    "<Incident type=\"fail\" file=\"/path/to/footest.cpp\" line=\"66\">\n"
    "<Description><![CDATA[failure message]]></Description>\n"
    "</Incident>\n"
    "</TestFunction>\n"
    QTEST_XML_CLEANUPTESTCASE
    QTEST_XML_FOOTER
QByteArray failingCommandOutput =
    "********* Started testing of FooTest *********\n"
    "PASS   : FooTest::initTestCase()\n"
    "FAIL!  : FooTest::foo() failure message\n"
    "   Loc: [/path/to/footest.cpp(66)]\n"
    "PASS   : FooTest::cleanupTestCase()\n"
    "********* Finished testing of FooTest *********\n";

QByteArray failingSlotInput =
    QTEST_XML_HEADER
    QTEST_XML_INITTESTCASE
    "<TestFunction name=\"foo\">\n"
    "<Incident type=\"fail\" file=\"/path/to/footest.cpp\" line=\"66\">\n"
        "<DataTag><![CDATA[slot_name]]></DataTag>\n"
        "<Description><![CDATA[failure message]]></Description>\n"
    "</Incident>\n"
    "</TestFunction>\n"
    QTEST_XML_CLEANUPTESTCASE
    QTEST_XML_FOOTER
QByteArray failingSlotOutput =
    "********* Started testing of FooTest *********\n"
    "PASS   : FooTest::initTestCase()\n"
    "FAIL!  : FooTest::foo(slot_name) failure message\n"
    "   Loc: [/path/to/footest.cpp(66)]\n"
    "PASS   : FooTest::cleanupTestCase()\n"
    "********* Finished testing of FooTest *********\n";

QByteArray spamInput =
    QTEST_XML_HEADER
    QTEST_XML_INITTESTCASE
    "<TestFunction name=\"foo\">\n"
    "<Message type=\"system\" file=\"\" line=\"0\">\n"
        "<Description><![CDATA[system message]]></Description>\n"
    "</Message>\n"
    "<Message type=\"debug\" file=\"\" line=\"0\">\n"
        "<Description><![CDATA[debug message]]></Description>\n"
    "</Message>\n"
    "<Incident type=\"pass\" file=\"\" line=\"0\" />\n"
    "</TestFunction>\n"
    QTEST_XML_CLEANUPTESTCASE
    QTEST_XML_FOOTER
QByteArray spamOutput =
    "********* Started testing of FooTest *********\n"
    "PASS   : FooTest::initTestCase()\n"
    "QSYSTEM: FooTest::foo() system message\n"
    "QDEBUG : FooTest::foo() debug message\n"
    "PASS   : FooTest::foo()\n"
    "PASS   : FooTest::cleanupTestCase()\n"
    "********* Finished testing of FooTest *********\n";

QByteArray multiCommandsInput =
    QTEST_XML_HEADER
    QTEST_XML_INITTESTCASE
    "<TestFunction name=\"foo\">\n"
    "<Incident type=\"pass\" file=\"\" line=\"0\" />\n"
    "</TestFunction>\n"
    "<TestFunction name=\"bar\">\n"
    "<Incident type=\"fail\" file=\"/path/to/bartest.cpp\" line=\"66\">\n"
    "<Description><![CDATA[failure message]]></Description>\n"
    "</Incident>\n"
    "</TestFunction>\n"
    QTEST_XML_CLEANUPTESTCASE
    QTEST_XML_FOOTER
QByteArray multiCommandsOutput =
    "********* Started testing of FooTest *********\n"
    "PASS   : FooTest::initTestCase()\n"
    "PASS   : FooTest::foo()\n"
    "FAIL!  : FooTest::bar() failure message\n"
    "   Loc: [/path/to/bartest.cpp(66)]\n"
    "PASS   : FooTest::cleanupTestCase()\n"
    "********* Finished testing of FooTest *********\n";

QByteArray assertFailureInput =
    QTEST_XML_HEADER
    QTEST_XML_INITTESTCASE
    "<TestFunction name=\"foo\">\n"
    "<Message type=\"qfatal\" file=\"\" line=\"0\">\n"
    "<Description><![CDATA[ASSERT: \"false\" in file /path/to/origin.cpp, line 66]]></Description>\n"
    "</Message>\n"
    "<Incident type=\"fail\" file=\"Unknown file\" line=\"0\">\n"
        "<Description><![CDATA[Received a fatal error.]]></Description>\n"
    "</Incident>\n"
    "</TestFunction>\n"
    QTEST_XML_CLEANUPTESTCASE
    QTEST_XML_FOOTER
QByteArray assertFailureOutput =
    "********* Started testing of FooTest *********\n"
    "PASS   : FooTest::initTestCase()\n"
    "QFATAL : FooTest::foo() ASSERT: \"false\" in file /path/to/origin.cpp, line 66\n"
    "FAIL!  : FooTest::foo() Received a fatal error.\n"
    "   Loc: [Unknown file(0)]\n"
    "PASS   : FooTest::cleanupTestCase()\n"
    "********* Finished testing of FooTest *********\n";

QByteArray expectedFailureInput =
    QTEST_XML_HEADER
    QTEST_XML_INITTESTCASE
    "<TestFunction name=\"foo\">\n"
    "<Incident type=\"xfail\" file=\"/path/to/footest.cpp\" line=\"66\">\n"
    "<Description><![CDATA[expected failure message]]></Description>\n"
    "</Incident>\n"
    "</TestFunction>\n"
    QTEST_XML_CLEANUPTESTCASE
    QTEST_XML_FOOTER
QByteArray expectedFailureOutput =
    "********* Started testing of FooTest *********\n"
    "PASS   : FooTest::initTestCase()\n"
    "XFAIL!  : FooTest::foo() expected failure message\n"
    "   Loc: [/path/to/footest.cpp(66)]\n"
    "PASS   : FooTest::cleanupTestCase()\n"
    "********* Finished testing of FooTest *********\n";

void OutputMorpherTest::parse_data()
{
    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<QByteArray>("output");

    QTest::newRow("empty") << QByteArray("") << QByteArray("");
    QTest::newRow("empty_case") << emptyCaseInput << emptyCaseOutput;
    QTest::newRow("single_command") << singleCommandInput << singleCommandOutput;
    QTest::newRow("failing_command") << failingCommandInput << failingCommandOutput;
    QTest::newRow("failing_slot") << failingSlotInput << failingSlotOutput;
    QTest::newRow("spam_messages") << spamInput << spamOutput;
    QTest::newRow("multiple_commands") << multiCommandsInput << multiCommandsOutput;
    QTest::newRow("assert_failure") << assertFailureInput << assertFailureOutput;
    QTest::newRow("expected_failure") << expectedFailureInput << expectedFailureOutput;
}


// ********* Start testing of QTest::ut::CaseTest *********
// Config: Using QTest library 4.4.0, Qt 4.4.0
// PASS   : QTest::ut::CaseTest::initTestCase()
// QWARN  : QTest::ut::CaseTest::construct() QFile::remove: Empty or null file name
// QWARN  : QTest::ut::CaseTest::construct() QFile::remove: Empty or null file name
// QWARN  : QTest::ut::CaseTest::construct() QFile::remove: Empty or null file name
// PASS   : QTest::ut::CaseTest::construct()
// QWARN  : QTest::ut::CaseTest::addCommand() QFile::remove: Empty or null file name
// QWARN  : QTest::ut::CaseTest::addCommand() QFile::remove: Empty or null file name
// QWARN  : QTest::ut::CaseTest::addCommand() QFile::remove: Empty or null file name
// PASS   : QTest::ut::CaseTest::addCommand()
// QWARN  : QTest::ut::CaseTest::addCommands() QFile::remove: Empty or null file name
// QWARN  : QTest::ut::CaseTest::addCommands() QFile::remove: Empty or null file name
// QWARN  : QTest::ut::CaseTest::addCommands() QFile::remove: Empty or null file name
// PASS   : QTest::ut::CaseTest::addCommands()
// PASS   : QTest::ut::CaseTest::cleanupTestCase()
// Totals: 5 passed, 0 failed, 0 skipped
// ********* Finished testing of QTest::ut::CaseTest *********
// nix:~/KdeDev/kdevelop/build/plugins/xtest/qtest/tests> ./qtest-ut-qtestcase -xml
// <?xml version="1.0" encoding="ISO-8859-1"?>
// <TestCase name="QTest::ut::CaseTest">
// <Environment>
//     <QtVersion>4.4.0</QtVersion>
//     <QTestVersion>4.4.0</QTestVersion>
// </Environment>
// <TestFunction name="initTestCase">
// <Incident type="pass" file="" line="0" />
// </TestFunction>
// <TestFunction name="construct">
// <Message type="qwarn" file="" line="0">
//     <Description><![CDATA[QFile::remove: Empty or null file name]]></Description>
// </Message>
// <Message type="qwarn" file="" line="0">
//     <Description><![CDATA[QFile::remove: Empty or null file name]]></Description>
// </Message>
// <Message type="qwarn" file="" line="0">
//     <Description><![CDATA[QFile::remove: Empty or null file name]]></Description>
// </Message>
// <Incident type="pass" file="" line="0" />
// </TestFunction>
// <TestFunction name="addCommand">
// <Message type="qwarn" file="" line="0">
//     <Description><![CDATA[QFile::remove: Empty or null file name]]></Description>
// </Message>
// <Message type="qwarn" file="" line="0">
//     <Description><![CDATA[QFile::remove: Empty or null file name]]></Description>
// </Message>
// <Message type="qwarn" file="" line="0">
//     <Description><![CDATA[QFile::remove: Empty or null file name]]></Description>
// </Message>
// <Incident type="pass" file="" line="0" />
// </TestFunction>
// <TestFunction name="addCommands">
// <Message type="qwarn" file="" line="0">
//     <Description><![CDATA[QFile::remove: Empty or null file name]]></Description>
// </Message>
// <Message type="qwarn" file="" line="0">
//     <Description><![CDATA[QFile::remove: Empty or null file name]]></Description>
// </Message>
// <Message type="qwarn" file="" line="0">
//     <Description><![CDATA[QFile::remove: Empty or null file name]]></Description>
// </Message>
// <Incident type="pass" file="" line="0" />
// </TestFunction>
// <TestFunction name="cleanupTestCase">
// <Incident type="pass" file="" line="0" />
// </TestFunction>
// </TestCase>
//



#include "qtestoutputmorphertest.moc"
QTEST_KDEMAIN(OutputMorpherTest, NoGUI)
