/*
 * This file is part of KDevelop
 * Copyright 2012 Miha Čančula <miha@noughmad.eu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "test_templateclassgenerator.h"
#include "codegen_tests_config.h"

#include "language/codegen/templateclassgenerator.h"
#include "language/codegen/documentchangeset.h"
#include "language/codegen/sourcefiletemplate.h"
#include "language/codegen/templaterenderer.h"
#include "language/codegen/templatesmodel.h"

#include "tests/autotestshell.h"
#include "tests/testcore.h"

#define CHECK_TEMPLATE_VARIABLE(name, type, val)            \
QVERIFY(variables.contains(#name));                         \
QCOMPARE(variables.value(#name).value<type>(), val)

#define COMPARE_FILES(one, two)                             \
QCOMPARE(QString(one.readAll()), QString(two.readAll()))    \

using namespace KDevelop;

void TestTemplateClassGenerator::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);

    // Use a temporary directory for the template work
    tempDir.setAutoRemove(true);
    baseUrl = QUrl::fromLocalFile(tempDir.path() + '/');

    // Needed for extracting description out of template archives
    TemplatesModel model("kdevcodegentest");
    model.refresh();

    description.members << VariableDescription("QString", "name")
                        << VariableDescription("int", "number")
                        << VariableDescription("SomeCustomType", "data");

    FunctionDescription function;
    function.name = "doSomething";
    function.isVirtual = true;
    function.arguments << VariableDescription("double", "howMuch")
                       << VariableDescription("bool", "doSomethingElse");

    VariableDescriptionList args;
    VariableDescriptionList returnArgs;
    returnArgs << VariableDescription("int", "someOtherNumber");
    FunctionDescription otherFunction("getSomeOtherNumber", args, returnArgs);
    description.methods << function << otherFunction;
}

void TestTemplateClassGenerator::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestTemplateClassGenerator::fileLabelsCpp()
{
    TemplateClassGenerator* generator = loadTemplate("test_cpp");
    QHash<QString,QString> labels = generator->fileLabels();
    QCOMPARE(labels.size(), 2);

    /*
     * File labels can be translated, so we don't check their equality here.
     * But they have to be present and non-empty
     */
    QVERIFY(labels.contains("Header"));
    QVERIFY(!labels["Header"].isEmpty());
    QVERIFY(labels.contains("Implementation"));
    QVERIFY(!labels["Implementation"].isEmpty());
}

void TestTemplateClassGenerator::fileLabelsYaml()
{
    TemplateClassGenerator* generator = loadTemplate("test_yaml");
    QHash<QString,QString> labels = generator->fileLabels();
    QCOMPARE(labels.size(), 1);

    QVERIFY(labels.contains("Description"));
    QVERIFY(!labels["Description"].isEmpty());
}

void TestTemplateClassGenerator::defaultFileUrlsCpp()
{
    TemplateClassGenerator* generator = loadTemplate("test_cpp");
    QHash<QString,QUrl> files = generator->fileUrls();
    QCOMPARE(files.size(), 2);

    QVERIFY(files.contains("Header"));
    QCOMPARE(files["Header"], baseUrl.resolved(QUrl("ClassName.h")));

    QVERIFY(files.contains("Implementation"));
    QCOMPARE(files["Implementation"], baseUrl.resolved(QUrl("ClassName.cpp")));
}

void TestTemplateClassGenerator::defaultFileUrlsYaml()
{
    TemplateClassGenerator* generator = loadTemplate("test_yaml");
    QHash<QString,QUrl> files = generator->fileUrls();
    QCOMPARE(files.size(), 1);

    QVERIFY(files.contains("Description"));
    QCOMPARE(files["Description"], baseUrl.resolved(QUrl("ClassName.yaml")));
}

void TestTemplateClassGenerator::customOptions()
{
    TemplateClassGenerator* generator = loadTemplate("test_yaml");
    QCOMPARE(generator->sourceFileTemplate().hasCustomOptions(), false);
}

void TestTemplateClassGenerator::templateVariablesCpp()
{
    TemplateClassGenerator* generator = loadTemplate("test_cpp");
    setLowercaseFileNames(generator);

    QVariantHash variables = generator->renderer()->variables();
    CHECK_TEMPLATE_VARIABLE(name, QString, QStringLiteral("ClassName"));

    CHECK_TEMPLATE_VARIABLE(output_file_header, QString, QStringLiteral("classname.h"));
    CHECK_TEMPLATE_VARIABLE(output_file_header_absolute, QString, baseUrl.resolved(QUrl("classname.h")).toLocalFile());
}

void TestTemplateClassGenerator::templateVariablesYaml()
{
    TemplateClassGenerator* generator = loadTemplate("test_yaml");
    setLowercaseFileNames(generator);

    QVariantHash variables = generator->renderer()->variables();
    CHECK_TEMPLATE_VARIABLE(name, QString, QStringLiteral("ClassName"));

    CHECK_TEMPLATE_VARIABLE(output_file_description, QString, QStringLiteral("classname.yaml"));
    CHECK_TEMPLATE_VARIABLE(output_file_description_absolute, QString, baseUrl.resolved(QUrl("classname.yaml")).toLocalFile());
}

