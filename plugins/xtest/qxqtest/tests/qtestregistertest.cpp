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

#include "qtestregistertest.h"
#include <qtest_kde.h>
#include <qtestregister.h>
#include <qtestsuite.h>
#include <qtestcase.h>
#include <qtestcommand.h>

#include <kasserts.h>

#include <QByteArray>
#include <QFileInfo>
#include <QBuffer>

using QxQTest::QTestRegister;
using QxQTest::QTestSuite;
using QxQTest::QTestCase;
using QxQTest::QTestCommand;

// <?xml version="1.0" encoding="ISO-8859-1"?>
// <root>
//  <suite name="suite1" dir="/a/b">
//    <case name="test1" exe="t.sh">
//      <command name="cmd11" />
//      <command name="cmd12" />
//    </case>
//    <case name="test2" exe="t2.sh">
//      <command name="cmd21" />
//    </case>
//  </suite>
// </root>

namespace
{
QByteArray headerXml = "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
                       "<root>\n";
QByteArray suite1Xml = "<suite name=\"suite1\" dir=\"/a/b\" />";
QByteArray suite2Xml = "<suite name=\"suite2\" dir=\"/c/d\" />";
QByteArray footerXml = "</root>\n";
QByteArray suiteStart = "<suite name=\"suite1\" dir=\"/a/b\" >";
QByteArray suiteEnd = "</suite>";
QByteArray caze1 = "<case name=\"test1\" exe=\"t.sh\" />";
QByteArray caze2 = "<case name=\"test2\" exe=\"t2.sh\" />";
QByteArray cazeStart = "<case name=\"test1\" exe=\"t.sh\" >";
QByteArray cazeEnd = "</case>";
QByteArray cmd1 = "<command name=\"cmd11\" />";
QByteArray cmd2 = "<command name=\"cmd12\" />";
}

namespace QTest
{
template<> inline char* toString(const QFileInfo& fi)
{
    return qstrdup(fi.filePath().toLatin1().constData());
}
}

void QTestRegisterTest::init()
{
    reg = new QTestRegister();
}

void QTestRegisterTest::cleanup()
{
    delete reg;
}

void QTestRegisterTest::compareSuites(QTestSuite* exp, QTestSuite* actual)
{
    KOMPARE(exp->name(), actual->name());
    KOMPARE(exp->path(), actual->path());
    KOMPARE(exp->parent(), actual->parent());
}

void QTestRegisterTest::parseSuiteXml()
{
    QByteArray xml = headerXml + suite1Xml + footerXml;
    registerTests(xml);

    KOMPARE(1, reg->testSuiteCount());
    QTestSuite expected("suite1", QFileInfo("/a/b"), 0);
    compareSuites(&expected, reg->takeSuite(0));
    KOMPARE(0, reg->takeSuite(0)->testCaseCount());
}
void QTestRegisterTest::parseMultiSuitesXml()
{
    QByteArray xml = headerXml + suite1Xml + suite2Xml + footerXml;
    registerTests(xml);

    KOMPARE(2, reg->testSuiteCount());
    // suite1
    QTestSuite exp1("suite1", QFileInfo("/a/b"), 0);
    compareSuites(&exp1, reg->takeSuite(0));
    KOMPARE(0, reg->takeSuite(0)->testCaseCount());
    // suite2
    QTestSuite exp2("suite2", QFileInfo("/c/d"), 0);
    compareSuites(&exp2, reg->takeSuite(1));
    KOMPARE(0, reg->takeSuite(1)->testCaseCount());
}

void QTestRegisterTest::compareCase(QTestCase* expected, QTestCase* actual)
{
    KOMPARE(expected->name(), actual->name());
    KOMPARE(expected->executable(),  actual->executable());
    KOMPARE(expected->parent(), actual->parent());
}

void QTestRegisterTest::parseCaseXml()
{
    QByteArray xml = headerXml + suiteStart + caze1 + suiteEnd + footerXml;
    registerTests(xml);

    KOMPARE(1, reg->takeSuite(0)->testCaseCount());
    QTestCase exp("test1", QFileInfo("t.sh"), reg->takeSuite(0));
    compareCase(&exp, reg->takeSuite(0)->testAt(0));
}

void QTestRegisterTest::parseMultiCaseXml()
{
    QByteArray xml = headerXml + suiteStart + caze1 + caze2 + suiteEnd + footerXml;
    registerTests(xml);

    KOMPARE(2, reg->takeSuite(0)->testCaseCount());
    // caze1
    QTestCase exp1("test1", QFileInfo("t.sh"), reg->takeSuite(0));
    compareCase(&exp1, reg->takeSuite(0)->testAt(0));
    // caze2
    QTestCase exp2("test2", QFileInfo("t2.sh"), reg->takeSuite(0));
    compareCase(&exp2, reg->takeSuite(0)->testAt(1));
}

void QTestRegisterTest::registerTests(QByteArray& xml)
{
    QBuffer buff(&xml);
    reg->addFromXml(&buff);
}

void QTestRegisterTest::parseCmdXml()
{
    KTODO;
}

void QTestRegisterTest::parseMultiCmdXMl()
{
    QByteArray xml = headerXml + suiteStart + cazeStart + cmd1 + cmd2 + cazeEnd + suiteEnd + footerXml;
    registerTests(xml);

    KOMPARE(1, reg->testSuiteCount());
    QTestSuite* suite = reg->takeSuite(0);
    KOMPARE(1, suite->testCaseCount());
    QTestCase* caze = suite->testAt(0);
    KOMPARE(2, caze->testCommandCount());
    // cmd1
    QTestCommand* actual = caze->testAt(0);
    KOMPARE("cmd11", actual->name());
    KOMPARE("/a/b/t.sh cmd11", actual->command());
    actual = caze->testAt(1);
    KOMPARE("cmd12", actual->name());
    KOMPARE("/a/b/t.sh cmd12", actual->command());
}

#include "qtestregistertest.moc"
QTEST_KDEMAIN(QTestRegisterTest, NoGUI);
