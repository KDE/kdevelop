/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "test_lldbformatters.h"

#include "controllers/variable.h"
#include "controllers/variablecontroller.h"
#include "debugsession.h"
#include "stringhelpers.h"
#include "tests/debuggers-tests-config.h"
#include "tests/testhelper.h"

#include <execute/iexecuteplugin.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include <KConfigGroup>
#include <KSharedConfig>

#include <QDebug>
#include <QString>
#include <QTest>
#include <QUrl>

#include <algorithm>
#include <vector>

#define WAIT_FOR_A_WHILE_AND_IDLE(session, ms) \
    do { if (!KDevMI::Testing::waitForAWhile((session), (ms), __FILE__, __LINE__)) return; \
         if (!KDevMI::Testing::waitForState((session), DebugSession::PausedState, __FILE__, __LINE__, true)) \
             return; \
    } while (0)

#define VERIFY_LOCAL(row, name, summary, children) \
    do { \
        if (!verifyVariable((row), (name), (summary), (children), __FILE__, __LINE__)) \
            return; \
    } while (0)

#define VERIFY_WATCH(row, name, summary, children) \
    do { \
        if (!verifyVariable((row), (name), (summary), (children), __FILE__, __LINE__, false)) \
            return; \
    } while (0)

using namespace KDevelop;
using namespace KDevMI::LLDB;
using KDevMI::Testing::compareData;
using KDevMI::Testing::findExecutable;
using KDevMI::Testing::findFile;
using KDevMI::Testing::findSourceFile;
using KDevMI::Testing::TestLaunchConfiguration;
using KDevMI::Testing::waitForAWhile;

class TestDebugSession : public DebugSession
{
    Q_OBJECT
public:
    TestDebugSession() : DebugSession()
    {
        setSourceInitFile(false);
        // explicit set formatter path to force use in-tree formatters, not the one installed in system.
        auto formatter = findFile(LLDB_SRC_DIR, "formatters/all.py");
        setFormatterPath(formatter);

        KDevelop::ICore::self()->debugController()->addSession(this);

        variableController()->setAutoUpdate(IVariableController::UpdateLocals);
    }
};

VariableCollection *LldbFormattersTest::variableCollection()
{
    return m_core->debugController()->variableCollection();
}

QModelIndex LldbFormattersTest::watchVariableIndexAt(int i, int col)
{
    auto watchRoot = variableCollection()->indexForItem(variableCollection()->watches(), 0);
    return variableCollection()->index(i, col, watchRoot);
}

QModelIndex LldbFormattersTest::localVariableIndexAt(int i, int col)
{
    auto localRoot = variableCollection()->indexForItem(variableCollection()->locals(), 0);
    return variableCollection()->index(i, col, localRoot);
}

// Note: line is zero-based
KDevelop::Breakpoint* LldbFormattersTest::addCodeBreakpoint(const QUrl& location, int line)
{
    return m_core->debugController()->breakpointModel()->addCodeBreakpoint(location, line);
}

