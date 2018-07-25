/*
 * Unit tests for LLDB debugger plugin
   Copyright 2009 Niko Sams <niko.sams@gmail.com>
   Copyright 2013 Vlas Puhov <vlas.puhov@mail.ru>
 * Copyright 2016  Aetf <aetf@unlimitedcodeworks.xyz>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "test_lldb.h"

#include "controllers/framestackmodel.h"
#include "debugsession.h"
#include "tests/debuggers-tests-config.h"
#include "tests/testhelper.h"

#include <execute/iexecuteplugin.h>
#include <debugger/breakpoint/breakpointmodel.h>
#include <debugger/variable/variablecollection.h>
#include <interfaces/idebugcontroller.h>
#include <interfaces/ilaunchconfiguration.h>
#include <interfaces/iplugincontroller.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <util/environmentprofilelist.h>

#include <KConfigGroup>
#include <KIO/Global>
#include <KSharedConfig>

#include <QFileInfo>
#include <QSignalSpy>
#include <QString>
#include <QStringList>
#include <QTest>
#include <QTemporaryFile>
#include <QUrl>
#include <QDir>

#define WAIT_FOR_STATE(session, state) \
    do { if (!KDevMI::waitForState((session), (state), __FILE__, __LINE__)) return; } while (0)

#define WAIT_FOR_STATE_AND_IDLE(session, state) \
    do { if (!KDevMI::waitForState((session), (state), __FILE__, __LINE__, true)) return; } while (0)

#define WAIT_FOR_A_WHILE(session, ms) \
    do { if (!KDevMI::waitForAWhile((session), (ms), __FILE__, __LINE__)) return; } while (0)

#define WAIT_FOR(session, condition) \
    do { \
        KDevMI::TestWaiter w((session), #condition, __FILE__, __LINE__); \
        while (w.waitUnless((condition))) /* nothing */ ; \
    } while(0)

#define COMPARE_DATA(index, expected) \
    do { if (!KDevMI::compareData((index), (expected), __FILE__, __LINE__)) return; } while (0)

#define SKIP_IF_ATTACH_FORBIDDEN() \
    do { \
        if (KDevMI::isAttachForbidden(__FILE__, __LINE__)) \
            return; \
    } while(0)

using namespace KDevelop;
using namespace KDevMI::LLDB;
using KDevMI::findExecutable;
using KDevMI::findFile;
using KDevMI::findSourceFile;

namespace {
class WritableEnvironmentProfileList : public EnvironmentProfileList
{
public:
    explicit WritableEnvironmentProfileList(KConfig* config) : EnvironmentProfileList(config) {}

    using EnvironmentProfileList::variables;
    using EnvironmentProfileList::saveSettings;
    using EnvironmentProfileList::removeProfile;
};

class TestLaunchConfiguration : public ILaunchConfiguration
{
public:
    explicit TestLaunchConfiguration(const QUrl& executable = findExecutable(QStringLiteral("debuggee_debugee")),
                            const QUrl& workingDirectory = QUrl()) {
        qDebug() << "FIND" << executable;
        c = KSharedConfig::openConfig();
        c->deleteGroup("launch");
        cfg = c->group("launch");
        cfg.writeEntry("isExecutable", true);
        cfg.writeEntry("Executable", executable);
        cfg.writeEntry("Working Directory", workingDirectory);
    }
    const KConfigGroup config() const override { return cfg; }
    KConfigGroup config() override { return cfg; };
    QString name() const override { return QStringLiteral("Test-Launch"); }
    KDevelop::IProject* project() const override { return nullptr; }
    KDevelop::LaunchConfigurationType* type() const override { return nullptr; }

    KConfig* rootConfig() { return c.data(); }
private:
    KConfigGroup cfg;
    KSharedConfigPtr c;
};

class TestFrameStackModel : public LldbFrameStackModel
{
public:

    explicit TestFrameStackModel(DebugSession* session)
        : LldbFrameStackModel(session), fetchFramesCalled(0), fetchThreadsCalled(0) {}

    void fetchFrames(int threadNumber, int from, int to) override
    {
        fetchFramesCalled++;
        LldbFrameStackModel::fetchFrames(threadNumber, from, to);
    }

    void fetchThreads() override
    {
        fetchThreadsCalled++;
        LldbFrameStackModel::fetchThreads();
    }

    int fetchFramesCalled;
    int fetchThreadsCalled;
};

class TestDebugSession : public DebugSession
{
    Q_OBJECT
public:
    TestDebugSession() : DebugSession()
    {
        // explicit set formatter path to force use in-tree formatters, not the one installed in system.
        auto formatter = findFile(LLDB_SRC_DIR, "formatters/all.py");
        setFormatterPath(formatter);

        setSourceInitFile(false);
        m_frameStackModel = new TestFrameStackModel(this);

        KDevelop::ICore::self()->debugController()->addSession(this);
    }

    TestFrameStackModel* frameStackModel() const override
    {
        return m_frameStackModel;
    }

private:
    TestFrameStackModel* m_frameStackModel;
};

} // end of anonymous namespace


BreakpointModel* LldbTest::breakpoints()
{
    return m_core->debugController()->breakpointModel();
}

VariableCollection *LldbTest::variableCollection()
{
    return m_core->debugController()->variableCollection();
}

Variable *LldbTest::watchVariableAt(int i)
{
    auto watchRoot = variableCollection()->indexForItem(variableCollection()->watches(), 0);
    auto idx = variableCollection()->index(i, 0, watchRoot);
    return dynamic_cast<Variable*>(variableCollection()->itemForIndex(idx));
}

QModelIndex LldbTest::localVariableIndexAt(int i, int col)
{
    auto localRoot = variableCollection()->indexForItem(variableCollection()->locals(), 0);
    return variableCollection()->index(i, col, localRoot);
}

// Called before the first testfunction is executed
void LldbTest::initTestCase()
{
    AutoTestShell::init();
    m_core = TestCore::initialize(Core::NoUi);

    m_iface = m_core->pluginController()
                ->pluginForExtension(QStringLiteral("org.kdevelop.IExecutePlugin"), QStringLiteral("kdevexecute"))
                ->extension<IExecutePlugin>();
    Q_ASSERT(m_iface);

    m_debugeeFileName = findSourceFile("debugee.cpp");

    const QString lldbMiExecutable = QStandardPaths::findExecutable(QStringLiteral("lldb-mi"));
    if (lldbMiExecutable.isEmpty()) {
        QSKIP("Skipping, lldb-mi not available");
    }
}

// Called after the last testfunction was executed
void LldbTest::cleanupTestCase()
{
    TestCore::shutdown();
}

