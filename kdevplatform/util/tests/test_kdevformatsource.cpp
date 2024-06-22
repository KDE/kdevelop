/*
    SPDX-FileCopyrightText: 2016 Anton Anikin <anton.anikin@htower.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_kdevformatsource.h"
#include "../kdevformatfile.h"
#include "../filesystemhelpers.h"

#include <QTest>
#include <QByteArray>
#include <QByteArrayList>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QTextStream>
#include <QStandardPaths>

#include <vector>

QTEST_MAIN(KDevelop::TestKdevFormatSource)

using namespace KDevelop;

namespace {
QString applyFormatting(const QString& path, bool expectedFormattingResult)
{
    KDevFormatFile formatFile(path, path);
    if (!formatFile.find()) {
        return "found no format_sources file for " + path;
    }
    if (!formatFile.read()) {
        return "reading format_sources file failed for " + path;
    }
    if (formatFile.apply() != expectedFormattingResult) {
        if (expectedFormattingResult) {
            return "formatting was expected to succeed but actually failed for " + path;
        } else {
            return "formatting was expected to fail but actually succeeded for " + path;
        }
    }
    return QString{};
}
}

TestKdevFormatSource::TestKdevFormatSource()
{
}

TestKdevFormatSource::~TestKdevFormatSource()
{
}

void TestKdevFormatSource::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void TestKdevFormatSource::testNotFound_data()
{
    static const QStringList formatFileData = {};

    QCOMPARE(initTest(formatFileData), true);

    for (const Source& source : std::as_const(m_sources)) {
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

    for (const Source& source : std::as_const(m_sources)) {
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

    for (const Source& source : std::as_const(m_sources)) {
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

    for (const Source& source : std::as_const(m_sources)) {
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

void TestKdevFormatSource::testWildcardPathMatching_data()
{
    struct FormatInfo{ const char* dir; const char* contents; };
    struct Row{
        const char* dataTag;
        std::vector<FormatInfo> formatInfos;
        std::vector<const char*> unmatchedPaths;
        std::vector<const char*> matchedPaths;
    };

    const std::vector<Row> dataRows{
        Row{"format_sources without wildcards (simple syntax)",
        {FormatInfo{"", "true"}},
        {},
        {"x", "a/b", "exclude", "x.c", "p q\tr", "v/l/p/a/t/h.x"}
    }, Row{"Single root format_sources with a single command",
        {FormatInfo{"", "rd/* *include* *.h : true"}},
        {"x", "r", "r.d", "includ", "includh", "rdh", "rd.h/x", "a/b.hh", "rc/x.h/y"},
        {"x.h", "rd/x", "rd/x.h", "aincludeb", "include", "include.h", "rd/a/b/c", "a/b/c.h", "a/include"}
    }, Row{"Single root format_sources with different commands",
        {FormatInfo{"", "*inc/*:\n q/* *x?z:true \n dd/*: \n *.c:false \n *ab*:true"}},
        {"q", "a.b", "xz", "xyzc", "c", "ac", "inc", "inc-/x", "ayz", "xy", "add/s", "incc", "a./c", "x/yz", "a-b", "minc"},
        {"xyz", "x.c", "incxyz", "ainc/b.c", "a/b/.c", "a/.c", "x/z", "a/x-z", "p/x.z", "asinc/v", "a/b/cab/d/e", "dd/d", "dd/.c"}
    }, Row{"Multiple format_sources files",
        {FormatInfo{"a/b/", "q/* *x?z : false"}, FormatInfo{"", "*.c *cab* : true"}},
        {"a/q", "a/xyz", "q/x", "xz", "a/b/qu", "a/bu/xyz", "ab/q/x", "a/b/qt/x", "a/bxyz", "a/x/z", "a/b/xz", "a/b/.c", "a/b/x-z.c"},
        {"a/b/xyz", "x.c", "a/b/cdxyz", "a/b/cd/xyz", "a/b/q/x", "a/.c", "a/b/x/z", "exclude.c", "a/bcab/d/e"}
    }, Row{"Case sensitivity",
        {FormatInfo{"", "pQ* *RS* : true"}},
        {"a/b/CDE", "cdpq", "a/b/.e", "a/b/cDe", "prcpQ.Eqs"},
        {"a/b/pQrs", "a/b/c/d/pq/rs", "a/b/RSPQ", "pq", "uvrs", "PQa/b"}
    }};

    QTest::addColumn<QStringList>("formatDirs");
    QTest::addColumn<QByteArrayList>("formatContents");
    QTest::addColumn<QStringList>("unmatchedPaths");
    QTest::addColumn<QStringList>("matchedPaths");

    for (const Row& row : dataRows) {
        QStringList formatDirs;
        QByteArrayList formatContents;
        for (const FormatInfo& info : row.formatInfos) {
            formatDirs.push_back(info.dir);
            formatContents.push_back(info.contents);
        }
        const QStringList unmatchedPaths(row.unmatchedPaths.cbegin(), row.unmatchedPaths.cend());
        const QStringList matchedPaths(row.matchedPaths.cbegin(), row.matchedPaths.cend());
        QTest::newRow(row.dataTag) << formatDirs << formatContents << unmatchedPaths << matchedPaths;
    }
}

void TestKdevFormatSource::testWildcardPathMatching()
{
    QFETCH(QStringList, formatDirs);
    QFETCH(QByteArrayList, formatContents);
    QFETCH(QStringList, unmatchedPaths);
    QFETCH(QStringList, matchedPaths);

    QTemporaryDir tmpDir;
    QVERIFY2(tmpDir.isValid(), qPrintable("couldn't create temporary directory: " + tmpDir.errorString()));

    using FilesystemHelpers::makeAbsoluteCreateAndWrite;

    for (auto& dir : formatDirs) {
        dir = QFileInfo{QDir{dir}, "format_sources"}.filePath();
    }
    QString errorPath = makeAbsoluteCreateAndWrite(tmpDir.path(), formatDirs, formatContents);
    QVERIFY2(errorPath.isEmpty(), qPrintable("couldn't create or write to temporary file or directory " + errorPath));

    errorPath = makeAbsoluteCreateAndWrite(tmpDir.path(), unmatchedPaths);
    if (errorPath.isEmpty()) {
        errorPath = makeAbsoluteCreateAndWrite(tmpDir.path(), matchedPaths);
    }
    QVERIFY2(errorPath.isEmpty(), qPrintable("couldn't create temporary file or directory " + errorPath));

    bool expectedFormattingResult = false; // for unmatchedPaths
    for (const auto& paths : { unmatchedPaths, matchedPaths }) {
        for (const auto& path : paths) {
            QVERIFY2(QFileInfo{path}.isFile(), qPrintable(path + ": file was not created or was deleted"));
            const QString error = applyFormatting(path, expectedFormattingResult);
            QVERIFY2(error.isEmpty(), qPrintable(error));
        }
        expectedFormattingResult = true; // for matchedPaths
    }
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

    for (const Source& source : std::as_const(m_sources)) {
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

#include "moc_test_kdevformatsource.cpp"
