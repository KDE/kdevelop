/*
 * KDevelop xUnit test support
 * Copyright 2008 Manuel Breugelmans
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

#include "stubcreationtest.h"

#include "kdevtest.h"

#include "../stubconstructor.h"
#include "../classskeleton.h"
#include "../constructorskeleton.h"
#include "../methodskeleton.h"

#include "classdeclarationfactory.h"

#include "../cppduchain/classdeclaration.h"
#include <language/editor/simplecursor.h>


using Veritas::StubConstructor;
using Veritas::ClassSkeleton;
using Veritas::ConstructorSkeleton;
using Veritas::MethodSkeleton;
using Veritas::Test::StubCreationTest;
using Veritas::Test::DeclarationFactory;

using namespace Cpp;
using namespace KDevelop;

/////////////////////////// fixture //////////////////////////////////////////

void StubCreationTest::init()
{
    m_factory = new DeclarationFactory;
    m_constructor = new StubConstructor;
}

void StubCreationTest::cleanup()
{
    if (m_factory) {
        //m_factory->release();
        delete m_factory;
    }
    if (m_constructor) delete m_constructor;
}

/////////////////////////// commands /////////////////////////////////////////

void StubCreationTest::appendStubToClassName()
{
    ClassSkeleton actual = createStubClassFrom(
      "class IFoo\n"
      "{};\n");
    
    KVERIFY(!actual.isEmpty());
    ClassSkeleton expected;
    expected.setName("IFooStub");
    KOMPARE("IFoo", actual.super());
    KOMPARE(expected.name(), actual.name());
}

void StubCreationTest::basicConstructor()
{
    ClassSkeleton actual = createStubClassFrom(
      "class IFoo\n"
      "{ public: IFoo(); virtual ~IFoo(); };\n");

    ConstructorSkeleton cs = actual.constructor();
    KVERIFY(!cs.isEmpty());
    KOMPARE("IFooStub", cs.name());
}

void StubCreationTest::basicDestructor()
{
    ClassSkeleton actual = createStubClassFrom(
      "class IFoo\n"
      "{ public: IFoo(); virtual ~IFoo(); };\n");

    MethodSkeleton d = actual.destructor();
    KOMPARE("~IFooStub", d.name());
}

void StubCreationTest::basicInterface()
{
    ClassSkeleton actual = createStubClassFrom(
      "class IFoo\n"
      "{\n"
      "public:\n"
      "  virtual void foo() = 0;\n"
      "};\n");

    KVERIFY(!actual.isEmpty());
    MethodSkeleton ms;
    ms.setName("foo");
    ms.setReturnType("void");
    ms.setBody("");
    KOMPARE("IFooStub", actual.name());
    KOMPARE("IFoo", actual.super());
    KOMPARE(1, actual.methods().count());
    kompareMethods(ms, actual.methods()[0]);
    
    KVERIFY(!actual.hasMembers());
    KOMPARE(0, actual.memberCount());
}

void StubCreationTest::nonVoidReturnType()
{
    ClassSkeleton actual = createStubClassFrom(
      "class IFoo\n"
      "{\n"
      "public:\n"
      "  virtual int foo() = 0;\n"
      "};\n");

    KVERIFY(!actual.isEmpty());
    MethodSkeleton ms;
    ms.setName("foo");
    ms.setReturnType("int");
    ms.setBody("return m_foo;");
    KOMPARE("IFooStub", actual.name());
    KOMPARE(1, actual.methods().count());
    kompareMethods(ms, actual.methods()[0]);
    KOMPARE(1, actual.memberCount());
    KOMPARE("int m_foo", actual.member(0));
}

void StubCreationTest::nonVirtualMethod()
{
    ClassSkeleton actual = createStubClassFrom(
      "class IFoo\n"
      "{\n"
      "public:\n"
      "  int foo();\n"
      "};\n");

    KVERIFY(!actual.isEmpty());
    KVERIFY(actual.methods().isEmpty());
}

void StubCreationTest::privateMethod()
{
    ClassSkeleton actual = createStubClassFrom(
      "class IFoo\n"
      "{\n"
      "private:   virtual bool bar();\n"
      "};\n");

    KVERIFY(!actual.isEmpty());
    KVERIFY(actual.methods().isEmpty());
}

void StubCreationTest::signal()
{
    ClassSkeleton actual = createStubClassFrom(
      "class IFoo\n"
      "{\n"
      "signals: void foo();\n"
      "};\n");

    KVERIFY(!actual.isEmpty());
    KVERIFY(actual.methods().isEmpty());
}

void StubCreationTest::signalWithEmptyPublic()
{
    ClassSkeleton actual = createStubClassFrom(
      "class IFoo\n"
      "{\n"
      "public:\n"
      "signals: void foo();\n"
      "};\n");

    KVERIFY(!actual.isEmpty());
    KVERIFY(actual.methods().isEmpty());
}

void StubCreationTest::prependNamespace()
{
    ClassSkeleton actual = createStubClassFrom(
      "namespace Zoo { class IFoo {}; }\n");
      
    KVERIFY(!actual.isEmpty());
    KOMPARE("Zoo::IFooStub", actual.name());
}

void StubCreationTest::prependNestedNamespace()
{
    ClassSkeleton actual = createStubClassFrom(
      "namespace Zoo { namespace Loo { class IFoo {}; } }\n");

    KVERIFY(!actual.isEmpty());
    KOMPARE("Zoo::Loo::IFooStub", actual.name());
}

void StubCreationTest::constMethod()
{
    ClassSkeleton actual = createStubClassFrom(
      "class IFoo\n"
      "{\n"
      "public:\n"
      "  virtual void foo() const = 0;\n"
      "};\n");

    KVERIFY(!actual.isEmpty());
    MethodSkeleton expectedMtd;
    expectedMtd.setName("foo");
    expectedMtd.setReturnType("void");
    expectedMtd.setBody("");
    expectedMtd.setConst(true);
    KOMPARE(1, actual.methods().count());
    kompareMethods(expectedMtd, actual.methods()[0]);
}

void StubCreationTest::dropMemberConstness()
{
    ClassSkeleton actual = createStubClassFrom(
      "class IFoo\n"
      "{\n"
      "public:\n"
      "  virtual const int foo() = 0;\n"
      "};\n");

    KVERIFY(!actual.isEmpty());
    KOMPARE(1, actual.memberCount());
    KOMPARE("int m_foo", actual.member(0));
}

void StubCreationTest::pointerConstructorInitializer()
{
    ClassSkeleton actual = createStubClassFrom(
      "class IFoo\n"
      "{\n"
      "public:\n"
      "  virtual IFoo* foo() = 0;\n"
      "};\n");

    KVERIFY(!actual.isEmpty());
    ConstructorSkeleton cs = actual.constructor();
    KOMPARE(1, cs.initializerList().size());
    KOMPARE("m_foo(0)", cs.initializerList()[0]);
}

void StubCreationTest::boolConstructorInitializer()
{
    ClassSkeleton actual = createStubClassFrom(
      "class IFoo\n"
      "{\n"
      "public:\n"
      "  virtual bool foo() = 0;\n"
      "};\n");

    KVERIFY(!actual.isEmpty());
    ConstructorSkeleton cs = actual.constructor();
    KOMPARE(1, cs.initializerList().size());
    KOMPARE("m_foo(false)", cs.initializerList()[0]);
}

void StubCreationTest::intConstructorInitializer()
{
    ClassSkeleton actual = createStubClassFrom(
      "class IFoo\n"
      "{\n"
      "public:\n"
      "  virtual int foo() = 0;\n"
      "};\n");

    KVERIFY(!actual.isEmpty());
    ConstructorSkeleton cs = actual.constructor();
    KOMPARE(1, cs.initializerList().size());
    KOMPARE("m_foo(0)", cs.initializerList()[0]);
}

void StubCreationTest::noInitializerRegularClass()
{
      ClassSkeleton actual = createStubClassFrom(
      "class IFoo\n"
      "{\n"
      "public:\n"
      "  virtual IFoo foo() = 0;\n"
      "};\n");

    KVERIFY(!actual.isEmpty());
    ConstructorSkeleton cs = actual.constructor();
    KOMPARE(0, cs.initializerList().size());
}

void StubCreationTest::noInitializerVoidReturn()
{
      ClassSkeleton actual = createStubClassFrom(
      "class IFoo\n"
      "{\n"
      "public:\n"
      "  virtual void foo() = 0;\n"
      "};\n");

    KVERIFY(!actual.isEmpty());
    ConstructorSkeleton cs = actual.constructor();
    KOMPARE(0, cs.initializerList().size());
}

/////////////////////////// helpers //////////////////////////////////////////

void StubCreationTest::kompareMethods(const MethodSkeleton& expected, const MethodSkeleton& actual)
{
    KOMPARE(expected.name(), actual.name());
    KOMPARE(expected.returnType(), actual.returnType());
    KOMPARE(expected.arguments(), actual.arguments());
    KOMPARE(expected.body(), actual.body());
    KOMPARE(expected.isConst(), actual.isConst());
}

ClassSkeleton StubCreationTest::createStubClassFrom(const QByteArray& text)
{
    ClassDeclaration* clazz = m_factory->classFromText(text);
    kDebug() << clazz;
    return m_constructor->morph(clazz);
}

QTEST_KDEMAIN( StubCreationTest, NoGUI )
#include "stubcreationtest.moc"