// Called before each testfunction is executed
void LldbTest::init()
{
    //remove all breakpoints - so we can set our own in the test
    KConfigGroup bpCfg = KSharedConfig::openConfig()->group("breakpoints");
    bpCfg.writeEntry("number", 0);
    bpCfg.sync();

    breakpoints()->removeRows(0, breakpoints()->rowCount());

    while (variableCollection()->watches()->childCount() > 0) {
        auto var = watchVariableAt(0);
        if (!var) break;
        var->die();
    }
}

void LldbTest::cleanup()
{
    // Called after every testfunction
}

void LldbTest::testStdout()
{
    TestDebugSession *session = new TestDebugSession;

    QSignalSpy outputSpy(session, &TestDebugSession::inferiorStdoutLines);

    TestLaunchConfiguration cfg;
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, KDevelop::IDebugSession::EndedState);

    QVERIFY(outputSpy.count() > 0);

    QStringList outputLines;
    while (outputSpy.count() > 0) {
        QList<QVariant> arguments = outputSpy.takeFirst();
        for (const auto &item : arguments) {
            outputLines.append(item.toStringList());
        }
    }
    QCOMPARE(outputLines, QStringList() << "Hello, world!"
                                        << "Hello");
}

void LldbTest::testEnvironmentSet()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg(findExecutable(QStringLiteral("debuggee_debugeeechoenv")));

    cfg.config().writeEntry("EnvironmentGroup", "LldbTestGroup");

    WritableEnvironmentProfileList envProfiles(cfg.rootConfig());
    envProfiles.removeProfile(QStringLiteral("LldbTestGroup"));
    auto &envs = envProfiles.variables(QStringLiteral("LldbTestGroup"));
    envs[QStringLiteral("VariableA")] = QStringLiteral("-A' \" complex --value");
    envs[QStringLiteral("VariableB")] = QStringLiteral("-B' \" complex --value");
    envProfiles.saveSettings(cfg.rootConfig());

    QSignalSpy outputSpy(session, &TestDebugSession::inferiorStdoutLines);

    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, KDevelop::IDebugSession::EndedState);

    QVERIFY(outputSpy.count() > 0);

    QStringList outputLines;
    while (outputSpy.count() > 0) {
        QList<QVariant> arguments = outputSpy.takeFirst();
        for (const auto &item : arguments) {
            outputLines.append(item.toStringList());
        }
    }
    QCOMPARE(outputLines, QStringList() << "-A' \" complex --value"
                                        << "-B' \" complex --value");
}

void LldbTest::testBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;

    TestLaunchConfiguration cfg;

    KDevelop::Breakpoint * b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 29);
    QCOMPARE(b->state(), KDevelop::Breakpoint::NotStartedState);

    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(b->state(), KDevelop::Breakpoint::CleanState);
    QCOMPARE(session->currentLine(), 29);

    session->stepInto();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    session->stepInto();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
    QCOMPARE(b->state(), KDevelop::Breakpoint::NotStartedState);
}

