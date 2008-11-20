/*
* KDevelop xUnit integration
* Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301, USA.
*/

#include "uutcreationtest.h"
#include "kdevtest.h"
#include "classdeclarationfactory.h"
#include "../uutconstructor.h"
#include "../classskeleton.h"
#include "documentaccessstub.h"
#include <language/duchain/use.h>

using Veritas::Test::UUTCreationTest;
using Veritas::Test::DeclarationFactory;
using Veritas::Test::DocumentAccessStub;
using Veritas::UUTConstructor;
using Veritas::ClassSkeleton;
using Veritas::MethodSkeleton;

using namespace KDevelop;

void UUTCreationTest::init()
{
    m_factory = new DeclarationFactory;
    m_document = new DocumentAccessStub;
    m_constructor = new UUTConstructor;
    m_constructor->setDocumentAccess(m_document);
}

void UUTCreationTest::cleanup()
{
    if (m_factory) delete m_factory;
    if (m_document) delete m_document;
    if (m_constructor) delete m_constructor;
}

void UUTCreationTest::unresolvedVariable()
{
    QList<Declaration*> variables = m_factory->unresolvedVariablesFromText("Foo f;");
    Q_ASSERT(variables.count() == 1);
    ClassSkeleton cs = m_constructor->morph(variables[0]);

    KVERIFY(!cs.isEmpty());
    KOMPARE("Foo", cs.name());
    KVERIFY(cs.methods().isEmpty());
    KOMPARE(0, cs.memberCount());
}

void UUTCreationTest::resolvedVariables()
{
    QList<Declaration*> variables = m_factory->variablesFromText(
      "class Foo {}; int i; Foo f;");
    Q_ASSERT(variables.count() == 2);

    KVERIFY(m_constructor->morph(variables[0]).isEmpty());
    KVERIFY(m_constructor->morph(variables[1]).isEmpty());
}

// helper to construct a class from it's implementation
ClassSkeleton UUTCreationTest::classFromImplementation(const QByteArray& text)
{
    QList<Declaration*> variables =
        m_factory->unresolvedVariablesFromText(text);
    m_document->m_text = QString(text);
    if (!variables.count()) return ClassSkeleton();
    return m_constructor->morph(variables[0]);
}

void UUTCreationTest::singleUse()
{
    ClassSkeleton cs = classFromImplementation(
        "void fun() { Foo moo; moo.foo(); } ");

    assertNamed("Foo", cs);
    assertSimpleFooMethod(cs);
}

void UUTCreationTest::tdd_nonVoidReturn()
{
    TDD_TODO;

    ClassSkeleton cs = classFromImplementation(
        "void fun() { Foo f; int i = f.foo(); } ");

    assertNamed("Foo", cs);
    KOMPARE(1, cs.methods().count());

    MethodSkeleton ms = cs.methods()[0];
    assertNamed("foo", ms);
    KOMPARE("int", ms.returnType());
    assertNoArguments(ms);
    assertDefaultBody(ms);
}

void UUTCreationTest::singleArgument()
{
    ClassSkeleton cs = classFromImplementation(
        "void fun() { Foo f; int i; f.foo(i); } ");

    assertNamed("Foo", cs);
    KOMPARE(1, cs.methods().count());

    MethodSkeleton ms = cs.methods()[0];
    assertNamed("foo", ms);
    KOMPARE("(int)", ms.arguments());
    assertReturnsVoid(ms);
    assertDefaultBody(ms);
}

void UUTCreationTest::tdd_pointerUse()
{
    TDD_TODO;

    ClassSkeleton cs = classFromImplementation(
        "void fun() { Foo *f; f->foo(); } ");
    assertNamed("Foo", cs);
    assertSimpleFooMethod(cs);
}

void UUTCreationTest::multipleUses()
{
    ClassSkeleton cs = classFromImplementation(
        "void fun() { Foo f; f.foo(); f.foo1(); } ");

    assertNamed("Foo", cs);
    KOMPARE(2, cs.methods().count());

    MethodSkeleton ms = cs.methods()[0];
    assertNamed("foo", ms);
    assertNoArguments(ms);
    assertReturnsVoid(ms);
    assertDefaultBody(ms);

    MethodSkeleton ms2 = cs.methods()[1];
    assertNamed("foo1", ms2);
    assertNoArguments(ms2);
    assertReturnsVoid(ms2);
    assertDefaultBody(ms2);
}

void UUTCreationTest::useInIfConstruct()
{
    ClassSkeleton cs = classFromImplementation(
        "void fun() { Foo f; if (true) { f.foo(); } } ");
    assertSimpleFooMethod(cs);
}

void UUTCreationTest::assertSimpleFooMethod(const ClassSkeleton& cs)
{
    KOMPARE(1, cs.methods().count());
    MethodSkeleton ms = cs.methods()[0];
    assertNamed("foo", ms);
    assertNoArguments(ms);
    assertReturnsVoid(ms);
    assertDefaultBody(ms);
}

void UUTCreationTest::spacesBeforeDot()
{
    ClassSkeleton cs = classFromImplementation(
        "void fun() { Foo f; f  .foo(); } ");
    assertSimpleFooMethod(cs);
}

