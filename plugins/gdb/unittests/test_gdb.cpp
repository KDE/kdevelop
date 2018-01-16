/*
   Copyright 2009 Niko Sams <niko.sams@gmail.com>
   Copyright 2013 Vlas Puhov <vlas.puhov@mail.ru>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "test_gdb.h"

#include "debugsession.h"
#include "gdbframestackmodel.h"
#include "mi/micommand.h"
#include "mi/milexer.h"
#include "mi/miparser.h"
#include "tests/debuggers-tests-config.h"
#include "tests/testhelper.h"

#include <execute/iexecuteplugin.h>
#include <debugger/breakpoint/breakpoint.h>
#include <debugger/breakpoint/breakpointmodel.h>
#include <debugger/framestack/framestackmodel.h>
#include <debugger/interfaces/ivariablecontroller.h>
#include <debugger/variable/variablecollection.h>
#include <interfaces/idebugcontroller.h>
#include <interfaces/ilaunchconfiguration.h>
#include <interfaces/iplugincontroller.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <shell/shellextension.h>
#include <util/environmentprofilelist.h>

#include <KIO/Global>
#include <KProcess>
#include <KSharedConfig>

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QSignalSpy>
#include <QTest>
#include <QTemporaryFile>

#define SKIP_IF_ATTACH_FORBIDDEN() \
    do { \
        if (KDevMI::isAttachForbidden(__FILE__, __LINE__)) \
            return; \
    } while(0)

using KDevelop::AutoTestShell;
using KDevMI::findExecutable;
using KDevMI::findSourceFile;
using KDevMI::findFile;

namespace KDevMI { namespace GDB {

void GdbTest::initTestCase()
{
    AutoTestShell::init();
    KDevelop::TestCore::initialize(KDevelop::Core::NoUi);

    m_iface = KDevelop::ICore::self()->pluginController()->pluginForExtension(QStringLiteral("org.kdevelop.IExecutePlugin"), QStringLiteral("kdevexecute"))->extension<IExecutePlugin>();
    Q_ASSERT(m_iface);
}

void GdbTest::cleanupTestCase()
{
    KDevelop::TestCore::shutdown();
}

void GdbTest::init()
{
    //remove all breakpoints - so we can set our own in the test
    KConfigGroup breakpoints = KSharedConfig::openConfig()->group("breakpoints");
    breakpoints.writeEntry("number", 0);
    breakpoints.sync();

    KDevelop::BreakpointModel* m = KDevelop::ICore::self()->debugController()->breakpointModel();
    m->removeRows(0, m->rowCount());

    KDevelop::VariableCollection *vc = KDevelop::ICore::self()->debugController()->variableCollection();
    for (int i=0; i < vc->watches()->childCount(); ++i) {
        delete vc->watches()->child(i);
    }
    vc->watches()->clear();
}

class WritableEnvironmentProfileList : public KDevelop::EnvironmentProfileList
{
public:
    explicit WritableEnvironmentProfileList(KConfig* config) : EnvironmentProfileList(config) {}

    using EnvironmentProfileList::variables;
    using EnvironmentProfileList::saveSettings;
    using EnvironmentProfileList::removeProfile;
};

class TestLaunchConfiguration : public KDevelop::ILaunchConfiguration
{
public:
    explicit TestLaunchConfiguration(const QUrl& executable = findExecutable(QStringLiteral("debuggee_debugee")),
                            const QUrl& workingDirectory = QUrl()) {
        qDebug() << "FIND" << executable;
        c = new KConfig();
        c->deleteGroup("launch");
        cfg = c->group("launch");
        cfg.writeEntry("isExecutable", true);
        cfg.writeEntry("Executable", executable);
        cfg.writeEntry("Working Directory", workingDirectory);
    }
    ~TestLaunchConfiguration() override {
        delete c;
    }
    const KConfigGroup config() const override { return cfg; }
    KConfigGroup config() override { return cfg; };
    QString name() const override { return QStringLiteral("Test-Launch"); }
    KDevelop::IProject* project() const override { return nullptr; }
    KDevelop::LaunchConfigurationType* type() const override { return nullptr; }

    KConfig *rootConfig() { return c; }
private:
    KConfigGroup cfg;
    KConfig *c;
};

class TestFrameStackModel : public GdbFrameStackModel
{
public:

    explicit TestFrameStackModel(DebugSession* session)
        : GdbFrameStackModel(session), fetchFramesCalled(0), fetchThreadsCalled(0) {}

    int fetchFramesCalled;
    int fetchThreadsCalled;
    void fetchFrames(int threadNumber, int from, int to) override
    {
        fetchFramesCalled++;
        GdbFrameStackModel::fetchFrames(threadNumber, from, to);
    }

    void fetchThreads() override
    {
        fetchThreadsCalled++;
        GdbFrameStackModel::fetchThreads();
    }
};

class TestDebugSession : public DebugSession
{
    Q_OBJECT
public:
    TestDebugSession() : DebugSession()
    {
        setSourceInitFile(false);
        setAutoDisableASLR(false);
        m_frameStackModel = new TestFrameStackModel(this);
        KDevelop::ICore::self()->debugController()->addSession(this);
    }

    QUrl url() { return currentUrl(); }
    int line() { return currentLine(); }

    TestFrameStackModel* frameStackModel() const override
    {
        return m_frameStackModel;
    }

private:
    TestFrameStackModel* m_frameStackModel;
};

class TestWaiter
{
public:
    TestWaiter(DebugSession * session_, const char * condition_, const char * file_, int line_)
        : session(session_)
        , condition(condition_)
        , file(file_)
        , line(line_)
    {
        stopWatch.start();
    }

    bool waitUnless(bool ok)
    {
        if (ok) {
            qDebug() << "Condition " << condition << " reached in " << file << ':' << line;
            return false;
        }

        if (stopWatch.elapsed() > 5000) {
            QTest::qFail(qPrintable(QString("Timeout before reaching condition %0").arg(condition)),
                file, line);
            return false;
        }

        QTest::qWait(100);

        if (!session) {
            QTest::qFail(qPrintable(QString("Session ended without reaching condition %0").arg(condition)),
                file, line);
            return false;
        }

        return true;
    }

private:
    QTime stopWatch;
    QPointer<DebugSession> session;
    const char * condition;
    const char * file;
    int line;
};

#define WAIT_FOR_STATE(session, state) \
    do { if (!waitForState((session), (state), __FILE__, __LINE__)) return; } while (0)

#define WAIT_FOR_STATE_AND_IDLE(session, state) \
    do { if (!waitForState((session), (state), __FILE__, __LINE__, true)) return; } while (0)

#define WAIT_FOR(session, condition) \
    do { \
        TestWaiter w((session), #condition, __FILE__, __LINE__); \
        while (w.waitUnless((condition))) /* nothing */ ; \
    } while(0)

#define COMPARE_DATA(index, expected) \
    do { if(!compareData((index), (expected), __FILE__, __LINE__)) return; } while (0)

bool compareData(QModelIndex index, const QString& expected, const char *file, int line)
{
    QString s = index.model()->data(index, Qt::DisplayRole).toString();
    if (s != expected) {
        QTest::qFail(qPrintable(QString("'%0' didn't match expected '%1' in %2:%3")
                                .arg(s).arg(expected).arg(file).arg(line)),
                     file, line);
        return false;
    }
    return true;
}

