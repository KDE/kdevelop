/*
 * This file is part of KDevelop
 *
 * Copyright 2012 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "test_makeoutputmodel.h"

#include <outputview/outputmodel.h>
//#include "../makeoutputmodel.h"
//#include "../outputfilters.h"

#include <QStringList>
#include <QTest>
#include <QSignalSpy>
#include <QDebug>

QTEST_MAIN(TestMakeOutputModel)

TestMakeOutputModel::TestMakeOutputModel(QObject* parent): QObject(parent)
{

}

void TestMakeOutputModel::benchAddLines()
{
    // see also: https://bugs.kde.org/show_bug.cgi?id=295361
    KDevelop::OutputModel model(KUrl("/tmp/build-foo"));
    QStringList lines;
    const int numLines = 1000;
    const int charsPerLine = 100;
    for(int i = 0; i < numLines; ++i) {
        lines << (QString::number(i) + QString().fill('a', charsPerLine));
    }

    qRegisterMetaType<QModelIndex>("QModelIndex");
    QSignalSpy spy(&model, SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)));

    QElapsedTimer totalTime;
    totalTime.start();

    QBENCHMARK {
        model.appendLines(lines);
        while(model.rowCount() != lines.count()) {
            QCoreApplication::instance()->processEvents();
        }
    }

    QVERIFY(model.rowCount() == lines.count());
    const qint64 elapsed = totalTime.elapsed();

    qDebug() << "ms elapsed to add lines: " << elapsed;
    qDebug() << "total number of added lines: " << lines.count();
    const double avgUiLockup = double(elapsed) / spy.count();
    qDebug() << "average UI lockup in ms: " << avgUiLockup;
    QVERIFY(avgUiLockup < 200);
}

void TestMakeOutputModel::benchAddLongLine()
{
    // see also: https://bugs.kde.org/show_bug.cgi?id=295361
    const int objects = 100; // *.o files
    const int libs = 20; // -l...
    const int libPaths = 20; // -L...
    QString line = "g++ -m64 -Wl,-rpath,/home/gabo/md/qt/lib -o bin/flap_ui";
    for(int i = 0; i < objects; ++i) {
        line += QString(" .obj/file%1.o").arg(i);
    }
    for(int i = 0; i < libPaths; ++i) {
        line += QString(" -Lsome/path/to/lib%1").arg(i);
    }
    for(int i = 0; i < libs; ++i) {
        line += QString(" -lsomelib%1").arg(i);
    }

    KDevelop::OutputModel model(KUrl("/tmp/build-foo"));

    qRegisterMetaType<QModelIndex>("QModelIndex");
    QSignalSpy spy(&model, SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)));

    QElapsedTimer totalTime;
    totalTime.start();

    QStringList lines;
    lines << line;
    QBENCHMARK {
        model.appendLines(lines);
        while(model.rowCount() != lines.count()) {
            QCoreApplication::instance()->processEvents();
        }
    }

    QVERIFY(model.rowCount() == lines.count());
    const qint64 elapsed = totalTime.elapsed();

    qDebug() << "ms elapsed to add lines: " << elapsed;
    qDebug() << "total number of added lines: " << lines.count();
    const double avgUiLockup = double(elapsed) / spy.count();
    qDebug() << "average UI lockup in ms: " << avgUiLockup;
    QVERIFY(avgUiLockup < 200);
}

void TestMakeOutputModel::testErrors_data()
{
    QTest::addColumn<QString>("line");
    QTest::addColumn<QString>("file");
    QTest::addColumn<int>("lineNr");
    QTest::addColumn<int>("column");

    QTest::newRow("gcc-with-col")
        << "/path/to/file.cpp:123:45: fatal error: ..."
        << "/path/to/file.cpp" << 123 << 45;
    QTest::newRow("gcc-no-col")
        << "/path/to/file.cpp:123: error ..."
        << "/path/to/file.cpp" << 123 << -1;
    QTest::newRow("fortcom")
        << "fortcom: Error: Ogive8.f90, line 123: ..."
        << "Ogive8.f90" << 123 << -1;
    QTest::newRow("libtool")
        << "libtool: link: warning: ..."
        << "" << -1 << -1;
    QTest::newRow("gfortran")
        << "/path/to/file.f90:123.456:"
        << "/path/to/file.f90" << 123 << 456;
}

void TestMakeOutputModel::testErrors()
{
    QFETCH(QString, line);
    QFETCH(QString, file);
    QFETCH(int, lineNr);
    QFETCH(int, column);
    bool matched = false;
//     foreach(const ErrorFormat& format, ErrorFormat::errorFormats) {
//         if (format.expression.indexIn(line) != -1) {
//             matched = true;
//             if (format.fileGroup > 0) {
//                 QCOMPARE(format.expression.cap( format.fileGroup ), file);
//             } else {
//                 QVERIFY(file.isEmpty());
//             }
//             if (format.lineGroup > 0) {
//                 QCOMPARE(format.expression.cap( format.lineGroup ).toInt(), lineNr);
//             } else {
//                 QCOMPARE(lineNr, -1);
//             }
//             if(format.columnGroup > 0) {
//                 QCOMPARE(format.expression.cap( format.columnGroup ).toInt(), column);
//             } else {
//                 QCOMPARE(column, -1);
//             }
//             break;
//         }
//     }
//     QVERIFY(matched);
}

#include "test_makeoutputmodel.moc"
