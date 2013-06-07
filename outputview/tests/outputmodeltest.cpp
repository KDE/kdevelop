/*
    This file is part of KDevelop
    Copyright 2012 Milian Wolff <mail@milianw.de>
    Copyright (C) 2012  Morten Danielsen Volden mvolden2@gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "outputmodeltest.h"
#include "testlinebuilderfunctions.h"
#include "../outputmodel.h"

#include <QTest>

#include <qtest_kde.h>


QTEST_MAIN(KDevelop::OutputModelTest)

namespace KDevelop
{

OutputModelTest::OutputModelTest(QObject* parent): QObject(parent)
{
    // fix error:
    // KGlobal::locale() must be called from the main thread before using i18n()
    // in threads. KApplication takes care of this. If not using KApplication,
    // call KGlobal::locale() during initialization.
    KGlobal::locale();
}

QStringList generateLines()
{
    const int numLines = 10000;
    QStringList outputlines;
    do {
        outputlines << buildCompilerErrorLine();
        outputlines << buildCompilerLine();
        outputlines << buildCompilerActionLine();
        outputlines << buildCppCheckErrorLine();
        outputlines << buildCppCheckInformationLine();
        outputlines << buildPythonErrorLine();
    }
    while(outputlines.size() < numLines ); // gives us numLines (-ish)
    return outputlines;
}

QStringList generateLongLine()
{
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
    return QStringList() << line;
}

void OutputModelTest::bench()
{
    QFETCH(KDevelop::OutputModel::OutputFilterStrategy, strategy);
    QFETCH(QStringList, lines);

    OutputModel testee(KUrl("/tmp/build-foo"));
    testee.setFilteringStrategy(strategy);

    quint64 processEventsCounter = 1;
    QElapsedTimer totalTime;
    totalTime.start();

    testee.appendLines(lines);
    while(testee.rowCount() != lines.count()) {
        QCoreApplication::instance()->processEvents();
        processEventsCounter++;
    }

    QVERIFY(testee.rowCount() == lines.count());
    const qint64 elapsed = totalTime.elapsed();

    qDebug() << "ms elapsed to add lines: " << elapsed;
    qDebug() << "total number of added lines: " << lines.count();
    const double avgUiLockup = double(elapsed) / processEventsCounter;
    qDebug() << "average UI lockup in ms: " << avgUiLockup;
    QVERIFY(avgUiLockup < 200);
}

void OutputModelTest::bench_data()
{
    QTest::addColumn<KDevelop::OutputModel::OutputFilterStrategy>("strategy");
    QTest::addColumn<QStringList>("lines");

    const QStringList lines = generateLines();
    const QStringList longLine = generateLongLine();

    QTest::newRow("no-filter") << OutputModel::NoFilter << lines;
    QTest::newRow("no-filter-longline") << OutputModel::NoFilter << longLine;

    QTest::newRow("compiler-filter") << OutputModel::CompilerFilter << lines;
    QTest::newRow("compiler-filter-longline") << OutputModel::CompilerFilter << longLine;

    QTest::newRow("script-error-filter") << OutputModel::ScriptErrorFilter << lines;
    QTest::newRow("script-error-filter-longline") << OutputModel::ScriptErrorFilter << longLine;

    QTest::newRow("static-analysis-filter") << OutputModel::StaticAnalysisFilter << lines;
    QTest::newRow("static-analysis-filter-longline") << OutputModel::StaticAnalysisFilter << longLine;
}

}
#include "outputmodeltest.moc"
