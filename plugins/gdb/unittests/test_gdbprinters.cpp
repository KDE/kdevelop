/*
   Copyright 2010 Niko Sams <niko.sams@gmail.com>

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

#include "test_gdbprinters.h"

#include "tests/debuggers-tests-config.h"

#include <QTest>
#include <QProcess>
#include <QDebug>
#include <QFileInfo>
#include <QDir>

const QString BINARY_PATH(DEBUGGEE_BIN_DIR);

class GdbProcess : private QProcess
{
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
        foreach (const QByteArray &i, p) {
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

void QtPrintersTest::testQString()
{
    GdbProcess gdb(QStringLiteral("debuggee_qstring"));
    gdb.execute("break qstring.cpp:5");
    gdb.execute("run");
    QVERIFY(gdb.execute("print s").contains("\"test最后一个不是特殊字符'\\\"\\\\u6211\""));
    gdb.execute("next");
    QVERIFY(gdb.execute("print s").contains("\"test最后一个不是特殊字符'\\\"\\\\u6211x\""));
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
}

void QtPrintersTest::testQListContainer_data()
{
    QTest::addColumn<QString>("container");

    QTest::newRow("QList") << "QList";
    QTest::newRow("QQueue") << "QQueue";
    QTest::newRow("QVector") << "QVector";
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
    { // <int>
    gdb.execute("break qlistcontainer.cpp:34");
    gdb.execute("cont");
    QByteArray out = gdb.execute("print intList");
    qWarning() << "FOO" << out;
    QVERIFY(out.contains(QString("empty %1<int>").arg(container).toLocal8Bit()));
    gdb.execute("next");
    out = gdb.execute("print intList");
    QVERIFY(out.contains(QString("%1<int>").arg(container).toLocal8Bit()));
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
    QVERIFY(out.contains(QString("%1<int>").arg(container).toLocal8Bit()));
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
    gdb.execute("next");
    QByteArray out = gdb.execute("print stringList");
    QVERIFY(out.contains(QString("empty %1<QString>").arg(container).toLocal8Bit()));
    gdb.execute("next");
    out = gdb.execute("print stringList");
    QVERIFY(out.contains(QString("%1<QString>").arg(container).toLocal8Bit()));
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
    QVERIFY(out.contains(QString("%1<QString>").arg(container).toLocal8Bit()));
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
    gdb.execute("next");
    QByteArray out = gdb.execute("print structList");
    QVERIFY(out.contains(QString("empty %1<A>").arg(container).toLocal8Bit()));
    gdb.execute("next");
    out = gdb.execute("print structList");
    QVERIFY(out.contains(QString("%1<A>").arg(container).toLocal8Bit()));
    QVERIFY(out.contains("[0] = {"));
    QVERIFY(out.contains("a = \"a\""));
    QVERIFY(out.contains("b = \"b\""));
    QVERIFY(out.contains("c = 100"));
    QVERIFY(out.contains("d = -200"));
    gdb.execute("next");
    out = gdb.execute("print structList");
    QVERIFY(out.contains(QString("%1<A>").arg(container).toLocal8Bit()));
    QVERIFY(out.contains("[1] = {"));
    }
    { // <int*>
    gdb.execute("next");
    QByteArray out = gdb.execute("print pointerList");
    QVERIFY(out.contains(QString("empty %1<int *>").arg(container).toLocal8Bit()));
    gdb.execute("next");
    out = gdb.execute("print pointerList");
    QVERIFY(out.contains(QString("%1<int *>").arg(container).toLocal8Bit()));
    QVERIFY(out.contains("[0] = 0x"));
    QVERIFY(out.contains("[1] = 0x"));
    QVERIFY(!out.contains("[2] = 0x"));
    gdb.execute("next");
    out = gdb.execute("print pointerList");
    QVERIFY(out.contains("[0] = 0x"));
    QVERIFY(out.contains("[1] = 0x"));
    QVERIFY(out.contains("[2] = 0x"));
	gdb.execute("next");
    }
    { // <QPair<int, int> >
    gdb.execute("next");
    QByteArray out = gdb.execute("print pairList");
    QVERIFY(out.contains(QString("empty %1<QPair<int, int>>").arg(container).toLocal8Bit()));
    gdb.execute("next");
    out = gdb.execute("print pairList");
    QVERIFY(out.contains(QString("%1<QPair<int, int>>").arg(container).toLocal8Bit()));
    if (container != QLatin1String("QSet")) {
        QVERIFY(out.contains("[0] = {\n    first = 1, \n    second = 2\n  }"));
        QVERIFY(out.contains("[1] = {\n    first = 2, \n    second = 3\n  }"));
    } else { // order is undefined in QSet
        QVERIFY(out.contains("] = {\n    first = 1, \n    second = 2\n  }"));
        QVERIFY(out.contains("] = {\n    first = 2, \n    second = 3\n  }"));
    }
    QVERIFY(!out.contains("[2] = "));
    gdb.execute("next");
    out = gdb.execute("print pairList");
    if (container != QLatin1String("QSet")) {
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

void QtPrintersTest::testQMapInt()
{
    GdbProcess gdb(QStringLiteral("debuggee_qmapint"));
    gdb.execute("break qmapint.cpp:7");
    gdb.execute("run");
    QByteArray out = gdb.execute("print m");
    QVERIFY(out.contains("QMap<int, int>"));
    QVERIFY(out.contains("[10] = 100"));
    QVERIFY(out.contains("[20] = 200"));
    gdb.execute("next");
    out = gdb.execute("print m");
    QVERIFY(out.contains("[30] = 300"));
}

void QtPrintersTest::testQMapString()
{
    GdbProcess gdb(QStringLiteral("debuggee_qmapstring"));
    gdb.execute("break qmapstring.cpp:8");
    gdb.execute("run");
    QByteArray out = gdb.execute("print m");
    QVERIFY(out.contains("QMap<QString, QString>"));
    QVERIFY(out.contains("[\"10\"] = \"100\""));
    QVERIFY(out.contains("[\"20\"] = \"200\""));
    gdb.execute("next");
    out = gdb.execute("print m");
    QVERIFY(out.contains("[\"30\"] = \"300\""));
}

void QtPrintersTest::testQMapStringBool()
{
    GdbProcess gdb(QStringLiteral("debuggee_qmapstringbool"));
    gdb.execute("break qmapstringbool.cpp:8");
    gdb.execute("run");
    QByteArray out = gdb.execute("print m");
    QVERIFY(out.contains("QMap<QString, bool>"));
    QVERIFY(out.contains("[\"10\"] = true"));
    QVERIFY(out.contains("[\"20\"] = false"));
    gdb.execute("next");
    out = gdb.execute("print m");
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
    gdb.execute("break qurl.cpp:5");
    gdb.execute("run");
    QByteArray out = gdb.execute("print u");
    QVERIFY(out.contains("http://user@www.kdevelop.org:12345/foo?xyz=bar#asdf"));
}

void QtPrintersTest::testQHashInt()
{
    GdbProcess gdb(QStringLiteral("debuggee_qhashint"));
    gdb.execute("break qhashint.cpp:7");
    gdb.execute("run");
    QByteArray out = gdb.execute("print h");
    QVERIFY(out.contains("[10] = 100"));
    QVERIFY(out.contains("[20] = 200"));
    gdb.execute("next");
    out = gdb.execute("print h");
    QVERIFY(out.contains("[30] = 300"));
}

void QtPrintersTest::testQHashString()
{
    GdbProcess gdb(QStringLiteral("debuggee_qhashstring"));
    gdb.execute("break qhashstring.cpp:8");
    gdb.execute("run");
    QByteArray out = gdb.execute("print h");
    QVERIFY(out.contains("[\"10\"] = \"100\""));
    QVERIFY(out.contains("[\"20\"] = \"200\""));
    gdb.execute("next");
    out = gdb.execute("print h");
    QVERIFY(out.contains("[\"30\"] = \"300\""));
}

void QtPrintersTest::testQSetInt()
{
    GdbProcess gdb(QStringLiteral("debuggee_qsetint"));
    gdb.execute("break qsetint.cpp:7");
    gdb.execute("run");
    QByteArray out = gdb.execute("print s");
    QVERIFY(out.contains("] = 10"));
    QVERIFY(out.contains("] = 20"));
    gdb.execute("next");
    out = gdb.execute("print s");
    QVERIFY(out.contains("] = 30"));
}

void QtPrintersTest::testQSetString()
{
    GdbProcess gdb(QStringLiteral("debuggee_qsetstring"));
    gdb.execute("break qsetstring.cpp:8");
    gdb.execute("run");
    QByteArray out = gdb.execute("print s");
    QVERIFY(out.contains("] = \"10\""));
    QVERIFY(out.contains("] = \"20\""));
    gdb.execute("next");
    out = gdb.execute("print s");
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

void QtPrintersTest::testQUuid()
{
    GdbProcess gdb(QStringLiteral("debuggee_quuid"));
    gdb.execute("break quuid.cpp:4");
    gdb.execute("run");
    QByteArray data = gdb.execute("print id");
    QVERIFY(data.contains("{9ec3b70b-d105-42bf-b3b4-656e44d2e223}"));
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
    gdb.execute("run");

    QVERIFY(gdb.execute("print path1").contains("(\"tmp\", \"foo\")"));
    QVERIFY(gdb.execute("print path2").contains("(\"http://www.test.com\", \"tmp\", \"asdf.txt\")"));
}

QTEST_MAIN(QtPrintersTest)
