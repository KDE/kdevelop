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

#include "qtprinters.h"

#include <QTest>
#include <QProcess>
#include <QDebug>
#include <QFileInfo>
#include <QDir>

const QString BINARY_PATH(PRINTER_BIN_DIR);

namespace GDBDebugger
{

class GdbProcess : private QProcess
{
public:
    GdbProcess(const QString &program) : QProcess()
    {
        setProcessChannelMode(MergedChannels);
        // don't attempt to load .gdbinit in home (may cause unexpected results)
        QProcess::start("gdb", (QStringList() << "-nh" << (BINARY_PATH + '/' + program)));
        waitForStarted();
        QByteArray prompt = waitForPrompt();
        QVERIFY(!prompt.contains("No such file or directory"));
        execute("set confirm off");
        execute("set print pretty on");
        QList<QByteArray> p;
        QDir printersDir = QFileInfo(__FILE__).dir();
        printersDir.cdUp(); // go up to get to the main printers directory
        p << "python"
          << "import sys"
          << "sys.path.insert(0, '"+printersDir.path().toAscii()+"')"
          << "from qt4 import register_qt4_printers"
          << "register_qt4_printers (None)"
          << "from kde4 import register_kde4_printers"
          << "register_kde4_printers (None)"
          << "end";
        foreach (const QByteArray &i, p) {
            write(i + "\n");
        }
        waitForPrompt();
    }
    ~GdbProcess()
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
            qFatal("Unexpected Python Exception");
        }
        return output;
    }

    QByteArray execute(const QByteArray &cmd)
    {
        write(cmd + "\n");
        return waitForPrompt();
    }
};

void QtPrintersTest::testQString()
{
    GdbProcess gdb("qstring");
    gdb.execute("break qstring.cpp:5");
    gdb.execute("run");
    QVERIFY(gdb.execute("print s").contains("\"test string\""));
    gdb.execute("next");
    QVERIFY(gdb.execute("print s").contains("\"test stringx\""));
}

void QtPrintersTest::testQByteArray()
{
    GdbProcess gdb("qbytearray");
    gdb.execute("break qbytearray.cpp:5");
    gdb.execute("run");
    QByteArray out = gdb.execute("print ba");
    QVERIFY(out.contains("\"test byte array\""));
    QVERIFY(out.contains("[0] = 116 't'"));
    QVERIFY(out.contains("[4] = 32 ' '"));
    gdb.execute("next");
    QVERIFY(gdb.execute("print ba").contains("\"test byte arrayx\""));
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
    GdbProcess gdb("qlistcontainer");
    gdb.execute("break main");
    gdb.execute("run");
    gdb.execute(QString("break 'doStuff<%1>()'").arg(container).toLocal8Bit());
    gdb.execute("cont");
    { // <int>
    gdb.execute("next");
    QByteArray out = gdb.execute("print intList");
    QVERIFY(out.contains(QString("empty %1<int>").arg(container).toLocal8Bit()));
    gdb.execute("next");
    out = gdb.execute("print intList");
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
    gdb.execute("next");
    out = gdb.execute("print intList");
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
    gdb.execute("next");
    QByteArray out = gdb.execute("print stringList");
    QVERIFY(out.contains(QString("empty %1<QString>").arg(container).toLocal8Bit()));
    gdb.execute("next");
    out = gdb.execute("print stringList");
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
    gdb.execute("next");
    out = gdb.execute("print stringList");
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
    QVERIFY(out.contains("[0] = {\n    first = 1, \n    second = 2\n  }"));
    QVERIFY(out.contains("[1] = {\n    first = 2, \n    second = 3\n  }"));
    QVERIFY(!out.contains("[2] = "));
    gdb.execute("next");
    out = gdb.execute("print pairList");
    QVERIFY(out.contains("[0] = {\n    first = 1, \n    second = 2\n  }"));
    QVERIFY(out.contains("[1] = {\n    first = 2, \n    second = 3\n  }"));
    QVERIFY(out.contains("[2] = {\n    first = 4, \n    second = 5\n  }"));
    }
}

void QtPrintersTest::testQMapInt()
{
    GdbProcess gdb("qmapint");
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
    GdbProcess gdb("qmapstring");
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
    GdbProcess gdb("qmapstringbool");
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
    GdbProcess gdb("qdate");
    gdb.execute("break qdate.cpp:5");
    gdb.execute("run");
    QByteArray out = gdb.execute("print d");
    QVERIFY(out.contains("2010-01-20"));
}

void QtPrintersTest::testQTime()
{
    GdbProcess gdb("qtime");
    gdb.execute("break qtime.cpp:5");
    gdb.execute("run");
    QByteArray out = gdb.execute("print t");
    QVERIFY(out.contains("15:30:10.123"));
}

void QtPrintersTest::testQDateTime()
{
    GdbProcess gdb("qdatetime");
    gdb.execute("break qdatetime.cpp:5");
    gdb.execute("run");
    QByteArray out = gdb.execute("print dt");
    QVERIFY(out.contains("2010-01-20 15:31:13"));
}

void QtPrintersTest::testQUrl()
{
    GdbProcess gdb("qurl");
    gdb.execute("break qurl.cpp:5");
    gdb.execute("run");
    QByteArray out = gdb.execute("print u");
    QVERIFY(out.contains("http://www.kdevelop.org/foo"));
}

void QtPrintersTest::testQHashInt()
{
    GdbProcess gdb("qhashint");
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
    GdbProcess gdb("qhashstring");
    gdb.execute("break qhashstring.cpp:7");
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
    GdbProcess gdb("qsetint");
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
    GdbProcess gdb("qsetstring");
    gdb.execute("break qsetstring.cpp:7");
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
    GdbProcess gdb("qchar");
    gdb.execute("break qchar.cpp:5");
    gdb.execute("run");
    QVERIFY(gdb.execute("print c").contains("\"k\""));
}

void QtPrintersTest::testQUuid()
{
    GdbProcess gdb("quuid");
    gdb.execute("break quuid.cpp:4");
    gdb.execute("run");
    QByteArray data = gdb.execute("print id");
    QVERIFY(data.contains("{9ec3b70b-d105-42bf-b3b4-656e44d2e223}"));
}

void QtPrintersTest::testKTextEditorTypes()
{
    GdbProcess gdb("ktexteditortypes");
    gdb.execute("break ktexteditortypes.cpp:9");
    gdb.execute("run");

    QByteArray data = gdb.execute("print cursor");
    QCOMPARE(data, QByteArray("$1 = [1, 1]"));
    data = gdb.execute("print range");
    QCOMPARE(data, QByteArray("$2 = [(1, 1) -> (2, 2)]"));
}

}
QTEST_MAIN(GDBDebugger::QtPrintersTest)

#include "qtprinters.moc"