static const QString debugeeFileName = findSourceFile(QStringLiteral("debugee.cpp"));

KDevelop::BreakpointModel* breakpoints()
{
    return KDevelop::ICore::self()->debugController()->breakpointModel();
}

void GdbTest::testStdOut()
{
    TestDebugSession *session = new TestDebugSession;

    QSignalSpy outputSpy(session, &TestDebugSession::inferiorStdoutLines);

    TestLaunchConfiguration cfg;
    session->startDebugging(&cfg, m_iface);
    WAIT_FOR_STATE(session, KDevelop::IDebugSession::EndedState);

    {
        QCOMPARE(outputSpy.count(), 1);
        QList<QVariant> arguments = outputSpy.takeFirst();
        QCOMPARE(arguments.count(), 1);
        QCOMPARE(arguments.first().toStringList(), QStringList() << "Hello, world!" << "Hello");
    }
}

void GdbTest::testEnvironmentSet()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg(findExecutable(QStringLiteral("debuggee_debugeeechoenv")));

    cfg.config().writeEntry("EnvironmentGroup", "GdbTestGroup");

    WritableEnvironmentProfileList envProfiles(cfg.rootConfig());
    envProfiles.removeProfile(QStringLiteral("GdbTestGroup"));
    auto &envs = envProfiles.variables(QStringLiteral("GdbTestGroup"));
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

void GdbTest::testBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;

    TestLaunchConfiguration cfg;

    KDevelop::Breakpoint * b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 28);
    QCOMPARE(b->state(), KDevelop::Breakpoint::NotStartedState);

    session->startDebugging(&cfg, m_iface);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(b->state(), KDevelop::Breakpoint::CleanState);
    session->stepInto();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    session->stepInto();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);

    QCOMPARE(b->state(), KDevelop::Breakpoint::NotStartedState);
}

void GdbTest::testDisableBreakpoint()
{
    //Description: We must stop only on the third breakpoint

    int firstBreakLine=28;
    int secondBreakLine=23;
    int thirdBreakLine=24;
    int fourthBreakLine=31;

    TestDebugSession *session = new TestDebugSession;

    TestLaunchConfiguration cfg;

    KDevelop::Breakpoint *b;

    b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), firstBreakLine);
    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);


    //this is needed to emulate debug from GUI. If we are in edit mode, the debugSession doesn't exist.
    KDevelop::ICore::self()->debugController()->breakpointModel()->blockSignals(true);
    b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), secondBreakLine);
    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);
    //all disabled breakpoints were added

    KDevelop::Breakpoint * thirdBreak = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), thirdBreakLine);
    KDevelop::ICore::self()->debugController()->breakpointModel()->blockSignals(false);


    session->startDebugging(&cfg, m_iface);
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(session->currentLine(), thirdBreak->line());

    //disable existing breakpoint
    thirdBreak->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);

    //add another disabled breakpoint
    b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), fourthBreakLine);
    QTest::qWait(300);
    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);

    QTest::qWait(300);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testChangeLocationBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;

    TestLaunchConfiguration cfg;

    KDevelop::Breakpoint *b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 27);

    session->startDebugging(&cfg, m_iface);
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 27);

    QTest::qWait(100);
    b->setLine(28);
    QTest::qWait(100);
    session->run();

    QTest::qWait(100);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 28);
    QTest::qWait(500);
    breakpoints()->setData(breakpoints()->index(0, KDevelop::Breakpoint::LocationColumn), QString(debugeeFileName+":30"));
    QCOMPARE(b->line(), 29);
    QTest::qWait(100);
    QCOMPARE(b->line(), 29);
    session->run();
    QTest::qWait(100);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 29);
    session->run();

    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testDeleteBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;

    TestLaunchConfiguration cfg;

    QCOMPARE(breakpoints()->rowCount(), 0);
    //add breakpoint before startDebugging
    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 21);
    QCOMPARE(breakpoints()->rowCount(), 1);
    breakpoints()->removeRow(0);
    QCOMPARE(breakpoints()->rowCount(), 0);

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 22);

    session->startDebugging(&cfg, m_iface);
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    breakpoints()->removeRow(0);
    session->run();

    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testPendingBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 28);

    KDevelop::Breakpoint * b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile(QStringLiteral("debugeeqt.cpp"))), 10);
    QCOMPARE(b->state(), KDevelop::Breakpoint::NotStartedState);

    session->startDebugging(&cfg, m_iface);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(b->state(), KDevelop::Breakpoint::PendingState);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testUpdateBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    // breakpoint 1: line 29
    KDevelop::Breakpoint * b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 28);
    QCOMPARE(breakpoints()->rowCount(), 1);

    session->startDebugging(&cfg, m_iface);

    // breakpoint 2: line 28
    //insert custom command as user might do it using GDB console
    session->addCommand(new MI::UserCommand(MI::NonMI, "break "+debugeeFileName+":28"));

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState); // stop at line 28
    session->stepInto();
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState); // stop after step
    QCOMPARE(breakpoints()->rowCount(), 2);
    b = breakpoints()->breakpoint(1);
    QCOMPARE(b->url(), QUrl::fromLocalFile(debugeeFileName));
    QCOMPARE(b->line(), 27);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState); // stop at line 29
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testIgnoreHitsBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    KDevelop::Breakpoint * b1 = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 21);
    b1->setIgnoreHits(1);

    KDevelop::Breakpoint * b2 = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 22);

    session->startDebugging(&cfg, m_iface);

    //WAIT_FOR_STATE(session, DebugSession::PausedState);
    WAIT_FOR(session, session->state() == DebugSession::PausedState && b2->hitCount() == 1);
    b2->setIgnoreHits(1);
    session->run();
    //WAIT_FOR_STATE(session, DebugSession::PausedState);
    WAIT_FOR(session, session->state() == DebugSession::PausedState && b1->hitCount() == 1);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testConditionBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    KDevelop::Breakpoint * b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 39);
    b->setCondition(QStringLiteral("x[0] == 'H'"));

    b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 23);
    b->setCondition(QStringLiteral("i==2"));

    b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 24);

    session->startDebugging(&cfg, m_iface);

    WAIT_FOR(session, session->state() == DebugSession::PausedState && session->line() == 24);
    b->setCondition(QStringLiteral("i == 0"));
    QTest::qWait(100);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 23);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 39);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testBreakOnWriteBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 24);

    session->startDebugging(&cfg, m_iface);

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 24);

    breakpoints()->addWatchpoint(QStringLiteral("i"));
    QTest::qWait(100);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 22); // line 23: ++i; int j = i;
    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 24);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testBreakOnWriteWithConditionBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 24);

    session->startDebugging(&cfg, m_iface);

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 24);

    KDevelop::Breakpoint *b = breakpoints()->addWatchpoint(QStringLiteral("i"));
    b->setCondition(QStringLiteral("i==2"));
    QTest::qWait(100);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 22); // line 23: ++i; int j = i;
    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 24);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testBreakOnReadBreakpoint()
{
    /*
    test disabled because of gdb bug: http://sourceware.org/bugzilla/show_bug.cgi?id=10136

    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    KDevelop::Breakpoint *b = breakpoints()->addReadWatchpoint("foo::i");

    session->startDebugging(&cfg, m_iface);

    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 23);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
    */
}

