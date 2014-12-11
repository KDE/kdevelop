/*
 * This file is part of KDevelop
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

#include "test_generationtest.h"
#include "tests_config.h"

#include <tests/testcore.h>
#include <tests/autotestshell.h>

#include <language/codegen/templatesmodel.h>
#include <language/codegen/sourcefiletemplate.h>
#include <language/codegen/documentchangeset.h>
#include <language/codegen/templaterenderer.h>

#include <util/path.h>

using namespace KDevelop;


#define COMPARE_FILES(name)                                                 \
do {                                                                        \
QFile actualFile(Path(Path(baseUrl), name).toLocalFile());                        \
QVERIFY(actualFile.open(QIODevice::ReadOnly));                              \
QFile expectedFile(TESTS_EXPECTED_DIR "/" name);                  \
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

    AutoTestShell::init();
    TestCore::initialize (Core::NoUi);

    TemplatesModel model("testgenerationtest");
    model.refresh();

    renderer = new TemplateRenderer;
    renderer->setEmptyLinesPolicy(TemplateRenderer::TrimEmptyLines);
    renderer->addVariable("name", "TestName");
    renderer->addVariable("license", "Test license header\nIn two lines");

    QStringList testCases;
    testCases << "firstTestCase";
    testCases << "secondTestCase";
    testCases << "thirdTestCase";
    renderer->addVariable("testCases", testCases);
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
    QString description = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "testgenerationtest/template_descriptions/test_yaml2.desktop");
    QVERIFY(!description.isEmpty());
    SourceFileTemplate file;
    file.addAdditionalSearchLocation(TESTS_DATA_DIR "/testgenerationtest/templates");
    file.setTemplateDescription(description, "testgenerationtest");
    QCOMPARE(file.name(), QString("Testing YAML Template"));

    DocumentChangeSet changes = renderer->renderFileTemplate(file, baseUrl, urls(file));
    changes.applyAllChanges();

    COMPARE_FILES("testname.yaml");
}

void TestGenerationTest::cppTemplate()
{
    QString description = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "testgenerationtest/template_descriptions/test_qtestlib.desktop");
    QVERIFY(!description.isEmpty());
    SourceFileTemplate file;
    file.addAdditionalSearchLocation(TESTS_DATA_DIR "/testgenerationtest/templates");
    file.setTemplateDescription(description, "testgenerationtest");

    QCOMPARE(file.name(), QString("Testing C++ Template"));

    DocumentChangeSet changes = renderer->renderFileTemplate(file, baseUrl, urls(file));
    changes.applyAllChanges();

    COMPARE_FILES("testname.h");
    COMPARE_FILES("testname.cpp");
}

QHash< QString, QUrl > TestGenerationTest::urls (const SourceFileTemplate& file)
{
    QHash<QString, QUrl> ret;
    foreach (const SourceFileTemplate::OutputFile& output, file.outputFiles()) {
        QUrl url = Path(Path(baseUrl), renderer->render(output.outputName).toLower()).toUrl();
        ret.insert(output.identifier, url);
    }
    return ret;
}


QTEST_GUILESS_MAIN(TestGenerationTest);
