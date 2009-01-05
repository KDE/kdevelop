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
#include "../xmlregister.h"
#include "../qtestsettings.h"
#include "../qtestmodelitems.h"
#include "../qtestmodelitems.h"
#include "../qtestmodelitems.h"

#include <kasserts.h>

#include <QByteArray>
#include <QFileInfo>
#include <QBuffer>

using QTest::XmlRegister;
using QTest::Suite;
using QTest::Case;
using QTest::Command;
using QTest::ISettings;
using QTest::Test::QTestRegisterTest;

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
    template<> inline char* toString(const Veritas::Test& tb)
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

class FakeSettings : public ISettings
{
    virtual bool printAsserts() const { return false; }
    virtual bool printSignals() const { return false; }
    virtual QString makeBinary() const { return QString(); }
    virtual KUrl cmakeProjectLibraryPath() const { return KUrl(); }
};

}

void QTestRegisterTest::init()
{
    reg = new XmlRegister();
    reg->setSettings( new FakeSettings() );
}

void QTestRegisterTest::cleanup()
{
    if (reg->root()) delete reg->root();
    delete reg;
}

void QTestRegisterTest::compareSuites(Suite* exp, Suite* actual)
{
    KOMPARE(exp->name(), actual->name());
    KOMPARE(exp->path(), actual->path());
    KOMPARE(exp->parent(), actual->parent());
}

void QTestRegisterTest::parseSuiteXml()
{
    QByteArray xml = headerXml + suite1Xml + footerXml;
    Veritas::Test* root = registerTests(xml);

    KOMPARE(1, root->childCount());
    Suite* expected = new Suite("suite1", QFileInfo("/a/b"), root);
    Suite* suite = qobject_cast<Suite*>(root->child(0));
    compareSuites(expected, suite);
    KOMPARE(0, suite->childCount());
}
void QTestRegisterTest::parseMultiSuitesXml()
{
    QByteArray xml = headerXml + suite1Xml + suite2Xml + footerXml;
    Veritas::Test* root = registerTests(xml);

    KOMPARE(2, root->childCount());
    // suite1
    Suite exp1("suite1", QFileInfo("/a/b"), root);
    Suite* suite = qobject_cast<Suite*>(root->child(0));
    compareSuites(&exp1, suite);
    KOMPARE(0, suite->childCount());
    // suite2
    Suite exp2("suite2", QFileInfo("/c/d"), root);
    suite = qobject_cast<Suite*>(root->child(1));
    compareSuites(&exp2, suite);
    KOMPARE(0, suite->childCount());
}

void QTestRegisterTest::compareCase(Case* expected, Case* actual)
{
    KOMPARE(expected->name(), actual->name());
    KOMPARE(expected->parent(), actual->parent());
}

void QTestRegisterTest::parseCaseXml()
{
    QByteArray xml = headerXml + suiteStart + caze1 + suiteEnd + footerXml;
    Veritas::Test* root = registerTests(xml);

    Suite* suite = qobject_cast<Suite*>(root->child(0));
    KOMPARE(1, suite->childCount());
    Case exp("test1", QFileInfo("t.sh"), suite);
    compareCase(&exp, suite->child(0));
}

void QTestRegisterTest::parseMultiCaseXml()
{
    QByteArray xml = headerXml + suiteStart + caze1 + caze2 + suiteEnd + footerXml;
    Veritas::Test* root = registerTests(xml);

    Suite* suite = qobject_cast<Suite*>(root->child(0));
    KOMPARE(2, suite->childCount());
    // caze1
    Case exp1("test1", QFileInfo("t.sh"), suite);
    compareCase(&exp1, suite->child(0));
    // caze2
    Case exp2("test2", QFileInfo("t2.sh"), suite);
    compareCase(&exp2, suite->child(1));
}

Veritas::Test* QTestRegisterTest::registerTests(QByteArray& xml)
{
    QBuffer buff(&xml);
    reg->setSource(&buff);
    reg->reload(0);
    return reg->root();
}

void QTestRegisterTest::parseCmdXml()
{
    // TODO parse a single test command
}

void QTestRegisterTest::parseMultiCmdXMl()
{
    QByteArray xml = headerXml + suiteStart + cazeStart + cmd1 + cmd2 + cazeEnd + suiteEnd + footerXml;
    Veritas::Test* root = registerTests(xml);

    KOMPARE(1, root->childCount());
    Suite* suite = qobject_cast<Suite*>(root->child(0));
    KOMPARE(1, suite->childCount());
    Case* caze = suite->child(0);
    KOMPARE(2, caze->childCount());
    // cmd1
    Command* actual = caze->child(0);
    KOMPARE("cmd11", actual->name());
    KOMPARE("/a/b/t.sh cmd11", actual->command());
    actual = caze->child(1);
    KOMPARE("cmd12", actual->name());
    KOMPARE("/a/b/t.sh cmd12", actual->command());
}

#include "qtestregistertest.moc"
QTEST_KDEMAIN(QTestRegisterTest, NoGUI)