void GdbTest::testBreakOnReadBreakpoint2()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 24);

    session->startDebugging(&cfg, m_iface);

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 24);

    breakpoints()->addReadWatchpoint(QStringLiteral("i"));

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 22); // ++i

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 22); // int j = i

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 24);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testBreakOnAccessBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 24);

    session->startDebugging(&cfg, m_iface);

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 24);

    breakpoints()->addAccessWatchpoint(QStringLiteral("i"));

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 22); // line 23: ++i (read)

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 22); // line 23: ++i (write)

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 22); // line 23: int j = i (read)

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 24);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testInsertBreakpointWhileRunning()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg(findExecutable(QStringLiteral("debuggee_debugeeslow")));
    QString fileName = findSourceFile(QStringLiteral("debugeeslow.cpp"));

    session->startDebugging(&cfg, m_iface);

    WAIT_FOR_STATE(session, DebugSession::ActiveState);
    QTest::qWait(2000);
    qDebug() << "adding breakpoint";
    KDevelop::Breakpoint *b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 25);
    QTest::qWait(500);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(500);
    QCOMPARE(session->line(), 25);
    b->setDeleted();
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testInsertBreakpointWhileRunningMultiple()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg(findExecutable(QStringLiteral("debuggee_debugeeslow")));
    QString fileName = findSourceFile(QStringLiteral("debugeeslow.cpp"));

    session->startDebugging(&cfg, m_iface);

    WAIT_FOR_STATE(session, DebugSession::ActiveState);
    QTest::qWait(2000);
    qDebug() << "adding breakpoint";
    KDevelop::Breakpoint *b1 = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 24);
    KDevelop::Breakpoint *b2 = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 25);
    QTest::qWait(500);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(500);
    QCOMPARE(session->line(), 24);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(500);
    QCOMPARE(session->line(), 25);
    b1->setDeleted();
    b2->setDeleted();
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testInsertBreakpointFunctionName()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QStringLiteral("main"));

    session->startDebugging(&cfg, m_iface);
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 27);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testManualBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QStringLiteral("main"));

    session->startDebugging(&cfg, m_iface);
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 27);

    breakpoints()->removeRows(0, 1);
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(breakpoints()->rowCount(), 0);

    session->addCommand(MI::NonMI, QStringLiteral("break debugee.cpp:23"));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(breakpoints()->rowCount(), 1);

    KDevelop::Breakpoint* b = breakpoints()->breakpoint(0);
    QCOMPARE(b->line(), 22);

    session->addCommand(MI::NonMI, QStringLiteral("disable 2"));
    session->addCommand(MI::NonMI, QStringLiteral("condition 2 i == 1"));
    session->addCommand(MI::NonMI, QStringLiteral("ignore 2 1"));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(b->enabled(), false);
    QCOMPARE(b->condition(), QString("i == 1"));
    QCOMPARE(b->ignoreHits(), 1);

    session->addCommand(MI::NonMI, QStringLiteral("delete 2"));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(breakpoints()->rowCount(), 0);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testShowStepInSource()
{
    TestDebugSession *session = new TestDebugSession;

    QSignalSpy showStepInSourceSpy(session, &TestDebugSession::showStepInSource);

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 29);
    session->startDebugging(&cfg, m_iface);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    session->stepInto();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    session->stepInto();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);

    {
        QCOMPARE(showStepInSourceSpy.count(), 3);
        QList<QVariant> arguments = showStepInSourceSpy.takeFirst();
        QCOMPARE(arguments.first().value<QUrl>(), QUrl::fromLocalFile(debugeeFileName));
        QCOMPARE(arguments.at(1).toInt(), 29);

        arguments = showStepInSourceSpy.takeFirst();
        QCOMPARE(arguments.first().value<QUrl>(), QUrl::fromLocalFile(debugeeFileName));
        QCOMPARE(arguments.at(1).toInt(), 22);

        arguments = showStepInSourceSpy.takeFirst();
        QCOMPARE(arguments.first().value<QUrl>(), QUrl::fromLocalFile(debugeeFileName));
        QCOMPARE(arguments.at(1).toInt(), 23);
    }
}

