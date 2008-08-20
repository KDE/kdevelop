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
}

void SuiteBuilderTest::cleanup()
{
}

void SuiteBuilderTest::construct()
{
    SuiteBuilder* sb = new SuiteBuilder;
    QList<KUrl> shellFiles;
    sb->setTestExecutables(shellFiles);
}

// TODO clean this mess. move common verification sequences to custom
// assertion helpers. Extract similar construction stuff as well

void SuiteBuilderTest::singleSuiteCaseCommand()
{
    CustomSuiteBuilder* sb = new CustomSuiteBuilder;
    ExecutableStub* foo = new ExecutableStub;
    foo->m_name = "footest";
    foo->m_fetchFunctions = QStringList() << "foocommand()";
    sb->m_exes = QList<ExecutableStub*>() << foo; // inject
    sb->setTestExecutables(QList<KUrl>() << KUrl("/path/to/foosuite/footest.shell"));

    sb->start();
    Veritas::Test* root = sb->root();

    KVERIFY(root);
    KOMPARE_(1, root->childCount());
    QTestSuite* suite = dynamic_cast<QTestSuite*>(root->child(0));
    KVERIFY(suite);
    KOMPARE_(root, suite->parent());
    KOMPARE_("foosuite", suite->name());
    KOMPARE_(1, suite->childCount());
    QTestCase* caze = suite->child(0);
    KVERIFY(caze);
    KOMPARE("footest", caze->name());
    KOMPARE_(1, caze->childCount());
    QTestCommand* cmd = caze->child(0);
    KVERIFY(cmd);
    KOMPARE("foocommand", cmd->name());
}

void SuiteBuilderTest::removeDirPrefix()
{
    CustomSuiteBuilder* sb = new CustomSuiteBuilder;
    ExecutableStub* foo = new ExecutableStub;
    foo->m_name= "dir-footest";
    foo->m_fetchFunctions = QStringList();
    sb->m_exes = QList<ExecutableStub*>() << foo;
    sb->setTestExecutables(QList<KUrl>() << KUrl("/path/to/dir/dir-footest.shell"));

    sb->start();
    Veritas::Test* root = sb->root();

    KVERIFY(root);
    KOMPARE_(1, root->childCount());

    QTestSuite* foosuite = dynamic_cast<QTestSuite*>(root->child(0));
    KVERIFY(foosuite);
    KOMPARE_(root, foosuite->parent());
    KOMPARE_("dir", foosuite->name());
    KOMPARE_(1, foosuite->childCount());

    QTestCase* caze = foosuite->child(0);
    KVERIFY(caze);
    KOMPARE("footest", caze->name());
    KOMPARE_(0, caze->childCount());
}

void SuiteBuilderTest::multiSuitesCasesCommands()
{
    CustomSuiteBuilder* sb = new CustomSuiteBuilder;
    ExecutableStub* foo = new ExecutableStub;
    foo->m_name = "footest";
    foo->m_fetchFunctions = QStringList() << "foocommand()" << "foocommand2()";
    ExecutableStub* bar = new ExecutableStub;
    bar->m_name = "bartest";
    bar->m_fetchFunctions = QStringList() << "barcommand()";
    sb->m_exes = QList<ExecutableStub*>() << foo << bar; // inject
    sb->setTestExecutables(QList<KUrl>() << KUrl("/path/to/foosuite/footest.shell")
                                         << KUrl("/path/to/barsuite/bartest.shell"));

    sb->start();
    Veritas::Test* root = sb->root();

    KVERIFY(root);
    KOMPARE_(2, root->childCount());

    QTestSuite* foosuite = dynamic_cast<QTestSuite*>(root->child(0));
    KVERIFY(foosuite);
    KOMPARE_(root, foosuite->parent());
    KOMPARE_("foosuite", foosuite->name());
    KOMPARE_(1, foosuite->childCount());

    QTestCase* caze = foosuite->child(0);
    KVERIFY(caze);
    KOMPARE("footest", caze->name());
    KOMPARE_(2, caze->childCount());

    QTestCommand* cmd = caze->child(0);
    KVERIFY(cmd);
    KOMPARE("foocommand", cmd->name());

    QTestCommand* cmd2 = caze->child(1);
    KVERIFY(cmd2);
    KOMPARE("foocommand2", cmd2->name());

    QTestSuite* barSuite = dynamic_cast<QTestSuite*>(root->child(1));
    KVERIFY(barSuite);
    KOMPARE_(root, barSuite->parent());
    KOMPARE_("barsuite", barSuite->name());
    KOMPARE_(1, barSuite->childCount());

    QTestCase* barC = barSuite->child(0);
    KVERIFY(barC);
    KOMPARE("bartest", barC->name());
    KOMPARE(1, barC->childCount());

    QTestCommand* cmd0 = barC->child(0);
    KVERIFY(cmd0);
    KOMPARE_("barcommand", cmd0->name());
}

void SuiteBuilderTest::identicalSuiteNames()
{
    // two equally named suites but in a different parent directory
    // this should construct 2 different suites [but currently doesnt]
    QFAIL("Not Implemented, and not supported [yet] either");
}

QTEST_MAIN( SuiteBuilderTest )
#include "suitebuildertest.moc"