void LldbTest::testBreakOnStart()
{
    TestDebugSession *session = new TestDebugSession;

    TestLaunchConfiguration cfg;
    cfg.config().writeEntry(KDevMI::Config::BreakOnStartEntry, true);

    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    // line 28 is the start of main function in debugee.cpp
    QCOMPARE(session->currentLine(), 27); // currentLine is zero-based

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testDisableBreakpoint()
{
    QSKIP("Skipping... In lldb-mi -d flag has no effect when mixed with -f");
    //Description: We must stop only on the third breakpoint

    int firstBreakLine=28;
    int secondBreakLine=23;
    int thirdBreakLine=24;
    int fourthBreakLine=31;

    TestDebugSession *session = new TestDebugSession;

    TestLaunchConfiguration cfg;

    KDevelop::Breakpoint *b;

    b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), firstBreakLine);
    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);


    //this is needed to emulate debug from GUI. If we are in edit mode, the debugSession doesn't exist.
    m_core->debugController()->breakpointModel()->blockSignals(true);
    b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), secondBreakLine);
    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);
    //all disabled breakpoints were added

    auto *thirdBreak = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName),
                                                        thirdBreakLine);
    m_core->debugController()->breakpointModel()->blockSignals(false);


    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), thirdBreak->line());

    //disable existing breakpoint
    thirdBreak->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);

    //add another disabled breakpoint
    b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), fourthBreakLine);
    WAIT_FOR_A_WHILE(session, 300);
    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);

    WAIT_FOR_A_WHILE(session, 300);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testChangeLocationBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;

    TestLaunchConfiguration cfg;

    auto *b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 27);

    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 27);

    WAIT_FOR_A_WHILE(session, 100);
    b->setLine(28);
    WAIT_FOR_A_WHILE(session, 100);
    session->run();

    WAIT_FOR_A_WHILE(session, 100);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 28);
    WAIT_FOR_A_WHILE(session, 500);
    breakpoints()->setData(breakpoints()->index(0, KDevelop::Breakpoint::LocationColumn), QString(m_debugeeFileName+":30"));
    QCOMPARE(b->line(), 29);
    WAIT_FOR_A_WHILE(session, 100);
    QCOMPARE(b->line(), 29);
    session->run();
    WAIT_FOR_A_WHILE(session, 100);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 29);
    session->run();

    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testDeleteBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;

    TestLaunchConfiguration cfg;

    QCOMPARE(breakpoints()->rowCount(), 0);
    //add breakpoint before startDebugging
    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 21);
    QCOMPARE(breakpoints()->rowCount(), 1);
    breakpoints()->removeRow(0);
    QCOMPARE(breakpoints()->rowCount(), 0);

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 22);

    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    breakpoints()->removeRow(0);
    session->run();

    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testPendingBreakpoint()
{
    QSKIP("Skipping... Pending breakpoint not work on lldb-mi");
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 28);

    auto * b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile("test_lldb.cpp")), 10);
    QCOMPARE(b->state(), Breakpoint::NotStartedState);

    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(b->state(), Breakpoint::PendingState);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testUpdateBreakpoint()
{
    // Description: user might insert breakpoints using lldb console. model should
    // pick up the manually set breakpoint
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    // break at line 29
    auto b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 28);
    QCOMPARE(breakpoints()->rowCount(), 1);

    QVERIFY(session->startDebugging(&cfg, m_iface));

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState); // stop at line 29

    session->stepInto();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState); // stop after step

    QCOMPARE(session->currentLine(), 23-1); // at the beginning of foo():23: ++i;

    session->addUserCommand(QStringLiteral("break set --file %1 --line %2").arg(m_debugeeFileName).arg(33));
    WAIT_FOR_A_WHILE(session, 20);

    QCOMPARE(breakpoints()->rowCount(), 2);
    b = breakpoints()->breakpoint(1);
    QCOMPARE(b->url(), QUrl::fromLocalFile(m_debugeeFileName));
    QCOMPARE(b->line(), 33-1);

    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState); // stop at line 25

    QCOMPARE(session->currentLine(), 33-1);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testIgnoreHitsBreakpoint()
{
    QSKIP("Skipping... lldb-mi doesn't provide breakpoint hit count update");
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    KDevelop::Breakpoint * b1 = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 21);
    b1->setIgnoreHits(1);

    KDevelop::Breakpoint * b2 = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 22);

    QVERIFY(session->startDebugging(&cfg, m_iface));

    //WAIT_FOR_STATE(session, DebugSession::PausedState);
    WAIT_FOR(session, session->state() == DebugSession::PausedState && b2->hitCount() == 1);
    b2->setIgnoreHits(1);
    session->run();
    //WAIT_FOR_STATE(session, DebugSession::PausedState);
    WAIT_FOR(session, session->state() == DebugSession::PausedState && b1->hitCount() == 1);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testConditionBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    auto b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 39);
    b->setCondition(QStringLiteral("x[0] == 'H'"));

    b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 23);
    b->setCondition(QStringLiteral("i==2"));

    b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 24);

    QVERIFY(session->startDebugging(&cfg, m_iface));

    WAIT_FOR(session, session->state() == DebugSession::PausedState && session->currentLine() == 24);
    b->setCondition(QStringLiteral("i == 0"));
    WAIT_FOR_A_WHILE(session, 100);
    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(session->currentLine(), 23);

    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(session->currentLine(), 39);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testBreakOnWriteBreakpoint()
{
    QSKIP("Skipping... lldb-mi doesn't have proper watchpoint support");
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 24);

    QVERIFY(session->startDebugging(&cfg, m_iface));

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 24);

    breakpoints()->addWatchpoint(QStringLiteral("i"));
    WAIT_FOR_A_WHILE(session, 100);

    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 23);
    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 24);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testBreakOnWriteWithConditionBreakpoint()
{
    QSKIP("Skipping... lldb-mi doesn't have proper watchpoint support");
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 24);

    QVERIFY(session->startDebugging(&cfg, m_iface));

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 24);

    KDevelop::Breakpoint *b = breakpoints()->addWatchpoint(QStringLiteral("i"));
    b->setCondition(QStringLiteral("i==2"));
    QTest::qWait(100);

    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 23);
    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 24);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testBreakOnReadBreakpoint()
{
    QSKIP("Skipping... lldb-mi doesn't have proper watchpoint support");
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addReadWatchpoint(QStringLiteral("foo::i"));

    QVERIFY(session->startDebugging(&cfg, m_iface));

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 23);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testBreakOnReadBreakpoint2()
{
    QSKIP("Skipping... lldb-mi doesn't have proper watchpoint support");
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 24);

    QVERIFY(session->startDebugging(&cfg, m_iface));

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 24);

    breakpoints()->addReadWatchpoint(QStringLiteral("i"));

    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 22);

    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 24);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testBreakOnAccessBreakpoint()
{
    QSKIP("Skipping... lldb-mi doesn't have proper watchpoint support");
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 24);

    QVERIFY(session->startDebugging(&cfg, m_iface));

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 24);

    breakpoints()->addAccessWatchpoint(QStringLiteral("i"));

    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 22);

    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 23);


    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 24);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testInsertBreakpointWhileRunning()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg(findExecutable(QStringLiteral("debuggee_debugeeslow")));
    QString fileName = findSourceFile("debugeeslow.cpp");

    QVERIFY(session->startDebugging(&cfg, m_iface));

    WAIT_FOR_STATE(session, DebugSession::ActiveState);
    WAIT_FOR_A_WHILE(session, 2000);

    qDebug() << "adding breakpoint";
    KDevelop::Breakpoint *b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 25);
    WAIT_FOR_A_WHILE(session, 500);

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    WAIT_FOR_A_WHILE(session, 500);

    QCOMPARE(session->currentLine(), 25);
    b->setDeleted();
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testInsertBreakpointWhileRunningMultiple()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg(findExecutable(QStringLiteral("debuggee_debugeeslow")));
    QString fileName = findSourceFile("debugeeslow.cpp");

    QVERIFY(session->startDebugging(&cfg, m_iface));

    WAIT_FOR_STATE(session, DebugSession::ActiveState);
    WAIT_FOR_A_WHILE(session, 2000);

    qDebug() << "adding breakpoint";
    auto b1 = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 24);
    auto b2 = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 25);

    WAIT_FOR_A_WHILE(session, 500);
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    WAIT_FOR_A_WHILE(session, 500);
    QCOMPARE(session->currentLine(), 24);

    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    WAIT_FOR_A_WHILE(session, 500);
    QCOMPARE(session->currentLine(), 25);
    b1->setDeleted();
    b2->setDeleted();

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testInsertBreakpointFunctionName()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QStringLiteral("main"));

    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 27);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testManualBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QStringLiteral("main"));

    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 27);

    breakpoints()->removeRows(0, 1);
    WAIT_FOR_A_WHILE(session, 100);
    QCOMPARE(breakpoints()->rowCount(), 0);

    session->addCommand(MI::NonMI, QStringLiteral("break set --file debugee.cpp --line 23"));
    WAIT_FOR_A_WHILE(session, 100);
    QCOMPARE(breakpoints()->rowCount(), 1);

    auto b = breakpoints()->breakpoint(0);
    QCOMPARE(b->line(), 22);

    session->addCommand(MI::NonMI, QStringLiteral("break disable 2"));
    session->addCommand(MI::NonMI, QStringLiteral("break modify -c 'i == 1' 2"));
    session->addCommand(MI::NonMI, QStringLiteral("break modify -i 1 2"));
    WAIT_FOR_A_WHILE(session, 1000);
    QCOMPARE(b->enabled(), false);
    QEXPECT_FAIL("", "LLDB 4.0 does not report condition in mi response", Continue);
    QCOMPARE(b->condition(), QString("i == 1"));
    QEXPECT_FAIL("", "LLDB 4.0 does not report ignore hits in mi response", Continue);
    QCOMPARE(b->ignoreHits(), 1);

    session->addCommand(MI::NonMI, QStringLiteral("break delete 2"));
    WAIT_FOR_A_WHILE(session, 100);
    QEXPECT_FAIL("", "LLDB 4.0 does not report breakpoint deletion as mi notification", Continue);
    QCOMPARE(breakpoints()->rowCount(), 0);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