void GdbTest::testStack()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    TestFrameStackModel *stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 21);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QModelIndex tIdx = stackModel->index(0,0);
    QCOMPARE(stackModel->rowCount(QModelIndex()), 1);
    QCOMPARE(stackModel->columnCount(QModelIndex()), 3);
    COMPARE_DATA(tIdx, "#1 at foo");

    QCOMPARE(stackModel->rowCount(tIdx), 2);
    QCOMPARE(stackModel->columnCount(tIdx), 3);
    COMPARE_DATA(stackModel->index(0, 0, tIdx), "0");
    COMPARE_DATA(stackModel->index(0, 1, tIdx), "foo");
    COMPARE_DATA(stackModel->index(0, 2, tIdx), debugeeFileName+":23");
    COMPARE_DATA(stackModel->index(1, 0, tIdx), "1");
    COMPARE_DATA(stackModel->index(1, 1, tIdx), "main");
    COMPARE_DATA(stackModel->index(1, 2, tIdx), debugeeFileName+":29");


    session->stepOut();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    COMPARE_DATA(tIdx, "#1 at main");
    QCOMPARE(stackModel->rowCount(tIdx), 1);
    COMPARE_DATA(stackModel->index(0, 0, tIdx), "0");
    COMPARE_DATA(stackModel->index(0, 1, tIdx), "main");
    COMPARE_DATA(stackModel->index(0, 2, tIdx), debugeeFileName+":30");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testStackFetchMore()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg(findExecutable(QStringLiteral("debuggee_debugeerecursion")));
    QString fileName = findSourceFile(QStringLiteral("debugeerecursion.cpp"));

    TestFrameStackModel *stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 25);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->frameStackModel()->fetchFramesCalled, 1);

    QModelIndex tIdx = stackModel->index(0,0);
    QCOMPARE(stackModel->rowCount(QModelIndex()), 1);
    QCOMPARE(stackModel->columnCount(QModelIndex()), 3);
    COMPARE_DATA(tIdx, "#1 at foo");

    QCOMPARE(stackModel->rowCount(tIdx), 21);
    COMPARE_DATA(stackModel->index(0, 0, tIdx), "0");
    COMPARE_DATA(stackModel->index(0, 1, tIdx), "foo");
    COMPARE_DATA(stackModel->index(0, 2, tIdx), fileName+":26");
    COMPARE_DATA(stackModel->index(1, 0, tIdx), "1");
    COMPARE_DATA(stackModel->index(1, 1, tIdx), "foo");
    COMPARE_DATA(stackModel->index(1, 2, tIdx), fileName+":24");
    COMPARE_DATA(stackModel->index(2, 0, tIdx), "2");
    COMPARE_DATA(stackModel->index(2, 1, tIdx), "foo");
    COMPARE_DATA(stackModel->index(2, 2, tIdx), fileName+":24");
    COMPARE_DATA(stackModel->index(19, 0, tIdx), "19");
    COMPARE_DATA(stackModel->index(20, 0, tIdx), "20");

    stackModel->fetchMoreFrames();
    QTest::qWait(200);
    QCOMPARE(stackModel->fetchFramesCalled, 2);
    QCOMPARE(stackModel->rowCount(tIdx), 41);
    COMPARE_DATA(stackModel->index(20, 0, tIdx), "20");
    COMPARE_DATA(stackModel->index(21, 0, tIdx), "21");
    COMPARE_DATA(stackModel->index(22, 0, tIdx), "22");
    COMPARE_DATA(stackModel->index(39, 0, tIdx), "39");
    COMPARE_DATA(stackModel->index(40, 0, tIdx), "40");

    stackModel->fetchMoreFrames();
    QTest::qWait(200);
    QCOMPARE(stackModel->fetchFramesCalled, 3);
    QCOMPARE(stackModel->rowCount(tIdx), 121);
    COMPARE_DATA(stackModel->index(40, 0, tIdx), "40");
    COMPARE_DATA(stackModel->index(41, 0, tIdx), "41");
    COMPARE_DATA(stackModel->index(42, 0, tIdx), "42");
    COMPARE_DATA(stackModel->index(119, 0, tIdx), "119");
    COMPARE_DATA(stackModel->index(120, 0, tIdx), "120");

    stackModel->fetchMoreFrames();
    QTest::qWait(200);
    QCOMPARE(stackModel->fetchFramesCalled, 4);
    QCOMPARE(stackModel->rowCount(tIdx), 299);
    COMPARE_DATA(stackModel->index(120, 0, tIdx), "120");
    COMPARE_DATA(stackModel->index(121, 0, tIdx), "121");
    COMPARE_DATA(stackModel->index(122, 0, tIdx), "122");
    COMPARE_DATA(stackModel->index(298, 0, tIdx), "298");
    COMPARE_DATA(stackModel->index(298, 1, tIdx), "main");
    COMPARE_DATA(stackModel->index(298, 2, tIdx), fileName+":30");

    stackModel->fetchMoreFrames(); //nothing to fetch, we are at the end
    QTest::qWait(200);
    QCOMPARE(stackModel->fetchFramesCalled, 4);
    QCOMPARE(stackModel->rowCount(tIdx), 299);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testStackDeactivateAndActive()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    TestFrameStackModel *stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 21);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    QModelIndex tIdx = stackModel->index(0,0);

    session->stepOut();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(200);
    COMPARE_DATA(tIdx, "#1 at main");
    QCOMPARE(stackModel->rowCount(tIdx), 1);
    COMPARE_DATA(stackModel->index(0, 0, tIdx), "0");
    COMPARE_DATA(stackModel->index(0, 1, tIdx), "main");
    COMPARE_DATA(stackModel->index(0, 2, tIdx), debugeeFileName+":30");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testStackSwitchThread()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg(findExecutable(QStringLiteral("debuggee_debugeethreads")));
    QString fileName = findSourceFile(QStringLiteral("debugeethreads.cpp"));

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
    QTest::qWait(200);
    int rows = stackModel->rowCount(tIdx);
    QVERIFY(rows > 3);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testAttach()
{
    SKIP_IF_ATTACH_FORBIDDEN();

    QString fileName = findSourceFile(QStringLiteral("debugeeslow.cpp"));

    KProcess debugeeProcess;
    debugeeProcess << QStringLiteral("nice") << findExecutable(QStringLiteral("debuggee_debugeeslow")).toLocalFile();
    debugeeProcess.start();
    QVERIFY(debugeeProcess.waitForStarted());
    QTest::qWait(100);

    TestDebugSession *session = new TestDebugSession;
    session->attachToProcess(debugeeProcess.pid());
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 34);
    QTest::qWait(100);
    session->run();
    QTest::qWait(2000);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    if (session->line() < 34 || session->line() < 35) {
        QCOMPARE(session->line(), 34);
    }

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testManualAttach()
{
    SKIP_IF_ATTACH_FORBIDDEN();

    QString fileName = findSourceFile(QStringLiteral("debugeeslow.cpp"));

    KProcess debugeeProcess;
    debugeeProcess << QStringLiteral("nice") << findExecutable(QStringLiteral("debuggee_debugeeslow")).toLocalFile();
    debugeeProcess.start();
    QVERIFY(debugeeProcess.waitForStarted());

    TestDebugSession *session = new TestDebugSession;

    TestLaunchConfiguration cfg;
    cfg.config().writeEntry(Config::RemoteGdbRunEntry,
                            QUrl::fromLocalFile(findFile(GDB_SRC_DIR,
                                                         QStringLiteral("unittests/gdb_script_empty"))));
    QVERIFY(session->startDebugging(&cfg, m_iface));

    session->addCommand(MI::NonMI, QStringLiteral("attach %0").arg(debugeeProcess.pid()));
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    session->run();
    QTest::qWait(2000); // give the slow inferior some extra time to run
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testCoreFile()
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
    qDebug() << "Debuggee output:\n" << debugeeProcess.readAll();

    bool coreFileFound = f.exists();
    if (!coreFileFound) {
        // Try to use coredumpctl
        auto coredumpctl = QStandardPaths::findExecutable(QStringLiteral("coredumpctl"));
        if (!coredumpctl.isEmpty()) {
            KProcess::execute(coredumpctl, {"-1", "-o", f.absoluteFilePath(), "dump", "debuggee_crash"}, 5000);
            coreFileFound = f.exists();
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
    COMPARE_DATA(tIdx, "#1 at foo");

    session->stopDebugger();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}


KDevelop::VariableCollection *variableCollection()
{
    return KDevelop::ICore::self()->debugController()->variableCollection();
}

void GdbTest::testVariablesLocals()
{
    TestDebugSession *session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 22);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(1000);

    QCOMPARE(variableCollection()->rowCount(), 2);
    QModelIndex i = variableCollection()->index(1, 0);
    COMPARE_DATA(i, "Locals");
    QCOMPARE(variableCollection()->rowCount(i), 2);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "i");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "0");
    COMPARE_DATA(variableCollection()->index(1, 0, i), "j");
    // COMPARE_DATA(variableCollection()->index(1, 1, i), "1"); // j is not initialized yet
    session->run();
    QTest::qWait(1000);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "i");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "1");
    COMPARE_DATA(variableCollection()->index(1, 0, i), "j");
    COMPARE_DATA(variableCollection()->index(1, 1, i), "1");
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testVariablesLocalsStruct()
{
    TestDebugSession *session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 38);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(1000);

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
    QTest::qWait(100);
    COMPARE_DATA(variableCollection()->index(0, 0, ts), "a");
    COMPARE_DATA(variableCollection()->index(0, 1, ts), "0");
    COMPARE_DATA(variableCollection()->index(1, 0, ts), "b");
    COMPARE_DATA(variableCollection()->index(1, 1, ts), "1");
    COMPARE_DATA(variableCollection()->index(2, 0, ts), "c");
    COMPARE_DATA(variableCollection()->index(2, 1, ts), "2");

    session->stepInto();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(1000);
    COMPARE_DATA(variableCollection()->index(structIndex, 0, i), "ts");
    COMPARE_DATA(variableCollection()->index(structIndex, 1, i), "{...}");
    COMPARE_DATA(variableCollection()->index(0, 1, ts), "1");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testVariablesWatches()
{
    TestDebugSession *session = new TestDebugSession;
    KDevelop::ICore::self()->debugController()->variableCollection()->variableWidgetShown();

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 38);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    variableCollection()->watches()->add(QStringLiteral("ts"));
    QTest::qWait(300);

    QModelIndex i = variableCollection()->index(0, 0);
    QCOMPARE(variableCollection()->rowCount(i), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "ts");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "{...}");
    QModelIndex ts = variableCollection()->index(0, 0, i);
    COMPARE_DATA(variableCollection()->index(0, 0, ts), "...");
    variableCollection()->expanded(ts);
    QTest::qWait(100);
    COMPARE_DATA(variableCollection()->index(0, 0, ts), "a");
    COMPARE_DATA(variableCollection()->index(0, 1, ts), "0");
    COMPARE_DATA(variableCollection()->index(1, 0, ts), "b");
    COMPARE_DATA(variableCollection()->index(1, 1, ts), "1");
    COMPARE_DATA(variableCollection()->index(2, 0, ts), "c");
    COMPARE_DATA(variableCollection()->index(2, 1, ts), "2");

    session->stepInto();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(100);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "ts");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "{...}");
    COMPARE_DATA(variableCollection()->index(0, 1, ts), "1");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testVariablesWatchesQuotes()
{
    TestDebugSession *session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateWatches);

    TestLaunchConfiguration cfg;

    // the unquoted string (the actual content):               t\"t
    // quoted string (what we would write as a c string):     "t\\\"t"
    // written in source file:                             R"("t\\\"t")"
    const QString testString(QStringLiteral("t\\\"t")); // the actual content
    const QString quotedTestString(QStringLiteral(R"("t\\\"t")"));

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 38);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    variableCollection()->watches()->add(quotedTestString); //just a constant string
    QTest::qWait(300);

    QModelIndex i = variableCollection()->index(0, 0);
    QCOMPARE(variableCollection()->rowCount(i), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, i), quotedTestString);
    COMPARE_DATA(variableCollection()->index(0, 1, i), "[" + QString::number(testString.length() + 1) + "]");

    QModelIndex testStr = variableCollection()->index(0, 0, i);
    COMPARE_DATA(variableCollection()->index(0, 0, testStr), "...");
    variableCollection()->expanded(testStr);
    QTest::qWait(100);
    int len = testString.length();
    for (int ind = 0; ind < len; ind++)
    {
        COMPARE_DATA(variableCollection()->index(ind, 0, testStr), QString::number(ind));
        QChar c = testString.at(ind);
        QString value = QString::number(c.toLatin1()) + " '";
        if (c == '\\')
            value += QLatin1String("\\\\");
        else if (c == '\'')
            value += QLatin1String("\\'");
        else
            value += c;
        value += QLatin1String("'");
        COMPARE_DATA(variableCollection()->index(ind, 1, testStr), value);
    }
    COMPARE_DATA(variableCollection()->index(len, 0, testStr), QString::number(len));
    COMPARE_DATA(variableCollection()->index(len, 1, testStr), "0 '\\000'");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testVariablesWatchesTwoSessions()
{
    TestDebugSession *session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateWatches);

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 38);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    variableCollection()->watches()->add(QStringLiteral("ts"));
    QTest::qWait(300);

    QModelIndex ts = variableCollection()->index(0, 0, variableCollection()->index(0, 0));
    variableCollection()->expanded(ts);
    QTest::qWait(100);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);

    //check if variable is marked as out-of-scope
    QCOMPARE(variableCollection()->watches()->childCount(), 1);
    KDevelop::Variable* v = dynamic_cast<KDevelop::Variable*>(variableCollection()->watches()->child(0));
    QVERIFY(v);
    QVERIFY(!v->inScope());
    QCOMPARE(v->childCount(), 3);
    v = dynamic_cast<KDevelop::Variable*>(v->child(0));
    QVERIFY(!v->inScope());

    //start a second debug session
    session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateWatches);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(300);

    QCOMPARE(variableCollection()->watches()->childCount(), 1);
    ts = variableCollection()->index(0, 0, variableCollection()->index(0, 0));
    v = dynamic_cast<KDevelop::Variable*>(variableCollection()->watches()->child(0));
    QVERIFY(v);
    QVERIFY(v->inScope());
    QCOMPARE(v->childCount(), 3);

    v = dynamic_cast<KDevelop::Variable*>(v->child(0));
    QVERIFY(v->inScope());
    QCOMPARE(v->data(1, Qt::DisplayRole).toString(), QString::number(0));

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);

    //check if variable is marked as out-of-scope
    v = dynamic_cast<KDevelop::Variable*>(variableCollection()->watches()->child(0));
    QVERIFY(!v->inScope());
    QVERIFY(!dynamic_cast<KDevelop::Variable*>(v->child(0))->inScope());
}