// Called before the first testfunction is executed
void LldbFormattersTest::initTestCase()
{
    AutoTestShell::init({QStringLiteral("kdevlldb"), QStringLiteral("kdevexecute")});
    m_core = TestCore::initialize(Core::NoUi);

    m_iface = m_core->pluginController()
                ->pluginForExtension(QStringLiteral("org.kdevelop.IExecutePlugin"), QStringLiteral("kdevexecute"))
                ->extension<IExecutePlugin>();
    Q_ASSERT(m_iface);

    const QString lldbMiExecutable = QStandardPaths::findExecutable(QStringLiteral("lldb-mi"));
    if (lldbMiExecutable.isEmpty()) {
        QSKIP("Skipping, lldb-mi not available");
    }
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
        auto idx = watchVariableIndexAt(0);
        auto var = qobject_cast<LldbVariable*>(variableCollection()->itemForIndex(idx));
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

bool LldbFormattersTest::verifyVariable(int index, const QString &name,
                                        const QString &expectedSummary,
                                        QStringList expectedChildren,
                                        const char *file, int line,
                                        bool isLocal, bool useRE, bool unordered)
{
    QList<QPair<QString, QString>> childrenPairs;
    childrenPairs.reserve(expectedChildren.size());
    if (unordered) {
        qDebug() << "useRE set to true when unordered = true";
        useRE = true;
        expectedChildren.sort();
        for (auto& c : expectedChildren) {
            childrenPairs << qMakePair(QStringLiteral(R"(^\[\d+\]$)"), c);
        }
    } else {
        for (int i = 0; i != expectedChildren.size(); ++i) {
            childrenPairs << qMakePair(QStringLiteral("[%0]").arg(i), expectedChildren[i]);
        }
    }
    return verifyVariable(index, name, expectedSummary, childrenPairs, file, line, isLocal, useRE, unordered);
}

bool LldbFormattersTest::verifyVariable(int index, const QString &name,
                                        const QString &expectedSummary,
                                        QList<QPair<QString, QString>> expectedChildren,
                                        const char *file, int line,
                                        bool isLocal, bool useRE, bool unordered)
{
    QModelIndex varIdx, summaryIdx;
    if (isLocal) {
        varIdx = localVariableIndexAt(index, 0);
        summaryIdx = localVariableIndexAt(index, 1);
    } else {
        varIdx = watchVariableIndexAt(index, 0);
        summaryIdx = watchVariableIndexAt(index, 1);
    }

    if (!compareData(varIdx, name, file, line)) {
        return false;
    }
    if (!compareData(summaryIdx, expectedSummary, file, line, useRE)) {
        return false;
    }

    // fetch all children
    auto var = variableCollection()->itemForIndex(varIdx);
    auto childCount = 0;
    while (childCount != variableCollection()->rowCount(varIdx)) {
        childCount = variableCollection()->rowCount(varIdx);
        var->fetchMoreChildren();
        if (!waitForAWhile(m_session, 50, file, line))
            return false;
    }
    if (childCount != expectedChildren.length()) {
        QTest::qFail(qPrintable(QString("'%0' didn't match expected '%1' in %2:%3")
                                .arg(childCount).arg(expectedChildren.length()).arg(file).arg(line)),
                     file, line);
        return false;
    }

    QVector<int> theOrder;
    theOrder.reserve(childCount);
    for (int i = 0; i != childCount; ++i) {
        theOrder.push_back(i);
    }
    if (unordered) {
        qDebug() << "actual list sorted for unordered compare";
        std::sort(theOrder.begin(), theOrder.end(), [&](int a, int b){
            auto indexA = variableCollection()->index(a, 1, varIdx);
            auto indexB = variableCollection()->index(b, 1, varIdx);
            return indexA.model()->data(indexA, Qt::DisplayRole).toString()
                < indexB.model()->data(indexB, Qt::DisplayRole).toString();
        });
        std::sort(expectedChildren.begin(), expectedChildren.end(),
                  [](const QPair<QString, QString> &a, const QPair<QString, QString> &b){
            return a.second < b.second;
        });
        qDebug() << "sorted actual order" << theOrder;
        qDebug() << "sorted expectedChildren" << expectedChildren;
    }

    for (int i = 0; i != childCount; ++i) {
        if (!compareData(variableCollection()->index(theOrder[i], 0, varIdx),
                         expectedChildren[i].first, file, line, useRE)) {
            return false;
        }

        if (!compareData(variableCollection()->index(theOrder[i], 1, varIdx),
                         expectedChildren[i].second, file, line, useRE)) {
            return false;
        }
    }
    return true;
}

void LldbFormattersTest::testQChar()
{
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_qchar"));
    addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile("qchar.cpp")), 4);

    QVERIFY(m_session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);

    // Should be two rows ('auto', 'local')
    QCOMPARE(variableCollection()->rowCount(), 2);

    variableCollection()->expanded(localVariableIndexAt(0));
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    QList<QPair<QString, QString>> children;
    children << qMakePair(QStringLiteral("ucs"), QStringLiteral("107"));

    VERIFY_LOCAL(0, "c", "'k'", children);
}