//Bug 201771
void LldbTest::testInsertAndRemoveBreakpointWhileRunning()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg(findExecutable(QStringLiteral("debuggee_debugeeslow")));

    QString fileName = findSourceFile("debugeeslow.cpp");

    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::ActiveState);
    WAIT_FOR_A_WHILE(session, 1000);

    KDevelop::Breakpoint *b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 25);
    WAIT_FOR_A_WHILE(session, 200); // wait for feedback notification from lldb-mi
    b->setDeleted();


    WAIT_FOR_A_WHILE(session, 3000); // give slow debugee extra time to run
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testPickupManuallyInsertedBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QStringLiteral("main"));
    QVERIFY(session->startDebugging(&cfg, m_iface));

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    session->addCommand(MI::NonMI, QStringLiteral("break set --file debugee.cpp --line 32"));
    session->stepInto();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(breakpoints()->breakpoints().count(), 2);
    QCOMPARE(breakpoints()->rowCount(), 2);
    KDevelop::Breakpoint *b = breakpoints()->breakpoint(1);
    QVERIFY(b);
    QCOMPARE(b->line(), 31); //we start with 0, gdb with 1
    QCOMPARE(b->url().fileName(), QString("debugee.cpp"));
}

//Bug 270970
void LldbTest::testPickupManuallyInsertedBreakpointOnlyOnce()
{
    TestDebugSession *session = new TestDebugSession;

    QString sourceFile = findSourceFile("debugee.cpp");
    //inject here, so it behaves similar like a command from .lldbinit
    QTemporaryFile configScript;
    configScript.open();
    configScript.write(QStringLiteral("break set --file %0 --line 32\n").arg(sourceFile).toLocal8Bit());
    configScript.close();

    TestLaunchConfiguration cfg;
    KConfigGroup grp = cfg.config();
    grp.writeEntry(Config::LldbConfigScriptEntry, QUrl::fromLocalFile(configScript.fileName()));

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(QStringLiteral("debugee.cpp")), 31);

    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(breakpoints()->breakpoints().count(), 1);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testBreakpointWithSpaceInPath()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg(findExecutable(QStringLiteral("debuggee_debugeespace")));

    KConfigGroup grp = cfg.config();
    QString fileName = findSourceFile("debugee space.cpp");

    KDevelop::Breakpoint * b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 20);
    QCOMPARE(b->state(), KDevelop::Breakpoint::NotStartedState);

    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(session->currentLine(), 20);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testBreakpointDisabledOnStart()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    auto b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 23);
    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);
    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 29);
    b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 34);
    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);

    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QEXPECT_FAIL("", "See LLDB bug 28703: -d flag has no effect", Abort);
    QCOMPARE(session->currentLine(), 29);
    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Checked);

    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 34);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testMultipleLocationsBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg(findExecutable(QStringLiteral("debuggee_debugeemultilocbreakpoint")));

    breakpoints()->addCodeBreakpoint(QStringLiteral("aPlusB"));

    //TODO check if the additional location breakpoint is added

    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 19);

    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 23);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testMultipleBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;

    //there'll be about 3-4 breakpoints, but we treat it like one.
    TestLaunchConfiguration c(findExecutable(QStringLiteral("debuggee_debugeemultiplebreakpoint")));
    auto b = breakpoints()->addCodeBreakpoint(QStringLiteral("debugeemultiplebreakpoint.cpp:52"));

    QVERIFY(session->startDebugging(&c, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(breakpoints()->breakpoints().count(), 1);

    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testRegularExpressionBreakpoint()
{
    QSKIP("Skipping... lldb has only one breakpoint for multiple locations"
          " (and lldb-mi returns the first one), not support this yet");
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration c(findExecutable(QStringLiteral("debuggee_debugeemultilocbreakpoint")));

    breakpoints()->addCodeBreakpoint(QStringLiteral("main"));
    QVERIFY(session->startDebugging(&c, m_iface));

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    session->addCommand(MI::NonMI, QStringLiteral("break set --func-regex .*aPl.*B"));
    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(breakpoints()->breakpoints().count(), 3);

    session->addCommand(MI::BreakDelete, QLatin1String(""));
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testChangeBreakpointWhileRunning()
{
    QSKIP("Skipping... lldb-mi command -break-enable doesn't enable breakpoint");
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration c(findExecutable(QStringLiteral("debuggee_debugeeslow")));

    KDevelop::Breakpoint* b = breakpoints()->addCodeBreakpoint(QStringLiteral("debugeeslow.cpp:25"));
    QVERIFY(session->startDebugging(&c, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QVERIFY(session->currentLine() >= 24 && session->currentLine() <= 26 );

    session->run();
    WAIT_FOR_STATE(session, DebugSession::ActiveState);
    qDebug() << "Disabling breakpoint";
    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);
    //to make one loop
    WAIT_FOR_A_WHILE(session, 2500);
    qDebug() << "Waiting for active";
    WAIT_FOR_STATE(session, DebugSession::ActiveState);
    qDebug() << "Enabling breakpoint";

    // Use native user command works, but not through -break-enable, which is triggered by setData
    session->addCommand(MI::NonMI, QStringLiteral("break enable"));
    //b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Checked);

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testCatchpoint()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg(findExecutable(QStringLiteral("debuggee_debugeeexception")));

    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);
    TestFrameStackModel* fsModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile("debugeeexception.cpp")), 29);

    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(fsModel->currentFrame(), 0);
    QCOMPARE(session->currentLine(), 29);

    session->addCommand(MI::NonMI, QStringLiteral("break set -E c++"));
    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    const auto frames = fsModel->frames(fsModel->currentThread());
    QVERIFY(frames.size() >= 2);
    // frame 0 is somewhere inside libstdc++
    QCOMPARE(frames[1].file, QUrl::fromLocalFile(findSourceFile("debugeeexception.cpp")));
    QCOMPARE(frames[1].line, 22);

    QCOMPARE(breakpoints()->rowCount(),2);
    QVERIFY(!breakpoints()->breakpoint(0)->location().isEmpty());
    QVERIFY(!breakpoints()->breakpoint(1)->location().isEmpty());

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testShowStepInSource()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    QSignalSpy showStepInSourceSpy(session, &TestDebugSession::showStepInSource);

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 29);

    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    session->stepInto();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    session->stepInto();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);

    {
        QCOMPARE(showStepInSourceSpy.count(), 3);
        QList<QVariant> arguments = showStepInSourceSpy.takeFirst();
        QCOMPARE(arguments.first().value<QUrl>(), QUrl::fromLocalFile(m_debugeeFileName));
        QCOMPARE(arguments.at(1).toInt(), 29);

        arguments = showStepInSourceSpy.takeFirst();
        QCOMPARE(arguments.first().value<QUrl>(), QUrl::fromLocalFile(m_debugeeFileName));
        QCOMPARE(arguments.at(1).toInt(), 22);

        arguments = showStepInSourceSpy.takeFirst();
        QCOMPARE(arguments.first().value<QUrl>(), QUrl::fromLocalFile(m_debugeeFileName));
        QCOMPARE(arguments.at(1).toInt(), 23);
    }
}

