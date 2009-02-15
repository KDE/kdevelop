/*
* KDevelop xUnit testing support
* Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
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

#include "classwritertest.h"
#include "../classwriter.h"
#include "../classskeleton.h"
#include "kdevtest.h"
#include <KDebug>
#include <QRegExp>

using Veritas::Test::ClassWriterTest;
using Veritas::ClassSerializer;
using Veritas::ClassSkeleton;

/////////////////////////// test commands ////////////////////////////////////

void ClassWriterTest::noMethods()
{
    ClassSkeleton cs;
    cs.setName("Foo");

    QString actual = serialize(cs);
    QString expected("class Foo { };");

    assertAlike(expected, actual);
}

void ClassWriterTest::method()
{
    ClassSkeleton cs;
    cs.setName("Foo");
    MethodSkeleton ms;
    ms.setName("foo");
    ms.setVirtual();
    cs.addMethod(ms);

    QString expected("class Foo { public: virtual void foo() { } };");
    QString actual = serialize(cs);

    assertAlike(expected, actual);
}

void ClassWriterTest::constructor()
{
    ClassSkeleton cs;
    cs.setName("Foo");
    ConstructorSkeleton ctr;
    ctr.setName("Foo");
    ctr.addInitializer("m_foo(0)");
    ctr.setAccess(Veritas::Public);
    cs.setConstructor(ctr);

    QString actual = serialize(cs);
    QString expected =
      "class Foo { public: Foo() : m_foo(0) { } }; ";

    assertAlike(expected, actual);
}

void ClassWriterTest::constMethod()
{
    ClassSkeleton cs;
    cs.setName("Foo");
    MethodSkeleton ms;
    ms.setName( "foo" );
    ms.setConst( true );
    ms.setReturnType( "void" );
    cs.addMethod( ms );
    
    QString actual = serialize(cs);
    QVERIFY(actual.contains( " const" ));    
}

void ClassWriterTest::singleNamespace()
{
    ClassSkeleton cs;
    cs.setName("FooSpace::Foo");
    
    QString actual = serialize(cs);
    QString expected =
      "namespace FooSpace { "
      "class Foo { }; } ";

    assertAlike(expected, actual);
}

void ClassWriterTest::nestedNamespace()
{
    ClassSkeleton cs;
    cs.setName("FooSpace::BarSpace::Foo");
    
    QString actual = serialize(cs);
    QString expected =
      "namespace FooSpace { namespace BarSpace { "
      "class Foo { }; }} ";

    assertAlike(expected, actual);
}


////////////////////////// helpers ///////////////////////////////////////////

/*! Construct a regular expression that is tolerant on
spaces, newlines tabs etc */
QRegExp ClassWriterTest::whiteSpaceRegex(const QString& text)
{
  QString ws("[\\s\\n\\r\\t]*");
  QString escaped = QRegExp::escape(text);
  QStringList r = escaped.split(" ");
  r.push_front(ws);
  r.push_back(ws);
  return QRegExp(r.join(ws));
}

// helper
QString ClassWriterTest::serialize(const ClassSkeleton& cs)
{
  QBuffer buff;
  ClassSerializer cw;
  cw.write(cs, &buff);
  return QString(buff.data());
}

// custom assertion
void ClassWriterTest::assertAlike(const QString& expected, const QString& actual)
{
  QRegExp pattern = whiteSpaceRegex(expected);
  KVERIFY_MSG(pattern.isValid(), pattern.errorString() + " " +pattern.pattern());
  QString failMsg = QString("\nExpected:\n%1\nActual:\n%2").arg(expected).arg(actual);
  KVERIFY_MSG(pattern.exactMatch(actual), failMsg);
}

QTEST_MAIN( ClassWriterTest )
#include "classwritertest.moc"
