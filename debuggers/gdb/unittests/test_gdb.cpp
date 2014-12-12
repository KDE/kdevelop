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

#include <QtTest/QTest>
#include <QSignalSpy>
#include <QDebug>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QTemporaryFile>

#include <KSharedConfig>
#include <KProcess>
#include <KIO/Global>

#include <tests/testcore.h>
#include <shell/shellextension.h>
#include <debugger/breakpoint/breakpointmodel.h>
#include <interfaces/idebugcontroller.h>
#include <debugger/breakpoint/breakpoint.h>
#include <debugger/interfaces/ibreakpointcontroller.h>
#include <interfaces/ilaunchconfiguration.h>
#include <interfaces/iplugincontroller.h>
#include <debugger/variable/variablecollection.h>
#include <debugger/interfaces/ivariablecontroller.h>
#include <debugger/framestack/framestackmodel.h>
#include <tests/autotestshell.h>
#include <execute/iexecuteplugin.h>

#include "gdbcommand.h"
#include "debugsession.h"
#include "gdbframestackmodel.h"
#include <mi/milexer.h>
#include <mi/miparser.h>
#include <QStandardPaths>

using KDevelop::AutoTestShell;

namespace GDBDebugger {

QUrl findExecutable(const QString& name)
{
    QFileInfo info(qApp->applicationDirPath()  + "/unittests/" + name);
    Q_ASSERT(info.exists());
    Q_ASSERT(info.isExecutable());
    return QUrl::fromLocalFile(info.canonicalFilePath());
}

QString findSourceFile(const QString& name)
{
    QFileInfo info(QFileInfo(__FILE__).dir().absoluteFilePath(name));
    Q_ASSERT(info.exists());
    return info.canonicalFilePath();
}

static bool isAttachForbidden(const char * file, int line)
{
    // if on linux, ensure we can actually attach
    QFile canRun("/proc/sys/kernel/yama/ptrace_scope");
    if (canRun.exists()) {
        if (!canRun.open(QIODevice::ReadOnly)) {
            QTest::qFail("Something is wrong: /proc/sys/kernel/yama/ptrace_scope exists but cannot be read", file, line);
            return true;
        }
        if (canRun.read(1).toInt() != 0) {
            QTest::qSkip("ptrace attaching not allowed, skipping test. To enable it, set /proc/sys/kernel/yama/ptrace_scope to 0.", file, line);
            return true;
        }
    }

    return false;
}

#define SKIP_IF_ATTACH_FORBIDDEN() \
    do { \
        if (isAttachForbidden(__FILE__, __LINE__)) \
            return; \
    } while(0)

void GdbTest::initTestCase()
{
    AutoTestShell::init();
    KDevelop::TestCore::initialize(KDevelop::Core::NoUi);

    m_iface = KDevelop::ICore::self()->pluginController()->pluginForExtension("org.kdevelop.IExecutePlugin", "kdevexecute")->extension<IExecutePlugin>();
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

class TestLaunchConfiguration : public KDevelop::ILaunchConfiguration
{
public:
    TestLaunchConfiguration(const QUrl& executable = findExecutable("debugee"),
                            const QUrl& workingDirectory = QUrl()) {
        qDebug() << "FIND" << executable;
        c = new KConfig();
        c->deleteGroup("launch");
        cfg = c->group("launch");
        cfg.writeEntry("isExecutable", true);
        cfg.writeEntry("Executable", executable);
        cfg.writeEntry("Working Directory", workingDirectory);
    }
    ~TestLaunchConfiguration() {
        delete c;
    }
    virtual const KConfigGroup config() const { return cfg; }
    virtual KConfigGroup config() { return cfg; };
    virtual QString name() const { return QString("Test-Launch"); }
    virtual KDevelop::IProject* project() const { return 0; }
    virtual KDevelop::LaunchConfigurationType* type() const { return 0; }
private:
    KConfigGroup cfg;
    KConfig *c;
};

class TestFrameStackModel : public KDevelop::GdbFrameStackModel
{
public:

    TestFrameStackModel(DebugSession* session)
        : GdbFrameStackModel(session), fetchFramesCalled(0), fetchThreadsCalled(0) {}

    int fetchFramesCalled;
    int fetchThreadsCalled;
    virtual void fetchFrames(int threadNumber, int from, int to)
    {
        fetchFramesCalled++;
        GdbFrameStackModel::fetchFrames(threadNumber, from, to);
    }

    virtual void fetchThreads()
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
        setTesting(true);
        KDevelop::ICore::self()->debugController()->addSession(this);
    }

    KDevelop::IFrameStackModel* createFrameStackModel()
    {
        return new TestFrameStackModel(this);
    }

    QUrl url() { return currentUrl(); }
    int line() { return currentLine(); }
    TestFrameStackModel *frameStackModel() const
    { return static_cast<TestFrameStackModel*>(DebugSession::frameStackModel()); }


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
    waitForState((session), (state), __FILE__, __LINE__)

#define WAIT_FOR_STATE_AND_IDLE(session, state) \
    waitForState((session), (state), __FILE__, __LINE__, true)

#define WAIT_FOR(session, condition) \
    do { \
        TestWaiter w((session), #condition, __FILE__, __LINE__); \
        while (w.waitUnless((condition))) /* nothing */ ; \
    } while(0)

#define COMPARE_DATA(index, expected) \
    compareData((index), (expected), __FILE__, __LINE__)
void compareData(QModelIndex index, QString expected, const char *file, int line)
{
    QString s = index.model()->data(index, Qt::DisplayRole).toString();
    if (s != expected) {
        QFAIL(qPrintable(QString("'%0' didn't match expected '%1' in %2:%3").arg(s).arg(expected).arg(file).arg(line)));
    }
}

static const QString debugeeFileName = findSourceFile("debugee.cpp");

KDevelop::BreakpointModel* breakpoints()
{
    return KDevelop::ICore::self()->debugController()->breakpointModel();
}

void GdbTest::testStdOut()
{
    TestDebugSession *session = new TestDebugSession;

    QSignalSpy outputSpy(session, SIGNAL(applicationStandardOutputLines(QStringList)));

    TestLaunchConfiguration cfg;
    session->startProgram(&cfg, m_iface);
    WAIT_FOR_STATE(session, KDevelop::IDebugSession::EndedState);

    {
        QCOMPARE(outputSpy.count(), 1);
        QList<QVariant> arguments = outputSpy.takeFirst();
        QCOMPARE(arguments.count(), 1);
        QCOMPARE(arguments.first().toStringList(), QStringList() << "Hello, world!" << "Hello");
    }
}

void GdbTest::testBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;

    TestLaunchConfiguration cfg;

    KDevelop::Breakpoint * b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 28);
    QCOMPARE(session->breakpointController()->breakpointState(b), KDevelop::Breakpoint::NotStartedState);

    session->startProgram(&cfg, m_iface);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->breakpointController()->breakpointState(b), KDevelop::Breakpoint::CleanState);
    session->stepInto();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    session->stepInto();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
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