void LldbTest::testStack()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    TestFrameStackModel *stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 21);

    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QModelIndex tIdx = stackModel->index(0,0);
    QCOMPARE(stackModel->rowCount(QModelIndex()), 1);
    QCOMPARE(stackModel->columnCount(QModelIndex()), 3);
    COMPARE_DATA(tIdx, "#1 at foo()");

    QCOMPARE(stackModel->rowCount(tIdx), 4);
    QCOMPARE(stackModel->columnCount(tIdx), 3);
    COMPARE_DATA(stackModel->index(0, 0, tIdx), "0");
    COMPARE_DATA(stackModel->index(0, 1, tIdx), "foo()");
    COMPARE_DATA(stackModel->index(0, 2, tIdx), m_debugeeFileName+":23");
    COMPARE_DATA(stackModel->index(1, 0, tIdx), "1");
    COMPARE_DATA(stackModel->index(1, 1, tIdx), "main");
    COMPARE_DATA(stackModel->index(1, 2, tIdx), m_debugeeFileName+":29");
    COMPARE_DATA(stackModel->index(2, 0, tIdx), "2");
    COMPARE_DATA(stackModel->index(2, 1, tIdx), "__libc_start_main");
    COMPARE_DATA(stackModel->index(3, 0, tIdx), "3");
    COMPARE_DATA(stackModel->index(3, 1, tIdx), "_start");


    session->stepOut();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    COMPARE_DATA(tIdx, "#1 at main");
    QCOMPARE(stackModel->rowCount(tIdx), 3);
    COMPARE_DATA(stackModel->index(0, 0, tIdx), "0");
    COMPARE_DATA(stackModel->index(0, 1, tIdx), "main");
    COMPARE_DATA(stackModel->index(0, 2, tIdx), m_debugeeFileName+":30");
    COMPARE_DATA(stackModel->index(1, 0, tIdx), "1");
    COMPARE_DATA(stackModel->index(1, 1, tIdx), "__libc_start_main");
    COMPARE_DATA(stackModel->index(2, 0, tIdx), "2");
    COMPARE_DATA(stackModel->index(2, 1, tIdx), "_start");

    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testStackFetchMore()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg(findExecutable(QStringLiteral("debuggee_debugeerecursion")));
    QString fileName = findSourceFile("debugeerecursion.cpp");

    TestFrameStackModel *stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 25);

    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->frameStackModel()->fetchFramesCalled, 1);

    QModelIndex tIdx = stackModel->index(0,0);
    QCOMPARE(stackModel->rowCount(QModelIndex()), 1);
    QCOMPARE(stackModel->columnCount(QModelIndex()), 3);
    COMPARE_DATA(tIdx, "#1 at foo()");

    QCOMPARE(stackModel->rowCount(tIdx), 21);
    COMPARE_DATA(stackModel->index(0, 0, tIdx), "0");
    COMPARE_DATA(stackModel->index(0, 1, tIdx), "foo()");
    COMPARE_DATA(stackModel->index(0, 2, tIdx), fileName+":26");
    COMPARE_DATA(stackModel->index(1, 0, tIdx), "1");
    COMPARE_DATA(stackModel->index(1, 1, tIdx), "foo()");
    COMPARE_DATA(stackModel->index(1, 2, tIdx), fileName+":24");
    COMPARE_DATA(stackModel->index(2, 0, tIdx), "2");
    COMPARE_DATA(stackModel->index(2, 1, tIdx), "foo()");
    COMPARE_DATA(stackModel->index(2, 2, tIdx), fileName+":24");
    COMPARE_DATA(stackModel->index(19, 0, tIdx), "19");
    COMPARE_DATA(stackModel->index(20, 0, tIdx), "20");

    stackModel->fetchMoreFrames();
    WAIT_FOR_A_WHILE(session, 200);
    QCOMPARE(stackModel->fetchFramesCalled, 2);
    QCOMPARE(stackModel->rowCount(tIdx), 41);
    COMPARE_DATA(stackModel->index(20, 0, tIdx), "20");
    COMPARE_DATA(stackModel->index(21, 0, tIdx), "21");
    COMPARE_DATA(stackModel->index(22, 0, tIdx), "22");
    COMPARE_DATA(stackModel->index(39, 0, tIdx), "39");
    COMPARE_DATA(stackModel->index(40, 0, tIdx), "40");

    stackModel->fetchMoreFrames();
    WAIT_FOR_A_WHILE(session, 200);
    QCOMPARE(stackModel->fetchFramesCalled, 3);
    QCOMPARE(stackModel->rowCount(tIdx), 121);
    COMPARE_DATA(stackModel->index(40, 0, tIdx), "40");
    COMPARE_DATA(stackModel->index(41, 0, tIdx), "41");
    COMPARE_DATA(stackModel->index(42, 0, tIdx), "42");
    COMPARE_DATA(stackModel->index(119, 0, tIdx), "119");
    COMPARE_DATA(stackModel->index(120, 0, tIdx), "120");

    stackModel->fetchMoreFrames();
    WAIT_FOR_A_WHILE(session, 200);
    QCOMPARE(stackModel->fetchFramesCalled, 4);
    QCOMPARE(stackModel->rowCount(tIdx), 301);
    COMPARE_DATA(stackModel->index(120, 0, tIdx), "120");
    COMPARE_DATA(stackModel->index(121, 0, tIdx), "121");
    COMPARE_DATA(stackModel->index(122, 0, tIdx), "122");
    COMPARE_DATA(stackModel->index(298, 0, tIdx), "298");
    COMPARE_DATA(stackModel->index(298, 1, tIdx), "main");
    COMPARE_DATA(stackModel->index(298, 2, tIdx), fileName+":30");
    COMPARE_DATA(stackModel->index(299, 0, tIdx), "299");
    COMPARE_DATA(stackModel->index(299, 1, tIdx), "__libc_start_main");
    COMPARE_DATA(stackModel->index(300, 0, tIdx), "300");
    COMPARE_DATA(stackModel->index(300, 1, tIdx), "_start");

    stackModel->fetchMoreFrames(); //nothing to fetch, we are at the end
    WAIT_FOR_A_WHILE(session, 200);
    QCOMPARE(stackModel->fetchFramesCalled, 4);
    QCOMPARE(stackModel->rowCount(tIdx), 301);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testStackDeactivateAndActive()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    TestFrameStackModel *stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 21);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    QModelIndex tIdx = stackModel->index(0,0);

    session->stepOut();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    COMPARE_DATA(tIdx, "#1 at main");
    QCOMPARE(stackModel->rowCount(tIdx), 3);
    COMPARE_DATA(stackModel->index(0, 0, tIdx), "0");
    COMPARE_DATA(stackModel->index(0, 1, tIdx), "main");
    COMPARE_DATA(stackModel->index(0, 2, tIdx), m_debugeeFileName+":30");
    COMPARE_DATA(stackModel->index(1, 0, tIdx), "1");
    COMPARE_DATA(stackModel->index(1, 1, tIdx), "__libc_start_main");
    COMPARE_DATA(stackModel->index(2, 0, tIdx), "2");
    COMPARE_DATA(stackModel->index(2, 1, tIdx), "_start");

    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testStackSwitchThread()
{
    QSKIP("Skipping... lldb-mi crashes when break at a location with multiple threads running");
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg(findExecutable(QStringLiteral("debuggee_debugeethreads")));
    QString fileName = findSourceFile("debugeethreads.cpp");

    TestFrameStackModel *stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 38);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(stackModel->rowCount(), 4);

    QModelIndex tIdx = stackModel->index(0,0);
    COMPARE_DATA(tIdx, "#1 at main");
    QCOMPARE(stackModel->rowCount(tIdx), 1);
    COMPARE_DATA(stackModel->index(0, 0, tIdx), "0");
    COMPARE_DATA(stackModel->index(0, 1, tIdx), "main");
    COMPARE_DATA(stackModel->index(0, 2, tIdx), fileName+":39");

    tIdx = stackModel->index(1,0);
    QVERIFY(stackModel->data(tIdx).toString().startsWith("#2 at "));
    stackModel->setCurrentThread(2);
    WAIT_FOR_A_WHILE(session, 200);
    int rows = stackModel->rowCount(tIdx);
    QVERIFY(rows > 3);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testAttach()
{
    SKIP_IF_ATTACH_FORBIDDEN();

    QString fileName = findSourceFile("debugeeslow.cpp");

    KProcess debugeeProcess;
    debugeeProcess << QStringLiteral("nice") << findExecutable(QStringLiteral("debuggee_debugeeslow")).toLocalFile();
    debugeeProcess.start();
    QVERIFY(debugeeProcess.waitForStarted());
    QTest::qWait(100);

    TestDebugSession *session = new TestDebugSession;
    session->attachToProcess(debugeeProcess.pid());

    WAIT_FOR_A_WHILE(session, 100);

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 35);

    // lldb-mi sliently stops when attaching to a process. Force it continue to run.
    session->addCommand(MI::ExecContinue, QString(), MI::CmdMaybeStartsRunning);
    WAIT_FOR_A_WHILE(session, 2000);
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(session->currentLine(), 35);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testRemoteDebugging()
{
    KProcess gdbServer;
    gdbServer << QStringLiteral("lldb-server") << QStringLiteral("gdbserver") << QStringLiteral("*:1234");
    gdbServer.start();
    QVERIFY(gdbServer.waitForStarted());

    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    cfg.config().writeEntry(Config::LldbRemoteDebuggingEntry, true);
    cfg.config().writeEntry(Config::LldbRemoteServerEntry, "localhost:1234");
    cfg.config().writeEntry(Config::LldbRemotePathEntry, "/tmp");

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 34);

    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testCoreFile()
{
    QFileInfo f(QStringLiteral("core"));
    f.setCaching(false); // don't cache information
    if (f.exists()) {
        QVERIFY(QFile::remove(f.canonicalFilePath()));
    }

    KProcess debugeeProcess;
    debugeeProcess.setOutputChannelMode(KProcess::MergedChannels);
    debugeeProcess << QStringLiteral("bash") << QStringLiteral("-c")
                   << "ulimit -c unlimited; "
                      + findExecutable(QStringLiteral("debuggee_crash")).toLocalFile();
    debugeeProcess.start();
    debugeeProcess.waitForFinished();
    qDebug() << debugeeProcess.readAll();

    bool coreFileFound = f.exists();
    if (!coreFileFound) {
        // Try to use coredumpctl
        qDebug() << "try to use coredumpctl";
        auto coredumpctl = QStandardPaths::findExecutable(QStringLiteral("coredumpctl"));
        if (!coredumpctl.isEmpty()) {
            KProcess::execute(coredumpctl, {"-1", "-o", f.absoluteFilePath(), "dump", "debuggee_crash"});
            // coredumpctl seems to create an empty file "core" even if no cores can be delivered
            // (like when run inside docker containers as on KDE CI or with kernel.core_pattern=|/dev/null)
            // so also check for size != 0
            coreFileFound = f.exists() && (f.size() > 0);
        }
    }
    if (!coreFileFound)
        QSKIP("no core dump found, check your system configuration (see /proc/sys/kernel/core_pattern).", SkipSingle);

    TestDebugSession *session = new TestDebugSession;
    session->examineCoreFile(findExecutable(QStringLiteral("debuggee_crash")),
                             QUrl::fromLocalFile(f.canonicalFilePath()));

    TestFrameStackModel *stackModel = session->frameStackModel();

    WAIT_FOR_STATE(session, DebugSession::StoppedState);

    QModelIndex tIdx = stackModel->index(0,0);
    QCOMPARE(stackModel->rowCount(QModelIndex()), 1);
    QCOMPARE(stackModel->columnCount(QModelIndex()), 3);
    COMPARE_DATA(tIdx, "#1 at foo()");

    session->stopDebugger();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testVariablesLocals()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    session->variableController()->setAutoUpdate(IVariableController::UpdateLocals);

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 24);

    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(variableCollection()->rowCount(), 2);
    QModelIndex i = variableCollection()->index(1, 0);
    COMPARE_DATA(i, "Locals");

    QCOMPARE(variableCollection()->rowCount(i), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "j");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "1");

    session->run();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    COMPARE_DATA(variableCollection()->index(0, 0, i), "j");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "2");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testVariablesLocalsStruct()
{
    TestDebugSession *session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 38);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    WAIT_FOR_A_WHILE(session, 1000);

    QModelIndex i = variableCollection()->index(1, 0);
    QCOMPARE(variableCollection()->rowCount(i), 4);

    int structIndex = 0;
    for(int j=0; j<3; ++j) {
        if (variableCollection()->index(j, 0, i).data().toString() == QLatin1String("ts")) {
            structIndex = j;
        }
    }

    COMPARE_DATA(variableCollection()->index(structIndex, 0, i), "ts");
    COMPARE_DATA(variableCollection()->index(structIndex, 1, i), "{...}");
    QModelIndex ts = variableCollection()->index(structIndex, 0, i);
    COMPARE_DATA(variableCollection()->index(0, 0, ts), "...");
    variableCollection()->expanded(ts);
    WAIT_FOR_A_WHILE(session, 100);
    COMPARE_DATA(variableCollection()->index(0, 0, ts), "a");
    COMPARE_DATA(variableCollection()->index(0, 1, ts), "0");
    COMPARE_DATA(variableCollection()->index(1, 0, ts), "b");
    COMPARE_DATA(variableCollection()->index(1, 1, ts), "1");
    COMPARE_DATA(variableCollection()->index(2, 0, ts), "c");
    COMPARE_DATA(variableCollection()->index(2, 1, ts), "2");

    session->stepInto();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    WAIT_FOR_A_WHILE(session, 1000);
    COMPARE_DATA(variableCollection()->index(structIndex, 0, i), "ts");
    COMPARE_DATA(variableCollection()->index(structIndex, 1, i), "{...}");
    COMPARE_DATA(variableCollection()->index(0, 1, ts), "1");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testVariablesWatches()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    m_core->debugController()->variableCollection()->variableWidgetShown();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 38);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    variableCollection()->watches()->add(QStringLiteral("ts"));
    WAIT_FOR_A_WHILE(session, 300);

    QModelIndex i = variableCollection()->index(0, 0);
    QCOMPARE(variableCollection()->rowCount(i), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "ts");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "{...}");
    QModelIndex ts = variableCollection()->index(0, 0, i);
    COMPARE_DATA(variableCollection()->index(0, 0, ts), "...");
    variableCollection()->expanded(ts);
    WAIT_FOR_A_WHILE(session, 100);
    COMPARE_DATA(variableCollection()->index(0, 0, ts), "a");
    COMPARE_DATA(variableCollection()->index(0, 1, ts), "0");
    COMPARE_DATA(variableCollection()->index(1, 0, ts), "b");
    COMPARE_DATA(variableCollection()->index(1, 1, ts), "1");
    COMPARE_DATA(variableCollection()->index(2, 0, ts), "c");
    COMPARE_DATA(variableCollection()->index(2, 1, ts), "2");

    session->stepInto();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    WAIT_FOR_A_WHILE(session, 100);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "ts");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "{...}");
    COMPARE_DATA(variableCollection()->index(0, 1, ts), "1");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testVariablesWatchesQuotes()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateWatches);

    // the unquoted string (the actual content):               t\"t
    // quoted string (what we would write as a c string):     "t\\\"t"
    // written in source file:                             R"("t\\\"t")"
    const QString testString(QStringLiteral("t\\\"t")); // the actual content
    const QString quotedTestString(QStringLiteral(R"("t\\\"t")"));

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 38);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    variableCollection()->watches()->add(quotedTestString); //just a constant string
    WAIT_FOR_A_WHILE(session, 3000);

    QModelIndex i = variableCollection()->index(0, 0);
    QCOMPARE(variableCollection()->rowCount(i), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, i), quotedTestString);

    QEXPECT_FAIL("",
                 "LLDB 4.0 cannot deal with string literal in expression when debugging, causing memory access error",
                 Abort);
    COMPARE_DATA(variableCollection()->index(0, 1, i), quotedTestString);

    QModelIndex testStr = variableCollection()->index(0, 0, i);
    COMPARE_DATA(variableCollection()->index(0, 0, testStr), "...");
    variableCollection()->expanded(testStr);
    WAIT_FOR_A_WHILE(session, 100);
    int len = testString.length();
    for (int ind = 0; ind < len; ind++)
    {
        COMPARE_DATA(variableCollection()->index(ind, 0, testStr), QStringLiteral("[%0]").arg(ind));
        QChar c = testString.at(ind);
        QString value = QString::number(c.toLatin1()) + " '" + c + "'";
        COMPARE_DATA(variableCollection()->index(ind, 1, testStr), value);
    }
    COMPARE_DATA(variableCollection()->index(len, 0, testStr), QStringLiteral("[%0]").arg(len));
    COMPARE_DATA(variableCollection()->index(len, 1, testStr), "0 '\\0'");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testVariablesWatchesTwoSessions()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateWatches);

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 38);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    variableCollection()->watches()->add(QStringLiteral("ts"));
    WAIT_FOR_A_WHILE(session, 300);

    QModelIndex ts = variableCollection()->index(0, 0, variableCollection()->index(0, 0));
    variableCollection()->expanded(ts);
    WAIT_FOR_A_WHILE(session, 100);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);

    //check if variable is marked as out-of-scope
    QCOMPARE(variableCollection()->watches()->childCount(), 1);
    auto v = dynamic_cast<LldbVariable*>(watchVariableAt(0));
    QVERIFY(v);
    QVERIFY(!v->inScope());
    QCOMPARE(v->childCount(), 3);
    v = dynamic_cast<LldbVariable*>(v->child(0));
    QVERIFY(!v->inScope());

    //start a second debug session
    session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateWatches);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(variableCollection()->watches()->childCount(), 1);
    ts = variableCollection()->index(0, 0, variableCollection()->index(0, 0));
    v = dynamic_cast<LldbVariable*>(watchVariableAt(0));
    QVERIFY(v);
    QVERIFY(v->inScope());
    QCOMPARE(v->childCount(), 3);
    v = dynamic_cast<LldbVariable*>(v->child(0));
    QVERIFY(v->inScope());
    COMPARE_DATA(variableCollection()->indexForItem(v, 1), QString::number(0));

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);

    //check if variable is marked as out-of-scope
    v = dynamic_cast<LldbVariable*>(watchVariableAt(0));
    QVERIFY(!v->inScope());
    QVERIFY(!dynamic_cast<KDevelop::Variable*>(v->child(0))->inScope());
}