void LldbFormattersTest::testQString()
{
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_qstring"));
    addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile("qstring.cpp")), 4);

    QVERIFY(m_session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);

    // Should be two rows ('auto', 'local')
    QCOMPARE(variableCollection()->rowCount(), 2);

    variableCollection()->expanded(localVariableIndexAt(0));
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    QString expected = QStringLiteral("test最后一个不是特殊字符'\"\\u6211");
    QStringList children;
    for (auto ch : qAsConst(expected)) {
        children << Utils::quote(ch, '\'');
    }

    VERIFY_LOCAL(0, "s", Utils::quote(expected), children);

    m_session->stepOver();
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);
    QCOMPARE(m_session->currentLine(), 5);

    expected.append("x");
    children << QStringLiteral("'x'");

    VERIFY_LOCAL(0, "s", Utils::quote(expected), children);

    m_session->run();
    WAIT_FOR_STATE(m_session, DebugSession::EndedState);
}

void LldbFormattersTest::testQByteArray()
{
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_qbytearray"));
    addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile("qbytearray.cpp")), 4);

    QVERIFY(m_session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);

    // Should be two rows ('auto', 'local')
    QCOMPARE(variableCollection()->rowCount(), 2);

    variableCollection()->expanded(localVariableIndexAt(0));
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    QStringList charlist {
        R"(-26 '\xe6')",
        R"(-104 '\x98')",
        R"(-81 '\xaf')",
        R"(39 ''')",
        R"(34 '"')",
        R"(92 '\')",
        R"(117 'u')",
        R"(54 '6')",
        R"(50 '2')",
        R"(49 '1')",
        R"(49 '1')",
    };

    VERIFY_LOCAL(0, "ba", R"("\xe6\x98\xaf'\"\\u6211")", charlist);

    m_session->stepOver();
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);
    QCOMPARE(m_session->currentLine(), 5);

    charlist << QStringLiteral("120 'x'");
    VERIFY_LOCAL(0, "ba", R"("\xe6\x98\xaf'\"\\u6211x")", charlist);

    m_session->run();
    WAIT_FOR_STATE(m_session, DebugSession::EndedState);
}


void LldbFormattersTest::testQListContainer_data()
{
    QTest::addColumn<QString>("container");
    QTest::addColumn<bool>("unordered");

    QTest::newRow("QList") << "QList" << false;
    QTest::newRow("QQueue") << "QQueue" << false;
    QTest::newRow("QStack") << "QStack" << false;
    QTest::newRow("QLinkedList") << "QLinkedList" << false;
    QTest::newRow("QSet") << "QSet" << true;
}