void GdbTest::testVariablesStopDebugger()
{
    TestDebugSession *session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 38);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    session->stopDebugger();
    QTest::qWait(300);
}


void GdbTest::testVariablesStartSecondSession()
{
    QPointer<TestDebugSession> session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 38);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    QPointer<TestDebugSession> session2 = new TestDebugSession;
    session2->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 38);
    QVERIFY(session2->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session2, DebugSession::PausedState);

    session2->run();
    WAIT_FOR_STATE(session2, DebugSession::EndedState);
}

void GdbTest::testVariablesSwitchFrame()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);
    TestFrameStackModel *stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 24);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(500);

    QModelIndex i = variableCollection()->index(1, 0);
    COMPARE_DATA(i, "Locals");
    QCOMPARE(variableCollection()->rowCount(i), 2);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "i");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "1");
    COMPARE_DATA(variableCollection()->index(1, 0, i), "j");

    stackModel->setCurrentFrame(1);
    QTest::qWait(200);

    i = variableCollection()->index(1, 0);
    QCOMPARE(variableCollection()->rowCount(i), 4);
    COMPARE_DATA(variableCollection()->index(2, 0, i), "argc");
    COMPARE_DATA(variableCollection()->index(2, 1, i), "1");
    COMPARE_DATA(variableCollection()->index(3, 0, i), "argv");

    breakpoints()->removeRow(0);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testVariablesQuicklySwitchFrame()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);
    TestFrameStackModel *stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 24);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(500);

    QModelIndex i = variableCollection()->index(1, 0);
    COMPARE_DATA(i, "Locals");
    QCOMPARE(variableCollection()->rowCount(i), 2);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "i");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "1");
    COMPARE_DATA(variableCollection()->index(1, 0, i), "j");

    stackModel->setCurrentFrame(1);
    QTest::qWait(300);
    stackModel->setCurrentFrame(0);
    QTest::qWait(1);
    stackModel->setCurrentFrame(1);
    QTest::qWait(1);
    stackModel->setCurrentFrame(0);
    QTest::qWait(1);
    stackModel->setCurrentFrame(1);
    QTest::qWait(500);

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


