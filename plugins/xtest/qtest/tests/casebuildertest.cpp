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

// Aborts a testcase if a custom assertion has set the m_stop flag.
#define CHECK_STOP \
if (m_stop) { \
    kDebug() << "Aborted TestCommand due to fatal assertion failure "\
                "[m_stop == true]."; \
    return;\
}

/////////////////////// fixture //////////////////////////////////////////////

void CaseBuilderTest::init()
{
    m_builder = new CaseBuilder;
    m_caseExeStub = new ExecutableStub;
    m_stop = false;
    m_caze = 0;
}

void CaseBuilderTest::cleanup()
{
    if (m_builder) delete m_builder;
    if (m_caze) delete m_caze;
}

/////////////////////// commands //////////////////////////////////////////////

// command
void CaseBuilderTest::construct()
{
    m_builder->setExecutable(m_caseExeStub);
    m_caze = m_builder->construct();
}

// command
void CaseBuilderTest::noCommands()
{
    initTestExeStub("emptyCase", QStringList());
    m_caze = m_builder->construct();
    QVERIFY(!m_caze);
}

// command
void CaseBuilderTest::singleCommand()
{
    initTestExeStub("footest", QStringList() << "testFoo()");
    m_caze = m_builder->construct();

    assertNotNull(m_caze); CHECK_STOP
    assertNamed("footest", m_caze);
    assertNrofChildren(1, m_caze);
    assertChildCommand(0, "testFoo", m_caze);
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
    m_caze = m_builder->construct();

    assertNotNull(m_caze); CHECK_STOP
    assertNamed("bartest", m_caze);
    assertNrofChildren(2, m_caze);

    assertChildCommand(0, "testFoo", m_caze); CHECK_STOP
    assertChildCommand(1, "testBar", m_caze); CHECK_STOP
}

// command
void CaseBuilderTest::garbageInFront()
{
    initTestExeStub("footest", QStringList() << "garbage" << "testFoo()");
    m_caze = m_builder->construct();

    assertNotNull(m_caze); CHECK_STOP
    assertNamed("footest", m_caze);
    assertNrofChildren(1, m_caze);
    assertChildCommand(0, "testFoo", m_caze); CHECK_STOP
}

// command
void CaseBuilderTest::angleBracketGarbage()
{
    initTestExeStub("footest", QStringList() << "<garbage>" << "more_" << "testFoo()");
    m_caze = m_builder->construct();

    assertNotNull(m_caze); CHECK_STOP
    assertNrofChildren(1, m_caze);
    assertChildCommand(0, "testFoo", m_caze); CHECK_STOP
}

void CaseBuilderTest::garbageInFunctionsOutput()
{
    initTestExeStub("footest", QStringList() << "testFoo()" << "garbage" << "testBar()");
    m_caze = m_builder->construct();

    assertNotNull(m_caze); CHECK_STOP
    assertNrofChildren(2, m_caze);
    assertChildCommand(0, "testFoo", m_caze); CHECK_STOP
    assertChildCommand(1, "testBar", m_caze); CHECK_STOP
}

QStringList someCommand()
{
    return QStringList() << "test()";
}

void CaseBuilderTest::removeDirPrefix()
{
    initTestExeStub("dir-footest", someCommand());
    m_builder->setSuiteName("dir");
    m_caze = m_builder->construct();

    assertNotNull(m_caze); CHECK_STOP
    assertNamed("footest", m_caze);
}

// command
void CaseBuilderTest::keepSecondaryPrefixes()
{
    initTestExeStub("dir-more-footest", someCommand());
    m_builder->setSuiteName("dir");
    m_caze = m_builder->construct();

    assertNotNull(m_caze); CHECK_STOP
    assertNamed("more-footest", m_caze);
}

// command
void CaseBuilderTest::dontRemoveNonMatchinPrefix()
{
    initTestExeStub("dir-footest", someCommand());
    m_builder->setSuiteName("bar");
    m_caze = m_builder->construct();

    assertNotNull(m_caze); CHECK_STOP
    assertNamed("dir-footest", m_caze);
}

// command
void CaseBuilderTest::onlyGarbageOutput()
{
    initTestExeStub("garbageCase", QStringList() << "_something_" << "more");
    m_caze = m_builder->construct();
    QVERIFY(!m_caze);
}

/////////////////////// helpers //////////////////////////////////////////////

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
    KOMPARE_MSG(parent, child->parent(), failMsg);
}

// custom assertion
void CaseBuilderTest::assertNrofChildren(int count, QTestCase* caze)
{
    int actual = caze->childCount();
    QString failMsg = QString(
        "Builder constructed a wrong number of testcommand children for %1.\n"
        "Expected %2 Got %3").arg(caze->name()).arg(count).arg(actual);
    KOMPARE_MSG(count, actual, failMsg);
}

// custom assertion
void CaseBuilderTest::assertNamed(QString name, QTestCase* caze)
{
    QString actual = caze->name();
    QString failMsg = QString(
        "Constructed QTestCase has a wrong name.\n"
        "Expected %1 but Got %2").arg(name).arg(actual);
    KOMPARE_MSG(name, actual, failMsg);
}

// custom assertion
void CaseBuilderTest::assertNamed(QString name, QTestCommand* cmd)
{
    QString actual = cmd->name();
    QString failMsg = QString(
        "Constructed QTestCommand has a wrong name.\n"
        "Expected %1 but Got %2").arg(name).arg(actual);
    KOMPARE_MSG(name, actual, failMsg);
}

QTEST_MAIN( CaseBuilderTest )
#include "casebuildertest.moc"