void TestTemplateClassGenerator::codeDescription()
{
    TemplateClassGenerator* generator = loadTemplate("test_yaml");

    QVariantHash variables = generator->renderer()->variables();

    qDebug() << variables;

    QVERIFY(variables.contains("base_classes"));
    QVariantList inheritance = variables["base_classes"].toList();
    QCOMPARE(inheritance.size(), 1);
    QCOMPARE(inheritance.first().value<InheritanceDescription>().baseType, QStringLiteral("QObject"));
    QCOMPARE(inheritance.first().value<InheritanceDescription>().inheritanceMode, QStringLiteral("public"));

    QVERIFY(variables.contains("members"));
    QVariantList members = variables["members"].toList();
    QCOMPARE(members.size(), 3);
    QCOMPARE(members.first().value<VariableDescription>().type, QStringLiteral("QString"));
    QCOMPARE(members.first().value<VariableDescription>().name, QStringLiteral("name"));

    QVERIFY(variables.contains("functions"));
    QVariantList methods = variables["functions"].toList();
    QCOMPARE(methods.size(), 2);
    QCOMPARE(methods.first().value<FunctionDescription>().name, QStringLiteral("doSomething"));
    QCOMPARE(methods.first().value<FunctionDescription>().arguments.size(), 2);
    QCOMPARE(methods.first().value<FunctionDescription>().returnArguments.size(), 0);
    QCOMPARE(methods.last().value<FunctionDescription>().name, QStringLiteral("getSomeOtherNumber"));
    QCOMPARE(methods.last().value<FunctionDescription>().arguments.size(), 0);
    QCOMPARE(methods.last().value<FunctionDescription>().returnArguments.size(), 1);
}

void TestTemplateClassGenerator::generate()
{
    TemplateClassGenerator* generator = loadTemplate("test_cpp");

    DocumentChangeSet changes = generator->generate();
    DocumentChangeSet::ChangeResult result = changes.applyAllChanges();
    QVERIFY(result.m_success);

    QDir dir(baseUrl.toLocalFile());
    QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    QCOMPARE(entries.size(), 2);
}

void TestTemplateClassGenerator::cppOutput()
{
    TemplateClassGenerator* generator = loadTemplate("test_cpp");
    setLowercaseFileNames(generator);
    DocumentChangeSet changes = generator->generate();
    changes.setFormatPolicy(DocumentChangeSet::NoAutoFormat);
    changes.applyAllChanges();

    QFile header(baseUrl.resolved(QUrl("classname.h")).toLocalFile());
    QVERIFY(header.open(QIODevice::ReadOnly));

    QFile testHeader(CODEGEN_TESTS_EXPECTED_DIR "/classname.h");
    testHeader.open(QIODevice::ReadOnly);
    COMPARE_FILES(header, testHeader);

    QFile implementation(baseUrl.resolved(QUrl("classname.cpp")).toLocalFile());
    QVERIFY(implementation.open(QIODevice::ReadOnly));

    QFile testImplementation(CODEGEN_TESTS_EXPECTED_DIR "/classname.cpp");
    testImplementation.open(QIODevice::ReadOnly);
    COMPARE_FILES(implementation, testImplementation);
}

void TestTemplateClassGenerator::yamlOutput()
{
    TemplateClassGenerator* generator = loadTemplate("test_yaml");
    setLowercaseFileNames(generator);
    generator->generate().applyAllChanges();

    QFile yaml(baseUrl.resolved(QUrl("classname.yaml")).toLocalFile());
    QVERIFY(yaml.open(QIODevice::ReadOnly));

    QFile testYaml(CODEGEN_TESTS_EXPECTED_DIR "/classname.yaml");
    testYaml.open(QIODevice::ReadOnly);
    COMPARE_FILES(yaml, testYaml);
}

TemplateClassGenerator* TestTemplateClassGenerator::loadTemplate (const QString& name)
{
    QDir dir(baseUrl.toLocalFile());
    foreach (const QString& fileName, dir.entryList(QDir::Files | QDir::NoDotAndDotDot))
    {
        dir.remove(fileName);
    }

    TemplateClassGenerator* generator = new TemplateClassGenerator(baseUrl);

    QString tplDescription = QString(CODEGEN_DATA_DIR) + "/kdevcodegentest/templates/" + name + "/" + name + ".desktop";
    Q_ASSERT(!tplDescription.isEmpty());
    SourceFileTemplate tpl;
    tpl.addAdditionalSearchLocation(QString(CODEGEN_TESTS_DATA_DIR) + "/kdevcodegentest/templates/");
    tpl.setTemplateDescription(tplDescription, "kdevcodegentest");
    Q_ASSERT(tpl.isValid());
    generator->setTemplateDescription(tpl);
    generator->setDescription(description);
    generator->setIdentifier("ClassName");
    generator->addBaseClass("public QObject");
    generator->setLicense("This is just a test.\nYou may do with it as you please.");
    return generator;
}

void TestTemplateClassGenerator::setLowercaseFileNames(TemplateClassGenerator* generator)
{
    QHash<QString, QUrl> urls = generator->fileUrls();
    QHash<QString, QUrl>::const_iterator it = urls.constBegin();
    for (; it != urls.constEnd(); ++it)
    {
        QString fileName = it.value().fileName().toLower();
        QUrl base = it.value().resolved(QUrl(QStringLiteral("./%1").arg(fileName)));
        generator->setFileUrl(it.key(), base);
    }
}


QTEST_GUILESS_MAIN(TestTemplateClassGenerator)