void UUTCreationTest::spacesAfterDot()
{
    ClassSkeleton cs = classFromImplementation(
        "void fun() { Foo f; f.  foo(); } ");
    assertSimpleFooMethod(cs);
}

void UUTCreationTest::methodOnNextLine()
{
    ClassSkeleton cs = classFromImplementation(
        "void fun() { Foo f; f.\nfoo(); } ");
    assertSimpleFooMethod(cs);
}

void UUTCreationTest::dataMemberUse()
{
    ClassSkeleton cs = classFromImplementation(
        "void fun() { Foo f; f.m_foo; } ");
    KOMPARE(0, cs.methods().count());
}

void UUTCreationTest::multipleParameters()
{
    ClassSkeleton cs = classFromImplementation(
        "void fun() { Foo f; int i; char c; f.foo(i,c); } ");

    KOMPARE(1, cs.methods().count());
    MethodSkeleton ms = cs.methods()[0];
    assertNamed("foo", ms);
    KOMPARE("(int, char)", ms.arguments());
    assertReturnsVoid(ms);
    assertDefaultBody(ms);
}

void UUTCreationTest::classParameter()
{
    ClassSkeleton cs = classFromImplementation(
        "class Bar; void fun() { Foo f; Bar b; f.foo(b); } ");

    KOMPARE(1, cs.methods().count());
    MethodSkeleton ms = cs.methods()[0];
    KOMPARE("(Bar)", ms.arguments());
    assertReturnsVoid(ms);
    assertDefaultBody(ms);
}

void UUTCreationTest::threeParameters()
{
    ClassSkeleton cs = classFromImplementation(
      "void fun() { Foo f; int i; char c; bool b; f.foo(i,c,b); } ");

    KOMPARE(1, cs.methods().count());
    MethodSkeleton ms = cs.methods()[0];
    assertNamed("foo", ms);
    KOMPARE("(int, char, bool)", ms.arguments());
    assertReturnsVoid(ms);
    assertDefaultBody(ms);
}

void UUTCreationTest::functionUseWithIdenticalName()
{
    ClassSkeleton cs = classFromImplementation(
        "void f() {} void fun() { Foo f; f.foo(); f(); } ");
    assertSimpleFooMethod(cs);
}


void UUTCreationTest::multipleParametersSmearedOverMultipleLines()
{
  kDebug() << 4;
    ClassSkeleton cs = classFromImplementation(
        "void fun() {\n"
        "   Foo f; int i; char c;\n"
        "   f.foo(\n"
        "     i,\n"
        "     c);\n"
        "}\n");

    KOMPARE(1, cs.methods().count());
    MethodSkeleton ms = cs.methods()[0];
    assertNamed("foo", ms);
    KOMPARE("(int, char)", ms.arguments());
    assertReturnsVoid(ms);
    assertDefaultBody(ms);
}

void UUTCreationTest::nestedBraces()
{
    ClassSkeleton cs = classFromImplementation(
        "int fn() {\n"
        "  return 0;\n"
        "}\n"
        "void fun() {\n"
        "  Foo f;\n"
        "  int i;\n"
        "  f.foo(fn());\n"
        "}\n");
    MethodSkeleton ms = cs.methods()[0];
    assertNamed("foo", ms);
    KOMPARE("(int)", ms.arguments());
    assertReturnsVoid(ms);
    assertDefaultBody(ms);
}

void UUTCreationTest::noSemicolon()
{
    ClassSkeleton cs = classFromImplementation(
        "void fun() { Foo f; f.foo() } ");
    assertNamed("Foo", cs);
    KOMPARE(0, cs.methods().count());
}

//////////////////// Custom assertions ///////////////////////////////////////

void UUTCreationTest::assertReturnsVoid(const MethodSkeleton& ms)
{
    QString failMsg = QString("Was expecting void return type but got: %1").arg(ms.returnType());
    KOMPARE_MSG("void", ms.returnType(), failMsg);
}

void UUTCreationTest::assertNoArguments(const MethodSkeleton& ms)
{
    QString failMsg = QString("Was expecting no arguments but got: %1").arg(ms.returnType());
    KOMPARE_MSG("()", ms.arguments(), failMsg);
}

void UUTCreationTest::assertNamed(const QString& name, const ClassSkeleton& t)
{
    QString failMsg = QString("Wrong name. Expected : ``%1'' but got ``%2''").arg(name).arg(t.name());
    KOMPARE_MSG(name, t.name(), failMsg);
}

void UUTCreationTest::assertNamed(const QString& name, const MethodSkeleton& t)
{
    QString failMsg = QString("Wrong name. Expected : ``%1'' but got ``%2''").arg(name).arg(t.name());
    KOMPARE_MSG(name, t.name(), failMsg);
}

void UUTCreationTest::assertDefaultBody(const MethodSkeleton& ms)
{
    KOMPARE("// GENERATED", ms.body());
}

QTEST_MAIN( UUTCreationTest )
#include "uutcreationtest.moc"
