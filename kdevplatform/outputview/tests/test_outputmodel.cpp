/*
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2012 Morten Danielsen Volden <mvolden2@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_outputmodel.h"
#include "testlinebuilderfunctions.h"
#include "../outputmodel.h"

#include <QTest>
#include <QStandardPaths>
#include <QElapsedTimer>

QTEST_MAIN(KDevelop::TestOutputModel)

namespace KDevelop
{

TestOutputModel::TestOutputModel(QObject* parent): QObject(parent)
{
    QStandardPaths::setTestModeEnabled(true);
}

QStringList generateLines()
{
    const int numLines = 10000;
    QStringList outputlines;
    do {
        outputlines << buildCompilerActionLine();
        outputlines << buildCppCheckInformationLine();
        for (TestPathType pathType :
#ifdef Q_OS_WIN
            {WindowsFilePathNoSpaces, WindowsFilePathWithSpaces}
#else
            {UnixFilePathNoSpaces, UnixFilePathWithSpaces}
#endif
        ) {
            outputlines << buildCompilerErrorLine(pathType);
            outputlines << buildCompilerLine(pathType);
            outputlines << buildCppCheckErrorLine(pathType);
            outputlines << buildPythonErrorLine(pathType);
        }
    }
    while(outputlines.size() < numLines ); // gives us numLines (-ish)
    return outputlines;
}

QStringList generateLongLine()
{
    const int objects = 100; // *.o files
    const int libs = 20; // -l...
    const int libPaths = 20; // -L...
    QString line = QStringLiteral("g++ -m64 -Wl,-rpath,/home/gabo/md/qt/lib -o bin/flap_ui");
    for(int i = 0; i < objects; ++i) {
        line += QStringLiteral(" .obj/file%1.o").arg(i);
    }
    for(int i = 0; i < libPaths; ++i) {
        line += QStringLiteral(" -Lsome/path/to/lib%1").arg(i);
    }
    for(int i = 0; i < libs; ++i) {
        line += QStringLiteral(" -lsomelib%1").arg(i);
    }
    return QStringList() << line;
}

void TestOutputModel::bench()
{
    QFETCH(KDevelop::OutputModel::OutputFilterStrategy, strategy);
    QFETCH(QStringList, lines);

    OutputModel testee(QUrl::fromLocalFile(QStringLiteral("/tmp/build-foo")));
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

void TestOutputModel::bench_data()
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

#include "moc_test_outputmodel.cpp"
