/*
 * Unit tests for LLDB debugger plugin
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

#include "test_lldbformatters.h"

#include "controllers/variable.h"
#include "controllers/variablecontroller.h"
#include "debugsession.h"
#include "stringhelpers.h"
#include "testhelper.h"

#include <execute/iexecuteplugin.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/ilaunchconfiguration.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>

#include <QDebug>
#include <QString>
#include <QTest>
#include <QUrl>

#define WAIT_FOR_STATE(session, state) \
    do { if (!KDevMI::LLDB::waitForState((session), (state), __FILE__, __LINE__)) return; } while (0)

#define WAIT_FOR_STATE_AND_IDLE(session, state) \
    do { if (!KDevMI::LLDB::waitForState((session), (state), __FILE__, __LINE__, true)) return; } while (0)

#define WAIT_FOR_A_WHILE(session, ms) \
    do { if (!KDevMI::LLDB::waitForAWhile((session), (ms), __FILE__, __LINE__)) return; } while (0)

#define WAIT_FOR(session, condition) \
    do { \
        KDevMI::LLDB::TestWaiter w((session), #condition, __FILE__, __LINE__); \
        while (w.waitUnless((condition))) /* nothing */ ; \
    } while(0)

#define COMPARE_DATA(index, expected) \
    do { if (!KDevMI::LLDB::compareData((index), (expected), __FILE__, __LINE__)) return; } while (0)

#define VERIFY_QSTRING(row, name, expected) \
    do { if (!verifyQString((row), (name), (expected), __FILE__, __LINE__)) return; } while (0)

#define findSourceFile(name) \
    findSourceFile(__FILE__, (name))

using namespace KDevelop;
using namespace KDevMI::LLDB;

class TestLaunchConfiguration : public ILaunchConfiguration
{
public:
    TestLaunchConfiguration(const QString& executable,
                            const QUrl& workingDirectory = QUrl()) {
        auto execPath = findExecutable(executable);
        qDebug() << "FIND" << execPath;
        c = new KConfig();
        c->deleteGroup("launch");
        cfg = c->group("launch");
        cfg.writeEntry("isExecutable", true);
        cfg.writeEntry("Executable", execPath);
        cfg.writeEntry("Working Directory", workingDirectory);
    }
    ~TestLaunchConfiguration() override {
        delete c;
    }
    const KConfigGroup config() const override { return cfg; }
    KConfigGroup config() override { return cfg; };
    QString name() const override { return QString("Test-Launch"); }
    KDevelop::IProject* project() const override { return nullptr; }
    KDevelop::LaunchConfigurationType* type() const override { return nullptr; }

    KConfig *rootConfig() { return c; }
private:
    KConfigGroup cfg;
    KConfig *c;
};

class TestDebugSession : public DebugSession
{
    Q_OBJECT
public:
    TestDebugSession() : DebugSession()
    {
        setSourceInitFile(false);
        setFormatterPath(findSourceFile("../formatters/all.py"));

        KDevelop::ICore::self()->debugController()->addSession(this);

        variableController()->setAutoUpdate(IVariableController::UpdateLocals);
    }
};

VariableCollection *LldbFormattersTest::variableCollection()
{
    return m_core->debugController()->variableCollection();
}

LldbVariable *LldbFormattersTest::watchVariableAt(int i)
{
    auto watchRoot = variableCollection()->indexForItem(variableCollection()->watches(), 0);
    auto idx = variableCollection()->index(i, 0, watchRoot);
    return dynamic_cast<LldbVariable*>(variableCollection()->itemForIndex(idx));
}

QModelIndex LldbFormattersTest::localVariableIndexAt(int i, int col)
{
    auto localRoot = variableCollection()->indexForItem(variableCollection()->locals(), 0);
    return variableCollection()->index(i, col, localRoot);
}

KDevelop::Breakpoint* LldbFormattersTest::addCodeBreakpoint(const QUrl& location, int line)
{
    return m_core->debugController()->breakpointModel()->addCodeBreakpoint(location, line);
}

// Called before the first testfunction is executed
void LldbFormattersTest::initTestCase()
{
    AutoTestShell::init();
    m_core = TestCore::initialize(Core::NoUi);

    m_iface = m_core->pluginController()
                ->pluginForExtension("org.kdevelop.IExecutePlugin", "kdevexecute")
                ->extension<IExecutePlugin>();
    Q_ASSERT(m_iface);
}