void GdbTest::testSegfaultDebugee()
{
    TestDebugSession *session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);
    TestLaunchConfiguration cfg(findExecutable(QStringLiteral("debuggee_crash")));
    QString fileName = findSourceFile(QStringLiteral("debugeecrash.cpp"));

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 23);

    QVERIFY(session->startDebugging(&cfg, m_iface));

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 23);
    session->run();

    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 24);

    session->stopDebugger();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testSwitchFrameGdbConsole()
{
    TestDebugSession *session = new TestDebugSession;

    TestLaunchConfiguration cfg;

    TestFrameStackModel *stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 24);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(stackModel->currentFrame(), 0);
    stackModel->setCurrentFrame(1);
    QCOMPARE(stackModel->currentFrame(), 1);
    QTest::qWait(500);
    QCOMPARE(stackModel->currentFrame(), 1);

    session->addUserCommand(QStringLiteral("print x"));
    QTest::qWait(500);
    //currentFrame must not reset to 0; Bug 222882
    QCOMPARE(stackModel->currentFrame(), 1);

}

//Bug 201771
void GdbTest::testInsertAndRemoveBreakpointWhileRunning()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg(findExecutable(QStringLiteral("debuggee_debugeeslow")));
    QString fileName = findSourceFile(QStringLiteral("debugeeslow.cpp"));

    session->startDebugging(&cfg, m_iface);

    WAIT_FOR_STATE(session, DebugSession::ActiveState);
    QTest::qWait(2000);
    qDebug() << "adding breakpoint";
    KDevelop::Breakpoint *b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 25);
    b->setDeleted();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

//Bug 274390
void GdbTest::testCommandOrderFastStepping()
{
    TestDebugSession *session = new TestDebugSession;

    TestLaunchConfiguration cfg(findExecutable(QStringLiteral("debuggee_debugeeqt")));

    breakpoints()->addCodeBreakpoint(QStringLiteral("main"));
    QVERIFY(session->startDebugging(&cfg, m_iface));
    for(int i=0; i<20; i++) {
        session->stepInto();
    }
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testPickupManuallyInsertedBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QStringLiteral("main"));
    QVERIFY(session->startDebugging(&cfg, m_iface));
    session->addCommand(MI::NonMI, QStringLiteral("break debugee.cpp:32"));
    session->stepInto();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(1000); //wait for breakpoints update
    QCOMPARE(breakpoints()->breakpoints().count(), 2);
    QCOMPARE(breakpoints()->rowCount(), 2);
    KDevelop::Breakpoint *b = breakpoints()->breakpoint(1);
    QVERIFY(b);
    QCOMPARE(b->line(), 31); //we start with 0, gdb with 1
    QCOMPARE(b->url().fileName(), QString("debugee.cpp"));
}

//Bug 270970
void GdbTest::testPickupManuallyInsertedBreakpointOnlyOnce()
{
    TestDebugSession *session = new TestDebugSession;

    //inject here, so it behaves similar like a command from .gdbinit
    QTemporaryFile configScript;
    configScript.open();
    configScript.write(QStringLiteral("file %0\n").arg(findExecutable(QStringLiteral("debuggee_debugee")).toLocalFile()).toLocal8Bit());
    configScript.write("break debugee.cpp:32\n");
    configScript.close();

    TestLaunchConfiguration cfg;
    KConfigGroup grp = cfg.config();
    grp.writeEntry(Config::RemoteGdbConfigEntry, QUrl::fromLocalFile(configScript.fileName()));

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(QStringLiteral("debugee.cpp")), 31);
    QVERIFY(session->startDebugging(&cfg, m_iface));

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(breakpoints()->breakpoints().count(), 1);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testPickupCatchThrowOnlyOnce()
{
    QTemporaryFile configScript;
    configScript.open();
    configScript.write("catch throw\n");
    configScript.close();

    TestLaunchConfiguration cfg;
    KConfigGroup grp = cfg.config();
    grp.writeEntry(Config::RemoteGdbConfigEntry, QUrl::fromLocalFile(configScript.fileName()));


    for (int i = 0; i < 2; ++i) {
        TestDebugSession* session = new TestDebugSession;
        QVERIFY(session->startDebugging(&cfg, m_iface));
        WAIT_FOR_STATE(session, DebugSession::EndedState);
    }

    QCOMPARE(breakpoints()->rowCount(), 1); //one from kdevelop, one from runScript
}

void GdbTest::testRunGdbScript()
{
    TestDebugSession *session = new TestDebugSession;

    QTemporaryFile runScript;
    runScript.open();

    runScript.write("file " + findExecutable(QStringLiteral("debuggee_debugee")).toLocalFile().toUtf8() + "\n");
    runScript.write("break main\n");
    runScript.write("run\n");
    runScript.close();

    TestLaunchConfiguration cfg;
    KConfigGroup grp = cfg.config();
    grp.writeEntry(Config::RemoteGdbRunEntry, QUrl::fromLocalFile(runScript.fileName()));

    QVERIFY(session->startDebugging(&cfg, m_iface));

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(session->line(), 27);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testRemoteDebug()
{
    const QString gdbserverExecutable = QStandardPaths::findExecutable(QStringLiteral("gdbserver"));
    if (gdbserverExecutable.isEmpty()) {
        QSKIP("Skipping, gdbserver not available", SkipSingle);
    }

    TestDebugSession *session = new TestDebugSession;

    QTemporaryFile shellScript(QDir::currentPath()+"/shellscript");
    shellScript.open();
    shellScript.write("gdbserver localhost:2345 " + findExecutable(QStringLiteral("debuggee_debugee")).toLocalFile().toUtf8() + "\n");
    shellScript.close();
    shellScript.setPermissions(shellScript.permissions() | QFile::ExeUser);
    QFile::copy(shellScript.fileName(), shellScript.fileName()+"-copy"); //to avoid "Text file busy" on executing (why?)

    QTemporaryFile runScript(QDir::currentPath()+"/runscript");
    runScript.open();
    runScript.write("file " + findExecutable(QStringLiteral("debuggee_debugee")).toLocalFile().toUtf8() + "\n");
    runScript.write("target remote localhost:2345\n");
    runScript.write("break debugee.cpp:30\n");
    runScript.write("continue\n");
    runScript.close();

    TestLaunchConfiguration cfg;
    KConfigGroup grp = cfg.config();
    grp.writeEntry(Config::RemoteGdbShellEntry, QUrl::fromLocalFile((shellScript.fileName()+"-copy")));
    grp.writeEntry(Config::RemoteGdbRunEntry, QUrl::fromLocalFile(runScript.fileName()));

    QVERIFY(session->startDebugging(&cfg, m_iface));

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(session->line(), 29);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);

    QFile::remove(shellScript.fileName()+"-copy");
}

