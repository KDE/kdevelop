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

using Veritas::Test;
using QTest::QTestRegister;
using QTest::QTestSuite;
using QTest::QTestCase;
using QTest::QTestCommand;
using QTest::ut::QTestRegisterTest;

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

namespace QTest
{
    template<> inline char* toString(const Test& tb)
    {
        return qstrdup(tb.name().toLatin1().constData());
    }
}

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

    KOMPARE(1, reg->rootItem()->childCount());
    QTestSuite* expected = new QTestSuite("suite1", QFileInfo("/a/b"), reg->rootItem());
    QTestSuite* suite = qobject_cast<QTestSuite*>(reg->rootItem()->child(0));
    compareSuites(expected, suite);
    KOMPARE(0, suite->childCount());
}
void QTestRegisterTest::parseMultiSuitesXml()
{
    QByteArray xml = headerXml + suite1Xml + suite2Xml + footerXml;
    registerTests(xml);

    KOMPARE(2, reg->rootItem()->childCount());
    // suite1
    QTestSuite exp1("suite1", QFileInfo("/a/b"), reg->rootItem());
    QTestSuite* suite = qobject_cast<QTestSuite*>(reg->rootItem()->child(0));
    compareSuites(&exp1, suite);
    KOMPARE(0, suite->childCount());
    // suite2
    QTestSuite exp2("suite2", QFileInfo("/c/d"), reg->rootItem());
    suite = qobject_cast<QTestSuite*>(reg->rootItem()->child(1));
    compareSuites(&exp2, suite);
    KOMPARE(0, suite->childCount());
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

    QTestSuite* suite = qobject_cast<QTestSuite*>(reg->rootItem()->child(0));
    KOMPARE(1, suite->childCount());
    QTestCase exp("test1", QFileInfo("t.sh"), suite);
    compareCase(&exp, suite->child(0));
}

void QTestRegisterTest::parseMultiCaseXml()
{
    QByteArray xml = headerXml + suiteStart + caze1 + caze2 + suiteEnd + footerXml;
    registerTests(xml);

    QTestSuite* suite = qobject_cast<QTestSuite*>(reg->rootItem()->child(0));
    KOMPARE(2, suite->childCount());
    // caze1
    QTestCase exp1("test1", QFileInfo("t.sh"), suite);
    compareCase(&exp1, suite->child(0));
    // caze2
    QTestCase exp2("test2", QFileInfo("t2.sh"), suite);
    compareCase(&exp2, suite->child(1));
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

    KOMPARE(1, reg->rootItem()->childCount());
    QTestSuite* suite = qobject_cast<QTestSuite*>(reg->rootItem()->child(0));
    KOMPARE(1, suite->childCount());
    QTestCase* caze = suite->child(0);
    KOMPARE(2, caze->childCount());
    // cmd1
    QTestCommand* actual = caze->child(0);
    KOMPARE("cmd11", actual->name());
    KOMPARE("/a/b/t.sh cmd11", actual->command());
    actual = caze->child(1);
    KOMPARE("cmd12", actual->name());
    KOMPARE("/a/b/t.sh cmd12", actual->command());
}

#include "qtestregistertest.moc"
QTEST_KDEMAIN(QTestRegisterTest, NoGUI)