void LldbFormattersTest::testQListContainer()
{
    QFETCH(QString, container);
    QFETCH(bool, unordered);

    TestLaunchConfiguration cfg(QStringLiteral("debuggee_qlistcontainer"));
    cfg.config().writeEntry(KDevMI::Config::BreakOnStartEntry, true);

    auto watchRoot = variableCollection()->indexForItem(variableCollection()->watches(), 0);
    variableCollection()->expanded(watchRoot);
    variableCollection()->variableWidgetShown();

    QVERIFY(m_session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);

    m_session->addUserCommand(QStringLiteral("break set --func doStuff<%1>()").arg(container));
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    m_session->run();
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);

    // <int>
    m_session->stepOver();
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);
    QCOMPARE(m_session->currentLine(), 33); // line 34: intList << 10 << 20;

    auto var = variableCollection()->watches()->add(QStringLiteral("intList"));
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    if (!verifyVariable(0, QStringLiteral("intList"), QStringLiteral("<size=0>"), QStringList{},
                        __FILE__, __LINE__, false, false, unordered)) {
        return;
    }

    m_session->stepOver();
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);
    QCOMPARE(m_session->currentLine(), 34); // line 35: intList << 30;

    variableCollection()->expanded(watchVariableIndexAt(0)); // expand this node for correct update.
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    if (!verifyVariable(0, QStringLiteral("intList"), QStringLiteral("<size=2>"), QStringList{"10", "20"},
                        __FILE__, __LINE__, false, false, unordered)) {
        return;
    }

    m_session->stepOver();
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);
    QCOMPARE(m_session->currentLine(), 36); // line 37: Container<QString> stringList;

    if (!verifyVariable(0, QStringLiteral("intList"), QStringLiteral("<size=3>"), QStringList{"10", "20", "30"},
                        __FILE__, __LINE__, false, false, unordered)) {
        return;
    }
    var->die();

    // <QString>
    m_session->stepOver();
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);
    QCOMPARE(m_session->currentLine(), 37); // line 38: stringList << "a" << "bc";

    var = variableCollection()->watches()->add(QStringLiteral("stringList"));
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    if (!verifyVariable(0, QStringLiteral("stringList"), QStringLiteral("<size=0>"), QStringList{},
                        __FILE__, __LINE__, false, false, unordered)) {
        return;
    }

    m_session->stepOver();
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);
    QCOMPARE(m_session->currentLine(), 38); // line 39: stringList << "d";

    variableCollection()->expanded(watchVariableIndexAt(0)); // expand this node for correct update.
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);


    if (!verifyVariable(0, QStringLiteral("stringList"), QStringLiteral("<size=2>"), QStringList{"\"a\"", "\"bc\""},
                        __FILE__, __LINE__, false, false, unordered)) {
        return;
    }

    m_session->stepOver();
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);
    QCOMPARE(m_session->currentLine(), 40); // line 41: Container<A> structList;

    if (!verifyVariable(0, QStringLiteral("stringList"), QStringLiteral("<size=3>"), QStringList{"\"a\"", "\"bc\"", "\"d\""},
                        __FILE__, __LINE__, false, false, unordered)) {
        return;
    }
    var->die();

    // <struct A>
    m_session->stepOver();
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);
    QCOMPARE(m_session->currentLine(), 41); // line 42: structList << A(QStringLiteral("a"), QStringLiteral("b"), 100, -200);

    var = variableCollection()->watches()->add(QStringLiteral("structList"));
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    if (!verifyVariable(0, QStringLiteral("structList"), QStringLiteral("<size=0>"), QStringList{},
                        __FILE__, __LINE__, false, false, unordered)) {
        return;
    }

    m_session->runUntil({}, 43);
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);
    QCOMPARE(m_session->currentLine(), 42); // line 43: structList << A();

    variableCollection()->expanded(watchVariableIndexAt(0)); // expand this node for correct update.
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    if (!verifyVariable(0, QStringLiteral("structList"), QStringLiteral("<size=1>"), QStringList{"{...}"},
                        __FILE__, __LINE__, false, false, unordered)) {
        return;
    }

    m_session->stepOver();
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);
    QCOMPARE(m_session->currentLine(), 44); // line 45: Container<int*> pointerList;

    if (!verifyVariable(0, QStringLiteral("structList"), QStringLiteral("<size=2>"), QStringList{"{...}", "{...}"},
                        __FILE__, __LINE__, false, false, unordered)) {
        return;
    }
    var->die();

    // <int*>
    m_session->stepOver();
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);
    QCOMPARE(m_session->currentLine(), 45); // line 46: pointerList << new int(1) << new int(2);

    var = variableCollection()->watches()->add(QStringLiteral("pointerList"));
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    if (!verifyVariable(0, QStringLiteral("pointerList"), QStringLiteral("<size=0>"), QStringList{},
                        __FILE__, __LINE__, false, false, unordered)) {
        return;
    }

    m_session->stepOver();
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);
    QCOMPARE(m_session->currentLine(), 46); // line 47: pointerList << new int(3);

    variableCollection()->expanded(watchVariableIndexAt(0)); // expand this node for correct update.
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    if (!verifyVariable(0, QStringLiteral("pointerList"), QStringLiteral("<size=2>"), QStringList{"^0x[0-9A-Fa-f]+$", "^0x[0-9A-Fa-f]+$"},
                        __FILE__, __LINE__, false, true, unordered)) {
        return;
    }

    m_session->stepOver();
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);
    QCOMPARE(m_session->currentLine(), 47); // line 48: qDeleteAll(pointerList);

    if (!verifyVariable(0, QStringLiteral("pointerList"), QStringLiteral("<size=3>"), QStringList{"^0x[0-9A-Fa-f]+$", "^0x[0-9A-Fa-f]+$",
                                                                  "^0x[0-9A-Fa-f]+$"},
                        __FILE__, __LINE__, false, true, unordered)) {
        return;
    }
    var->die();
    m_session->stepOver(); // step over qDeleteAll

    // <QPair<int, int>>
    m_session->stepOver();
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);
    QCOMPARE(m_session->currentLine(), 50); // line 51: pairList << QPair<int, int>(1, 2) << qMakePair(2, 3);

    var = variableCollection()->watches()->add(QStringLiteral("pairList"));
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    VERIFY_WATCH(0, "pairList", "<size=0>", QStringList{});

    m_session->stepOver();
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);
    QCOMPARE(m_session->currentLine(), 51); // line 52: pairList << qMakePair(4, 5);

    variableCollection()->expanded(watchVariableIndexAt(0)); // expand this node for correct update.
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    if (!verifyVariable(0, QStringLiteral("pairList"), QStringLiteral("<size=2>"), QStringList{"{...}", "{...}"},
                        __FILE__, __LINE__, false, false, unordered)) {
        return;
    }

    m_session->stepOver();
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);
    QCOMPARE(m_session->currentLine(), 54); // line 55: int i = 0;

    if (!verifyVariable(0, QStringLiteral("pairList"), QStringLiteral("<size=3>"), QStringList{"{...}", "{...}", "{...}"},
                        __FILE__, __LINE__, false, false, unordered)) {
        return;
    }
    var->die();
}

