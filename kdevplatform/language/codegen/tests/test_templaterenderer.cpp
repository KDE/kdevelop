/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "test_templaterenderer.h"
#include "language/codegen/templaterenderer.h"
#include "language/codegen/codedescription.h"

#include "tests/autotestshell.h"
#include "tests/testcore.h"

using namespace KDevelop;

void TestTemplateRenderer::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);

    renderer = new KDevelop::TemplateRenderer();

    QVariantHash variables;
    variables[QStringLiteral("name")] = "Tester";
    variables[QStringLiteral("age")] = 23;
    renderer->addVariables(variables);
}

void TestTemplateRenderer::cleanupTestCase()
{
    delete renderer;
    TestCore::shutdown();
}

void TestTemplateRenderer::simpleVariables_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("content");
    QTest::addColumn<QString>("result");

    QTest::newRow("string") << "string" << "Hello, {{ name }}!" << "Hello, Tester!";
    QTest::newRow("int") << "int" << "I am {{ age }} years old" << "I am 23 years old";
    QTest::newRow("filter") << "filter" << "HELLO, {{ name|upper }}!" << "HELLO, TESTER!";
    QTest::newRow("int+string") << "int+string" << "Mr. {{ name }} is {{ age }} years old" << "Mr. Tester is 23 years old";
}

void TestTemplateRenderer::simpleVariables()
{
    QFETCH(QString, name);
    QFETCH(QString, content);
    QFETCH(QString, result);

    QCOMPARE(renderer->render(content, name), result);
}

void TestTemplateRenderer::includeTemplates()
{
    renderer->addVariable(QStringLiteral("namespaces"), QStringList() << QStringLiteral("Entity") << QStringLiteral("Human") << QStringLiteral("ComputerPerson"));
    QString result = renderer->render(QStringLiteral("HELLO {% include 'include_guard_cpp.txt' %}"), QString());
    QString expected = QStringLiteral("HELLO ENTITY_HUMAN_COMPUTERPERSON_TESTER_H");

    QCOMPARE(result, expected);
}

void TestTemplateRenderer::kdevFilters()
{
    renderer->addVariable(QStringLiteral("activity"), QStringLiteral("testing"));
    QString result = renderer->render(QStringLiteral("{% load kdev_filters %}I am {{ activity }} software. {{ activity|upper_first }} is a very rewarding task. "), QString());
    QString expected = QStringLiteral("I am testing software. Testing is a very rewarding task. ");

    QCOMPARE(result, expected);
}

void TestTemplateRenderer::kdevFiltersWithLookup()
{
    VariableDescription description;
    description.type = QStringLiteral("int");
    description.name = QStringLiteral("number");
    
    renderer->addVariable(QStringLiteral("var"), QVariant::fromValue(description));
    QString result = renderer->render(QStringLiteral("{% load kdev_filters %}void set{{ var.name|upper_first }}({{ var.type }} {{ var.name }});"), QString());
    QString expected = QStringLiteral("void setNumber(int number);");
    
    QCOMPARE(result, expected);
}


QTEST_MAIN(TestTemplateRenderer)
