/*
    SPDX-FileCopyrightText: 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_replacementparser.h"

// SUT
#include "parsers/replacementparser.h"
// KDevPlatform
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <tests/testhelpermacros.h>
// Qt
#include <QDir>
#include <QFile>
#include <QTemporaryFile>
#include <QTest>

namespace {
ReplacementsParserTester::TestFilePaths prepareTestFiles(const char* relativeSourceFilePath,
                                                         QObject* temporaryFileParent)
{
    QFile sourceYamlFile(QFINDTESTDATA(QString::fromUtf8(relativeSourceFilePath) + ".yaml"));
    QVERIFY_RETURN(sourceYamlFile.open(QIODevice::ReadOnly), {});
    auto fileContents = sourceYamlFile.readAll();
    sourceYamlFile.close();

    auto* const yamlFile =
        new QTemporaryFile(QDir::tempPath() + "/kdevclangtidy_test_XXXXXX.yaml", temporaryFileParent);
    QVERIFY_RETURN(yamlFile->open(), {});

    ReplacementsParserTester::TestFilePaths filePaths{yamlFile->fileName(), QFINDTESTDATA(relativeSourceFilePath)};

    // Make the file path in the yaml file match the source file path in order to prevent
    // ReplacementParser::nextNode() from detecting file path mismatch and early-returning.
    // The file path check in ReplacementParser::nextNode() is the only reason for this helper function's existence.
    fileContents.replace(relativeSourceFilePath, filePaths.source.toUtf8());

    QCOMPARE_RETURN(yamlFile->write(fileContents), fileContents.size(), {});
    QVERIFY_RETURN(yamlFile->flush(), {});
    yamlFile->close();

    return filePaths;
}
} // namespace

using namespace KDevelop;
using namespace ClangTidy;

void ReplacementsParserTester::initTestCase()
{
    // Called before the first testfunction is executed
    AutoTestShell::init({ "kdevclangtidy" });
    TestCore::initialize(Core::NoUi);

    m_pluginFilePaths = prepareTestFiles("data/plugin.cpp", this);
    RETURN_IF_TEST_FAILED();
    m_nihonPluginFilePaths = prepareTestFiles("data/nihon_plugin.cpp", this);
    RETURN_IF_TEST_FAILED();
}

void ReplacementsParserTester::cleanupTestCase()
{
    // Called after the last testfunction was executed
    TestCore::shutdown();
}

void ReplacementsParserTester::doTest()
{
    ReplacementParser parser(m_pluginFilePaths.yaml, m_pluginFilePaths.source);
    parser.parse();
    auto v = parser.allReplacements();

    QVERIFY(!v.isEmpty());
    QCOMPARE(v.length(), static_cast<int>(parser.count()));

    QCOMPARE(v[0].range.document.str(), m_pluginFilePaths.source);
    QCOMPARE(v[0].offset, size_t(6263));
    QCOMPARE(v[0].length, size_t(1));
    QCOMPARE(v[0].replacementText, QString());
    QCOMPARE(v[0].range.start().line() + 1, 155); // as would appear in editor.
    QCOMPARE(v[0].range.start().column() + 1, 9); // as would appear in editor.

    QCOMPARE(v[1].range.document.str(), m_pluginFilePaths.source);
    QCOMPARE(v[1].offset, size_t(6267));
    QCOMPARE(v[1].length, size_t(0));
    QCOMPARE(v[1].replacementText, QString(" == nullptr"));
    QCOMPARE(v[1].range.start().line() + 1, 155);
    QCOMPARE(v[1].range.start().column() + 1, 13);

    QCOMPARE(v[2].range.document.str(), m_pluginFilePaths.source);
    QCOMPARE(v[2].offset, size_t(6561));
    QCOMPARE(v[2].length, size_t(1));
    QCOMPARE(v[2].replacementText, QString());
    QCOMPARE(v[2].range.start().line() + 1, 162);
    QCOMPARE(v[2].range.start().column() + 1, 9);

    QCOMPARE(v[3].range.document.str(), m_pluginFilePaths.source);
    QCOMPARE(v[3].offset, size_t(6569));
    QCOMPARE(v[3].length, size_t(0));
    QCOMPARE(v[3].replacementText, QStringLiteral(" == nullptr"));
    QCOMPARE(v[3].range.start().line() + 1, 162);
    QCOMPARE(v[3].range.start().column() + 1, 17);

    QCOMPARE(v[4].range.document.str(), m_pluginFilePaths.source);
    QCOMPARE(v[4].offset, size_t(7233));
    QCOMPARE(v[4].length, size_t(69));
    QCOMPARE(v[4].replacementText, QStringLiteral("// TODO(cnihelton): auto detect clang-tidy executable"
                                                  " instead of hard-coding it."));
    QCOMPARE(v[4].range.start().line() + 1, 181);
    QCOMPARE(v[4].range.start().column() + 1, 5);

    QCOMPARE(v[5].range.document.str(), m_pluginFilePaths.source);
    QCOMPARE(v[5].offset, size_t(8800));
    QCOMPARE(v[5].length, size_t(4));
    QCOMPARE(v[5].replacementText, QStringLiteral("auto "));
    QCOMPARE(v[5].range.start().line() + 1, 210);
    QCOMPARE(v[5].range.start().column() + 1, 9);

    QCOMPARE(v[6].range.document.str(), m_pluginFilePaths.source);
    QCOMPARE(v[6].offset, size_t(8945));
    QCOMPARE(v[6].length, size_t(4));
    QCOMPARE(v[6].replacementText, QStringLiteral("auto "));
    QCOMPARE(v[6].range.start().line() + 1, 214);
    QCOMPARE(v[6].range.start().column() + 1, 5);

    QCOMPARE(v[7].range.document.str(), m_pluginFilePaths.source);
    QCOMPARE(v[7].offset, size_t(9406));
    QCOMPARE(v[7].length, size_t(1));
    QCOMPARE(v[7].replacementText, QString());
    QCOMPARE(v[7].range.start().line() + 1, 238);
    QCOMPARE(v[7].range.start().column() + 1, 9);

    // Invalid stuff.
    // Modified the plugin.cpp.yaml file to try to cause and exception during parsing.
    // Added three lines from other file, which goes beyond the end of the plugin.cpp file in the last offsets.
    // The parser didn't throw exception event though it might have generated empty ranges for the last replacements.
    // And also invalidates the generated replacement.
    QVERIFY(!v[15].range.isValid());
    QCOMPARE(v[15].range.document.str(), QStringLiteral(""));
    QCOMPARE(v[15].offset, size_t(0));
    QCOMPARE(v[15].length, size_t(0));
    QCOMPARE(v[15].replacementText, QString());
    QCOMPARE(v[15].range.start().line() + 1, 0);
    QCOMPARE(v[15].range.start().column() + 1, 0);

    // testing multibyte chars in source code.
    ReplacementParser nihonParser(m_nihonPluginFilePaths.yaml, m_nihonPluginFilePaths.source);
    nihonParser.parse();
    auto nv = nihonParser.allReplacements();
    QVERIFY(!nv.isEmpty());
    QCOMPARE(nv.length(), static_cast<int>(nihonParser.count()));

    QCOMPARE(nv[0].range.document.str(), m_nihonPluginFilePaths.source);
    QCOMPARE(nv[0].offset, size_t(10165));
    QCOMPARE(nv[0].length, size_t(1));
    QCOMPARE(nv[0].replacementText, QString());
    QCOMPARE(nv[0].range.start().line() + 1, 288);
    QCOMPARE(nv[0].range.start().column() + 1, 9);

    QCOMPARE(nv[1].range.document.str(), m_nihonPluginFilePaths.source);
    QCOMPARE(nv[1].offset, size_t(10169));
    QCOMPARE(nv[1].length, size_t(0));
    QCOMPARE(nv[1].replacementText, QStringLiteral(" == nullptr"));
    QCOMPARE(nv[1].range.start().line() + 1, 288);
    QCOMPARE(nv[1].range.start().column() + 1, 13);

    QCOMPARE(nv[2].range.document.str(), m_nihonPluginFilePaths.source);
    QCOMPARE(nv[2].offset, size_t(10463));
    QCOMPARE(nv[2].length, size_t(1));
    QCOMPARE(nv[2].replacementText, QString());
    QCOMPARE(nv[2].range.start().line() + 1, 295);
    QCOMPARE(nv[2].range.start().column() + 1, 9);

    QCOMPARE(nv[3].range.document.str(), m_nihonPluginFilePaths.source);
    QCOMPARE(nv[3].offset, size_t(10471));
    QCOMPARE(nv[3].length, size_t(0));
    QCOMPARE(nv[3].replacementText, QStringLiteral(" == nullptr"));
    QCOMPARE(nv[3].range.start().line() + 1, 295);
    QCOMPARE(nv[3].range.start().column() + 1, 17);

    QCOMPARE(nv[4].range.document.str(), m_nihonPluginFilePaths.source);
    QCOMPARE(nv[4].offset, size_t(11135));
    QCOMPARE(nv[4].length, size_t(69));
    QCOMPARE(nv[4].replacementText, QStringLiteral("// TODO(cnihelton): auto detect clang-tidy executable "
                                                   "instead of hard-coding it."));
    QCOMPARE(nv[4].range.start().line() + 1, 314);
    QCOMPARE(nv[4].range.start().column() + 1, 5);
}

QTEST_GUILESS_MAIN(ReplacementsParserTester);

#include "moc_test_replacementparser.cpp"