void LldbFormattersTest::testQListPOD()
{
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_qlistpod"));
    addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile("qlistpod.cpp")), 30);

    QVERIFY(m_session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);

    // Should be two rows ('auto', 'local')
    QCOMPARE(variableCollection()->rowCount(), 2);

    auto watchRoot = variableCollection()->indexForItem(variableCollection()->watches(), 0);
    variableCollection()->expanded(watchRoot);
    variableCollection()->variableWidgetShown();
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    variableCollection()->watches()->add(QStringLiteral("b"));
    variableCollection()->watches()->add(QStringLiteral("c"));
    variableCollection()->watches()->add(QStringLiteral("uc"));
    variableCollection()->watches()->add(QStringLiteral("s"));
    variableCollection()->watches()->add(QStringLiteral("us"));
    variableCollection()->watches()->add(QStringLiteral("i"));
    variableCollection()->watches()->add(QStringLiteral("ui"));
    variableCollection()->watches()->add(QStringLiteral("l"));
    variableCollection()->watches()->add(QStringLiteral("ul"));
    variableCollection()->watches()->add(QStringLiteral("i64"));
    variableCollection()->watches()->add(QStringLiteral("ui64"));
    variableCollection()->watches()->add(QStringLiteral("f"));
    variableCollection()->watches()->add(QStringLiteral("d"));
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    VERIFY_WATCH(0, "b", "<size=1>", (QStringList{"false"}));
    VERIFY_WATCH(1, "c", "<size=1>", (QStringList{"50 '2'"}));
    VERIFY_WATCH(2, "uc", "<size=1>", (QStringList{"50 '2'"}));

    VERIFY_WATCH(3, "s", "<size=1>", (QStringList{"50"}));
    VERIFY_WATCH(4, "us", "<size=1>", (QStringList{"50"}));

    VERIFY_WATCH(5, "i", "<size=1>", (QStringList{"50"}));
    VERIFY_WATCH(6, "ui", "<size=1>", (QStringList{"50"}));

    VERIFY_WATCH(7, "l", "<size=1>", (QStringList{"50"}));
    VERIFY_WATCH(8, "ul", "<size=1>", (QStringList{"50"}));

    VERIFY_WATCH(9, "i64", "<size=1>", (QStringList{"50"}));
    VERIFY_WATCH(10, "ui64", "<size=1>", (QStringList{"50"}));

    VERIFY_WATCH(11, "f", "<size=1>", (QStringList{"50"}));
    VERIFY_WATCH(12, "d", "<size=1>", (QStringList{"50"}));
}

void LldbFormattersTest::testQMapInt()
{
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_qmapint"));
    addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile("qmapint.cpp")), 6);

    QVERIFY(m_session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);

    // Should be two rows ('auto', 'local')
    QCOMPARE(variableCollection()->rowCount(), 2);

    variableCollection()->expanded(localVariableIndexAt(0));
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    VERIFY_LOCAL(0, "m", "<size=2>", (QStringList{"(10, 100)", "(20, 200)"}));

    m_session->stepOver();
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);
    QCOMPARE(m_session->currentLine(), 7);

    VERIFY_LOCAL(0, "m", "<size=3>", (QStringList{"(10, 100)", "(20, 200)", "(30, 300)"}));
}

