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

#include <QTest>
#include <QProcess>
#include <QDebug>
#include <QFileInfo>
#include <QDir>

#include "lldbformatterconfig.h"

const QString BINARY_PATH(PRINTER_BIN_DIR);

class LldbProcess : private QProcess
{
public:
    LldbProcess(const QString &program) : QProcess()
    {
        setProcessChannelMode(MergedChannels);
        // don't attempt to load .lldbinit in home (may cause unexpected results)
        QProcess::start("lldb", (QStringList() << "--no-lldbinit" << (BINARY_PATH + '/' + program)));
        const bool started = waitForStarted();
        if (!started) {
            qDebug() << "Failed to start 'lldb' executable:" << errorString();
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
    ~LldbProcess() override
    {
        write("q\n");
        waitForFinished();
    }
    QByteArray waitForPrompt()
    {
        QByteArray output;
        while (!output.endsWith("(lldb) ")) {
            Q_ASSERT(state() == QProcess::Running);
            waitForReadyRead();
            QByteArray l = readAll();
            //qDebug() << l;
            output.append(l);
        }
        output.chop(7); //remove (lldb) prompt
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

void LldbFormattersTest::testQString()
{
    LldbProcess lldb("qstring");
    lldb.execute("break qstring.cpp:5");
    lldb.execute("run");
    QVERIFY(lldb.execute("print s").contains("\"test string\""));
    lldb.execute("next");
    QVERIFY(lldb.execute("print s").contains("\"test stringx\""));
}

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

QTEST_MAIN(LldbFormattersTest)
