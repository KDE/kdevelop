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
        QProcess::start("gdb", (QStringList() << (BINARY_PATH + '/' + program)));
        waitForStarted();
        QByteArray prompt = waitForPrompt();
        QVERIFY(!prompt.contains("No such file or directory"));
        execute("set confirm off");
        execute("set print pretty on");
        QList<QByteArray> p;
        p << "python"
          << "import sys"
          << "sys.path.insert(0, '"+QDir(__FILE__).dirName().toAscii()+"')"
          << "from qt4 import register_qt4_printers"
          << "register_qt4_printers (None)"
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
        if (output.contains("Traceback")) {
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

void QtPrintersTest::testQListInt()
{
    GdbProcess gdb("qlistint");
    gdb.execute("break qlistint.cpp:6");
    gdb.execute("run");
    QByteArray out = gdb.execute("print l");
    QVERIFY(out.contains("QList<int>"));
    QVERIFY(out.contains("[0] = 10"));
    QVERIFY(out.contains("[1] = 20"));
    gdb.execute("next");
    QVERIFY(gdb.execute("print l").contains("[2] = 30"));
}

void QtPrintersTest::testQListString()
{
    GdbProcess gdb("qliststring");
    gdb.execute("break qliststring.cpp:6");
    gdb.execute("run");
    QByteArray out = gdb.execute("print l");
    QVERIFY(out.contains("QStringList"));
    QVERIFY(out.contains("[0] = \"a\""));
    QVERIFY(out.contains("[1] = \"b\""));
    gdb.execute("next");
    out = gdb.execute("print l");
    QVERIFY(out.contains("[2] = \"c\""));
}

void QtPrintersTest::testQListStruct()
{
    GdbProcess gdb("qliststruct");
    gdb.execute("break qliststruct.cpp:15");
    gdb.execute("run");
    QByteArray out = gdb.execute("print l");
    QVERIFY(out.contains("QList<A>"));
    QVERIFY(out.contains("[0] = {"));
    QVERIFY(out.contains("a = \"a\""));
    QVERIFY(out.contains("c = 100"));
    gdb.execute("next");
    out = gdb.execute("print l");
    QVERIFY(out.contains("[1] = {"));
}

void QtPrintersTest::testQListPointer()
{
    GdbProcess gdb("qlistpointer");
    gdb.execute("break qlistpointer.cpp:7");
    gdb.execute("run");
    QByteArray out = gdb.execute("print l");
    QVERIFY(out.contains("QList<int *>"));
    QVERIFY(out.contains("[0] = 0x"));
    QVERIFY(out.contains("[1] = 0x"));
    gdb.execute("next");
    out = gdb.execute("print l");
    QVERIFY(out.contains("[2] = 0x"));
}

void QtPrintersTest::testQVectorInt()
{
    GdbProcess gdb("qvectorint");
    gdb.execute("break qvectorint.cpp:6");
    gdb.execute("run");
    QByteArray out = gdb.execute("print l");
    QVERIFY(out.contains("QVector<int>"));
    QVERIFY(out.contains("[0] = 10"));
    QVERIFY(out.contains("[1] = 20"));
    gdb.execute("next");
    QVERIFY(gdb.execute("print l").contains("[2] = 30"));
}

void QtPrintersTest::testQVectorString()
{
    GdbProcess gdb("qvectorstring");
    gdb.execute("break qvectorstring.cpp:7");
    gdb.execute("run");
    QByteArray out = gdb.execute("print l");
    QVERIFY(out.contains("QVector<QString>"));
    QVERIFY(out.contains("[0] = \"a\""));
    QVERIFY(out.contains("[1] = \"b\""));
    gdb.execute("next");
    out = gdb.execute("print l");
    QVERIFY(out.contains("[2] = \"c\""));
}

void QtPrintersTest::testQVectorStruct()
{
    GdbProcess gdb("qvectorstruct");
    gdb.execute("break qvectorstruct.cpp:15");
    gdb.execute("run");
    QByteArray out = gdb.execute("print l");
    QVERIFY(out.contains("QVector<A>"));
    QVERIFY(out.contains("[0] = {"));
    QVERIFY(out.contains("a = \"a\""));
    QVERIFY(out.contains("c = 100"));
    gdb.execute("next");
    out = gdb.execute("print l");
    QVERIFY(out.contains("[1] = {"));
}

void QtPrintersTest::testQVectorPointer()
{
    GdbProcess gdb("qvectorpointer");
    gdb.execute("break qvectorpointer.cpp:7");
    gdb.execute("run");
    QByteArray out = gdb.execute("print l");
    QVERIFY(out.contains("QVector<int *>"));
    QVERIFY(out.contains("[0] = 0x"));
    QVERIFY(out.contains("[1] = 0x"));
    gdb.execute("next");
    out = gdb.execute("print l");
    QVERIFY(out.contains("[2] = 0x"));
}

void QtPrintersTest::testQQueue()
{
    GdbProcess gdb("qqueue");
    gdb.execute("break qqueue.cpp:12");
    gdb.execute("run");
    { // <int>
    gdb.execute("next");
    QByteArray out = gdb.execute("print intList");
    QVERIFY(out.contains("empty QQueue<int>"));
    gdb.execute("next");
    out = gdb.execute("print intList");
    QVERIFY(out.contains("QQueue<int>"));
    QVERIFY(out.contains("[0] = 10"));
    QVERIFY(out.contains("[1] = 20"));
    QVERIFY(!out.contains("[2] = 30"));
    gdb.execute("next");
    out = gdb.execute("print intList");
    QVERIFY(out.contains("QQueue<int>"));
    QVERIFY(out.contains("[0] = 10"));
    QVERIFY(out.contains("[1] = 20"));
    QVERIFY(out.contains("[2] = 30"));
    }
    { // <QString>
    gdb.execute("next");
    QByteArray out = gdb.execute("print stringList");
    QVERIFY(out.contains("empty QQueue<QString>"));
    gdb.execute("next");
    out = gdb.execute("print stringList");
    QVERIFY(out.contains("QQueue<QString>"));
    QVERIFY(out.contains("[0] = \"a\""));
    QVERIFY(out.contains("[1] = \"bc\""));
    QVERIFY(!out.contains("[2] = \"d\""));
    gdb.execute("next");
    out = gdb.execute("print stringList");
    QVERIFY(out.contains("QQueue<QString>"));
    QVERIFY(out.contains("[0] = \"a\""));
    QVERIFY(out.contains("[1] = \"bc\""));
    QVERIFY(out.contains("[2] = \"d\""));
    }
    { // <struct A>
    gdb.execute("next");
    QByteArray out = gdb.execute("print structList");
    QVERIFY(out.contains("empty QQueue<A>"));
    gdb.execute("next");
    gdb.execute("next");
    gdb.execute("next");
    out = gdb.execute("print structList");
    QVERIFY(out.contains("QQueue<A>"));
    QVERIFY(out.contains("[0] = {"));
    QVERIFY(out.contains("a = \"a\""));
    QVERIFY(out.contains("c = 100"));
    gdb.execute("next");
    out = gdb.execute("print structList");
    QVERIFY(out.contains("QQueue<A>"));
    QVERIFY(out.contains("[1] = {"));
    }
    { // <int*>
    gdb.execute("next");
    QByteArray out = gdb.execute("print pointerList");
    QVERIFY(out.contains("empty QQueue<int *>"));
    gdb.execute("next");
    out = gdb.execute("print pointerList");
    QVERIFY(out.contains("QQueue<int *>"));
    QVERIFY(out.contains("[0] = 0x"));
    QVERIFY(out.contains("[1] = 0x"));
    QVERIFY(!out.contains("[2] = 0x"));
    gdb.execute("next");
    out = gdb.execute("print pointerList");
    QVERIFY(out.contains("[0] = 0x"));
    QVERIFY(out.contains("[1] = 0x"));
    QVERIFY(out.contains("[2] = 0x"));
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


}
QTEST_MAIN(GDBDebugger::QtPrintersTest)

#include "qtprinters.moc"