void LldbTest::testVariablesStopDebugger()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 38);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    session->stopDebugger();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testVariablesStartSecondSession()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 38);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 38);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testVariablesSwitchFrame()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);

    TestFrameStackModel *stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 24);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QModelIndex i = variableCollection()->index(1, 0);
    COMPARE_DATA(i, "Locals");
    QCOMPARE(variableCollection()->rowCount(i), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "j"); // only non-static variable works
    COMPARE_DATA(variableCollection()->index(0, 1, i), "1");

    stackModel->setCurrentFrame(1);
    WAIT_FOR_A_WHILE(session, 200);

    i = variableCollection()->index(1, 0);
    QCOMPARE(variableCollection()->rowCount(i), 4);
    COMPARE_DATA(variableCollection()->index(2, 0, i), "argc");
    COMPARE_DATA(variableCollection()->index(2, 1, i), "1");
    COMPARE_DATA(variableCollection()->index(3, 0, i), "argv");

    breakpoints()->removeRow(0);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testVariablesQuicklySwitchFrame()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);

    TestFrameStackModel *stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 24);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QModelIndex i = variableCollection()->index(1, 0);
    COMPARE_DATA(i, "Locals");
    QCOMPARE(variableCollection()->rowCount(i), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "j"); // only non-static variable works
    COMPARE_DATA(variableCollection()->index(0, 1, i), "1");

    stackModel->setCurrentFrame(1);
    WAIT_FOR_A_WHILE(session, 300);
    stackModel->setCurrentFrame(0);
    WAIT_FOR_A_WHILE(session, 1);
    stackModel->setCurrentFrame(1);
    WAIT_FOR_A_WHILE(session, 1);
    stackModel->setCurrentFrame(0);
    WAIT_FOR_A_WHILE(session, 1);
    stackModel->setCurrentFrame(1);
    WAIT_FOR_A_WHILE(session, 500);

    i = variableCollection()->index(1, 0);
    QCOMPARE(variableCollection()->rowCount(i), 4);
    QStringList locs;
    for (int j = 0; j < variableCollection()->rowCount(i); ++j) {
        locs << variableCollection()->index(j, 0, i).data().toString();
    }
    QVERIFY(locs.contains("argc"));
    QVERIFY(locs.contains("argv"));
    QVERIFY(locs.contains("x"));

    breakpoints()->removeRow(0);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testSwitchFrameLldbConsole()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    TestFrameStackModel *stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 24);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(stackModel->currentFrame(), 0);
    stackModel->setCurrentFrame(1);
    QCOMPARE(stackModel->currentFrame(), 1);
    WAIT_FOR_A_WHILE(session, 500);
    QCOMPARE(stackModel->currentFrame(), 1);

    session->addUserCommand(QStringLiteral("print i"));
    WAIT_FOR_A_WHILE(session, 500);
    //currentFrame must not reset to 0; Bug 222882
    QCOMPARE(stackModel->currentFrame(), 1);
}

