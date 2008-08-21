#include "casebuildertest.h"
#include "kdevtest.h"
#include "../qtestcase.h"
#include "../qtestcommand.h"
#include "../executable.h"
#include "../casebuilder.h"
#include <KDebug>
#include "executablestub.h"
#include <veritas/test.h>

using QTest::Test::CaseBuilderTest;
using QTest::Test::ExecutableStub;
using QTest::QTestCase;
using QTest::QTestCommand;
using QTest::Executable;
using Veritas::Test;

void CaseBuilderTest::init()
{
    m_builder = new CaseBuilder;
    m_caseExeStub = new ExecutableStub;
    m_stop = false;
}

void CaseBuilderTest::cleanup()
{
    if (m_builder) delete m_builder;
}

// command
void CaseBuilderTest::construct()
{
    m_builder->setExecutable(m_caseExeStub);
    QTestCase* caze = m_builder->construct();
}

// helper
void CaseBuilderTest::initTestExeStub(QString name, QStringList functionsOutput)
{
    m_caseExeStub->m_name = name;
    m_caseExeStub->m_fetchFunctions = functionsOutput;
    m_builder->setExecutable(m_caseExeStub);
}

// custom assertion
void CaseBuilderTest::assertNotNull(QTestCase* caze)
{
    m_stop = true; // make this a fatal assertion.
    KVERIFY_MSG(caze, "Builder should have allocated a QTestCase, but returned NULL.");
    m_stop = false; // if we get here, the assertion succeeded.
}

// custom assertion
void CaseBuilderTest::assertNotNull(QTestCommand* cmd)
{
    m_stop = true; // make this a fatal assertion.
    KVERIFY_MSG(cmd, "Builder should have allocated a QTestCommand, but returned NULL.");
    m_stop = false; // if we get here, the assertion succeeded.
}

// custom assertion
void CaseBuilderTest::assertParent(Veritas::Test* child, Veritas::Test* parent)
{
    QString failMsg = QString(
        "Wrong parent-test %1 for child %2").arg(parent->name()).arg(child->name());
    KOMPARE_MSG_(parent, child->parent(), failMsg);
}

// custom assertion
void CaseBuilderTest::assertNrofChildren(int count, QTestCase* caze)
{
    int actual = caze->childCount();
    QString failMsg = QString(
        "Builder constructed a wrong number of testcommand children for %1.\n"
        "Expected %2 Got %3").arg(caze->name()).arg(count).arg(actual);
    KOMPARE_MSG_(count, actual, failMsg);
}

// custom assertion
void CaseBuilderTest::assertNamed(QString name, QTestCase* caze)
{
    QString actual = caze->name();
    QString failMsg = QString(
        "Constructed QTestCase has a wrong name.\n"
        "Expected %1 but Got %2").arg(name).arg(actual);
    KOMPARE_MSG_(name, actual, failMsg);
}

// custom assertion
void CaseBuilderTest::assertNamed(QString name, QTestCommand* cmd)
{
    QString actual = cmd->name();
    QString failMsg = QString(
        "Constructed QTestCommand has a wrong name.\n"
        "Expected %1 but Got %2").arg(name).arg(actual);
    KOMPARE_MSG_(name, actual, failMsg);
}


// Aborts a testcase if a custom assertion has set the m_stop flag.
#define CHECK_STOP \
if (m_stop) { \
    kDebug() << "Aborted TestCommand due to fatal assertion failure "\
                "[m_stop == true]."; \
    return;\
}

// command
void CaseBuilderTest::noCommands()
{
    initTestExeStub("emptyCase", QStringList());
    QTestCase* caze = m_builder->construct();

    assertNotNull(caze); CHECK_STOP
    assertNamed("emptyCase", caze);
    assertNrofChildren(0, caze);
}

// command
void CaseBuilderTest::singleCommand()
{
    initTestExeStub("footest", QStringList() << "testFoo()");
    QTestCase* caze = m_builder->construct();

    assertNotNull(caze); CHECK_STOP
    assertNamed("footest", caze);
    assertNrofChildren(1, caze);
    assertChildCommand(0, "testFoo", caze);
}

void CaseBuilderTest::assertChildCommand(int nrof, QString name, QTestCase* parent)
{
    QTestCommand* cmd = parent->child(nrof);
    assertNotNull(cmd); CHECK_STOP
    assertNamed(name, cmd);
    assertParent(cmd, parent);
}

// command
void CaseBuilderTest::multipleCommands()
{
    initTestExeStub("bartest", QStringList() << "testFoo()" << "testBar()");
    QTestCase* caze = m_builder->construct();

    assertNotNull(caze); CHECK_STOP
    assertNamed("bartest", caze);
    assertNrofChildren(2, caze);

    assertChildCommand(0, "testFoo", caze); CHECK_STOP
    assertChildCommand(1, "testBar", caze); CHECK_STOP
}

// command
void CaseBuilderTest::garbageInFront()
{
    initTestExeStub("footest", QStringList() << "garbage" << "testFoo()");
    QTestCase* caze = m_builder->construct();

    assertNotNull(caze); CHECK_STOP
    assertNamed("footest", caze);
    assertNrofChildren(1, caze);
    assertChildCommand(0, "testFoo", caze); CHECK_STOP
}

// command
void CaseBuilderTest::angleBracketGarbage()
{
    initTestExeStub("footest", QStringList() << "<garbage>" << "more_" << "testFoo()");
    QTestCase* caze = m_builder->construct();

    assertNotNull(caze); CHECK_STOP
    assertNrofChildren(1, caze);
    assertChildCommand(0, "testFoo", caze); CHECK_STOP
}

void CaseBuilderTest::garbageInFunctionsOutput()
{
    initTestExeStub("footest", QStringList() << "testFoo()" << "garbage" << "testBar()");
    QTestCase* caze = m_builder->construct();

    assertNotNull(caze); CHECK_STOP
    assertNrofChildren(2, caze);
    assertChildCommand(0, "testFoo", caze); CHECK_STOP
    assertChildCommand(1, "testBar", caze); CHECK_STOP
}

void CaseBuilderTest::removeDirPrefix()
{
    initTestExeStub("dir-footest", QStringList());
    m_builder->setSuiteName("dir");
    QTestCase* caze = m_builder->construct();

    assertNotNull(caze); CHECK_STOP
    assertNamed("footest", caze);
}

// command
void CaseBuilderTest::keepSecondaryPrefixes()
{
    initTestExeStub("dir-more-footest", QStringList());
    m_builder->setSuiteName("dir");
    QTestCase* caze = m_builder->construct();

    assertNotNull(caze); CHECK_STOP
    assertNamed("more-footest", caze);
}

QTEST_MAIN( CaseBuilderTest )
#include "casebuildertest.moc"