// Called after the last testfunction was executed
void LldbFormattersTest::cleanupTestCase()
{
    TestCore::shutdown();
}

// Called before each testfunction is executed
void LldbFormattersTest::init()
{
    //remove all breakpoints - so we can set our own in the test
    KConfigGroup bpCfg = KSharedConfig::openConfig()->group("breakpoints");
    bpCfg.writeEntry("number", 0);
    bpCfg.sync();

    auto count = m_core->debugController()->breakpointModel()->rowCount();
    m_core->debugController()->breakpointModel()->removeRows(0, count);

    while (variableCollection()->watches()->childCount() > 0) {
        auto var = watchVariableAt(0);
        if (!var) break;
        var->die();
    }

    m_session = new TestDebugSession;
}

void LldbFormattersTest::cleanup()
{
    // Called after every testfunction
    if (m_session)
        m_session->stopDebugger();
    WAIT_FOR_STATE(m_session, DebugSession::EndedState);
    m_session.clear();
}

bool LldbFormattersTest::verifyQString(int index, const QString &name,
                                       const QString &expected,
                                       const char *file, int line)
{
    auto varidx = localVariableIndexAt(index, 0);
    auto var = variableCollection()->itemForIndex(varidx);

    if (!compareData(varidx, name, file, line)) {
        return false;
    }
    if (!compareData(localVariableIndexAt(index, 1), Utils::quote(expected), file, line)) {
        return false;
    }

    // fetch all children
    auto childCount = 0;
    while (childCount != variableCollection()->rowCount(varidx)) {
        childCount = variableCollection()->rowCount(varidx);
        var->fetchMoreChildren();
        if (!waitForAWhile(m_session, 50, file, line))
            return false;
    }
    if (childCount != expected.length()) {
        QTest::qFail(qPrintable(QString("'%0' didn't match expected '%1' in %2:%3")
                                .arg(childCount).arg(expected.length()).arg(file).arg(line)),
                     file, line);
        return false;
    }

    for (int i = 0; i != childCount; ++i) {
        if (!compareData(variableCollection()->index(i, 0, varidx),
                         QString("[%0]").arg(i), file, line)) {
            return false;
        }
        if (!compareData(variableCollection()->index(i, 1, varidx),
                         QString("'%0'").arg(expected[i]), file, line)) {
            return false;
        }
    }
    return true;
}