void LldbTest::testSegfaultDebugee()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg(findExecutable(QStringLiteral("debuggee_crash")));

    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);

    QString fileName = findSourceFile("debugeecrash.cpp");
    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 23);

    QVERIFY(session->startDebugging(&cfg, m_iface));

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 23);
    session->run();

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->currentLine(), 24);

    session->stopDebugger();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

//Bug 274390
void LldbTest::testCommandOrderFastStepping()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg(findExecutable(QStringLiteral("debuggee_debugeeqt")));

    breakpoints()->addCodeBreakpoint(QStringLiteral("main"));
    QVERIFY(session->startDebugging(&cfg, m_iface));
    for(int i=0; i<20; i++) {
        session->stepInto();
    }
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testRunLldbScript()
{
    TestDebugSession *session = new TestDebugSession;

    QTemporaryFile runScript;
    runScript.open();

    runScript.write(QStringLiteral("break set --file %1 --line 35\n").arg(findSourceFile("debugee.cpp")).toUtf8());
    runScript.close();

    TestLaunchConfiguration cfg;
    KConfigGroup grp = cfg.config();
    grp.writeEntry(Config::LldbConfigScriptEntry, QUrl::fromLocalFile(runScript.fileName()));

    QVERIFY(session->startDebugging(&cfg, m_iface));

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(session->currentLine(), 35);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testBug301287()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateWatches);

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 28);

    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    variableCollection()->watches()->add(QStringLiteral("argc"));
    WAIT_FOR_A_WHILE(session, 300);

    QModelIndex i = variableCollection()->index(0, 0);
    QCOMPARE(variableCollection()->rowCount(i), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "argc");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "1");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);

    //start second debug session (same cfg)
    session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateWatches);

    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    i = variableCollection()->index(0, 0);
    QCOMPARE(variableCollection()->rowCount(i), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "argc");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "1");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void LldbTest::testDebugInExternalTerminal()
{
    TestLaunchConfiguration cfg;

    foreach (const QString & console, QStringList() << "konsole" << "xterm" << "xfce4-terminal" << "gnome-terminal") {

        if (QStandardPaths::findExecutable(console).isEmpty()) {
            continue;
        }

        TestDebugSession* session = new TestDebugSession();

        cfg.config().writeEntry("External Terminal"/*ExecutePlugin::terminalEntry*/, console);
        cfg.config().writeEntry("Use External Terminal"/*ExecutePlugin::useTerminalEntry*/, true);

        KDevelop::Breakpoint* b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(m_debugeeFileName), 28);

        QVERIFY(session->startDebugging(&cfg, m_iface));
        WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

        QCOMPARE(b->state(), KDevelop::Breakpoint::CleanState);

        session->stepInto();
        WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

        session->run();
        WAIT_FOR_STATE(session, DebugSession::EndedState);
    }
}