void LldbFormattersTest::testQMapString()
{
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_qmapstring"));
    addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile("qmapstring.cpp")), 7);

    QVERIFY(m_session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);
    QCOMPARE(m_session->currentLine(), 7); // line 8: m[QStringLiteral("30")] = QStringLiteral("300");

    // Should be two rows ('auto', 'local')
    QCOMPARE(variableCollection()->rowCount(), 2);

    variableCollection()->expanded(localVariableIndexAt(0));
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    VERIFY_LOCAL(0, "m", "<size=2>", (QStringList{"(\"10\", \"100\")", "(\"20\", \"200\")"}));

    m_session->stepOver();
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);
    QCOMPARE(m_session->currentLine(), 8); // line 9: return 0;

    VERIFY_LOCAL(0, "m", "<size=3>",
                 (QStringList{"(\"10\", \"100\")", "(\"20\", \"200\")", "(\"30\", \"300\")"}));
}

void LldbFormattersTest::testQMapStringBool()
{
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_qmapstringbool"));
    addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile("qmapstringbool.cpp")), 7);

    QVERIFY(m_session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);

    // Should be two rows ('auto', 'local')
    QCOMPARE(variableCollection()->rowCount(), 2);

    variableCollection()->expanded(localVariableIndexAt(0));
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    VERIFY_LOCAL(0, "m", "<size=2>", (QStringList{"(\"10\", true)", "(\"20\", false)"}));

    m_session->stepOver();
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);
    QCOMPARE(m_session->currentLine(), 8);

    VERIFY_LOCAL(0, "m", "<size=3>", (QStringList{"(\"10\", true)", "(\"20\", false)", "(\"30\", true)"}));
}


void LldbFormattersTest::testQHashInt()
{
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_qhashint"));
    addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile("qhashint.cpp")), 6);

    QVERIFY(m_session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);

    // Should be two rows ('auto', 'local')
    QCOMPARE(variableCollection()->rowCount(), 2);

    variableCollection()->expanded(localVariableIndexAt(0));
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    if (!verifyVariable(0, QStringLiteral("h"), QStringLiteral("<size=2>"), QStringList{"(10, 100)", "(20, 200)"},
                        __FILE__, __LINE__, true, false, true)) {
        return;
    }

    m_session->stepOver();
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);
    QCOMPARE(m_session->currentLine(), 7);

    if (!verifyVariable(0, QStringLiteral("h"), QStringLiteral("<size=3>"), QStringList{"(10, 100)", "(20, 200)", "(30, 300)"},
                        __FILE__, __LINE__, true, false, true)) {
        return;
    }
}

void LldbFormattersTest::testQHashString()
{
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_qhashstring"));
    addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile("qhashstring.cpp")), 7);

    QVERIFY(m_session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);

    // Should be two rows ('auto', 'local')
    QCOMPARE(variableCollection()->rowCount(), 2);

    variableCollection()->expanded(localVariableIndexAt(0));
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    if (!verifyVariable(0, QStringLiteral("h"), QStringLiteral("<size=2>"), QStringList{"(\"10\", \"100\")", "(\"20\", \"200\")"},
                        __FILE__, __LINE__, true, false, true)) {
        return;
    }

    m_session->stepOver();
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);
    QCOMPARE(m_session->currentLine(), 8);

    if (!verifyVariable(0, QStringLiteral("h"), QStringLiteral("<size=3>"),
                        {"(\"10\", \"100\")", "(\"20\", \"200\")", "(\"30\", \"300\")"},
                        __FILE__, __LINE__, true, false, true)) {
        return;
    }
}

void LldbFormattersTest::testQSetInt()
{
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_qsetint"));
    addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile("qsetint.cpp")), 6);

    QVERIFY(m_session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);

    // Should be two rows ('auto', 'local')
    QCOMPARE(variableCollection()->rowCount(), 2);

    variableCollection()->expanded(localVariableIndexAt(0));
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    if (!verifyVariable(0, QStringLiteral("s"), QStringLiteral("<size=2>"), QStringList{"10", "20"},
                        __FILE__, __LINE__, true, false, true)) {
        return;
    }

    m_session->stepOver();
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);
    QCOMPARE(m_session->currentLine(), 7);

    if (!verifyVariable(0, QStringLiteral("s"), QStringLiteral("<size=3>"), QStringList{"10", "20", "30"},
                        __FILE__, __LINE__, true, false, true)) {
        return;
    }
}