    session->startProgram(&cfg, m_iface);
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

    session->startProgram(&cfg, m_iface);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
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
    //add breakpoint before startProgram
    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 21);
    QCOMPARE(breakpoints()->rowCount(), 1);
    breakpoints()->removeRow(0);
    QCOMPARE(breakpoints()->rowCount(), 0);

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 22);

    session->startProgram(&cfg, m_iface);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    breakpoints()->removeRow(0);
    QTest::qWait(100);
    session->run();

    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testPendingBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 28);

    KDevelop::Breakpoint * b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile("test_gdb.cpp")), 10);
    QCOMPARE(session->breakpointController()->breakpointState(b), KDevelop::Breakpoint::NotStartedState);

    session->startProgram(&cfg, m_iface);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->breakpointController()->breakpointState(b), KDevelop::Breakpoint::PendingState);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testUpdateBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    KDevelop::Breakpoint * b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 28);
    QCOMPARE(breakpoints()->rowCount(), 1);

    session->startProgram(&cfg, m_iface);

    //insert custom command as user might do it using GDB console
    session->addCommand(new UserCommand(GDBMI::NonMI, "break "+debugeeFileName+":28"));

    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(100);
    session->stepInto();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(breakpoints()->rowCount(), 2);
    b = breakpoints()->breakpoint(1);
    QCOMPARE(b->url(), QUrl::fromLocalFile(debugeeFileName));
    QCOMPARE(b->line(), 27);
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

    session->startProgram(&cfg, m_iface);

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
    b->setCondition("x[0] == 'H'");

    b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 23);
    b->setCondition("i==2");

    b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 24);

    session->startProgram(&cfg, m_iface);

    WAIT_FOR(session, session->state() == DebugSession::PausedState && session->line() == 24);
    b->setCondition("i == 0");
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

    session->startProgram(&cfg, m_iface);

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 24);

    breakpoints()->addWatchpoint("i");
    QTest::qWait(100);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 23);
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

    session->startProgram(&cfg, m_iface);

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 24);

    KDevelop::Breakpoint *b = breakpoints()->addWatchpoint("i");
    b->setCondition("i==2");
    QTest::qWait(100);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 23);
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

    session->startProgram(&cfg, m_iface);

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

    session->startProgram(&cfg, m_iface);

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 24);

    breakpoints()->addReadWatchpoint("i");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 22);

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

    session->startProgram(&cfg, m_iface);

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 24);

    breakpoints()->addAccessWatchpoint("i");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 22);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 23);


    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 24);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testInsertBreakpointWhileRunning()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg(findExecutable("debugeeslow"));
    QString fileName = findSourceFile("debugeeslow.cpp");

    session->startProgram(&cfg, m_iface);

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
    TestLaunchConfiguration cfg(findExecutable("debugeeslow"));
    QString fileName = findSourceFile("debugeeslow.cpp");

    session->startProgram(&cfg, m_iface);

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

    breakpoints()->addCodeBreakpoint("main");

    session->startProgram(&cfg, m_iface);
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 27);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testShowStepInSource()
{
    TestDebugSession *session = new TestDebugSession;

    QSignalSpy showStepInSourceSpy(session, SIGNAL(showStepInSource(QUrl,int,QString)));

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 29);
    session->startProgram(&cfg, m_iface);
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
    QVERIFY(session->startProgram(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QModelIndex tIdx = stackModel->index(0,0);
    QCOMPARE(stackModel->rowCount(QModelIndex()), 1);
    QCOMPARE(stackModel->columnCount(QModelIndex()), 3);
    COMPARE_DATA(tIdx, "#1 at foo");

    QCOMPARE(stackModel->rowCount(tIdx), 2);
    QCOMPARE(stackModel->columnCount(tIdx), 3);
    COMPARE_DATA(tIdx.child(0, 0), "0");
    COMPARE_DATA(tIdx.child(0, 1), "foo");
    COMPARE_DATA(tIdx.child(0, 2), debugeeFileName+":23");
    COMPARE_DATA(tIdx.child(1, 0), "1");
    COMPARE_DATA(tIdx.child(1, 1), "main");
    COMPARE_DATA(tIdx.child(1, 2), debugeeFileName+":29");


    session->stepOut();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    COMPARE_DATA(tIdx, "#1 at main");
    QCOMPARE(stackModel->rowCount(tIdx), 1);
    COMPARE_DATA(tIdx.child(0, 0), "0");
    COMPARE_DATA(tIdx.child(0, 1), "main");
    COMPARE_DATA(tIdx.child(0, 2), debugeeFileName+":30");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testStackFetchMore()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg(findExecutable("debugeerecursion"));
    QString fileName = findSourceFile("debugeerecursion.cpp");

    TestFrameStackModel *stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 25);
    QVERIFY(session->startProgram(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(session->frameStackModel()->fetchFramesCalled, 1);

    QModelIndex tIdx = stackModel->index(0,0);
    QCOMPARE(stackModel->rowCount(QModelIndex()), 1);
    QCOMPARE(stackModel->columnCount(QModelIndex()), 3);
    COMPARE_DATA(tIdx, "#1 at foo");

    QCOMPARE(stackModel->rowCount(tIdx), 21);
    COMPARE_DATA(tIdx.child(0, 0), "0");
    COMPARE_DATA(tIdx.child(0, 1), "foo");
    COMPARE_DATA(tIdx.child(0, 2), fileName+":26");
    COMPARE_DATA(tIdx.child(1, 0), "1");
    COMPARE_DATA(tIdx.child(1, 1), "foo");
    COMPARE_DATA(tIdx.child(1, 2), fileName+":24");
    COMPARE_DATA(tIdx.child(2, 0), "2");
    COMPARE_DATA(tIdx.child(2, 1), "foo");
    COMPARE_DATA(tIdx.child(2, 2), fileName+":24");
    COMPARE_DATA(tIdx.child(19, 0), "19");
    COMPARE_DATA(tIdx.child(20, 0), "20");

    stackModel->fetchMoreFrames();
    QTest::qWait(200);
    QCOMPARE(stackModel->fetchFramesCalled, 2);
    QCOMPARE(stackModel->rowCount(tIdx), 41);
    COMPARE_DATA(tIdx.child(20, 0), "20");
    COMPARE_DATA(tIdx.child(21, 0), "21");
    COMPARE_DATA(tIdx.child(22, 0), "22");
    COMPARE_DATA(tIdx.child(39, 0), "39");
    COMPARE_DATA(tIdx.child(40, 0), "40");

    stackModel->fetchMoreFrames();
    QTest::qWait(200);
    QCOMPARE(stackModel->fetchFramesCalled, 3);
    QCOMPARE(stackModel->rowCount(tIdx), 121);
    COMPARE_DATA(tIdx.child(40, 0), "40");
    COMPARE_DATA(tIdx.child(41, 0), "41");
    COMPARE_DATA(tIdx.child(42, 0), "42");
    COMPARE_DATA(tIdx.child(119, 0), "119");
    COMPARE_DATA(tIdx.child(120, 0), "120");

    stackModel->fetchMoreFrames();
    QTest::qWait(200);
    QCOMPARE(stackModel->fetchFramesCalled, 4);
    QCOMPARE(stackModel->rowCount(tIdx), 301);
    COMPARE_DATA(tIdx.child(120, 0), "120");
    COMPARE_DATA(tIdx.child(121, 0), "121");
    COMPARE_DATA(tIdx.child(122, 0), "122");
    COMPARE_DATA(tIdx.child(300, 0), "300");
    COMPARE_DATA(tIdx.child(300, 1), "main");
    COMPARE_DATA(tIdx.child(300, 2), fileName+":30");

    stackModel->fetchMoreFrames(); //nothing to fetch, we are at the end
    QTest::qWait(200);
    QCOMPARE(stackModel->fetchFramesCalled, 4);
    QCOMPARE(stackModel->rowCount(tIdx), 301);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testStackDeactivateAndActive()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    TestFrameStackModel *stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 21);
    QVERIFY(session->startProgram(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    QModelIndex tIdx = stackModel->index(0,0);

    session->stepOut();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(200);
    COMPARE_DATA(tIdx, "#1 at main");
    QCOMPARE(stackModel->rowCount(tIdx), 1);
    COMPARE_DATA(tIdx.child(0, 0), "0");
    COMPARE_DATA(tIdx.child(0, 1), "main");
    COMPARE_DATA(tIdx.child(0, 2), debugeeFileName+":30");

    session->run();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testStackSwitchThread()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg(findExecutable("debugeethreads"));
    QString fileName = findSourceFile("debugeethreads.cpp");

    TestFrameStackModel *stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 38);
    QVERIFY(session->startProgram(&cfg, m_iface));
    QTest::qWait(500);
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    QCOMPARE(stackModel->rowCount(), 4);

    QModelIndex tIdx = stackModel->index(0,0);
    COMPARE_DATA(tIdx, "#1 at main");
    QCOMPARE(stackModel->rowCount(tIdx), 1);
    COMPARE_DATA(tIdx.child(0, 0), "0");
    COMPARE_DATA(tIdx.child(0, 1), "main");
    COMPARE_DATA(tIdx.child(0, 2), fileName+":39");

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

    QString fileName = findSourceFile("debugeeslow.cpp");

    KProcess debugeeProcess;
    debugeeProcess << "nice" << findExecutable("debugeeslow").toLocalFile();
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

    QString fileName = findSourceFile("debugeeslow.cpp");

    KProcess debugeeProcess;
    debugeeProcess << "nice" << findExecutable("debugeeslow").toLocalFile();
    debugeeProcess.start();
    QVERIFY(debugeeProcess.waitForStarted());

    TestDebugSession *session = new TestDebugSession;

    TestLaunchConfiguration cfg;
    cfg.config().writeEntry(GDBDebugger::remoteGdbRunEntry, QUrl::fromLocalFile(findSourceFile("gdb_script_empty")));
    QVERIFY(session->startProgram(&cfg, m_iface));

    session->addCommand(GDBMI::NonMI, QString("attach %0").arg(debugeeProcess.pid()));
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    session->run();
    QTest::qWait(2000); // give the slow inferior some extra time to run
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testCoreFile()
{
    QFile f("core");
    if (f.exists()) f.remove();

    KProcess debugeeProcess;
    debugeeProcess.setOutputChannelMode(KProcess::MergedChannels);
    debugeeProcess << "bash" << "-c" << "ulimit -c unlimited; " + findExecutable("debugeecrash").toLocalFile();
    debugeeProcess.start();
    debugeeProcess.waitForFinished();
    qDebug() << debugeeProcess.readAll();
    if (!QFile::exists("core")) {
        QSKIP("no core dump found, check your system configuration (see /proc/sys/kernel/core_pattern).", SkipSingle);
    }

    TestDebugSession *session = new TestDebugSession;
    session->examineCoreFile(findExecutable("debugeecrash"), QUrl::fromLocalFile(QDir::currentPath()+"/core"));

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
    QVERIFY(session->startProgram(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(1000);

    QCOMPARE(variableCollection()->rowCount(), 2);
    QModelIndex i = variableCollection()->index(1, 0);
    COMPARE_DATA(i, "Locals");
    QCOMPARE(variableCollection()->rowCount(i), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "i");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "0");
    session->run();
    QTest::qWait(1000);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "i");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "1");
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testVariablesLocalsStruct()
{
    TestDebugSession *session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 38);
    QVERIFY(session->startProgram(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(1000);

    QModelIndex i = variableCollection()->index(1, 0);
    QCOMPARE(variableCollection()->rowCount(i), 4);

    int structIndex = 0;
    for(int j=0; j<3; ++j) {
        if (variableCollection()->index(j, 0, i).data().toString() == "ts") {
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
    QVERIFY(session->startProgram(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    variableCollection()->watches()->add("ts");
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

    const QString testString("test");
    const QString quotedTestString("\"" + testString + "\"");

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 38);
    QVERIFY(session->startProgram(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);

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
        QString value = QString::number(c.toLatin1()) + " '" + c + "'";
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
    QVERIFY(session->startProgram(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    variableCollection()->watches()->add("ts");
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
    QVERIFY(session->startProgram(&cfg, m_iface));
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
    QVERIFY(session->startProgram(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    session->stopDebugger();
    QTest::qWait(300);
}


void GdbTest::testVariablesStartSecondSession()
{
    TestDebugSession *session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 38);
    QVERIFY(session->startProgram(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    session = new TestDebugSession;
    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 38);
    QVERIFY(session->startProgram(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testVariablesSwitchFrame()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg;

    session->variableController()->setAutoUpdate(KDevelop::IVariableController::UpdateLocals);
    TestFrameStackModel *stackModel = session->frameStackModel();

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 24);
    QVERIFY(session->startProgram(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(500);

    QModelIndex i = variableCollection()->index(1, 0);
    COMPARE_DATA(i, "Locals");
    QCOMPARE(variableCollection()->rowCount(i), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "i");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "1");

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
    QVERIFY(session->startProgram(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(500);

    QModelIndex i = variableCollection()->index(1, 0);
    COMPARE_DATA(i, "Locals");
    QCOMPARE(variableCollection()->rowCount(i), 1);
    COMPARE_DATA(variableCollection()->index(0, 0, i), "i");
    COMPARE_DATA(variableCollection()->index(0, 1, i), "1");

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
    TestLaunchConfiguration cfg(findExecutable("debugeecrash"));
    QString fileName = findSourceFile("debugeecrash.cpp");

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 23);

    QVERIFY(session->startProgram(&cfg, m_iface));

    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->line(), 23);
    session->run();

    WAIT_FOR_STATE(session, DebugSession::StoppedState);
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
    QVERIFY(session->startProgram(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
    QCOMPARE(stackModel->currentFrame(), 0);
    stackModel->setCurrentFrame(1);
    QCOMPARE(stackModel->currentFrame(), 1);
    QTest::qWait(500);
    QCOMPARE(stackModel->currentFrame(), 1);

    session->slotUserGDBCmd("print x");
    QTest::qWait(500);
    //currentFrame must not reset to 0; Bug 222882
    QCOMPARE(stackModel->currentFrame(), 1);

}

//Bug 201771
void GdbTest::testInsertAndRemoveBreakpointWhileRunning()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg(findExecutable("debugeeslow"));
    QString fileName = findSourceFile("debugeeslow.cpp");

    session->startProgram(&cfg, m_iface);

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

    TestLaunchConfiguration cfg(findExecutable("debugeeqt"));

    breakpoints()->addCodeBreakpoint("main");
    QVERIFY(session->startProgram(&cfg, m_iface));
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

    breakpoints()->addCodeBreakpoint("main");
    QVERIFY(session->startProgram(&cfg, m_iface));
    session->addCommand(GDBMI::NonMI, "break debugee.cpp:32");
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

    TestLaunchConfiguration cfg;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile("debugee.cpp"), 31);
    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile("debugee.cpp"), 21);
    QVERIFY(session->startProgram(&cfg, m_iface));

    //inject here, so it behaves similar like a command from .gdbinit
    session->addCommandToFront(new GDBCommand(GDBMI::NonMI, "break debugee.cpp:32"));
    session->addCommandToFront(new GDBCommand(GDBMI::NonMI, "break foo"));

    WAIT_FOR_STATE(session, DebugSession::PausedState);

    session->stepInto();
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(1000); //wait for breakpoints update
    QCOMPARE(breakpoints()->breakpoints().count(), 2);

    KDevelop::Breakpoint *b = breakpoints()->breakpoint(0);
    QVERIFY(b);
    QCOMPARE(b->line(), 31); //we start with 0, gdb with 1
    QCOMPARE(b->url().fileName(), QString("debugee.cpp"));

    b = breakpoints()->breakpoint(1);
    QVERIFY(b);
    QCOMPARE(b->line(), 21);
    QCOMPARE(b->url().fileName(), QString("debugee.cpp"));
}

void GdbTest::testRunGdbScript()
{
    TestDebugSession *session = new TestDebugSession;

    QTemporaryFile runScript;
    runScript.open();

    runScript.write("file " + findExecutable("debugee").toLocalFile().toUtf8() + "\n");
    runScript.write("break main\n");
    runScript.write("run\n");
    runScript.close();

    TestLaunchConfiguration cfg;
    KConfigGroup grp = cfg.config();
    grp.writeEntry(GDBDebugger::remoteGdbRunEntry, QUrl::fromLocalFile(runScript.fileName()));

    QVERIFY(session->startProgram(&cfg, m_iface));

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(session->line(), 27);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testRemoteDebug()
{
    const QString gdbserverExecutable = QStandardPaths::findExecutable("gdbserver");
    if (gdbserverExecutable.isEmpty()) {
        QSKIP("Skipping, gdbserver not available", SkipSingle);
    }

    TestDebugSession *session = new TestDebugSession;

    QTemporaryFile shellScript(QDir::currentPath()+"/shellscript");
    shellScript.open();
    shellScript.write("gdbserver localhost:2345 " + findExecutable("debugee").toLocalFile().toUtf8() + "\n");
    shellScript.close();
    shellScript.setPermissions(shellScript.permissions() | QFile::ExeUser);
    QFile::copy(shellScript.fileName(), shellScript.fileName()+"-copy"); //to avoid "Text file busy" on executing (why?)

    QTemporaryFile runScript(QDir::currentPath()+"/runscript");
    runScript.open();
    runScript.write("file " + findExecutable("debugee").toLocalFile().toUtf8() + "\n");
    runScript.write("target remote localhost:2345\n");
    runScript.write("break debugee.cpp:30\n");
    runScript.write("continue\n");
    runScript.close();

    TestLaunchConfiguration cfg;
    KConfigGroup grp = cfg.config();
    grp.writeEntry(GDBDebugger::remoteGdbShellEntry, QUrl::fromLocalFile((shellScript.fileName()+"-copy")));
    grp.writeEntry(GDBDebugger::remoteGdbRunEntry, QUrl::fromLocalFile(runScript.fileName()));

    QVERIFY(session->startProgram(&cfg, m_iface));

    WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);

    QCOMPARE(session->line(), 29);

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);

    QFile::remove(shellScript.fileName()+"-copy");
}

void GdbTest::testRemoteDebugInsertBreakpoint()
{
    const QString gdbserverExecutable = QStandardPaths::findExecutable("gdbserver");
    if (gdbserverExecutable.isEmpty()) {
        QSKIP("Skipping, gdbserver not available", SkipSingle);
    }

    TestDebugSession *session = new TestDebugSession;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 29);
    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 35);

    QTemporaryFile shellScript(QDir::currentPath()+"/shellscript");
    shellScript.open();
    shellScript.write("gdbserver localhost:2345 " + findExecutable("debugee").toLocalFile().toUtf8() + "\n");
    shellScript.close();
    shellScript.setPermissions(shellScript.permissions() | QFile::ExeUser);
    QFile::copy(shellScript.fileName(), shellScript.fileName()+"-copy"); //to avoid "Text file busy" on executing (why?)

    QTemporaryFile runScript(QDir::currentPath()+"/runscript");
    runScript.open();
    runScript.write("file " + findExecutable("debugee").toLocalFile().toUtf8() + '\n');
    runScript.write("target remote localhost:2345\n");
    runScript.write("break debugee.cpp:30\n");
    runScript.write("continue\n");
    runScript.close();

    TestLaunchConfiguration cfg;
    KConfigGroup grp = cfg.config();
    grp.writeEntry(GDBDebugger::remoteGdbShellEntry, QUrl::fromLocalFile(shellScript.fileName()+"-copy"));
    grp.writeEntry(GDBDebugger::remoteGdbRunEntry, QUrl::fromLocalFile(runScript.fileName()));

    QVERIFY(session->startProgram(&cfg, m_iface));

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
    const QString gdbserverExecutable = QStandardPaths::findExecutable("gdbserver");
    if (gdbserverExecutable.isEmpty()) {
        QSKIP("Skipping, gdbserver not available", SkipSingle);
    }

    TestDebugSession *session = new TestDebugSession;

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 35);

    QTemporaryFile shellScript(QDir::currentPath()+"/shellscript");
    shellScript.open();
    shellScript.write("gdbserver localhost:2345 "+findExecutable("debugee").toLocalFile().toLatin1()+"\n");
    shellScript.close();
    shellScript.setPermissions(shellScript.permissions() | QFile::ExeUser);
    QFile::copy(shellScript.fileName(), shellScript.fileName()+"-copy"); //to avoid "Text file busy" on executing (why?)

    QTemporaryFile runScript(QDir::currentPath()+"/runscript");
    runScript.open();
    runScript.write("file "+findExecutable("debugee").toLocalFile().toLatin1()+"\n");
    runScript.write("target remote localhost:2345\n");
    runScript.write("break debugee.cpp:30\n");
    runScript.write("continue\n");
    runScript.close();

    TestLaunchConfiguration cfg;
    KConfigGroup grp = cfg.config();
    grp.writeEntry(GDBDebugger::remoteGdbShellEntry, QUrl::fromLocalFile((shellScript.fileName()+"-copy")));
    grp.writeEntry(GDBDebugger::remoteGdbRunEntry, QUrl::fromLocalFile(runScript.fileName()));

    QVERIFY(session->startProgram(&cfg, m_iface));

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
    QVERIFY(session->startProgram(&cfg, m_iface));

    WAIT_FOR_STATE(session, DebugSession::PausedState);

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

    TestLaunchConfiguration cfg(findExecutable("debugeespace"));
    KConfigGroup grp = cfg.config();
    QString fileName = findSourceFile("debugee space.cpp");

    KDevelop::Breakpoint * b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 20);
    QCOMPARE(session->breakpointController()->breakpointState(b), KDevelop::Breakpoint::NotStartedState);

    session->startProgram(&cfg, m_iface);
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

    session->startProgram(&cfg, m_iface);
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

    TestLaunchConfiguration cfg(findExecutable("debugeeexception"));

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile("debugeeexception.cpp")), 29);

    session->startProgram(&cfg, m_iface);
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(1000);
    TestFrameStackModel* fsModel = session->frameStackModel();
    QCOMPARE(fsModel->currentFrame(), 0);
    QCOMPARE(session->line(), 29);

    session->addCommand(new GDBCommand(GDBMI::NonMI, "catch throw"));
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

//TODO: figure out why do we need this test? And do we need it at all??
void GdbTest::testThreadAndFrameInfo()
{
    TestDebugSession *session = new TestDebugSession;
    TestLaunchConfiguration cfg(findExecutable("debugeethreads"));
    QString fileName = findSourceFile("debugeethreads.cpp");

    breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(fileName), 38);
    QVERIFY(session->startProgram(&cfg, m_iface));
    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QTest::qWait(1000);

    QSignalSpy outputSpy(session, SIGNAL(gdbUserCommandStdout(QString)));

    session->addCommand(
                new UserCommand(GDBMI::ThreadInfo,""));
    session->addCommand(new UserCommand(GDBMI::StackListLocals, QLatin1String("0")));
    QTest::qWait(1000);
    QCOMPARE(outputSpy.count(), 2);
    QVERIFY(outputSpy.last().at(0).toString().contains(QLatin1String("--thread 1")));

    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::parseBug304730()
{
    FileSymbol file;
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

    MIParser parser;

    std::unique_ptr<GDBMI::Record> record(parser.parse(&file));
    QVERIFY(record.get() != nullptr);
}

void GdbTest::testMultipleLocationsBreakpoint()
{
    TestDebugSession *session = new TestDebugSession;

    TestLaunchConfiguration cfg(findExecutable("debugeemultilocbreakpoint"));

    breakpoints()->addCodeBreakpoint("aPlusB");

    //TODO check if the additional location breakpoint is added

    session->startProgram(&cfg, m_iface);
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

    session->startProgram(&cfg, m_iface);
    WAIT_FOR_STATE(session, DebugSession::PausedState);

    variableCollection()->watches()->add("argc");
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

    session->startProgram(&cfg, m_iface);
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
        TestLaunchConfiguration c(findExecutable("debugeemultiplebreakpoint"));
        KDevelop::Breakpoint *b = breakpoints()->addCodeBreakpoint("debugeemultiplebreakpoint.cpp:52");
        session->startProgram(&c, m_iface);
        WAIT_FOR_STATE(session, DebugSession::PausedState);
        QCOMPARE(breakpoints()->breakpoints().count(), 1);

        b->setData(KDevelop::Breakpoint::EnableColumn, Qt::Unchecked);
        session->run();
        WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testRegularExpressionBreakpoint()
{
        TestDebugSession *session = new TestDebugSession;

        TestLaunchConfiguration c(findExecutable("debugeemultilocbreakpoint"));
        breakpoints()->addCodeBreakpoint("main");
        session->startProgram(&c, m_iface);
        WAIT_FOR_STATE(session, DebugSession::PausedState);
        session->addCommand(new GDBCommand(GDBMI::NonMI, "rbreak .*aPl.*B"));
        QTest::qWait(100);
        session->run();
        WAIT_FOR_STATE(session, DebugSession::PausedState);
        QCOMPARE(breakpoints()->breakpoints().count(), 3);

        session->addCommand(new GDBCommand(GDBMI::BreakDelete, ""));
        session->run();
        WAIT_FOR_STATE(session, DebugSession::EndedState);
}

void GdbTest::testChangeBreakpointWhileRunning() {

    TestDebugSession *session = new TestDebugSession;

    TestLaunchConfiguration c(findExecutable("debugeeslow"));
    KDevelop::Breakpoint* b = breakpoints()->addCodeBreakpoint("debugeeslow.cpp:25");
    session->startProgram(&c, m_iface);

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

        TestDebugSession* session = 0;
        if (QStandardPaths::findExecutable(console).isEmpty()) {
            continue;
        }

        session = new TestDebugSession();

        cfg.config().writeEntry("External Terminal"/*ExecutePlugin::terminalEntry*/, console);
        cfg.config().writeEntry("Use External Terminal"/*ExecutePlugin::useTerminalEntry*/, true);

        KDevelop::Breakpoint* b = breakpoints()->addCodeBreakpoint(QUrl::fromLocalFile(debugeeFileName), 28);

        session->startProgram(&cfg, m_iface);
        WAIT_FOR_STATE_AND_IDLE(session, DebugSession::PausedState);
        QCOMPARE(session->breakpointController()->breakpointState(b), KDevelop::Breakpoint::CleanState);
        session->stepInto();
        WAIT_FOR_STATE(session, DebugSession::PausedState);
        session->run();
        WAIT_FOR_STATE(session, DebugSession::EndedState);
    }
}

// see: https://bugs.kde.org/show_bug.cgi?id=339231
void GdbTest::testPathWithSpace()
{
#ifdef HAVE_PATH_WITH_SPACES_TEST
    TestDebugSession* session = new TestDebugSession;

    auto debugee = findExecutable("path with space/spacedebugee");
    TestLaunchConfiguration c(debugee, KIO::upUrl(debugee));
    KDevelop::Breakpoint* b = breakpoints()->addCodeBreakpoint("spacedebugee.cpp:30");
    QCOMPARE(session->breakpointController()->breakpointState(b), KDevelop::Breakpoint::NotStartedState);
    session->startProgram(&c, m_iface);

    WAIT_FOR_STATE(session, DebugSession::PausedState);
    QCOMPARE(session->breakpointController()->breakpointState(b), KDevelop::Breakpoint::CleanState);
    session->run();
    WAIT_FOR_STATE(session, DebugSession::EndedState);
#endif
}

void GdbTest::waitForState(GDBDebugger::DebugSession *session, DebugSession::DebuggerState state,
                            const char *file, int line, bool waitForIdle)
{
    QPointer<GDBDebugger::DebugSession> s(session); //session can get deleted in DebugController
    QTime stopWatch;
    stopWatch.start();
    while (s.data()->state() != state || (waitForIdle && s->stateIsOn(s_dbgBusy))) {
        if (stopWatch.elapsed() > 5000) {
            qWarning() << "current state" << s.data()->state() << "waiting for" << state;
            QTest::qFail(qPrintable(QString("Timeout before reaching state %0").arg(state)),
                file, line);
            return;
        }
        QTest::qWait(20);
        if (!s) {
            if (state == DebugSession::EndedState)
                break;
            QTest::qFail(qPrintable(QString("Session ended before reaching state %0").arg(state)),
                file, line);
            return;
        }
    }
    if (!waitForIdle && state != DebugSession::EndedState) {
        // legacy behavior for tests that implicitly may require waiting for idle,
        // but which were written before waitForIdle was added
        QTest::qWait(100);
    }

    qDebug() << "Reached state " << state << " in " << file << ':' << line;
}

}

QTEST_MAIN(GDBDebugger::GdbTest)


#include "test_gdb.moc"
#include "moc_test_gdb.cpp"

