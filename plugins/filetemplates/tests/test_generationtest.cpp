/*
    This file is part of KDevelop

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "test_generationtest.h"
#include "tests_config.h"

#include <tests/testcore.h>
#include <tests/autotestshell.h>

#include <language/codegen/templatesmodel.h>
#include <language/codegen/sourcefiletemplate.h>
#include <language/codegen/documentchangeset.h>
#include <language/codegen/templaterenderer.h>

#include <util/path.h>

#include <QStandardPaths>

using namespace KDevelop;


#define COMPARE_FILES(name)                                                 \
do {                                                                        \
QFile actualFile(Path(Path(baseUrl), QStringLiteral(name)).toLocalFile());                        \
QVERIFY(actualFile.open(QIODevice::ReadOnly));                              \
QFile expectedFile(QStringLiteral(TESTS_EXPECTED_DIR "/" name));                  \
QVERIFY(expectedFile.open(QIODevice::ReadOnly));                            \
QCOMPARE(actualFile.size(), expectedFile.size());                           \
QCOMPARE(QString(actualFile.readAll()), QString(expectedFile.readAll()));   \
} while(0)

void TestGenerationTest::initTestCase()
{
    QByteArray xdgData = qgetenv("XDG_DATA_DIRS");
    xdgData.prepend(TESTS_DATA_DIR ":");
    bool addedDir = qputenv("XDG_DATA_DIRS", xdgData);
    QVERIFY(addedDir);

    // avoid translated desktop entries, tests use untranslated strings
    QLocale::setDefault(QLocale::c());
    AutoTestShell::init();
    TestCore::initialize (Core::NoUi);

    TemplatesModel model(QStringLiteral("testgenerationtest"));
    model.refresh();

    renderer = new TemplateRenderer;
    renderer->setEmptyLinesPolicy(TemplateRenderer::TrimEmptyLines);
    renderer->addVariable(QStringLiteral("name"), "TestName");
    renderer->addVariable(QStringLiteral("license"), "Test license header\nIn two lines");

    QStringList testCases;
    testCases << QStringLiteral("firstTestCase");
    testCases << QStringLiteral("secondTestCase");
    testCases << QStringLiteral("thirdTestCase");
    renderer->addVariable(QStringLiteral("testCases"), testCases);
}

void TestGenerationTest::cleanupTestCase()
{
    delete renderer;
    TestCore::shutdown();
}

void TestGenerationTest::init()
{
    dir.reset(new QTemporaryDir);
    baseUrl = QUrl::fromLocalFile(dir->path());
}

void TestGenerationTest::yamlTemplate()
{
    QString description = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("testgenerationtest/template_descriptions/test_yaml2.desktop"));
    QVERIFY(!description.isEmpty());
    SourceFileTemplate file;
    file.addAdditionalSearchLocation(QStringLiteral(TESTS_DATA_DIR "/testgenerationtest/templates"));
    file.setTemplateDescription(description);

    QCOMPARE(file.name(), QStringLiteral("Testing YAML Template"));
    QVERIFY(file.isValid());

    DocumentChangeSet changes = renderer->renderFileTemplate(file, baseUrl, urls(file));
    changes.applyAllChanges();

    COMPARE_FILES("testname.yaml");
}

void TestGenerationTest::cppTemplate()
{
    QString description = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("testgenerationtest/template_descriptions/test_qtestlib.desktop"));
    QVERIFY(!description.isEmpty());
    SourceFileTemplate file;
    file.addAdditionalSearchLocation(QStringLiteral(TESTS_DATA_DIR "/testgenerationtest/templates"));
    file.setTemplateDescription(description);

    QCOMPARE(file.name(), QStringLiteral("Testing C++ Template"));
    QVERIFY(file.isValid());

    DocumentChangeSet changes = renderer->renderFileTemplate(file, baseUrl, urls(file));
    changes.applyAllChanges();

    COMPARE_FILES("testname.h");
    COMPARE_FILES("testname.cpp");
}

QHash< QString, QUrl > TestGenerationTest::urls (const SourceFileTemplate& file)
{
    QHash<QString, QUrl> ret;
    const auto outputFiles = file.outputFiles();
    for (const SourceFileTemplate::OutputFile& output : outputFiles) {
        QUrl url = Path(Path(baseUrl), renderer->render(output.outputName).toLower()).toUrl();
        ret.insert(output.identifier, url);
    }
    return ret;
}


QTEST_GUILESS_MAIN(TestGenerationTest)

#include "moc_test_generationtest.cpp"
