/*
 * Copyright 2014  David Stevens <dgedstevens@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "codecompletiontest.h"

#include <qtest_kde.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testfile.h>

#include <duchain/parsesession.h>
#include <duchain/clangtypes.h>
#include <language/codecompletion/codecompletiontesthelper.h>
#include <codecompletion/overridecompletionhelper.h>

QTEST_KDEMAIN(CodeCompletionTest, NoGUI);

using namespace KDevelop;

void CodeCompletionTest::initTestCase()
{
    QVERIFY(qputenv("KDEV_DISABLE_PLUGINS", "kdevcppsupport"));
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
}

void CodeCompletionTest::cleanupTestCase()
{
    TestCore::shutdown();
}

QString formatFunctionInfo(const FunctionInfo& info)
{
    return QString(info.returnType + ' ' + info.name + '(' + info.params.join(", ") +
                    ')' + (info.isConst ? " const" : "") + (info.isVirtual ? " = 0" : ""));
}

void CodeCompletionTest::testVirtualOverrideNonTemplate() {
    TestFile file("class Foo { virtual void foo(); virtual char foo(char c, int i, double d); };\n"
                  "class Bar : Foo \n{\n}","h");

    ClangIndex index;
    ParseSession session(IndexedString(file.url()), file.fileContents().toUtf8(), &index);

    SimpleCursor position(2,1);
    OverrideCompletionHelper overrides(session.unit(), position, file.url().c_str());

    QCOMPARE(overrides.getOverrides().count(),2);
    QCOMPARE(formatFunctionInfo(overrides.getOverrides().at(0)),QString("void foo()"));
    QCOMPARE(formatFunctionInfo(overrides.getOverrides().at(1)),QString("char foo(char c, int i, double d)"));
}

void CodeCompletionTest::testVirtualOverrideTemplate() {
    TestFile file("template<class T1, class T2> class Foo { virtual T2 foo(T1 a, T2 b, int i); } ;\n"
                  "class Bar : Foo<char,double> \n{\n}","h");

    ClangIndex index;
    ParseSession session(IndexedString(file.url()), file.fileContents().toUtf8(), &index);

    SimpleCursor position(2,1);
    OverrideCompletionHelper overrides(session.unit(), position, file.url().c_str());

    QCOMPARE(overrides.getOverrides().count(),1);
    QCOMPARE(formatFunctionInfo(overrides.getOverrides().at(0)),QString("double foo(char a, double b, int i)"));
}

void CodeCompletionTest::testVirtualOverrideNestedTemplate() {
    TestFile file("template<class T1, class T2> class Foo { virtual T2 foo(T1 a, T2 b, int i); } ; "
                  "template<class T1, class T2> class Baz { };"
                  "class Bar : Foo<char,Baz<char,double>> \n{\n}","h");

    ClangIndex index;
    ParseSession session(IndexedString(file.url()), file.fileContents().toUtf8(), &index);

    SimpleCursor position(2,1);
    OverrideCompletionHelper overrides(session.unit(), position, file.url().c_str());

    QCOMPARE(overrides.getOverrides().count(),1);
    QCOMPARE(formatFunctionInfo(overrides.getOverrides().at(0)),QString("Baz<char, double> foo(char a, Baz<char, double> b, int i)"));
}

void CodeCompletionTest::testVirtualOverrideMulti() {
    TestFile file("class Foo { virtual int foo(int i); } ;"
                  "class Baz { virtual char baz(char c); };"
                  "class Bar : Foo, Baz \n{\n}","h");

    ClangIndex index;
    ParseSession session(IndexedString(file.url()), file.fileContents().toUtf8(), &index);

    SimpleCursor position(2,1);
    OverrideCompletionHelper overrides(session.unit(), position, file.url().c_str());

    QCOMPARE(overrides.getOverrides().count(),2);
    QCOMPARE(formatFunctionInfo(overrides.getOverrides().at(0)),QString("int foo(int i)"));
    QCOMPARE(formatFunctionInfo(overrides.getOverrides().at(1)),QString("char baz(char c)"));
}

void CodeCompletionTest::testVirtualOverrideDeep() {
    TestFile file("class Foo { virtual int foo(int i); } ;"
                  "class Baz : Foo { };"
                  "class Bar : Baz \n{\n}","h");

    ClangIndex index;
    ParseSession session(IndexedString(file.url()), file.fileContents().toUtf8(), &index);

    SimpleCursor position(2,1);
    OverrideCompletionHelper overrides(session.unit(), position, file.url().c_str());

    QCOMPARE(overrides.getOverrides().count(),1);
    QCOMPARE(formatFunctionInfo(overrides.getOverrides().at(0)),QString("int foo(int i)"));
}

void CodeCompletionTest::testVirtualOverridePure() {
    TestFile file("class Foo { virtual void foo() = 0; foo() {} };"
                  "class Bar : Foo \n{\n}","h");

    ClangIndex index;
    ParseSession session(IndexedString(file.url()), file.fileContents().toUtf8(), &index);

    SimpleCursor position(2,1);
    OverrideCompletionHelper overrides(session.unit(), position, file.url().c_str());

    QCOMPARE(overrides.getOverrides().count(),1);
    QCOMPARE(formatFunctionInfo(overrides.getOverrides().at(0)),QString("void foo() = 0"));
}

void CodeCompletionTest::testVirtualOverrideConst() {
    TestFile file("class Foo { virtual void foo(const int b) const; };"
                  "class Bar : Foo \n{\n}","h");

    ClangIndex index;
    ParseSession session(IndexedString(file.url()), file.fileContents().toUtf8(), &index);

    SimpleCursor position(2,1);
    OverrideCompletionHelper overrides(session.unit(), position, file.url().c_str());

    QCOMPARE(overrides.getOverrides().count(),1);
    QCOMPARE(formatFunctionInfo(overrides.getOverrides().at(0)),QString("void foo(const int b) const"));
}

#include "codecompletiontest.moc"
