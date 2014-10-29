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
    variables["name"] = "Tester";
    variables["age"] = 23;
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
    renderer->addVariable("namespaces", QStringList() << "Entity" << "Human" << "ComputerPerson");
    QString result = renderer->render("HELLO {% include 'include_guard_cpp.txt' %}", QString());
    QString expected = "HELLO ENTITY_HUMAN_COMPUTERPERSON_TESTER_H";

    QCOMPARE(result, expected);
}

void TestTemplateRenderer::kdevFilters()
{
    renderer->addVariable("activity", QString("testing"));
    QString result = renderer->render("{% load kdev_filters %}I am {{ activity }} software. {{ activity|upper_first }} is a very rewarding task. ", QString());
    QString expected = "I am testing software. Testing is a very rewarding task. ";

    QCOMPARE(result, expected);
}

void TestTemplateRenderer::kdevFiltersWithLookup()
{
    VariableDescription description;
    description.type = "int";
    description.name = "number";
    
    renderer->addVariable("var", QVariant::fromValue(description));
    QString result = renderer->render("{% load kdev_filters %}void set{{ var.name|upper_first }}({{ var.type }} {{ var.name }});", QString());
    QString expected = "void setNumber(int number);";
    
    QCOMPARE(result, expected);
}


QTEST_MAIN(TestTemplateRenderer)
