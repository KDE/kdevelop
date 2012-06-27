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
#include "language/codegen/templatesmodel.h"
#include "language/codegen/documentchangeset.h"

#include "tests/autotestshell.h"
#include "tests/testcore.h"

#include <KComponentData>
#include <KStandardDirs>
#include <KUrl>

#include <qtest_kde.h>

#define CHECK_TEMPLATE_VARIABLE(name, type, val)          \
QVERIFY(variables.contains(#name));                        \
QCOMPARE(variables.value(#name).value<type>(), val)

using namespace KDevelop;

void TestTemplateClassGenerator::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);

    KComponentData data = ICore::self()->componentData();
    baseUrl.setDirectory(KStandardDirs::locateLocal("tmp", "test_templateclassgenerator/", data));

    KStandardDirs *dirs = data.dirs();
    bool addedDir = dirs->addResourceDir("filetemplates", CODEGEN_TESTS_TEMPLATES_DIR, true);
    QVERIFY(addedDir);

    // Needed for extracting description out of template archives
    TemplatesModel model(data, this);
    model.setDescriptionResourceType("filetemplate_descriptions");
    model.setTemplateResourceType("filetemplates");
    model.refresh();

    description.name = "ClassName";

    InheritanceDescription inheritance;
    inheritance.baseType = "QObject";
    inheritance.inheritanceMode = "public";
    description.baseClasses << inheritance;

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
    QHash<QString,KUrl> files = generator->fileUrlsFromBase(baseUrl, false);
    QCOMPARE(files.size(), 2);

    QVERIFY(files.contains("Header"));
    KUrl expectedHeaderUrl(baseUrl);
    expectedHeaderUrl.addPath("ClassName.h");
    QCOMPARE(files["Header"], expectedHeaderUrl);

    QVERIFY(files.contains("Implementation"));
    KUrl expectedImplementationUrl(baseUrl);
    expectedImplementationUrl.addPath("ClassName.cpp");
    QCOMPARE(files["Implementation"], expectedImplementationUrl);
}

void TestTemplateClassGenerator::defaultFileUrlsYaml()
{
    TemplateClassGenerator* generator = loadTemplate("test_yaml");
    QHash<QString,KUrl> files = generator->fileUrlsFromBase(baseUrl, false);
    QCOMPARE(files.size(), 1);

    QVERIFY(files.contains("Description"));
    KUrl expectedHeaderUrl(baseUrl);
    expectedHeaderUrl.addPath("ClassName.yaml");
    QCOMPARE(files["Description"], expectedHeaderUrl);
}

void TestTemplateClassGenerator::lowercaseFileUrls()
{
    TemplateClassGenerator* generator = loadTemplate("test_cpp");
    QHash<QString,KUrl> files = generator->fileUrlsFromBase(baseUrl, true);
    QCOMPARE(files.size(), 2);

    QVERIFY(files.contains("Header"));
    KUrl expectedHeaderUrl(baseUrl);
    expectedHeaderUrl.addPath("classname.h");
    QCOMPARE(files["Header"], expectedHeaderUrl);

    QVERIFY(files.contains("Implementation"));
    KUrl expectedImplementationUrl(baseUrl);
    expectedImplementationUrl.addPath("classname.cpp");
    QCOMPARE(files["Implementation"], expectedImplementationUrl);
}

void TestTemplateClassGenerator::customOptions()
{
    TemplateClassGenerator* generator = loadTemplate("test_yaml");
    QCOMPARE(generator->hasCustomOptions(), false);
}

void TestTemplateClassGenerator::templateVariablesCpp()
{
    TemplateClassGenerator* generator = loadTemplate("test_cpp");
    QHash<QString, KUrl> urls = generator->fileUrlsFromBase(baseUrl, true);
    foreach (const QString& file, QStringList() << "Header" << "Implementation")
    {
        generator->setFileUrl(file, urls[file]);
    }

    QVariantHash variables = generator->templateVariables();
    CHECK_TEMPLATE_VARIABLE(name, QString, QString("ClassName"));

    KUrl headerUrl(baseUrl);
    headerUrl.addPath("classname.h");
    CHECK_TEMPLATE_VARIABLE(output_file_header, QString, QString("classname.h"));
    CHECK_TEMPLATE_VARIABLE(output_file_header_absolute, QString, headerUrl.toLocalFile());
}

void TestTemplateClassGenerator::templateVariablesYaml()
{
    TemplateClassGenerator* generator = loadTemplate("test_yaml");
    QHash<QString, KUrl> urls = generator->fileUrlsFromBase(baseUrl, true);
    foreach (const QString& file, QStringList() << "Description")
    {
        generator->setFileUrl(file, urls[file]);
    }

    QVariantHash variables = generator->templateVariables();
    CHECK_TEMPLATE_VARIABLE(name, QString, QString("ClassName"));

    KUrl fileUrl(baseUrl);
    fileUrl.addPath("classname.yaml");
    CHECK_TEMPLATE_VARIABLE(output_file_description, QString, QString("classname.yaml"));
    CHECK_TEMPLATE_VARIABLE(output_file_description_absolute, QString, fileUrl.toLocalFile());
}

void TestTemplateClassGenerator::codeDescription()
{
    TemplateClassGenerator* generator = loadTemplate("test_yaml");
    QVariantHash variables = generator->templateVariables();

    QVERIFY(variables.contains("base_classes"));
    InheritanceDescriptionList inheritance = variables["base_classes"].value<InheritanceDescriptionList>();
    QCOMPARE(inheritance.size(), 1);
    QCOMPARE(inheritance.first().baseType, QString("QObject"));
    QCOMPARE(inheritance.first().inheritanceMode, QString("public"));

    QVERIFY(variables.contains("members"));
    VariableDescriptionList members = variables["members"].value<VariableDescriptionList>();
    QCOMPARE(members.size(), 3);
    QCOMPARE(members.first().type, QString("QString"));
    QCOMPARE(members.first().name, QString("name"));

    QVERIFY(variables.contains("functions"));
    FunctionDescriptionList methods = variables["functions"].value<FunctionDescriptionList>();
    QCOMPARE(methods.size(), 2);
    QCOMPARE(methods.first().name, QString("doSomething"));
    QCOMPARE(methods.first().arguments.size(), 2);
    QCOMPARE(methods.first().returnArguments.size(), 0);
    QCOMPARE(methods.last().name, QString("getSomeOtherNumber"));
    QCOMPARE(methods.last().arguments.size(), 0);
    QCOMPARE(methods.last().returnArguments.size(), 1);
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

TemplateClassGenerator* TestTemplateClassGenerator::loadTemplate (const QString& name)
{
    TemplateClassGenerator* generator = new TemplateClassGenerator(baseUrl);

    bool found = false;
    foreach (const QString& description, ICore::self()->componentData().dirs()->findAllResources("filetemplate_descriptions"))
    {
        kDebug() << "Found template description" << description;
        if (QFileInfo(description).baseName() == name)
        {
            generator->setTemplateDescription(description);
            found = true;
            break;
        }
    }

    generator->setDescription(description);
    generator->setIdentifier("ClassName");
    return generator;
}

QTEST_KDEMAIN(TestTemplateClassGenerator, NoGUI)