void GdbTest::testRemoteDebugInsertBreakpoint()
{
    const QString gdbserverExecutable = QStandardPaths::findExecutable(QStringLiteral("gdbserver"));
    if (gdbserverExecutable.isEmpty()) {
        QSKIP("Skipping, gdbserver not available", SkipSingle);
    }

    TestDebugSession *session = new TestDebugSession;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 29);
    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 35);

    QTemporaryFile shellScript(QDir::currentPath()+"/shellscript");
    shellScript.open();
    shellScript.write("gdbserver localhost:2345 " + findExecutable(QStringLiteral("debuggee_debugee")).toLocalFile().toUtf8() + "\n");
    shellScript.close();
    shellScript.setPermissions(shellScript.permissions() | QFile::ExeUser);
    QFile::copy(shellScript.fileName(), shellScript.fileName()+"-copy"); //to avoid "Text file busy" on executing (why?)

    QTemporaryFile runScript(QDir::currentPath()+"/runscript");
    runScript.open();
    runScript.write("file " + findExecutable(QStringLiteral("debuggee_debugee")).toLocalFile().toUtf8() + '\n');
    runScript.write("target remote localhost:2345\n");
    runScript.write("break debugee.cpp:30\n");
    runScript.write("continue\n");
    runScript.close();

    TestLaunchConfiguration cfg;
    KConfigGroup grp = cfg.config();
    grp.writeEntry(Config::RemoteGdbShellEntry, QUrl::fromLocalFile(shellScript.fileName()+"-copy"));
    grp.writeEntry(Config::RemoteGdbRunEntry, QUrl::fromLocalFile(runScript.fileName()));

    QVERIFY(session->startDebugging(&cfg, m_iface));

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(session->line(), 29);

    QCOMPARE(breakpoints()->breakpoints().count(), 2); //one from kdevelop, one from runScript

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    QCOMPARE(session->line(), 35);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);

    QFile::remove(shellScript.fileName()+"-copy");
}


void GdbTest::testRemoteDebugInsertBreakpointPickupOnlyOnce()
{
    const QString gdbserverExecutable = QStandardPaths::findExecutable(QStringLiteral("gdbserver"));
    if (gdbserverExecutable.isEmpty()) {
        QSKIP("Skipping, gdbserver not available", SkipSingle);
    }

    TestDebugSession *session = new TestDebugSession;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 35);

    QTemporaryFile shellScript(QDir::currentPath()+"/shellscript");
    shellScript.open();
    shellScript.write("gdbserver localhost:2345 "+findExecutable(QStringLiteral("debuggee_debugee")).toLocalFile().toLatin1()+"\n");
    shellScript.close();
    shellScript.setPermissions(shellScript.permissions() | QFile::ExeUser);
    QFile::copy(shellScript.fileName(), shellScript.fileName()+"-copy"); //to avoid "Text file busy" on executing (why?)

    QTemporaryFile runScript(QDir::currentPath()+"/runscript");
    runScript.open();
    runScript.write("file "+findExecutable(QStringLiteral("debuggee_debugee")).toLocalFile().toLatin1()+"\n");
    runScript.write("target remote localhost:2345\n");
    runScript.write("break debugee.cpp:30\n");
    runScript.write("continue\n");
    runScript.close();

    TestLaunchConfiguration cfg;
    KConfigGroup grp = cfg.config();
    grp.writeEntry(Config::RemoteGdbShellEntry, QUrl::fromLocalFile((shellScript.fileName()+"-copy")));
    grp.writeEntry(Config::RemoteGdbRunEntry, QUrl::fromLocalFile(runScript.fileName()));

    QVERIFY(session->startDebugging(&cfg, m_iface));

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(session->line(), 29);

    QCOMPARE(breakpoints()->breakpoints().count(), 2); //one from kdevelop, one from runScript

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    QCOMPARE(session->line(), 35);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);

    //************************** second session
    session = new TestDebugSession;
    QVERIFY(session->startDebugging(&cfg, m_iface));

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(session->line(), 29);

    QCOMPARE(breakpoints()->breakpoints().count(), 2); //one from kdevelop, one from runScript

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    QCOMPARE(session->line(), 35);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);

    QFile::remove(shellScript.fileName()+"-copy");
}

void GdbTest::testBreakpointWithSpaceInPath()
{
    TestDebugSession *session = new TestDebugSession;

    TestLaunchConfiguration cfg(findExecutable(QStringLiteral("debuggee_debugeespace")));
    KConfigGroup grp = cfg.config();
    QString fileName = findSourceFile(QStringLiteral("debugee space.cpp"));

    KDevelop::Breakpoint * b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 20);
    QCOMPARE(b->state(), KDevelop::Breakpoint::NotStartedState);

    session->startDebugging(&cfg, m_iface);
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 20);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testBreakpointDisabledOnStart()
{
    TestDebugSession *session = new TestDebugSession;

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 28)
        ->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);
    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 29);
    KDevelop::Breakpoint* b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 31);
    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);

    session->startDebugging(&cfg, m_iface);
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 29);
    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Checked);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 31);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testCatchpoint()
{
    TestDebugSession *session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);

    TestLaunchConfiguration cfg(findExecutable(QStringLiteral("debuggee_debugeeexception")));

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile(QStringLiteral("debugeeexception.cpp"))), 29);

    session->startDebugging(&cfg, m_iface);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(1000);
    TestFrameStackModel* fsModel = session->frameStackModel();
    QCOMPARE(fsModel->currentFrame(), 0);
    QCOMPARE(session->line(), 29);

    session->addCommand(MI::NonMI, QStringLiteral("catch throw"));
    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(1000);

    const QList<KDevelop::FrameStackModel::FrameItem> frames = fsModel->frames(fsModel->currentThread());
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

void GdbTest::testThreadAndFrameInfo()
{
    // Check if --thread is added to user commands

    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg(findExecutable(QStringLiteral("debuggee_debugeethreads")));
    QString fileName = findSourceFile(QStringLiteral("debugeethreads.cpp"));

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 38);
    QVERIFY(session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QSignalSpy outputSpy(session, &TestDebugSession::debuggerUserCommandOutput);

    session->addCommand(new MI::UserCommand(MI::ThreadInfo,QLatin1String("")));
    session->addCommand(new MI::UserCommand(MI::StackListLocals, QStringLiteral("0")));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState); // wait for command finish

    // outputs should be
    // 1. -thread-info
    // 2. ^done for thread-info
    // 3. -stack-list-locals
    // 4. ^done for -stack-list-locals
    QCOMPARE(outputSpy.count(), 4);
    QVERIFY(outputSpy.at(2).at(0).toString().contains(QLatin1String("--thread 1")));

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::parseBug304730()
{
    MI::FileSymbol file;
    file.contents = QByteArray("^done,bkpt={"
        "number=\"1\",type=\"breakpoint\",disp=\"keep\",enabled=\"y\",addr=\"<MULTIPLE>\",times=\"0\","
        "original-location=\"/media/portable/Projects/BDSInpainting/PatchMatch/PatchMatch.hpp:231\"},"
        "{number=\"1.1\",enabled=\"y\",addr=\"0x081d84aa\","
        "func=\"PatchMatch<itk::Image<itk::CovariantVector<unsigned char, 3u>, 2u> >"
        "::Propagation<ForwardPropagationNeighbors>(ForwardPropagationNeighbors)\","
        "file=\"/media/portable/Projects/BDSInpainting/Drivers/../PatchMatch/PatchMatch.hpp\","
        "fullname=\"/media/portable/Projects/BDSInpainting/PatchMatch/PatchMatch.hpp\",line=\"231\"},"
        "{number=\"1.2\",enabled=\"y\",addr=\"0x081d8ae2\","
        "func=\"PatchMatch<itk::Image<itk::CovariantVector<unsigned char, 3u>, 2u> >"
        "::Propagation<BackwardPropagationNeighbors>(BackwardPropagationNeighbors)\","
        "file=\"/media/portable/Projects/BDSInpainting/Drivers/../PatchMatch/PatchMatch.hpp\","
        "fullname=\"/media/portable/Projects/BDSInpainting/PatchMatch/PatchMatch.hpp\",line=\"231\"},"
        "{number=\"1.3\",enabled=\"y\",addr=\"0x081d911a\","
        "func=\"PatchMatch<itk::Image<itk::CovariantVector<unsigned char, 3u>, 2u> >"
        "::Propagation<AllowedPropagationNeighbors>(AllowedPropagationNeighbors)\","
        "file=\"/media/portable/Projects/BDSInpainting/Drivers/../PatchMatch/PatchMatch.hpp\","
        "fullname=\"/media/portable/Projects/BDSInpainting/PatchMatch/PatchMatch.hpp\",line=\"231\"}");

    MI::MIParser parser;

    std::unique_ptr<MI::Record> record(parser.parse(&file));
    QVERIFY(record.get() != nullptr);
}

