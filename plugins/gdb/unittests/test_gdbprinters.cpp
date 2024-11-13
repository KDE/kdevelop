/*
    SPDX-FileCopyrightText: 2010 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "test_gdbprinters.h"

#include "tests/debuggers-tests-config.h"

#include <tests/testhelpermacros.h>

#include <QCoreApplication>
#include <QTest>
#include <QProcess>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>
#include <QStandardPaths>

namespace {

const QString BINARY_PATH(DEBUGGEE_BIN_DIR);

class GdbProcess : private QProcess
{
    Q_OBJECT

public:
    explicit GdbProcess(const QString &program) : QProcess()
    {
        setProcessChannelMode(MergedChannels);
        // don't attempt to load .gdbinit in home (may cause unexpected results)
        QProcess::start(QStringLiteral("gdb"), (QStringList() << QStringLiteral("-nx") << (BINARY_PATH + '/' + program)));
        const bool started = waitForStarted();
        if (!started) {
            qDebug() << "Failed to start 'gdb' executable:" << errorString();
            Q_ASSERT(false);
            return;
        }

        QByteArray prompt = waitForPrompt();
        QVERIFY(!prompt.contains("No such file or directory"));
        execute("set confirm off");
        execute("set print pretty on");
        execute("set disable-randomization off"); // see https://phabricator.kde.org/D2188
        QList<QByteArray> p;
        QDir printersDir = QFileInfo(__FILE__).dir();
        printersDir.cdUp(); // go up to get to the main printers directory
        QVERIFY(printersDir.cd("printers"));
        p << "python"
          << "import sys"
          << "sys.path.insert(0, '"+printersDir.path().toLatin1()+"')"
          << "from qt import register_qt_printers"
          << "register_qt_printers (None)"
          << "from kde import register_kde_printers"
          << "register_kde_printers (None)"
          << "end";
        for (const QByteArray& i : std::as_const(p)) {
            write(i + "\n");
        }
        waitForPrompt();
    }
    ~GdbProcess() override
    {
        write("q\n");
        waitForFinished();
    }
    QByteArray waitForPrompt()
    {
        QByteArray output;
        while (!output.endsWith("(gdb) ")) {
            Q_ASSERT(state() == QProcess::Running);
            waitForReadyRead();
            QByteArray l = readAll();
            //qDebug() << l;
            output.append(l);
        }
        output.chop(7); //remove (gdb) prompt
        if (output.contains("Traceback") || output.contains("Exception")) {
            qDebug() << output;
            QTest::qFail("Unexpected Python Exception", __FILE__, __LINE__);
        }
        return output;
    }

    QByteArray execute(const QByteArray &cmd)
    {
        write(cmd + "\n");
        auto out = waitForPrompt();
        qDebug() << cmd << " = " << out;
        return out;
    }
};

[[nodiscard]] bool containsConsecutiveElements(const QByteArray& out, QStringList elements)
{
    for (auto& e : elements) {
        e = QRegularExpression::escape(e);
    }
    const auto pattern = elements.join("[,\\s]+");
    return QString::fromUtf8(out).contains(QRegularExpression{pattern});
}

[[nodiscard]] QByteArray printedValue(GdbProcess& gdb, const QByteArray& expression)
{
    // Do not use the GDB `output` command, because we want to test the `print` command,
    // which is more convenient for command line usage thanks to the newline character at end.
    auto output = gdb.execute("print " + expression);

    constexpr QByteArrayView separator(" = ");
    const auto pos = output.indexOf(separator);
    QVERIFY_RETURN(pos != -1, {});

    output.remove(0, pos + separator.size());
    return output;
}

} // unnamed namespace

void QtPrintersTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);

    // Prevent SIGPIPE, then "ICE default IO error handler doing an exit(), pid = <PID>, errno = 32"
    // crash when the test runs for at least 60 seconds. This is a workaround for QTBUG-58709.
    QCoreApplication::processEvents();
}

void QtPrintersTest::testQString()
{
    GdbProcess gdb(QStringLiteral("debuggee_qstring"));
    gdb.execute("break qstring.cpp:5");
    gdb.execute("run");

    QCOMPARE(printedValue(gdb, "s"), "\"test最后一个不是特殊字符'\\\"\\\\u6211\"");
    gdb.execute("next");
    QCOMPARE(printedValue(gdb, "s"), "\"test最后一个不是特殊字符'\\\"\\\\u6211x\"");
    gdb.execute("next");
    QCOMPARE(printedValue(gdb, "view"), "\"test最后一个不是特殊字符'\\\"\\\\u6211x\"");
    gdb.execute("next");
    QCOMPARE(printedValue(gdb, "view"), "\"test最\"");
    gdb.execute("next");
    QCOMPARE(printedValue(gdb, "latin1String"), "\"abcé\"");
    gdb.execute("next");
    QCOMPARE(printedValue(gdb, "utf8StringView"), "\"test最后\"");

    gdb.execute("break qstring.cpp:16");
    gdb.execute("continue");
    constexpr const char* emptyString = "\"\"";
    QCOMPARE(printedValue(gdb, "nullString"), emptyString);
    QCOMPARE(printedValue(gdb, "emptyString"), emptyString);
    QCOMPARE(printedValue(gdb, "emptyView"), emptyString);
    QCOMPARE(printedValue(gdb, "emptyLatin1String"), emptyString);
    QCOMPARE(printedValue(gdb, "emptyUtf8StringView"), emptyString);
}

void QtPrintersTest::testQByteArray()
{
    GdbProcess gdb(QStringLiteral("debuggee_qbytearray"));
    gdb.execute("break qbytearray.cpp:5");
    gdb.execute("run");
    QByteArray out = gdb.execute("print ba");
    qDebug() << out;
    QVERIFY(out.contains("\"\xE6\x98\xAF'\\\"\\\\u6211\""));
    QVERIFY(out.contains("[0] = -26 '\\346'"));
    QVERIFY(out.contains("[3] = 39 '\\''"));
    QVERIFY(out.contains("[4] = 34 '\"'"));
    gdb.execute("next");
    out = gdb.execute("print ba");
    QVERIFY(out.contains("\"\xE6\x98\xAF'\\\"\\\\u6211x\""));
    gdb.execute("next");
    out = gdb.execute("print nonUtf");
    QVERIFY(out.contains(R"( = "ABCÿ\000þ")"));
}

void QtPrintersTest::testQListContainer_data()
{
    QTest::addColumn<QString>("container");

    QTest::newRow("QList") << "QList";
    QTest::newRow("QQueue") << "QQueue";
    QTest::newRow("QStack") << "QStack";
    QTest::newRow("QLinkedList") << "QLinkedList";
    QTest::newRow("QSet") << "QSet";
}

void QtPrintersTest::testQListContainer()
{
    QFETCH(QString, container);
    GdbProcess gdb(QStringLiteral("debuggee_qlistcontainer"));
    gdb.execute("break main");
    gdb.execute("run");
    gdb.execute(QStringLiteral("break 'doStuff<%1>()'").arg(container).toLocal8Bit());
    gdb.execute("cont");

    auto runToLine = [&](int line) {
        gdb.execute("tbreak qlistcontainer.cpp:" + QByteArray::number(line));
        gdb.execute("cont");
    };

    { // <int>
        runToLine(34);
        QByteArray out = gdb.execute("print intList");
        qWarning() << "FOO" << out;
        QVERIFY(out.contains(QString("%1<int> (size = 0)").arg(container).toLocal8Bit()));
        gdb.execute("next");
        out = gdb.execute("print intList");
        QVERIFY(out.contains(QString("%1<int> (size = 2)").arg(container).toLocal8Bit()));
        if (container != QLatin1String("QSet")) {
            QVERIFY(out.contains("[0] = 10"));
            QVERIFY(out.contains("[1] = 20"));
            QVERIFY(!out.contains("[2] = 30"));
        } else { // QSet order is undefined
            QVERIFY(out.contains("] = 10"));
            QVERIFY(out.contains("] = 20"));
            QVERIFY(!out.contains("] = 30"));
        }
    gdb.execute("next");
    out = gdb.execute("print intList");
    QVERIFY(out.contains(QString("%1<int> (size = 3)").arg(container).toLocal8Bit()));
    if (container != QLatin1String("QSet")) {
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
        runToLine(38);
        QByteArray out = gdb.execute("print stringList");
        QVERIFY(out.contains(QString("%1<QString> (size = 0)").arg(container).toLocal8Bit()));
        gdb.execute("next");
        out = gdb.execute("print stringList");
        QVERIFY(out.contains(QString("%1<QString> (size = 2)").arg(container).toLocal8Bit()));
        if (container != QLatin1String("QSet")) {
            QVERIFY(out.contains("[0] = \"a\""));
            QVERIFY(out.contains("[1] = \"bc\""));
            QVERIFY(!out.contains("[2] = \"d\""));
        } else { // QSet order is undefined
            QVERIFY(out.contains("] = \"a\""));
            QVERIFY(out.contains("] = \"bc\""));
            QVERIFY(!out.contains("] = \"d\""));
        }
    gdb.execute("next");
    out = gdb.execute("print stringList");
    QVERIFY(out.contains(QString("%1<QString> (size = 3)").arg(container).toLocal8Bit()));
    if (container != QLatin1String("QSet")) {
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
        runToLine(42);
        QByteArray out = gdb.execute("print structList");
        QVERIFY(out.contains(QString("%1<A> (size = 0)").arg(container).toLocal8Bit()));
        gdb.execute("next");
        out = gdb.execute("print structList");
        QVERIFY(out.contains(QString("%1<A> (size = 1)").arg(container).toLocal8Bit()));
        QVERIFY(out.contains("[0] = {"));
        QVERIFY(out.contains("a = \"a\""));
        QVERIFY(out.contains("b = \"b\""));
        QVERIFY(out.contains("c = 100"));
        QVERIFY(out.contains("d = -200"));
        gdb.execute("next");
        out = gdb.execute("print structList");
        QVERIFY(out.contains(QString("%1<A> (size = 2)").arg(container).toLocal8Bit()));
        QVERIFY(out.contains("[1] = {"));
    }
    { // <int*>
        runToLine(46);
        QByteArray out = gdb.execute("print pointerList");
        QVERIFY(out.contains(QString("%1<int *> (size = 0)").arg(container).toLocal8Bit()));
        gdb.execute("next");
        out = gdb.execute("print pointerList");
        QVERIFY(out.contains(QString("%1<int *> (size = 2)").arg(container).toLocal8Bit()));
        QVERIFY(out.contains("[0] = 0x"));
        QVERIFY(out.contains("[1] = 0x"));
        QVERIFY(!out.contains("[2] = 0x"));
        gdb.execute("next");
        out = gdb.execute("print pointerList");
        QVERIFY(out.contains(QString("%1<int *> (size = 3)").arg(container).toLocal8Bit()));
        QVERIFY(out.contains("[0] = 0x"));
        QVERIFY(out.contains("[1] = 0x"));
        QVERIFY(out.contains("[2] = 0x"));
    }
    { // <QPair<int, int> >
        runToLine(51);
        QByteArray out = gdb.execute("print pairList");
        QVERIFY(out.contains(QString("%1<std::pair<int, int>> (size = 0)").arg(container).toLocal8Bit()));
        gdb.execute("next");
        out = gdb.execute("print pairList");
        QVERIFY(out.contains(QString("%1<std::pair<int, int>> (size = 2)").arg(container).toLocal8Bit()));
        if (container != QLatin1String("QSet")) {
            QVERIFY(out.contains("[0] = {\n    first = 1,\n    second = 2\n  }"));
            QVERIFY(out.contains("[1] = {\n    first = 2,\n    second = 3\n  }"));
        } else { // order is undefined in QSet
            QVERIFY(out.contains("] = {\n    first = 1,\n    second = 2\n  }"));
            QVERIFY(out.contains("] = {\n    first = 2,\n    second = 3\n  }"));
        }
    QVERIFY(!out.contains("[2] = "));
    gdb.execute("next");
    out = gdb.execute("print pairList");
    QVERIFY(out.contains(QString("%1<std::pair<int, int>> (size = 3)").arg(container).toLocal8Bit()));
    if (container != QLatin1String("QSet")) {
        QVERIFY(out.contains("[0] = {\n    first = 1,\n    second = 2\n  }"));
        QVERIFY(out.contains("[1] = {\n    first = 2,\n    second = 3\n  }"));
        QVERIFY(out.contains("[2] = {\n    first = 4,\n    second = 5\n  }"));
    } else { // order is undefined in QSet
        QVERIFY(out.contains("] = {\n    first = 1,\n    second = 2\n  }"));
        QVERIFY(out.contains("] = {\n    first = 2,\n    second = 3\n  }"));
        QVERIFY(out.contains("] = {\n    first = 4,\n    second = 5\n  }"));
    }
    }
}

[[nodiscard]] static bool isMissingStdMapPrettyPrinter(GdbProcess& gdb)
{
    return !gdb.execute("print m.d.d.m").contains("std::map");
}

[[nodiscard]] static bool verifyContainsMapElementCount(const QByteArray& out, const char* key, const char* value,
                                                        int elementCount)
{
    const auto pattern = QLatin1String("QMap<%1, %2> (size = %3)")
                             .arg(key, value, elementCount == -1 ? "?" : QString::number(elementCount));
    return out.contains(pattern.toUtf8());
}

void QtPrintersTest::testQMapInt()
{
    GdbProcess gdb(QStringLiteral("debuggee_qmapint"));

    gdb.execute("break qmapint.cpp:5");
    gdb.execute("run");
    QByteArray out = gdb.execute("print m");
    QVERIFY(verifyContainsMapElementCount(out, "int", "int", 0));

    gdb.execute("next");
    out = gdb.execute("print m");

    if (isMissingStdMapPrettyPrinter(gdb)) {
        QVERIFY(verifyContainsMapElementCount(out, "int", "int", -1));
        QSKIP("QMap pretty printing relies on availability of the std::map pretty printer");
    }

    QVERIFY(verifyContainsMapElementCount(out, "int", "int", 1));
    QVERIFY(out.contains("[10] = 100"));

    gdb.execute("next");
    out = gdb.execute("print m");
    QVERIFY(verifyContainsMapElementCount(out, "int", "int", 2));
    QVERIFY(out.contains("[10] = 100"));
    QVERIFY(out.contains("[20] = 200"));

    gdb.execute("next");
    out = gdb.execute("print m");
    QVERIFY(verifyContainsMapElementCount(out, "int", "int", 3));
    QVERIFY(out.contains("[30] = 300"));
}

void QtPrintersTest::testQMapString()
{
    GdbProcess gdb(QStringLiteral("debuggee_qmapstring"));
    gdb.execute("break qmapstring.cpp:8");
    gdb.execute("run");

    QByteArray out = gdb.execute("print m");

    if (isMissingStdMapPrettyPrinter(gdb)) {
        QVERIFY(verifyContainsMapElementCount(out, "QString", "QString", -1));
        QSKIP("QMap pretty printing relies on availability of the std::map pretty printer");
    }

    QVERIFY(verifyContainsMapElementCount(out, "QString", "QString", 2));
    QVERIFY(out.contains("[\"10\"] = \"100\""));
    QVERIFY(out.contains("[\"20\"] = \"200\""));
    gdb.execute("next");
    out = gdb.execute("print m");
    QVERIFY(verifyContainsMapElementCount(out, "QString", "QString", 3));
    QVERIFY(out.contains("[\"30\"] = \"300\""));
}

void QtPrintersTest::testQMapStringBool()
{
    GdbProcess gdb(QStringLiteral("debuggee_qmapstringbool"));
    gdb.execute("break qmapstringbool.cpp:8");
    gdb.execute("run");

    QByteArray out = gdb.execute("print m");

    if (isMissingStdMapPrettyPrinter(gdb)) {
        QVERIFY(verifyContainsMapElementCount(out, "QString", "bool", -1));
        QSKIP("QMap pretty printing relies on availability of the std::map pretty printer");
    }

    QVERIFY(verifyContainsMapElementCount(out, "QString", "bool", 2));
    QVERIFY(out.contains("[\"10\"] = true"));
    QVERIFY(out.contains("[\"20\"] = false"));
    gdb.execute("next");
    out = gdb.execute("print m");
    QVERIFY(verifyContainsMapElementCount(out, "QString", "bool", 3));
    QVERIFY(out.contains("[\"30\"] = true"));
}


void QtPrintersTest::testQDate()
{
    GdbProcess gdb(QStringLiteral("debuggee_qdate"));
    gdb.execute("break qdate.cpp:6");
    gdb.execute("run");
    QByteArray out = gdb.execute("print d");
    QVERIFY(out.contains("2010-01-20"));
}

void QtPrintersTest::testQTime()
{
    GdbProcess gdb(QStringLiteral("debuggee_qtime"));
    gdb.execute("break qtime.cpp:6");
    gdb.execute("run");
    QByteArray out = gdb.execute("print t");
    QVERIFY(out.contains("15:30:10.123"));
}

void QtPrintersTest::testQDateTime()
{
    GdbProcess gdb(QStringLiteral("debuggee_qdatetime"));
    gdb.execute("break qdatetime.cpp:5");
    gdb.execute("run");
    QByteArray out = gdb.execute("print dt");
    QVERIFY(out.contains("Wed Jan 20 15:31:13 2010"));
}

void QtPrintersTest::testQUrl()
{
    GdbProcess gdb(QStringLiteral("debuggee_qurl"));
    gdb.execute("break qurl.cpp:7");
    gdb.execute("run");

    QByteArray out = gdb.execute("print u");
    QVERIFY(out.contains("http://user@www.kdevelop.org:12345/foo?xyz=bar#asdf"));

    const auto localFile = gdb.execute("print localFile");
    QVERIFY(localFile.contains("file:///usr/bin/kdevelop"));

    const auto defaultConstructed = gdb.execute("print defaultConstructed");
    QVERIFY(defaultConstructed.contains("<invalid>"));
}

static void commonTestQHashOrMultiHashInt(GdbProcess& gdb, const QByteArray& containerName)
{
    const auto containerElementCount = [&containerName](int size) -> QByteArray {
        return containerName + "<int, int> (size = " + QByteArray::number(size) + ")";
    };

    QByteArray out = gdb.execute("print h");
    QVERIFY(out.contains(containerElementCount(0)));

    gdb.execute("next");
    out = gdb.execute("print h");
    QVERIFY(out.contains(containerElementCount(1)));
    QVERIFY(out.contains("[10] = 100"));

    gdb.execute("next");
    out = gdb.execute("print h");
    QVERIFY(out.contains(containerElementCount(2)));
    QVERIFY(out.contains("[10] = 100"));
    QVERIFY(out.contains("[20] = 200"));

    gdb.execute("next");
    out = gdb.execute("print h");
    QVERIFY(out.contains(containerElementCount(3)));
    QVERIFY(out.contains("[30] = 300"));
}

static void commonTestQHashOrMultiHashString(GdbProcess& gdb, const QByteArray& containerName)
{
    const auto containerElementCount = [&containerName](int size) -> QByteArray {
        return containerName + "<QString, QString> (size = " + QByteArray::number(size) + ")";
    };

    QByteArray out = gdb.execute("print h");
    QVERIFY(out.contains(containerElementCount(2)));
    QVERIFY(out.contains("[\"10\"] = \"100\""));
    QVERIFY(out.contains("[\"20\"] = \"200\""));

    gdb.execute("next");
    out = gdb.execute("print h");
    QVERIFY(out.contains(containerElementCount(3)));
    QVERIFY(out.contains("[\"30\"] = \"300\""));
}

void QtPrintersTest::testQHashInt()
{
    GdbProcess gdb(QStringLiteral("debuggee_qhashint"));
    gdb.execute("break qhashint.cpp:5");
    gdb.execute("run");

    commonTestQHashOrMultiHashInt(gdb, "QHash");
    RETURN_IF_TEST_FAILED();
}

void QtPrintersTest::testQHashString()
{
    GdbProcess gdb(QStringLiteral("debuggee_qhashstring"));
    gdb.execute("break qhashstring.cpp:8");
    gdb.execute("run");

    commonTestQHashOrMultiHashString(gdb, "QHash");
    RETURN_IF_TEST_FAILED();
}

void QtPrintersTest::testQMultiHashInt()
{
    GdbProcess gdb(QStringLiteral("debuggee_qmultihashint"));
    gdb.execute("break qmultihashint.cpp:5");
    gdb.execute("run");

    commonTestQHashOrMultiHashInt(gdb, "QMultiHash");
    RETURN_IF_TEST_FAILED();

    gdb.execute("next");
    QByteArray out = gdb.execute("print h");
    QVERIFY(out.contains("QMultiHash<int, int> (size = 4)"));
    QVERIFY(out.contains("[10] = 100"));
    QVERIFY(out.contains("[20] = 200"));
    QVERIFY(out.contains("[30] = 300"));
    QVERIFY(out.contains("[10] = 123"));

    gdb.execute("break qmultihashint.cpp:16");
    gdb.execute("cont");
    out = gdb.execute("print h");
    QVERIFY(out.contains("QMultiHash<int, int> (size = 7)"));
    QVERIFY(out.contains("[10] = 100"));
    QVERIFY(!out.contains("[20] = 200")); // removed
    QVERIFY(out.contains("[30] = 300"));
    QVERIFY(out.contains("[10] = 123"));
    QVERIFY(out.contains("[30] = 82"));
    QVERIFY(out.contains("[4] = 99"));
    QVERIFY(out.contains("[10] = 0"));

    // Now verify QMultiHash's guarantee: items that share the same key appear
    // consecutively, from the most recently to the least recently inserted value.
    QVERIFY(containsConsecutiveElements(out, {"[10] = 0", "[10] = 123", "[10] = 100"}));
    QVERIFY(containsConsecutiveElements(out, {"[30] = 300", "[30] = 82", "[30] = 300"}));
}

void QtPrintersTest::testQMultiHashString()
{
    GdbProcess gdb(QStringLiteral("debuggee_qmultihashstring"));
    gdb.execute("break qmultihashstring.cpp:8");
    gdb.execute("run");

    commonTestQHashOrMultiHashString(gdb, "QMultiHash");
    RETURN_IF_TEST_FAILED();

    gdb.execute("next");
    gdb.execute("next");
    QByteArray out = gdb.execute("print h");
    QVERIFY(out.contains("QMultiHash<QString, QString> (size = 5)"));
    QVERIFY(out.contains("[\"10\"] = \"100\""));
    QVERIFY(out.contains("[\"20\"] = \"200\""));
    QVERIFY(out.contains("[\"30\"] = \"300\""));
    QVERIFY(out.contains("[\"20\"] = \"x\""));
    QVERIFY(out.contains("[\"20\"] = \"11\""));
    QVERIFY(containsConsecutiveElements(out, {"[\"20\"] = \"11\"", "[\"20\"] = \"x\"", "[\"20\"] = \"200\""}));

    gdb.execute("next");
    out = gdb.execute("print h");
    QVERIFY(out.contains("QMultiHash<QString, QString> (size = 4)"));
    QVERIFY(out.contains("[\"10\"] = \"100\""));
    QVERIFY(out.contains("[\"20\"] = \"200\""));
    QVERIFY(out.contains("[\"30\"] = \"300\""));
    QVERIFY(!out.contains("[\"20\"] = \"x\"")); // removed
    QVERIFY(out.contains("[\"20\"] = \"11\""));
    QVERIFY(containsConsecutiveElements(out, {"[\"20\"] = \"11\"", "[\"20\"] = \"200\""}));
}

void QtPrintersTest::testQSetInt()
{
    GdbProcess gdb(QStringLiteral("debuggee_qsetint"));

    gdb.execute("break qsetint.cpp:5");
    gdb.execute("run");
    QByteArray out = gdb.execute("print s");
    QVERIFY(out.contains("QSet<int> (size = 0)"));

    gdb.execute("break qsetint.cpp:7");
    gdb.execute("cont");
    out = gdb.execute("print s");
    QVERIFY(out.contains("QSet<int> (size = 2)"));
    QVERIFY(out.contains("] = 10"));
    QVERIFY(out.contains("] = 20"));

    gdb.execute("next");
    out = gdb.execute("print s");
    QVERIFY(out.contains("QSet<int> (size = 3)"));
    QVERIFY(out.contains("] = 30"));
}

void QtPrintersTest::testQSetString()
{
    GdbProcess gdb(QStringLiteral("debuggee_qsetstring"));
    gdb.execute("break qsetstring.cpp:8");
    gdb.execute("run");
    QByteArray out = gdb.execute("print s");
    QVERIFY(out.contains("QSet<QString> (size = 2)"));
    QVERIFY(out.contains("] = \"10\""));
    QVERIFY(out.contains("] = \"20\""));
    gdb.execute("next");
    out = gdb.execute("print s");
    QVERIFY(out.contains("QSet<QString> (size = 3)"));
    QVERIFY(out.contains("] = \"30\""));
}

void QtPrintersTest::testQChar()
{
    GdbProcess gdb(QStringLiteral("debuggee_qchar"));
    gdb.execute("break qchar.cpp:5");
    gdb.execute("run");
    QVERIFY(gdb.execute("print c").contains("\"k\""));
}

void QtPrintersTest::testQListPOD()
{
    GdbProcess gdb(QStringLiteral("debuggee_qlistpod"));
    gdb.execute("break qlistpod.cpp:31");
    gdb.execute("run");
    QVERIFY(gdb.execute("print b").contains("false"));
    QVERIFY(gdb.execute("print c").contains("50"));
    QVERIFY(gdb.execute("print uc").contains("50"));
    QVERIFY(gdb.execute("print s").contains("50"));
    QVERIFY(gdb.execute("print us").contains("50"));
    QVERIFY(gdb.execute("print i").contains("50"));
    QVERIFY(gdb.execute("print ui").contains("50"));
    QVERIFY(gdb.execute("print l").contains("50"));
    QVERIFY(gdb.execute("print ul").contains("50"));
    QVERIFY(gdb.execute("print i64").contains("50"));
    QVERIFY(gdb.execute("print ui64").contains("50"));
    QVERIFY(gdb.execute("print f").contains("50"));
    QVERIFY(gdb.execute("print d").contains("50"));
}

void QtPrintersTest::testQPersistentModelIndex()
{
    GdbProcess gdb("debuggee_qpersistentmodelindex");
    gdb.execute("break qpersistentmodelindex.cpp:16");
    gdb.execute("run");

    // QModelIndex's 4th data member had been `const QAbstractItemModel *m` before and
    // is `Qt::totally_ordered_wrapper<const QAbstractItemModel *> m` since Qt 6.8.
    // Hence the default GDB struct printer outputs "m = 0x..." given Qt version < 6.8 and
    // m = {
    //   ptr = 0x...
    // }
    // given Qt version >= 6.8. Look for " = 0x..." to pass with any supported Qt version.

    QByteArray out = gdb.execute("print i");
    QVERIFY(out.contains("r = -1"));
    QVERIFY(out.contains("c = -1"));
    QVERIFY(out.contains("i = 0"));
    QVERIFY(out.contains(" = 0x0"));

    auto modelAddress = gdb.execute("print /a &model");
    modelAddress.remove(0, modelAddress.indexOf('=') + 1);
    modelAddress = modelAddress.trimmed();
    QCOMPARE(modelAddress.left(2), "0x");
    modelAddress.prepend(" = ");

    gdb.execute("next");
    out = gdb.execute("print i");
    QVERIFY(out.contains("r = 1"));
    QVERIFY(out.contains("c = 0"));
    QVERIFY(out.contains("i = "));
    QVERIFY(out.contains(modelAddress));

    gdb.execute("next");
    out = gdb.execute("print i");
    QVERIFY(out.contains("r = 1"));
    QVERIFY(out.contains("c = 1"));
    QVERIFY(out.contains("i = "));
    QVERIFY(out.contains(modelAddress));

    gdb.execute("break qpersistentmodelindex.cpp:24");
    gdb.execute("cont");
    out = gdb.execute("print i");
    QVERIFY(out.contains("r = 0"));
    QVERIFY(out.contains("c = 0"));
    QVERIFY(out.contains("i = "));
    QVERIFY(out.contains(modelAddress));
}

void QtPrintersTest::testQUuid()
{
    GdbProcess gdb(QStringLiteral("debuggee_quuid"));
    gdb.execute("break quuid.cpp:4");
    gdb.execute("run");
    QByteArray data = gdb.execute("print id");
    QVERIFY(data.contains("{9ec3b70b-d105-42bf-b3b4-656e44d2e223}"));
}

void QtPrintersTest::testQVariant()
{
    GdbProcess gdb(QStringLiteral("debuggee_qvariant"));

    auto printNext = [&]() {
        gdb.execute("next");
        return gdb.execute("print v");
    };

    gdb.execute("break qvariant.cpp:13");
    gdb.execute("run");

    QVERIFY(printNext().contains("QVariant(NULL)"));
    QVERIFY(printNext().contains("QVariant(QString, \"KDevelop (QString)\")"));
    QVERIFY(printNext().contains("QVariant(QByteArray, \"KDevelop (QByteArray)\" = {"));
    QVERIFY(printNext().contains("QVariant(signed char, -8"));
    QVERIFY(printNext().contains("QVariant(uchar, 8"));
    QVERIFY(printNext().contains("QVariant(short, -16)"));
    QVERIFY(printNext().contains("QVariant(ushort, 16)"));
    QVERIFY(printNext().contains("QVariant(int, -32)"));
    QVERIFY(printNext().contains("QVariant(uint, 32)"));
    QVERIFY(printNext().contains("QVariant(qlonglong, -64)"));
    QVERIFY(printNext().contains("QVariant(qulonglong, 64)"));
    QVERIFY(printNext().contains("QVariant(bool, true)"));
    QVERIFY(printNext().contains("QVariant(float, 4.5)"));
    QVERIFY(printNext().contains("QVariant(double, 42.5)"));

    QVERIFY(printNext().contains("QVariant(QObject*, 0x"));
    // Now verify that the pretty printer retrieves correct object address from the QVariant.
    {
        auto objectAddress = gdb.execute("print /a &myObj");
        objectAddress.remove(0, objectAddress.indexOf('=') + 1);
        objectAddress = objectAddress.trimmed();
        QCOMPARE(objectAddress.left(2), "0x");
        objectAddress.insert(0, "QVariant(QObject*, ");
        QVERIFY(gdb.execute("print v").contains(objectAddress));
    }

    QVERIFY(printNext().contains("QVariant(SomeCustomType, {\n  foo = 42\n})"));
}

void QtPrintersTest::testKTextEditorTypes()
{
    GdbProcess gdb(QStringLiteral("debuggee_ktexteditortypes"));
    gdb.execute("break ktexteditortypes.cpp:9");
    gdb.execute("run");

    QByteArray data = gdb.execute("print cursor");
    QCOMPARE(data, QByteArray("$1 = [1, 1]"));
    data = gdb.execute("print range");
    QCOMPARE(data, QByteArray("$2 = [(1, 1) -> (2, 2)]"));
}

void QtPrintersTest::testKDevelopTypes()
{
    GdbProcess gdb(QStringLiteral("debuggee_kdeveloptypes"));
    gdb.execute("break kdeveloptypes.cpp:12");
    const auto runMessage = gdb.execute("run");
    if (runMessage.contains("ASan runtime does not come first"))
        QSKIP("cannot run this test in an ASan build configuration");

    QVERIFY(gdb.execute("print path1").contains("(\"tmp\", \"foo\")"));
    QVERIFY(gdb.execute("print path2").contains("(\"http://www.test.com\", \"tmp\", \"asdf.txt\")"));
}

QTEST_MAIN(QtPrintersTest)

#include "test_gdbprinters.moc"
#include "moc_test_gdbprinters.cpp"
