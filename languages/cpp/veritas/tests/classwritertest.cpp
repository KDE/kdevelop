/*
 * KDevelop xUnit testing support
 *
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
#include "classwritertest.h"
#include "../classwriter.h"
#include "../classskeleton.h"
#include "kdevtest.h"
#include <KDebug>

using Veritas::Test::ClassWriterTest;
using Veritas::ClassSerializer;
using Veritas::ClassSkeleton;

void ClassWriterTest::init()
{
}

void ClassWriterTest::cleanup()
{
}

void ClassWriterTest::noMethods()
{
    ClassSerializer cw;
    ClassSkeleton cs;
    cs.setName("Foo");
    QBuffer buff;
    QByteArray expected = "class Foo\n{\n};\n";
    cw.write(cs, &buff);
    QString actual(buff.data());
    kDebug() << actual;
    KOMPARE_(expected, actual);   
}

void ClassWriterTest::methods()
{
    ClassSerializer cw;
    ClassSkeleton cs;
    cs.setName("Foo");
    ConstructorSkeleton ctr;
    ctr.setName("Foo");
    ctr.addInitializer("m_foo(0)");
    ctr.setAccess(Veritas::Public);
    MethodSkeleton dtr;
    dtr.setName("~Foo");
    dtr.setAccess(Veritas::Public);
    MethodSkeleton mtd;
    mtd.setName("foo");
    mtd.setReturnType("int");
    mtd.setArguments("(int moo)");
    mtd.setBody("return 0;\n");
    mtd.setAccess(Veritas::Public);
    cs.setConstructor(ctr);
    cs.setDestructor(dtr);
    cs.addMethod(mtd);
    cs.addMember("int m_foo");
    QBuffer buff;
    QByteArray expected = 
      "class Foo\n"
      "{\n"
      "public:\n"
      "    Foo() : m_foo(0) {}\n"
      "    virtual ~Foo() {}\n"
      "    virtual int foo(int moo) {\n"
      "        return 0;\n"
      "    }\n"
      "    int m_foo\n"
      "};\n";
    cw.write(cs, &buff);
    QString actual(buff.data());
    kDebug() << "ACTUAL\n" << actual << "EXPECTED\n" << QString(expected);
    KOMPARE_(expected, actual);
}

QTEST_MAIN( ClassWriterTest )
#include "classwritertest.moc"