void GdbTest::testMultipleLocationsBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;

    TestLaunchConfiguration cfg(findExecutable(QStringLiteral("debuggee_debugeemultilocbreakpoint")));

    breakpoints()->addCodeBreakpoint(QStringLiteral("aPlusB"));

    //TODO check if the additional location breakpoint is added

    session->startDebugging(&cfg, m_iface);
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 19);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 23);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testBug301287()
{
    TestDebugSession *session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateWatches);

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 28);

    session->startDebugging(&cfg, m_iface);
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    variableCollection()->watches()->add(QStringLiteral("argc"));
    QTest::qWait(300);

    QModelIndex i = variableCollection()->index(0, 0);
    QCOMPARE(variableCollection()->rowCount(i), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "argc");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "1");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);

    //start second debug session (same cfg)
    session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateWatches);

    session->startDebugging(&cfg, m_iface);
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    QTest::qWait(300);

    i = variableCollection()->index(0, 0);
    QCOMPARE(variableCollection()->rowCount(i), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "argc");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "1");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testMultipleBreakpoint()
{
        TestDebugSession *session = new TestDebugSession;

        //there'll be about 3-4 breakpoints, but we treat it like one.
        TestLaunchConfiguration c(findExecutable(QStringLiteral("debuggee_debugeemultiplebreakpoint")));
        KDevelop::Breakpoint *b = breakpoints()->addCodeBreakpoint(QStringLiteral("debugeemultiplebreakpoint.cpp:52"));
        session->startDebugging(&c, m_iface);
        WAIT_FOR_STATE(session, DebugSession::PausedState);
        QCOMPARE(breakpoints()->breakpoints().count(), 1);

        b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);
        session->run();
        WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testRegularExpressionBreakpoint()
{
        TestDebugSession *session = new TestDebugSession;

        TestLaunchConfiguration c(findExecutable(QStringLiteral("debuggee_debugeemultilocbreakpoint")));
        breakpoints()->addCodeBreakpoint(QStringLiteral("main"));
        session->startDebugging(&c, m_iface);
        WAIT_FOR_STATE(session, DebugSession::PausedState);
        session->addCommand(MI::NonMI, QStringLiteral("rbreak .*aPl.*B"));
        QTest::qWait(100);
        session->run();
        WAIT_FOR_STATE(session, DebugSession::PausedState);
        QCOMPARE(breakpoints()->breakpoints().count(), 3);

        session->addCommand(MI::BreakDelete, QLatin1String(""));
        session->run();
        WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testChangeBreakpointWhileRunning() {

    TestDebugSession *session = new TestDebugSession;

    TestLaunchConfiguration c(findExecutable(QStringLiteral("debuggee_debugeeslow")));
    KDevelop::Breakpoint* b = breakpoints()->addCodeBreakpoint(QStringLiteral("debugeeslow.cpp:25"));
    session->startDebugging(&c, m_iface);

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QVERIFY(session->currentLine() >= 24 && session->currentLine() <= 26 );
    session->run();
    WAIT_FOR_STATE(session, DebugSession::ActiveState);
    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);
    //to make one loop
    QTest::qWait(2000);
    WAIT_FOR_STATE(session, DebugSession::ActiveState);

    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Checked);
    QTest::qWait(100);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);
    session->run();
    QTest::qWait(100);
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testDebugInExternalTerminal()
{
    TestLaunchConfiguration cfg;

    foreach (const QString & console, QStringList() << "konsole" << "xterm" << "xfce4-terminal" << "gnome-terminal") {

        TestDebugSession* session = nullptr;
        if (QStandardPaths::findExecutable(console).isEmpty()) {
            continue;
        }

        session = new TestDebugSession();

        cfg.config().writeEntry("External Terminal"/*ExecutePlugin::terminalEntry*/, console);
        cfg.config().writeEntry("Use External Terminal"/*ExecutePlugin::useTerminalEntry*/, true);

        KDevelop::Breakpoint* b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 28);

        session->startDebugging(&cfg, m_iface);
        WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
        QCOMPARE(b->state(), KDevelop::Breakpoint::CleanState);
        session->stepInto();
        WAIT_FOR_STATE(session, DebugSession::PausedState);
        session->run();
        WAIT_FOR_STATE(session, DebugSession::EndedState);
    }
}

// see: https://bugs.kde.org/show_bug.cgi?id=339231
void GdbTest::testPathWithSpace()
{
    TestDebugSession* session = new TestDebugSession;

    auto debugee = findExecutable(QStringLiteral("path with space/debuggee_spacedebugee"));
    TestLaunchConfiguration c(debugee, KIO::upUrl(debugee));
    KDevelop::Breakpoint* b = breakpoints()->addCodeBreakpoint(QStringLiteral("spacedebugee.cpp:30"));
    QCOMPARE(b->state(), KDevelop::Breakpoint::NotStartedState);
    session->startDebugging(&c, m_iface);

    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(b->state(), KDevelop::Breakpoint::CleanState);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

bool GdbTest::waitForState(DebugSession *session, DebugSession::DebuggerState state,
                            const char *file, int line, bool waitForIdle)
{
    QPointer<MIDebugSession> s(session); //session can get deleted in DebugController
    QTime stopWatch;
    stopWatch.start();

    // legacy behavior for tests that implicitly may require waiting for idle,
    // but which were written before waitForIdle was added
    waitForIdle = waitForIdle || state != MIDebugSession::EndedState;

    while (s && (s->state() != state || (waitForIdle && s->debuggerStateIsOn(s_dbgBusy)))) {
        if (stopWatch.elapsed() > 5000) {
            qWarning() << "current state" << s->state() << "waiting for" << state;
            QTest::qFail(qPrintable(QString("Timeout before reaching state %0").arg(state)),
                         file, line);
            return false;
        }
        QTest::qWait(20);
    }

    // NOTE: don't wait anymore after leaving the loop. Waiting re-enters event loop and
    // may change session state.

    if (!s && state != MIDebugSession::EndedState) {
        QTest::qFail(qPrintable(QString("Session ended before reaching state %0").arg(state)),
                     file, line);
        return false;
    }

    qDebug() << "Reached state " << state << " in " << file << ':' << line;
    return true;
}
} // end of namespace GDB
} // end of namespace KDevMI

QTEST_MAIN(KDevMI::GDB::GdbTest)


#include "test_gdb.moc"
#include "moc_test_gdb.cpp"

