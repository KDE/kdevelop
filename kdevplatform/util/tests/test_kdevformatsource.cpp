/*
    Copyright 2016 Anton Anikin <anton.anikin@htower.ru>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "test_kdevformatsource.h"
#include "../kdevformatfile.h"

#include "qtcompat_p.h"

#include <QTest>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QTemporaryDir>

QTEST_MAIN(KDevelop::TestKdevFormatSource)

using namespace KDevelop;

TestKdevFormatSource::TestKdevFormatSource()
{
}

TestKdevFormatSource::~TestKdevFormatSource()
{
}

void TestKdevFormatSource::testNotFound_data()
{
    static const QStringList formatFileData = {};

    QCOMPARE(initTest(formatFileData), true);

    for (const Source& source : qAsConst(m_sources)) {
        QTest::newRow(source.path.toUtf8()) << source.path << false << false << false << source.lines;
    }
}

void TestKdevFormatSource::testNotFound()
{
    runTest();
}

void TestKdevFormatSource::testNoCommands_data()
{
    static const QStringList formatFileData = {QStringLiteral("# some comment")};

    QCOMPARE(initTest(formatFileData), true);

    for (const Source& source : qAsConst(m_sources)) {
        QTest::newRow(source.path.toUtf8()) << source.path << true << false << false << source.lines;
    }
}

void TestKdevFormatSource::testNoCommands()
{
    runTest();
}

void TestKdevFormatSource::testNotMatch_data()
{
    static const QStringList formatFileData = {QStringLiteral("notmatched.cpp : unused_command")};

    QCOMPARE(initTest(formatFileData), true);

    for (const Source& source : qAsConst(m_sources)) {
        QTest::newRow(source.path.toUtf8()) << source.path << true << true << false << source.lines;
    }
}

void TestKdevFormatSource::testNotMatch()
{
    runTest();
}

void TestKdevFormatSource::testMatch1_data()
{
    static const QStringList formatFileData({
        QStringLiteral("src1/source_1.cpp : cat $ORIGFILE | sed 's/foo/FOO/' > tmp && mv tmp $ORIGFILE"),
        QStringLiteral("src2/source_2.cpp : cat $ORIGFILE | sed 's/sqrt/std::sqrt/' > tmp && mv tmp $ORIGFILE"),
        QStringLiteral("*.cpp : cat $ORIGFILE | sed 's/z/Z/' > tmp && mv tmp $ORIGFILE"),
        QStringLiteral("notmatched.cpp : unused_command"),
    });

    QCOMPARE(initTest(formatFileData), true);

    m_sources[0].lines.replaceInStrings(QStringLiteral("foo"), QStringLiteral("FOO"));
    m_sources[1].lines.replaceInStrings(QStringLiteral("sqrt"), QStringLiteral("std::sqrt"));
    m_sources[2].lines.replaceInStrings(QStringLiteral("z"), QStringLiteral("Z"));

    for (const Source& source : qAsConst(m_sources)) {
        QTest::newRow(source.path.toUtf8()) << source.path << true << true << true << source.lines;
    }
}

void TestKdevFormatSource::testMatch1()
{
    runTest();
}

void TestKdevFormatSource::testMatch2_data()
{
    static const QStringList formatFileData({QStringLiteral("cat $ORIGFILE | sed 's/;/;;/' > tmp && mv tmp $ORIGFILE")});

    QCOMPARE(initTest(formatFileData), true);

    for (Source& source : m_sources) {
        source.lines.replaceInStrings(QStringLiteral(";"), QStringLiteral(";;"));
        QTest::newRow(source.path.toUtf8()) << source.path << true << true << true << source.lines;
    }
}

void TestKdevFormatSource::testMatch2()
{
    runTest();
}

bool TestKdevFormatSource::initTest(const QStringList& formatFileData)
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("isFound");
    QTest::addColumn<bool>("isRead");
    QTest::addColumn<bool>("isApplied");
    QTest::addColumn<QStringList>("lines");

    m_temporaryDir.reset(new QTemporaryDir);
    const QString workPath = m_temporaryDir->path();
    qDebug() << "Using temporary dir:" << workPath;

    if (!mkPath(workPath + "/src1"))
        return false;

    if (!mkPath(workPath + "/src2"))
        return false;

    if (!QDir::setCurrent(workPath)) {
        qDebug() << "unable to set current directory to" << workPath;
        return false;
    }

    m_sources.resize(3);

    m_sources[0].path = workPath + "/src1/source_1.cpp";
    m_sources[0].lines = QStringList({
        QStringLiteral("void foo(int x) {"),
        QStringLiteral("  printf(\"squared x = %d\\n\", x * x);"),
        QStringLiteral("}")
    });

    m_sources[1].path = workPath + "/src2/source_2.cpp";
    m_sources[1].lines = QStringList({
        QStringLiteral("void bar(double x) {"),
        QStringLiteral("  x = sqrt(x);"),
        QStringLiteral("  printf(\"sqrt(x) = %e\\n\", x);"),
        QStringLiteral("}")
    });

    m_sources[2].path = workPath + "/source_3.cpp";
    m_sources[2].lines = QStringList({
        QStringLiteral("void baz(double x, double y) {"),
        QStringLiteral("  double z = pow(x, y);"),
        QStringLiteral("  printf(\"x^y = %e\\n\", z);"),
        QStringLiteral("}")
    });

    for (const Source& source : qAsConst(m_sources)) {
        if (!writeLines(source.path, source.lines))
            return false;
    }

    if (!formatFileData.isEmpty() && !writeLines(QStringLiteral("format_sources"), formatFileData))
        return false;

    return true;
}

void TestKdevFormatSource::runTest() const
{
    QFETCH(QString, path);
    QFETCH(bool, isFound);
    QFETCH(bool, isRead);
    QFETCH(bool, isApplied);
    QFETCH(QStringList, lines);

    KDevFormatFile formatFile(path, path);

    QCOMPARE(formatFile.find(), isFound);

    if (isFound)
        QCOMPARE(formatFile.read(), isRead);

    if (isRead)
        QCOMPARE(formatFile.apply(), isApplied);

    QStringList processedLines;
    QCOMPARE(readLines(path, processedLines), true);

    QCOMPARE(processedLines, lines);
}

bool TestKdevFormatSource::mkPath(const QString& path) const
{
    if (!QDir().exists(path) && !QDir().mkpath(path)) {
        qDebug() << "unable to create directory" << path;
        return false;
    }

    return true;
}

bool TestKdevFormatSource::writeLines(const QString& path, const QStringList& lines) const
{
    QFile outFile(path);
    if (!outFile.open(QIODevice::WriteOnly)) {
        qDebug() << "unable to open file" << path << "for writing";
        return false;
    }

    QTextStream outStream(&outFile);
    for (const QString& line : lines) {
        outStream << line << "\n";
    }
    outStream.flush();
    outFile.close();

    return true;
}

bool TestKdevFormatSource::readLines(const QString& path, QStringList& lines) const
{
    QFile inFile(path);
    if (!inFile.open(QIODevice::ReadOnly)) {
        qDebug() << "unable to open file" << path << "for reading";
        return false;
    }

    lines.clear();

    QTextStream inStream(&inFile);
    while (!inStream.atEnd()) {
        lines += inStream.readLine();
    }
    inFile.close();

    return true;
}
