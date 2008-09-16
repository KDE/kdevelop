/*
* KDevelop xUnit integration
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

#include "suitebuildertest.h"
#include "kdevtest.h"
#include "executablestub.h"
#include "../casebuilder.h"
#include "../suitebuilder.h"
#include "../qtestsuite.h"
#include "../qtestcase.h"
#include "../qtestcommand.h"
#include "documentaccessstub.h"
#include <project/projectmodel.h>
#include <veritas/test.h>

using QTest::Test::SuiteBuilderTest;
using QTest::SuiteBuilder;
using QTest::CaseBuilder;
using QTest::QTestCase;
using QTest::QTestCommand;
using QTest::QTestSuite;
using QTest::Test::ExecutableStub;
using KDevelop::ProjectTestTargetItem;
using Veritas::Test;

namespace QTest { namespace Test
{
class CustomSuiteBuilder : public SuiteBuilder
{
public:
    CustomSuiteBuilder() : m_count(0) {}
    virtual ~CustomSuiteBuilder() {}
    QList<ExecutableStub*> m_exes;

protected:
    virtual CaseBuilder* createCaseBuilder(const KUrl& url) const {
        CaseBuilder* cb = new CaseBuilder;
        Q_ASSERT(m_count < m_exes.count());
        cb->setExecutable(m_exes[m_count]);
        m_count++;
        return cb;
    }
private:
    mutable int m_count;
};
}}

void SuiteBuilderTest::init()
{
    m_builder = new CustomSuiteBuilder;
    m_exe = new ExecutableStub;
}

void SuiteBuilderTest::cleanup()
{
    if (m_builder->root()) delete m_builder->root();
    if (m_builder) delete m_builder;
}

// command
void SuiteBuilderTest::singleSuiteCaseCommand()
{
    m_exe->m_name = "footest";
    m_exe->m_fetchFunctions = QStringList() << "foocommand()";
    m_builder->m_exes = QList<ExecutableStub*>() << m_exe; // inject
    m_builder->setTestExecutables(QList<KUrl>() << KUrl("/path/to/foosuite/footest.shell"));

    m_builder->start();
    Veritas::Test* root = m_builder->root();

    KVERIFY(root);
    KOMPARE(1, root->childCount());

    QTestSuite* suite = fetchSuite(root, 0);
    verifySuite(suite, "foosuite", 1);
    QTestCase* caze = suite->child(0);
    verifyCaze(caze, "footest", 1);
    verifyCommand(0, caze, "foocommand");
}

// command
void SuiteBuilderTest::multiSuitesCasesCommands()
{
    m_exe->m_name = "footest";
    m_exe->m_fetchFunctions = QStringList() << "foocommand()" << "foocommand2()";

    ExecutableStub* barExe = new ExecutableStub;
    barExe->m_name = "bartest";
    barExe->m_fetchFunctions = QStringList() << "barcommand()";
    m_builder->m_exes = QList<ExecutableStub*>() << m_exe << barExe; // inject
    m_builder->setTestExecutables(QList<KUrl>() << KUrl("/path/to/foosuite/footest.shell")
                                         << KUrl("/path/to/barsuite/bartest.shell"));

    m_builder->start();
    Veritas::Test* root = m_builder->root();
    KVERIFY(root);
    KOMPARE(2, root->childCount());

    QTestSuite* foosuite = fetchSuite(root, 0);
    verifySuite(foosuite, "foosuite", 1);
    QTestCase* caze = foosuite->child(0);
    verifyCaze(caze, "footest", 2);
    verifyCommand(0, caze, "foocommand");
    verifyCommand(1, caze, "foocommand2");

    QTestSuite* barSuite = fetchSuite(root, 1);
    verifySuite(barSuite, "barsuite", 1);
    QTestCase* barC = barSuite->child(0);
    verifyCaze(barC, "bartest", 1);
    verifyCommand(0, barC, "barcommand");
}

// command
void SuiteBuilderTest::identicalSuiteNames()
{
    // two equally named suites but in a different parent directory
    // this should construct 2 different suites [but currently doesnt]

    m_exe->m_name = "footest";
    m_exe->m_fetchFunctions = QStringList() << "foocommand()" << "foocommand2()";

    ExecutableStub* barExe = new ExecutableStub;
    barExe->m_name = "bartest";
    barExe->m_fetchFunctions = QStringList() << "barcommand()";
    m_builder->m_exes = QList<ExecutableStub*>() << m_exe << barExe; // inject
    m_builder->setTestExecutables(QList<KUrl>() << KUrl("/path/to/foo/suite/footest.shell")
                                                << KUrl("/path/to/bar/suite/bartest.shell"));

    m_builder->start();
    Veritas::Test* root = m_builder->root();
    KVERIFY(root);
    KOMPARE(2, root->childCount());

    QTestSuite* foosuite = fetchSuite(root, 0);
    verifySuite(foosuite, "foo-suite", 1);
    QTestCase* caze = foosuite->child(0);
    verifyCaze(caze, "footest", 2);
    verifyCommand(0, caze, "foocommand");
    verifyCommand(1, caze, "foocommand2");

    QTestSuite* barSuite = fetchSuite(root, 1);
    verifySuite(barSuite, "bar-suite", 1);
    QTestCase* barC = barSuite->child(0);
    verifyCaze(barC, "bartest", 1);
    verifyCommand(0, barC, "barcommand");
}

/////////////////////// helpers //////////////////////////////////////////////

QTestSuite* SuiteBuilderTest::fetchSuite(Veritas::Test* root, int nrofSuite)
{
    return dynamic_cast<QTestSuite*>(root->child(nrofSuite));
}

void SuiteBuilderTest::verifySuite(QTestSuite* suite, const QString name, int childCount)
{
    KVERIFY(suite);
    KOMPARE(name, suite->name());
    KOMPARE(childCount, suite->childCount());
}

void SuiteBuilderTest::verifyCommand(int nrofChild, QTestCase* parent, const QString name)
{
    QTestCommand* cmd = parent->child(nrofChild);
    KVERIFY(cmd);
    KOMPARE(name, cmd->name());
    KOMPARE(parent, cmd->parent());
}

void SuiteBuilderTest::verifyCaze(QTestCase* caze, const QString name, int childCount)
{
    KVERIFY(caze);
    KOMPARE(name, caze->name());
    KOMPARE(childCount, caze->childCount());
}

QTEST_MAIN( SuiteBuilderTest )
#include "suitebuildertest.moc"
