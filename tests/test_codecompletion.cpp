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

#include "test_codecompletion.h"

#include <qtest_kde.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testfile.h>

#include <duchain/parsesession.h>
#include <duchain/clangtypes.h>
#include <language/codecompletion/codecompletiontesthelper.h>
#include <codecompletion/completionhelper.h>

QTEST_KDEMAIN(TestCodeCompletion, NoGUI);

using namespace KDevelop;

void TestCodeCompletion::initTestCase()
{
    QVERIFY(qputenv("KDEV_DISABLE_PLUGINS", "kdevcppsupport"));
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
}

void TestCodeCompletion::cleanupTestCase()
{
    TestCore::shutdown();
}

QString formatFuncOverrideInfo(const FuncOverrideInfo& info)
{
    return QString(info.returnType + ' ' + info.name + '(' + info.params.join(", ") +
                    ')' + (info.isConst ? " const" : "") + (info.isVirtual ? " = 0" : ""));
}

QString formatFuncImplementInfo(const FuncImplementInfo& info)
{
    return QString(info.templatePrefix + (!info.isDestructor && !info.isConstructor ? info.returnType + ' ' : "") + info.prototype);
}

namespace {
    struct TestInfo {
        SimpleCursor position;
        QStringList completions;
    };
}

using TestList = QList<TestInfo>;
Q_DECLARE_TYPEINFO(TestInfo, Q_MOVABLE_TYPE);

void runOverrideTest(QString fileContents, QString extension, TestList expected)
{
    TestFile file(fileContents, extension);

    ClangIndex index;
    ParseSession session(IndexedString(file.url()), file.fileContents().toUtf8(), &index);

    foreach(TestInfo test, expected) {
        CompletionHelper overrides(session.unit(), test.position, file.url().c_str());

        foreach(FuncOverrideInfo info, overrides.overrides()) {
            QString result = formatFuncOverrideInfo(info);
            if(!test.completions.removeOne(result)) {
                QFAIL(QString("Did not expect " + result).toStdString().c_str());
            }
        }
        if(test.completions.count() != 0) {
            QFAIL(QString("Did not find: " + test.completions.join(";")).toStdString().c_str());
        }
    }
}

void runImplementsTest(QString fileContents, QString extension, TestList expected)
{
    TestFile file(fileContents, extension);

    ClangIndex index;
    ParseSession session(IndexedString(file.url()), file.fileContents().toUtf8(), &index);

    foreach(TestInfo test, expected) {
        CompletionHelper overrides(session.unit(), test.position, file.url().c_str());

        foreach(FuncImplementInfo info, overrides.implements()) {
            QString result = formatFuncImplementInfo(info);
            if(!test.completions.removeOne(result)) {
                QFAIL(QString("Did not expect " + result).toStdString().c_str());
            }
        }
        if(test.completions.count() != 0) {
            QFAIL(QString("Did not find: " + test.completions.join(";")).toStdString().c_str());
        }
    }
}

void TestCodeCompletion::testVirtualOverrideNonTemplate()
{
    QString file("class Foo { virtual void foo(); virtual char foo(char c, int i, double d); };\n"
                 "class Bar : Foo \n{\n}");
    auto expected = {QString("void foo()"), QString("char foo(char c, int i, double d)")};
    TestInfo info{SimpleCursor(3, 1), QStringList(expected)};
    runOverrideTest(file, "h", QList<TestInfo>({info}));
}

void TestCodeCompletion::testVirtualOverrideTemplate()
{
    QString file("template<class T1, class T2> class Foo { virtual T2 foo(T1 a, T2 b, int i); } ;\n"
                 "class Bar : Foo<char, double> \n{\n}");
    auto expected = { QString("double foo(char a, double b, int i)") };
    runOverrideTest(file, "h", QList<TestInfo>({TestInfo{SimpleCursor(3, 1), QStringList(expected)}}));
}