void LldbTest::testSpecialPath()
{
    QSKIP("Skipping... lldb-mi itself can't handle path with space in application dir");

    TestDebugSession* session = new TestDebugSession;

    auto debugee = findExecutable(QStringLiteral("path with space/debuggee_spacedebugee"));
    TestLaunchConfiguration c(debugee, KIO::upUrl(debugee));
    KDevelop::Breakpoint* b = breakpoints()->addCodeBreakpoint(QStringLiteral("spacedebugee.cpp:30"));
    QCOMPARE(b->state(), KDevelop::Breakpoint::NotStartedState);

    QVERIFY(session->startDebugging(&c, m_iface));

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(b->state(), KDevelop::Breakpoint::CleanState);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void KDevMI::LLDB::LldbTest::testEnvironmentCd()
{
    TestDebugSession *session = new TestDebugSession;

    QSignalSpy outputSpy(session, &TestDebugSession::inferiorStdoutLines);

    auto path = KIO::upUrl(findExecutable(QStringLiteral("path with space/debuggee_spacedebugee")));
    TestLaunchConfiguration cfg(findExecutable(QStringLiteral("debuggee_debugeepath")), path);

    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, KDevelop::IDebugSession::EndedState);

    QVERIFY(outputSpy.count() > 0);

    QStringList outputLines;
    while (outputSpy.count() > 0) {
        QList<QVariant> arguments = outputSpy.takeFirst();
        for (const auto &item : arguments) {
            outputLines.append(item.toStringList());
        }
    }
    QCOMPARE(outputLines, QStringList() << path.toLocalFile());
}

QTEST_MAIN(KDevMI::LLDB::LldbTest);

#include "test_lldb.moc"