void LldbFormattersTest::testQSetString()
{
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_qsetstring"));
    addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile("qsetstring.cpp")), 7);

    QVERIFY(m_session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);

    // Should be two rows ('auto', 'local')
    QCOMPARE(variableCollection()->rowCount(), 2);

    variableCollection()->expanded(localVariableIndexAt(0));
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    if (!verifyVariable(0, QStringLiteral("s"), QStringLiteral("<size=2>"), QStringList{"\"10\"", "\"20\""},
                        __FILE__, __LINE__, true, false, true)) {
        return;
    }

    m_session->stepOver();
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);
    QCOMPARE(m_session->currentLine(), 8);

    if (!verifyVariable(0, QStringLiteral("s"), QStringLiteral("<size=3>"),
                        {"\"10\"", "\"20\"", "\"30\""},
                        __FILE__, __LINE__, true, false, true)) {
        return;
    }
}

void LldbFormattersTest::testQDate()
{
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_qdate"));
    addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile("qdate.cpp")), 5);

    QVERIFY(m_session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);

    // Should be two rows ('auto', 'local')
    QCOMPARE(variableCollection()->rowCount(), 2);

    variableCollection()->expanded(localVariableIndexAt(0));
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    QList<QPair<QString, QString>> children;
    children.append({QStringLiteral("jd"), QStringLiteral("2455217")});
    children.append({QStringLiteral("(ISO)"), QStringLiteral("\"2010-01-20\"")});
    children.append({QStringLiteral("(Locale)"), QStringLiteral("\".+\"")}); // (Locale) and summary are locale dependent
    if (!verifyVariable(0, QStringLiteral("d"), QStringLiteral(".+"), children,
                        __FILE__, __LINE__, true, true, false)) {
        return;
    }
}

void LldbFormattersTest::testQTime()
{
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_qtime"));
    addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile("qtime.cpp")), 5);

    QVERIFY(m_session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);

    // Should be two rows ('auto', 'local')
    QCOMPARE(variableCollection()->rowCount(), 2);

    variableCollection()->expanded(localVariableIndexAt(0));
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    QList<QPair<QString, QString>> children;
    children.append({QStringLiteral("mds"), QStringLiteral("55810123")});
    children.append({QStringLiteral("(ISO)"), QStringLiteral("\"15:30:10.000123\"")});
    children.append({QStringLiteral("(Locale)"), QStringLiteral("\".+\"")}); // (Locale) and summary are locale dependent
    if (!verifyVariable(0, QStringLiteral("t"), QStringLiteral(".+"), children,
                        __FILE__, __LINE__, true, true, false)) {
        return;
    }
}

void LldbFormattersTest::testQDateTime()
{
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_qdatetime"));
    addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile("qdatetime.cpp")), 5);

    QVERIFY(m_session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);

    // Should be two rows ('auto', 'local')
    QCOMPARE(variableCollection()->rowCount(), 2);

    variableCollection()->expanded(localVariableIndexAt(0));
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    QList<QPair<QString, QString>> children;
    children.append({QStringLiteral("toSecsSinceEpoch"), QStringLiteral("1264019473")});
    children.append({QStringLiteral("(ISO)"), QStringLiteral("\"2010-01-20 20:31:13\"")});
    children.append({QStringLiteral("(Locale)"), QStringLiteral("\".+\"")}); // (Locale), (UTC) and summary are locale dependent
    children.append({QStringLiteral("(UTC)"), QStringLiteral("\".+\"")});
    if (!verifyVariable(0, QStringLiteral("dt"), QStringLiteral(".+"), children,
                        __FILE__, __LINE__, true, true, false)) {
        return;
    }
}