void TestCodeCompletion::testVirtualOverrideNestedTemplate()
{
    QString file("template<class T1, class T2> class Foo { virtual T2 foo(T1 a, T2 b, int i); } ;\n"
                 "template<class T1, class T2> class Baz { };\n"
                 "class Bar : Foo<char, Baz<char, double>> \n{\n}");
    auto expected = { QString("Baz<char, double> foo(char a, Baz<char, double> b, int i)") };
    runOverrideTest(file, "h", QList<TestInfo>({TestInfo{SimpleCursor(4, 1), QStringList(expected)}}));
}

void TestCodeCompletion::testVirtualOverrideMulti()
{
    QString file("class Foo { virtual int foo(int i); };\n"
                 "class Baz { virtual char baz(char c); };\n"
                 "class Bar : Foo, Baz \n{\n}");
    auto expected = { QString("int foo(int i)"), QString("char baz(char c)") };
    runOverrideTest(file, "h", QList<TestInfo>({TestInfo{SimpleCursor(4, 1), QStringList(expected)}}));
}

void TestCodeCompletion::testVirtualOverrideDeep()
{
    QString file("class Foo { virtual int foo(int i); };\n"
                 "class Baz : Foo { };\n"
                 "class Bar : Baz \n{\n}");
    auto expected = { QString("int foo(int i)") };
    runOverrideTest(file, "h", QList<TestInfo>({TestInfo{SimpleCursor(4, 1), QStringList(expected)}}));
}

void TestCodeCompletion::testVirtualOverridePure()
{
    QString file("class Foo { virtual void foo() = 0; foo() {} };\n"
                 "class Bar : Foo \n{\n}");
    auto expected = { QString("void foo() = 0") };
    runOverrideTest(file, "h", QList<TestInfo>({TestInfo{SimpleCursor(3, 1), QStringList(expected)}}));
}

void TestCodeCompletion::testVirtualOverrideConst()
{
    QString file("class Foo { virtual void foo(const int b) const; }\n;"
                 "class Bar : Foo \n{\n}");
    auto expected = { QString("void foo(const int b) const") };
    runOverrideTest(file, "h", QList<TestInfo>({TestInfo{SimpleCursor(3, 1), QStringList(expected)}}));
}

void TestCodeCompletion::testImplementBasic()
{
    QString file("int foo(char c, int i); \n"
                 "\n");
    auto expected = { QString("int foo(char c, int i)") };
    runImplementsTest(file, "cpp", QList<TestInfo>({TestInfo{SimpleCursor(1, 0), QStringList(expected)}}));
}

void TestCodeCompletion::testImplementClass()
{
    QString file("class Foo { \n"
                 "int bar(char c, int i); \n\n"
                 "}; \n");
    auto expected = { QString("int Foo::bar(char c, int i)") };
    runImplementsTest(file, "cpp", QList<TestInfo>({TestInfo{SimpleCursor(2, 0), QStringList()},
                                                    TestInfo{SimpleCursor(4, 0), QStringList(expected)}}));
}

void TestCodeCompletion::testImplementNamespace()
{
    QString file("namespace Foo { \n"
                 "int bar(char c, int i); \n\n"
                 "}; \n");
    auto expected1 = { QString("int bar(char c, int i)") };
    auto expected2 = { QString("int Foo::bar(char c, int i)") };
    runImplementsTest(file, "cpp", QList<TestInfo>({TestInfo{SimpleCursor(2, 0), QStringList(expected1)},
                                                    TestInfo{SimpleCursor(4, 0), QStringList(expected2)}}));
}

void TestCodeCompletion::testImplementTwoNamespace()
{
    QString file("namespace Foo { \n"
                 "int bar(char c, int i); \n"
                 "}; \n"
                 "namespace Foo { \n\n"
                 "}; \n\n");
    auto expected = { QString("int bar(char c, int i)") };
    runImplementsTest(file, "cpp", QList<TestInfo>({TestInfo{SimpleCursor(4, 0), QStringList(expected)}}));
}

void TestCodeCompletion::testImplementDestructor()
{
    QString file("class Foo { \n"
                 "~Foo(); \n"
                 "}; \n");
    auto expected = { QString("Foo::~Foo()") };
    runImplementsTest(file, "cpp", QList<TestInfo>({TestInfo{SimpleCursor(3, 1), QStringList(expected)}}));
}

