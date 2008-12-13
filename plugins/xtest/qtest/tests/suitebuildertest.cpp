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
#include <project/projectmodel.h>
#include <veritas/test.h>
#include <veritas/testexecutableinfo.h>

using QTest::SuiteBuilderTest;
using QTest::SuiteBuilder;
using QTest::CaseBuilder;
using QTest::Case;
using QTest::Command;
using QTest::Suite;
using QTest::ExecutableStub;
using Veritas::Test;
using Veritas::TestExecutableInfo;

namespace QTest {

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

} // namespace QTest

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

namespace
{
void setTestExecutables(SuiteBuilder* builder, const KUrl::List& exes)
{
    QList<TestExecutableInfo> testExeInfo;
    foreach(const KUrl& exe, exes) {
        TestExecutableInfo tinfo;
        tinfo.setWorkingDirectory( exe.upUrl() );
        tinfo.setCommand( exe.fileName(  ) );
        testExeInfo << tinfo;
    }
    builder->setTestExecutables( testExeInfo );
}
}

// command
void SuiteBuilderTest::singleSuiteCaseCommand()
{
    m_exe->m_name = "footest";
    m_exe->m_fetchFunctions = QStringList() << "foocommand()";
    m_builder->m_exes = QList<ExecutableStub*>() << m_exe; // inject
    setTestExecutables( m_builder, QList<KUrl>() << KUrl("/path/to/foosuite/footest.shell"));

    m_builder->start();
    Veritas::Test* root = m_builder->root();

    KVERIFY(root);
    KOMPARE(1, root->childCount());

    Suite* suite = fetchSuite(root, 0);
    verifySuite(suite, "foosuite", 1);
    Case* caze = suite->child(0);
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
    setTestExecutables(m_builder, QList<KUrl>() << KUrl("/path/to/foosuite/footest.shell")
                                         << KUrl("/path/to/barsuite/bartest.shell"));

    /** 
     * - foosuite
     *   - footest
     *     - foocommand
     *     - foocommand2
     * - barsuite
     *   - bartest
     *     - barcommand 
     **/

    m_builder->start();
    Veritas::Test* root = m_builder->root();
    KVERIFY(root);
    KOMPARE(2, root->childCount());

    Suite* foosuite = fetchSuite(root, 1); // suites are ordered alphabetically, so foosuite comes after barsuite
    verifySuite(foosuite, "foosuite", 1);
    Case* caze = foosuite->child(0);
    verifyCaze(caze, "footest", 2);
    verifyCommand(0, caze, "foocommand");
    verifyCommand(1, caze, "foocommand2");

    Suite* barSuite = fetchSuite(root, 0); // barsuite comes first, index 0
    verifySuite(barSuite, "barsuite", 1);
    Case* barC = barSuite->child(0);
    verifyCaze(barC, "bartest", 1);
    verifyCommand(0, barC, "barcommand");
}

// command
void SuiteBuilderTest::identicalSuiteNames()
{
    // two equally named suites but in a different parent directory
    // this should construct 2 different suites

    m_exe->m_name = "footest";
    m_exe->m_fetchFunctions = QStringList() << "foocommand()" << "foocommand2()";

    ExecutableStub* barExe = new ExecutableStub;
    barExe->m_name = "bartest";
    barExe->m_fetchFunctions = QStringList() << "barcommand()";
    m_builder->m_exes = QList<ExecutableStub*>() << m_exe << barExe; // inject
    setTestExecutables( m_builder, QList<KUrl>() << KUrl("/path/to/foo/suite/footest.shell")
                                                << KUrl("/path/to/bar/suite/bartest.shell"));

    /** 
     * - foo-suite # dir-name + one up
     *   - footest
     *     - foocommand
     *     - foocommand2
     * - bar-suite # dir-name + one up
     *   - bartest
     *     - barcommand 
     **/

    m_builder->start();
    Veritas::Test* root = m_builder->root();
    KVERIFY(root);
    KOMPARE(2, root->childCount());

    Suite* foosuite = fetchSuite(root, 1);
    verifySuite(foosuite, "foo-suite", 1);
    Case* caze = foosuite->child(0);
    verifyCaze(caze, "footest", 2);
    verifyCommand(0, caze, "foocommand");
    verifyCommand(1, caze, "foocommand2");

    Suite* barSuite = fetchSuite(root, 0);
    verifySuite(barSuite, "bar-suite", 1);
    Case* barC = barSuite->child(0);
    verifyCaze(barC, "bartest", 1);
    verifyCommand(0, barC, "barcommand");
}

void SuiteBuilderTest::keepPreviousCase()
{
    // a single testcase that did not change (timestamp equal).
    // expected is that the executable is not rerun

    Test* prevRoot = new Test("root", 0);
    Suite* prevSuite = new Suite("foosuite", QFileInfo("/path/to/foosuite/"), prevRoot);
    prevRoot->addChild(prevSuite);
    Case* prevCase = new Case("footest", QFileInfo("/path/to/foosuite/footest.shell"), prevSuite);
    prevSuite->addChild(prevCase);
    m_exe->m_wasModified = false;
    prevCase->setExecutable(m_exe);
    m_exe->m_name = "footest";
    m_exe->setLocation( KUrl("/path/to/foosuite/") );
    m_exe->m_fetchFunctions = QStringList() << "foocommand()";

    m_builder->m_exes = QList<ExecutableStub*>() << m_exe; // inject
    m_builder->setPreviousRoot(prevRoot);
    setTestExecutables( m_builder, QList<KUrl>() << KUrl("/path/to/foosuite/footest.shell"));

    m_builder->start();
    Veritas::Test* root = m_builder->root();

    KVERIFY(root);
    KOMPARE(1, root->childCount());

    Suite* suite = fetchSuite(root, 0);
    verifySuite(suite, "foosuite", 1);
    Case* caze = suite->child(0);
    verifyCaze(caze, "footest", 1);
    KVERIFY(!m_exe->fetchFunctionsCalled); // Should not have been called, since we specifically set 
}

// void SuiteBuilderTest::partialReload()
// {
//     m_builder->start();
//     m_builder->m_exes = QList<ExecutableStub*>() << m_exe; // inject
//     setTestExecutables( m_builder, QList<KUrl>() << KUrl("/path/to/foosuite/footest.shell"));
// }

/////////////////////// helpers //////////////////////////////////////////////

Suite* SuiteBuilderTest::fetchSuite(Veritas::Test* root, int nrofSuite)
{
    return dynamic_cast<Suite*>(root->child(nrofSuite));
}

void SuiteBuilderTest::verifySuite(Suite* suite, const QString name, int childCount)
{
    KVERIFY(suite);
    KOMPARE(name, suite->name());
    KOMPARE(childCount, suite->childCount());
}

void SuiteBuilderTest::verifyCommand(int nrofChild, Case* parent, const QString name)
{
    Command* cmd = parent->child(nrofChild);
    KVERIFY(cmd);
    KOMPARE(name, cmd->name());
    KOMPARE(parent, cmd->parent());
}

void SuiteBuilderTest::verifyCaze(Case* caze, const QString name, int childCount)
{
    KVERIFY(caze);
    KOMPARE(name, caze->name());
    KOMPARE(childCount, caze->childCount());
}

QTEST_MAIN( SuiteBuilderTest )
#include "suitebuildertest.moc"
