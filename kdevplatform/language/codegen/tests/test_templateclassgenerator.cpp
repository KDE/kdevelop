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

/*
* CHECK_TEMPLATE_VARIABLE expects second parameter as type with 'to' instead of 'Q' in begin
* For example, QString -> toString
*/
#define CHECK_TEMPLATE_VARIABLE(name, type, val)            \
QVERIFY(variables.contains(QStringLiteral(#name)));         \
QCOMPARE(variables.value(QStringLiteral(#name)).type(), val)

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
    TemplatesModel model(QStringLiteral("kdevcodegentest"));
    model.refresh();

    description.members << VariableDescription(QStringLiteral("QString"), QStringLiteral("name"))
                        << VariableDescription(QStringLiteral("int"), QStringLiteral("number"))
                        << VariableDescription(QStringLiteral("SomeCustomType"), QStringLiteral("data"));

    FunctionDescription function;
    function.name = QStringLiteral("doSomething");
    function.isVirtual = true;
    function.arguments << VariableDescription(QStringLiteral("double"), QStringLiteral("howMuch"))
                       << VariableDescription(QStringLiteral("bool"), QStringLiteral("doSomethingElse"));

    VariableDescriptionList args;
    VariableDescriptionList returnArgs;
    returnArgs << VariableDescription(QStringLiteral("int"), QStringLiteral("someOtherNumber"));
    FunctionDescription otherFunction(QStringLiteral("getSomeOtherNumber"), args, returnArgs);
    description.methods << function << otherFunction;
}

void TestTemplateClassGenerator::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestTemplateClassGenerator::fileLabelsCpp()
{
    TemplateClassGenerator* generator = loadTemplate(QStringLiteral("test_cpp"));
    QHash<QString,QString> labels = generator->fileLabels();
    QCOMPARE(labels.size(), 2);

    /*
     * File labels can be translated, so we don't check their equality here.
     * But they have to be present and non-empty
     */
    QVERIFY(labels.contains(QStringLiteral("Header")));
    QVERIFY(!labels[QStringLiteral("Header")].isEmpty());
    QVERIFY(labels.contains(QStringLiteral("Implementation")));
    QVERIFY(!labels[QStringLiteral("Implementation")].isEmpty());
}

void TestTemplateClassGenerator::fileLabelsYaml()
{
    TemplateClassGenerator* generator = loadTemplate(QStringLiteral("test_yaml"));
    QHash<QString,QString> labels = generator->fileLabels();
    QCOMPARE(labels.size(), 1);

    QVERIFY(labels.contains(QStringLiteral("Description")));
    QVERIFY(!labels[QStringLiteral("Description")].isEmpty());
}

void TestTemplateClassGenerator::defaultFileUrlsCpp()
{
    TemplateClassGenerator* generator = loadTemplate(QStringLiteral("test_cpp"));
    QHash<QString,QUrl> files = generator->fileUrls();
    QCOMPARE(files.size(), 2);

    QVERIFY(files.contains(QStringLiteral("Header")));
    QCOMPARE(files[QStringLiteral("Header")], baseUrl.resolved(QUrl(QStringLiteral("ClassName.h"))));

    QVERIFY(files.contains(QStringLiteral("Implementation")));
    QCOMPARE(files[QStringLiteral("Implementation")], baseUrl.resolved(QUrl(QStringLiteral("ClassName.cpp"))));
}

void TestTemplateClassGenerator::defaultFileUrlsYaml()
{
    TemplateClassGenerator* generator = loadTemplate(QStringLiteral("test_yaml"));
    QHash<QString,QUrl> files = generator->fileUrls();
    QCOMPARE(files.size(), 1);

    QVERIFY(files.contains(QStringLiteral("Description")));
    QCOMPARE(files[QStringLiteral("Description")], baseUrl.resolved(QUrl(QStringLiteral("ClassName.yaml"))));
}

void TestTemplateClassGenerator::customOptions()
{
    TemplateClassGenerator* generator = loadTemplate(QStringLiteral("test_yaml"));
    QCOMPARE(generator->sourceFileTemplate().hasCustomOptions(), false);

    generator = loadTemplate(QStringLiteral("test_options"));
    QCOMPARE(generator->sourceFileTemplate().hasCustomOptions(), true);

    // test if option data loaded with all values in same order as in kcfg file
    struct ExpectedOption
    {
        const char* name; const char* label; const char* type; const char* value; QStringList values;
    };
    const struct {
        const char* name;
        QVector<ExpectedOption> expectedOptions;
    } expectedGroupDataList[] = {
        { "A Group", {
            {"bool_option",   "A Bool",     "Bool",   "true",   {}},
            {"string_option", "Zzz String", "String", "Test",   {}},
            {"enum_option",   "Bb Enum",    "Enum",   "Second", {
                {QLatin1String("First")}, {QLatin1String("Second")}, {QLatin1String("Last")}
            }}
        }},
        { "Zzz Group", {
            {"z_option", "Z Bool", "Bool", "false", {}}
        }},
        { "Bb Group", {
            {"b_option", "B Bool", "Bool", "true",  {}}
        }}
    };
    const int expectedGroupDataCount = sizeof(expectedGroupDataList)/sizeof(expectedGroupDataList[0]);
    
    const auto customOptionGroups = generator->sourceFileTemplate().customOptions(generator->renderer());

    QCOMPARE(customOptionGroups.count(), expectedGroupDataCount);
    for (int i = 0; i < expectedGroupDataCount; ++i) {
        const auto& customOptionGroup = customOptionGroups[i];
        const auto& expectedGroupData = expectedGroupDataList[i];

        QCOMPARE(customOptionGroup.name, QString::fromLatin1(expectedGroupData.name));
        QCOMPARE(customOptionGroup.options.count(), expectedGroupData.expectedOptions.count());
        for (int j = 0; j < expectedGroupData.expectedOptions.count(); ++j) {
            const auto& customOption = customOptionGroup.options[j];
            const auto& expectedOptionData = expectedGroupData.expectedOptions[j];

            QCOMPARE(customOption.name, QString::fromLatin1(expectedOptionData.name));
            QCOMPARE(customOption.label, QString::fromLatin1(expectedOptionData.label));
            QCOMPARE(customOption.type, QString::fromLatin1(expectedOptionData.type));
            QCOMPARE(customOption.value.toString(), QString::fromLatin1(expectedOptionData.value));
            QCOMPARE(customOption.values, expectedOptionData.values);
        }
    }
}

void TestTemplateClassGenerator::templateVariablesCpp()
{
    TemplateClassGenerator* generator = loadTemplate(QStringLiteral("test_cpp"));
    setLowercaseFileNames(generator);

    QVariantHash variables = generator->renderer()->variables();
    CHECK_TEMPLATE_VARIABLE(name, toString, QStringLiteral("ClassName"));

    CHECK_TEMPLATE_VARIABLE(output_file_header, toString, QStringLiteral("classname.h"));
    CHECK_TEMPLATE_VARIABLE(output_file_header_absolute, toString, baseUrl.resolved(QUrl(QStringLiteral("classname.h"))).toLocalFile());
}

void TestTemplateClassGenerator::templateVariablesYaml()
{
    TemplateClassGenerator* generator = loadTemplate(QStringLiteral("test_yaml"));
    setLowercaseFileNames(generator);

    QVariantHash variables = generator->renderer()->variables();
    CHECK_TEMPLATE_VARIABLE(name, toString, QStringLiteral("ClassName"));

    CHECK_TEMPLATE_VARIABLE(output_file_description, toString, QStringLiteral("classname.yaml"));
    CHECK_TEMPLATE_VARIABLE(output_file_description_absolute, toString, baseUrl.resolved(QUrl(QStringLiteral("classname.yaml"))).toLocalFile());
}

void TestTemplateClassGenerator::codeDescription()
{
    TemplateClassGenerator* generator = loadTemplate(QStringLiteral("test_yaml"));

    QVariantHash variables = generator->renderer()->variables();

    qDebug() << variables;

    QVERIFY(variables.contains(QStringLiteral("base_classes")));
    QVariantList inheritance = variables[QStringLiteral("base_classes")].toList();
    QCOMPARE(inheritance.size(), 1);
    QCOMPARE(inheritance.first().value<InheritanceDescription>().baseType, QStringLiteral("QObject"));
    QCOMPARE(inheritance.first().value<InheritanceDescription>().inheritanceMode, QStringLiteral("public"));

    QVERIFY(variables.contains(QStringLiteral("members")));
    QVariantList members = variables[QStringLiteral("members")].toList();
    QCOMPARE(members.size(), 3);
    QCOMPARE(members.first().value<VariableDescription>().type, QStringLiteral("QString"));
    QCOMPARE(members.first().value<VariableDescription>().name, QStringLiteral("name"));

    QVERIFY(variables.contains(QStringLiteral("functions")));
    QVariantList methods = variables[QStringLiteral("functions")].toList();
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
    TemplateClassGenerator* generator = loadTemplate(QStringLiteral("test_cpp"));

    DocumentChangeSet changes = generator->generate();
    DocumentChangeSet::ChangeResult result = changes.applyAllChanges();
    QVERIFY(result.m_success);

    QDir dir(baseUrl.toLocalFile());
    QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    QCOMPARE(entries.size(), 2);
}

void TestTemplateClassGenerator::cppOutput()
{
    TemplateClassGenerator* generator = loadTemplate(QStringLiteral("test_cpp"));
    setLowercaseFileNames(generator);
    DocumentChangeSet changes = generator->generate();
    changes.setFormatPolicy(DocumentChangeSet::NoAutoFormat);
    changes.applyAllChanges();

    QFile header(baseUrl.resolved(QUrl(QStringLiteral("classname.h"))).toLocalFile());
    QVERIFY(header.open(QIODevice::ReadOnly));

    QFile testHeader(QStringLiteral(CODEGEN_TESTS_EXPECTED_DIR "/classname.h"));
    testHeader.open(QIODevice::ReadOnly);
    COMPARE_FILES(header, testHeader);

    QFile implementation(baseUrl.resolved(QUrl(QStringLiteral("classname.cpp"))).toLocalFile());
    QVERIFY(implementation.open(QIODevice::ReadOnly));

    QFile testImplementation(QStringLiteral(CODEGEN_TESTS_EXPECTED_DIR "/classname.cpp"));
    testImplementation.open(QIODevice::ReadOnly);
    COMPARE_FILES(implementation, testImplementation);
}

void TestTemplateClassGenerator::yamlOutput()
{
    TemplateClassGenerator* generator = loadTemplate(QStringLiteral("test_yaml"));
    setLowercaseFileNames(generator);
    generator->generate().applyAllChanges();

    QFile yaml(baseUrl.resolved(QUrl(QStringLiteral("classname.yaml"))).toLocalFile());
    QVERIFY(yaml.open(QIODevice::ReadOnly));

    QFile testYaml(QStringLiteral(CODEGEN_TESTS_EXPECTED_DIR "/classname.yaml"));
    testYaml.open(QIODevice::ReadOnly);
    COMPARE_FILES(yaml, testYaml);
}

TemplateClassGenerator* TestTemplateClassGenerator::loadTemplate (const QString& name)
{
    QDir dir(baseUrl.toLocalFile());
    const auto files = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    for (const QString& fileName : files) {
        dir.remove(fileName);
    }

    auto* generator = new TemplateClassGenerator(baseUrl);

    QString tplDescription = QStringLiteral(CODEGEN_DATA_DIR) + "/kdevcodegentest/templates/" + name + "/" + name + ".desktop";
    Q_ASSERT(!tplDescription.isEmpty());
    SourceFileTemplate tpl;
    tpl.addAdditionalSearchLocation(QStringLiteral(CODEGEN_TESTS_DATA_DIR) + "/kdevcodegentest/templates/");
    tpl.setTemplateDescription(tplDescription);
    Q_ASSERT(tpl.isValid());
    generator->setTemplateDescription(tpl);
    generator->setDescription(description);
    generator->setIdentifier(QStringLiteral("ClassName"));
    generator->addBaseClass(QStringLiteral("public QObject"));
    generator->setLicense(QStringLiteral("This is just a test.\nYou may do with it as you please."));
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