void LldbFormattersTest::testQUrl()
{
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_qurl"));
    addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile("qurl.cpp")), 4);

    QVERIFY(m_session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);

    // Should be two rows ('auto', 'local')
    QCOMPARE(variableCollection()->rowCount(), 2);

    variableCollection()->expanded(localVariableIndexAt(0));
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    QList<QPair<QString, QString>> children;
    children.append({QStringLiteral("(port)"), QStringLiteral("12345")});
    children.append({QStringLiteral("(scheme)"), QStringLiteral("\"http\"")});
    children.append({QStringLiteral("(userName)"), QStringLiteral("\"user\"")});
    children.append({QStringLiteral("(password)"), QStringLiteral("<Invalid>")});
    children.append({QStringLiteral("(host)"), QStringLiteral("\"www.kdevelop.org\"")});
    children.append({QStringLiteral("(path)"), QStringLiteral("\"/foo\"")});
    children.append({QStringLiteral("(query)"), QStringLiteral("\"xyz=bar\"")});
    children.append({QStringLiteral("(fragment)"), QStringLiteral("\"asdf\"")});
    VERIFY_LOCAL(0, "u", "http://user@www.kdevelop.org:12345/foo?xyz=bar#asdf", children);
}

void LldbFormattersTest::testQUuid()
{
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_quuid"));
    addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile("quuid.cpp")), 4);

    QVERIFY(m_session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);

    // Should be two rows ('auto', 'local')
    QCOMPARE(variableCollection()->rowCount(), 2);

    variableCollection()->expanded(localVariableIndexAt(0));
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    VERIFY_LOCAL(0, "id", "QUuid({9ec3b70b-d105-42bf-b3b4-656e44d2e223})", (QStringList{}));
}

void LldbFormattersTest::testKTextEditorTypes()
{
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_ktexteditortypes"));
    addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile("ktexteditortypes.cpp")), 8);

    QVERIFY(m_session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);

    // Should be two rows ('auto', 'local')
    QCOMPARE(variableCollection()->rowCount(), 2);

    auto watchRoot = variableCollection()->indexForItem(variableCollection()->watches(), 0);
    variableCollection()->expanded(watchRoot);
    variableCollection()->variableWidgetShown();
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    variableCollection()->watches()->add(QStringLiteral("cursor"));
    variableCollection()->watches()->add(QStringLiteral("range"));
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    QList<QPair<QString, QString>> cursorChildren;
    cursorChildren.append({QStringLiteral("m_line"), QStringLiteral("1")});
    cursorChildren.append({QStringLiteral("m_column"), QStringLiteral("1")});

    QList<QPair<QString, QString>> rangeChildren;
    rangeChildren.append({QStringLiteral("m_start"), QStringLiteral("(1, 1)")});
    rangeChildren.append({QStringLiteral("m_end"), QStringLiteral("(2, 2)")});

    VERIFY_WATCH(0, "cursor", "(1, 1)", cursorChildren);
    VERIFY_WATCH(1, "range", "[(1, 1) -> (2, 2)]", rangeChildren);
}

void LldbFormattersTest::testKDevelopTypes()
{
    TestLaunchConfiguration cfg(QStringLiteral("debuggee_kdeveloptypes"));
    addCodeBreakpoint(QUrl::fromLocalFile(findSourceFile("kdeveloptypes.cpp")), 11);

    QVERIFY(m_session->startDebugging(&cfg, m_iface));
    WAIT_FOR_STATE_AND_IDLE(m_session, DebugSession::PausedState);

    // Should be two rows ('auto', 'local')
    QCOMPARE(variableCollection()->rowCount(), 2);

    auto watchRoot = variableCollection()->indexForItem(variableCollection()->watches(), 0);
    variableCollection()->expanded(watchRoot);
    variableCollection()->variableWidgetShown();
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    variableCollection()->watches()->add(QStringLiteral("path1"));
    variableCollection()->watches()->add(QStringLiteral("path2"));
    WAIT_FOR_A_WHILE_AND_IDLE(m_session, 50);

    QList<QPair<QString, QString>> path1Children;
    path1Children.append({QStringLiteral("m_data"), QStringLiteral("<size=2>")});

    QList<QPair<QString, QString>> path2Children;
    path2Children.append({QStringLiteral("m_data"), QStringLiteral("<size=3>")});

    VERIFY_WATCH(0, "path1", "(\"tmp\", \"foo\")", path1Children);
    VERIFY_WATCH(1, "path2", "(\"http://www.test.com\", \"tmp\", \"asdf.txt\")", path2Children);
}

QTEST_MAIN(LldbFormattersTest)

#include "test_lldbformatters.moc"
#include "moc_test_lldbformatters.cpp"