void TestCodeCompletion::testImplementConstructor()
{
    QString file("class Foo { \n"
                 "Foo(int i); \n"
                 "}; \n");
    auto expected = { QString("Foo::Foo(int i)") };
    runImplementsTest(file, "cpp", QList<TestInfo>({TestInfo{ SimpleCursor(3, 1), QStringList(expected)}}));
}

void TestCodeCompletion::testImplementTemplate()
{
    QString file("template<typename T> class Foo { \n"
                 "T bar(T t); \n"
                 "}; \n");
    auto expected = { QString("template<typename T> T Foo<T>::bar(T t)") };
    runImplementsTest(file, "cpp", QList<TestInfo>({TestInfo{ SimpleCursor(3, 1), QStringList(expected)}}));
}

void TestCodeCompletion::testImplementSpecializedTemplate()
{
    QString file("template<typename T> class Foo { \n"
                 "T bar(T t); \n"
                 "}; \n"
                 "template<typename T> T Foo<T>::bar(T t){} \n"
                 "template<> class Foo<int> { \n"
                 "int bar(int t); \n"
                 "}\n");
    auto expected = { QString("int Foo<int>::bar(int t)") };
    runImplementsTest(file, "cpp", QList<TestInfo>({TestInfo{ SimpleCursor(6, 1), QStringList(expected)}}));
}

void TestCodeCompletion::testImplementNestedClass()
{
    QString file("class Foo { \n"
                 "class Bar { \n"
                 "int baz(char c, int i); \n\n"
                 "}; \n\n"
                 "}; \n\n");
    auto expected = { QString("int Foo::Bar::baz(char c, int i)") };
    runImplementsTest(file, "cpp", QList<TestInfo>({TestInfo{SimpleCursor(3, 1), QStringList()},
                                                    TestInfo{SimpleCursor(5, 1), QStringList()},
                                                    TestInfo{SimpleCursor(7, 1), QStringList(expected)}}));
}

void TestCodeCompletion::testImplementNestedNamespace()
{
    QString file("namespace Foo { \n"
                 "namespace Bar { \n"
                 "int baz(char c, int i); \n\n"
                 "}; \n\n"
                 "}; \n\n");
    auto expected1 = { QString("int baz(char c, int i)") };
    auto expected2 = { QString("int Bar::baz(char c, int i)") };
    auto expected3 = { QString("int Foo::Bar::baz(char c, int i)") };
    runImplementsTest(file, "cpp", QList<TestInfo>({TestInfo{SimpleCursor(3, 1), QStringList(expected1)},
                                                    TestInfo{SimpleCursor(5, 1), QStringList(expected2)},
                                                    TestInfo{SimpleCursor(7, 1), QStringList(expected3)}}));
}

void TestCodeCompletion::testImplementPartialTemplate()
{
    QString file("template<typename T> class Foo { \n"
                 "template<typename U> class Bar; \n"
                 "template<typename U> class Bar<U*> { \n"
                 "void baz(T t, U u); \n"
                 "}; \n"
                 "}; \n");
    auto expected = { QString("template<typename T> template<typename U> void Foo<T>::Bar<U *>::baz(T t, U u)") };
    runImplementsTest(file, "cpp", QList<TestInfo>({TestInfo{ SimpleCursor(6, 1), QStringList(expected)}}));
}

void TestCodeCompletion::testImplementVariadic()
{
    QString file("int foo(...); \n"
                 "int bar(int i, ...); \n");
    auto expected = { QString("int foo(...)"), QString("int bar(int i, ...)") };
    runImplementsTest(file, "cpp", QList<TestInfo>({TestInfo{ SimpleCursor(2, 1), QStringList(expected)}}));
}

void TestCodeCompletion::testImplementConst()
{
    QString file("class Foo { \n"
                 "int bar() const; \n"
                 "}; \n");
    auto expected = { QString("int Foo::bar() const") };
    runImplementsTest(file, "cpp", QList<TestInfo>({TestInfo{ SimpleCursor(3, 1), QStringList(expected)}}));
}

#include "test_codecompletion.moc"