void LldbFormattersTest::testQString()
{
    TestLaunchConfiguration cfg("lldb_qstring");
    addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile("qstring.cpp")), 4);

    QVERIFY(m_session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);

    QCOMPARE(variableCollection()->rowCount(), 2);

    VERIFY_QSTRING(0, "s", "test string");

    m_session->stepOver();
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);
    QCOMPARE(m_session->currentLine(), 5);

    VERIFY_QSTRING(0, "s", "test stringx");

    m_session->run();
    WAIT_FOR_STATE(m_session, DebugSession::EndedState);
}
/*
void LldbFormattersTest::testQByteArray()
{
    LldbProcess lldb("qbytearray");
    lldb.execute("break qbytearray.cpp:5");
    lldb.execute("run");
    QByteArray out = lldb.execute("print ba");
    QVERIFY(out.contains("\"test byte array\""));
    QVERIFY(out.contains("[0] = 116 't'"));
    QVERIFY(out.contains("[4] = 32 ' '"));
    lldb.execute("next");
    QVERIFY(lldb.execute("print ba").contains("\"test byte arrayx\""));
}

void LldbFormattersTest::testQListContainer_data()
{
    QTest::addColumn<QString>("container");

    QTest::newRow("QList") << "QList";
    QTest::newRow("QQueue") << "QQueue";
    QTest::newRow("QVector") << "QVector";
    QTest::newRow("QStack") << "QStack";
    QTest::newRow("QLinkedList") << "QLinkedList";
    QTest::newRow("QSet") << "QSet";
}

void LldbFormattersTest::testQListContainer()
{
    QFETCH(QString, container);
    LldbProcess lldb("qlistcontainer");
    lldb.execute("break main");
    lldb.execute("run");
    lldb.execute(QString("break 'doStuff<%1>()'").arg(container).toLocal8Bit());
    lldb.execute("cont");
    { // <int>
    lldb.execute("break qlistcontainer.cpp:34");
    lldb.execute("cont");
    QByteArray out = lldb.execute("print intList");
    QVERIFY(out.contains(QString("empty %1<int>").arg(container).toLocal8Bit()));
    lldb.execute("next");
    out = lldb.execute("print intList");
    QVERIFY(out.contains(QString("%1<int>").arg(container).toLocal8Bit()));
    if (container != "QSet") {
        QVERIFY(out.contains("[0] = 10"));
        QVERIFY(out.contains("[1] = 20"));
        QVERIFY(!out.contains("[2] = 30"));
    } else { // QSet order is undefined
        QVERIFY(out.contains("] = 10"));
        QVERIFY(out.contains("] = 20"));
        QVERIFY(!out.contains("] = 30"));
    }
    lldb.execute("next");
    out = lldb.execute("print intList");
    QVERIFY(out.contains(QString("%1<int>").arg(container).toLocal8Bit()));
    if (container != "QSet") {
        QVERIFY(out.contains("[0] = 10"));
        QVERIFY(out.contains("[1] = 20"));
        QVERIFY(out.contains("[2] = 30"));
    } else { // QSet order is undefined
        QVERIFY(out.contains("] = 10"));
        QVERIFY(out.contains("] = 20"));
        QVERIFY(out.contains("] = 30"));
    }
    }
    { // <QString>
    lldb.execute("next");
    QByteArray out = lldb.execute("print stringList");
    QVERIFY(out.contains(QString("empty %1<QString>").arg(container).toLocal8Bit()));
    lldb.execute("next");
    out = lldb.execute("print stringList");
    QVERIFY(out.contains(QString("%1<QString>").arg(container).toLocal8Bit()));
    if (container != "QSet") {
        QVERIFY(out.contains("[0] = \"a\""));
        QVERIFY(out.contains("[1] = \"bc\""));
        QVERIFY(!out.contains("[2] = \"d\""));
    } else { // QSet order is undefined
        QVERIFY(out.contains("] = \"a\""));
        QVERIFY(out.contains("] = \"bc\""));
        QVERIFY(!out.contains("] = \"d\""));
    }
    lldb.execute("next");
    out = lldb.execute("print stringList");
    QVERIFY(out.contains(QString("%1<QString>").arg(container).toLocal8Bit()));
    if (container != "QSet") {
        QVERIFY(out.contains("[0] = \"a\""));
        QVERIFY(out.contains("[1] = \"bc\""));
        QVERIFY(out.contains("[2] = \"d\""));
    } else { // QSet order is undefined
        QVERIFY(out.contains("] = \"a\""));
        QVERIFY(out.contains("] = \"bc\""));
        QVERIFY(out.contains("] = \"d\""));
    }
    }
    { // <struct A>
    lldb.execute("next");
    QByteArray out = lldb.execute("print structList");
    QVERIFY(out.contains(QString("empty %1<A>").arg(container).toLocal8Bit()));
    lldb.execute("next");
    out = lldb.execute("print structList");
    QVERIFY(out.contains(QString("%1<A>").arg(container).toLocal8Bit()));
    QVERIFY(out.contains("[0] = {"));
    QVERIFY(out.contains("a = \"a\""));
    QVERIFY(out.contains("b = \"b\""));
    QVERIFY(out.contains("c = 100"));
    QVERIFY(out.contains("d = -200"));
    lldb.execute("next");
    out = lldb.execute("print structList");
    QVERIFY(out.contains(QString("%1<A>").arg(container).toLocal8Bit()));
    QVERIFY(out.contains("[1] = {"));
    }
    { // <int*>
    lldb.execute("next");
    QByteArray out = lldb.execute("print pointerList");
    QVERIFY(out.contains(QString("empty %1<int *>").arg(container).toLocal8Bit()));
    lldb.execute("next");
    out = lldb.execute("print pointerList");
    QVERIFY(out.contains(QString("%1<int *>").arg(container).toLocal8Bit()));
    QVERIFY(out.contains("[0] = 0x"));
    QVERIFY(out.contains("[1] = 0x"));
    QVERIFY(!out.contains("[2] = 0x"));
    lldb.execute("next");
    out = lldb.execute("print pointerList");
    QVERIFY(out.contains("[0] = 0x"));
    QVERIFY(out.contains("[1] = 0x"));
    QVERIFY(out.contains("[2] = 0x"));
	lldb.execute("next");
    }
    { // <QPair<int, int> >
    lldb.execute("next");
    QByteArray out = lldb.execute("print pairList");
    QVERIFY(out.contains(QString("empty %1<QPair<int, int>>").arg(container).toLocal8Bit()));
    lldb.execute("next");
    out = lldb.execute("print pairList");
    QVERIFY(out.contains(QString("%1<QPair<int, int>>").arg(container).toLocal8Bit()));
    if (container != "QSet") {
        QVERIFY(out.contains("[0] = {\n    first = 1, \n    second = 2\n  }"));
        QVERIFY(out.contains("[1] = {\n    first = 2, \n    second = 3\n  }"));
    } else { // order is undefined in QSet
        QVERIFY(out.contains("] = {\n    first = 1, \n    second = 2\n  }"));
        QVERIFY(out.contains("] = {\n    first = 2, \n    second = 3\n  }"));
    }
    QVERIFY(!out.contains("[2] = "));
    lldb.execute("next");
    out = lldb.execute("print pairList");
    if (container != "QSet") {
        QVERIFY(out.contains("[0] = {\n    first = 1, \n    second = 2\n  }"));
        QVERIFY(out.contains("[1] = {\n    first = 2, \n    second = 3\n  }"));
        QVERIFY(out.contains("[2] = {\n    first = 4, \n    second = 5\n  }"));
    } else { // order is undefined in QSet
        QVERIFY(out.contains("] = {\n    first = 1, \n    second = 2\n  }"));
        QVERIFY(out.contains("] = {\n    first = 2, \n    second = 3\n  }"));
        QVERIFY(out.contains("] = {\n    first = 4, \n    second = 5\n  }"));
    }
    }
}

void LldbFormattersTest::testQMapInt()
{
    LldbProcess lldb("qmapint");
    lldb.execute("break qmapint.cpp:7");
    lldb.execute("run");
    QByteArray out = lldb.execute("print m");
    QVERIFY(out.contains("QMap<int, int>"));
    QVERIFY(out.contains("[10] = 100"));
    QVERIFY(out.contains("[20] = 200"));
    lldb.execute("next");
    out = lldb.execute("print m");
    QVERIFY(out.contains("[30] = 300"));
}

void LldbFormattersTest::testQMapString()
{
    LldbProcess lldb("qmapstring");
    lldb.execute("break qmapstring.cpp:8");
    lldb.execute("run");
    QByteArray out = lldb.execute("print m");
    QVERIFY(out.contains("QMap<QString, QString>"));
    QVERIFY(out.contains("[\"10\"] = \"100\""));
    QVERIFY(out.contains("[\"20\"] = \"200\""));
    lldb.execute("next");
    out = lldb.execute("print m");
    QVERIFY(out.contains("[\"30\"] = \"300\""));
}

void LldbFormattersTest::testQMapStringBool()
{
    LldbProcess lldb("qmapstringbool");
    lldb.execute("break qmapstringbool.cpp:8");
    lldb.execute("run");
    QByteArray out = lldb.execute("print m");
    QVERIFY(out.contains("QMap<QString, bool>"));
    QVERIFY(out.contains("[\"10\"] = true"));
    QVERIFY(out.contains("[\"20\"] = false"));
    lldb.execute("next");
    out = lldb.execute("print m");
    QVERIFY(out.contains("[\"30\"] = true"));
}


void LldbFormattersTest::testQDate()
{
    LldbProcess lldb("qdate");
    lldb.execute("break qdate.cpp:6");
    lldb.execute("run");
    QByteArray out = lldb.execute("print d");
    QVERIFY(out.contains("2010-01-20"));
}

void LldbFormattersTest::testQTime()
{
    LldbProcess lldb("qtime");
    lldb.execute("break qtime.cpp:6");
    lldb.execute("run");
    QByteArray out = lldb.execute("print t");
    QVERIFY(out.contains("15:30:10.123"));
}

void LldbFormattersTest::testQDateTime()
{
    LldbProcess lldb("qdatetime");
    lldb.execute("break qdatetime.cpp:5");
    lldb.execute("run");
    QByteArray out = lldb.execute("print dt");
    QVERIFY(out.contains("Wed Jan 20 15:31:13 2010"));
}

void LldbFormattersTest::testQUrl()
{
    LldbProcess lldb("qurl");
    lldb.execute("break qurl.cpp:5");
    lldb.execute("run");
    QByteArray out = lldb.execute("print u");
    QVERIFY(out.contains("http://www.kdevelop.org/foo"));
}

void LldbFormattersTest::testQHashInt()
{
    LldbProcess lldb("qhashint");
    lldb.execute("break qhashint.cpp:7");
    lldb.execute("run");
    QByteArray out = lldb.execute("print h");
    QVERIFY(out.contains("[10] = 100"));
    QVERIFY(out.contains("[20] = 200"));
    lldb.execute("next");
    out = lldb.execute("print h");
    QVERIFY(out.contains("[30] = 300"));
}

void LldbFormattersTest::testQHashString()
{
    LldbProcess lldb("qhashstring");
    lldb.execute("break qhashstring.cpp:8");
    lldb.execute("run");
    QByteArray out = lldb.execute("print h");
    QVERIFY(out.contains("[\"10\"] = \"100\""));
    QVERIFY(out.contains("[\"20\"] = \"200\""));
    lldb.execute("next");
    out = lldb.execute("print h");
    QVERIFY(out.contains("[\"30\"] = \"300\""));
}

void LldbFormattersTest::testQSetInt()
{
    LldbProcess lldb("qsetint");
    lldb.execute("break qsetint.cpp:7");
    lldb.execute("run");
    QByteArray out = lldb.execute("print s");
    QVERIFY(out.contains("] = 10"));
    QVERIFY(out.contains("] = 20"));
    lldb.execute("next");
    out = lldb.execute("print s");
    QVERIFY(out.contains("] = 30"));
}

void LldbFormattersTest::testQSetString()
{
    LldbProcess lldb("qsetstring");
    lldb.execute("break qsetstring.cpp:8");
    lldb.execute("run");
    QByteArray out = lldb.execute("print s");
    QVERIFY(out.contains("] = \"10\""));
    QVERIFY(out.contains("] = \"20\""));
    lldb.execute("next");
    out = lldb.execute("print s");
    QVERIFY(out.contains("] = \"30\""));
}

void LldbFormattersTest::testQChar()
{
    LldbProcess lldb("qchar");
    lldb.execute("break qchar.cpp:5");
    lldb.execute("run");
    QVERIFY(lldb.execute("print c").contains("\"k\""));
}

void LldbFormattersTest::testQListPOD()
{
    LldbProcess lldb("qlistpod");
    lldb.execute("break qlistpod.cpp:31");
    lldb.execute("run");
    QVERIFY(lldb.execute("print b").contains("false"));
    QVERIFY(lldb.execute("print c").contains("50"));
    QVERIFY(lldb.execute("print uc").contains("50"));
    QVERIFY(lldb.execute("print s").contains("50"));
    QVERIFY(lldb.execute("print us").contains("50"));
    QVERIFY(lldb.execute("print i").contains("50"));
    QVERIFY(lldb.execute("print ui").contains("50"));
    QVERIFY(lldb.execute("print l").contains("50"));
    QVERIFY(lldb.execute("print ul").contains("50"));
    QVERIFY(lldb.execute("print i64").contains("50"));
    QVERIFY(lldb.execute("print ui64").contains("50"));
    QVERIFY(lldb.execute("print f").contains("50"));
    QVERIFY(lldb.execute("print d").contains("50"));
}

void LldbFormattersTest::testQUuid()
{
    LldbProcess lldb("quuid");
    lldb.execute("break quuid.cpp:4");
    lldb.execute("run");
    QByteArray data = lldb.execute("print id");
    QVERIFY(data.contains("{9ec3b70b-d105-42bf-b3b4-656e44d2e223}"));
}

void LldbFormattersTest::testKTextEditorTypes()
{
    LldbProcess lldb("ktexteditortypes");
    lldb.execute("break ktexteditortypes.cpp:9");
    lldb.execute("run");

    QByteArray data = lldb.execute("print cursor");
    QCOMPARE(data, QByteArray("$1 = [1, 1]"));
    data = lldb.execute("print range");
    QCOMPARE(data, QByteArray("$2 = [(1, 1) -> (2, 2)]"));
}

void LldbFormattersTest::testKDevelopTypes()
{
    LldbProcess lldb("kdeveloptypes");
    lldb.execute("break kdeveloptypes.cpp:12");
    lldb.execute("run");

    QVERIFY(lldb.execute("print path1").contains("(\"tmp\", \"foo\")"));
    QVERIFY(lldb.execute("print path2").contains("(\"http://www.test.com\", \"tmp\", \"asdf.txt\")"));
}
*/

QTEST_MAIN(LldbFormattersTest)

#include "test_lldbformatters.moc"
