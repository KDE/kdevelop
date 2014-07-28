/*
 * Copyright 2014  David Stevens <dgedstevens@gmail.com>
 * Copyright 2014  Kevin Funk <kfunk@kde.org>
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

namespace {
QString formatFuncOverrideInfo(const FuncOverrideInfo& info)
{
    return QString(info.returnType + ' ' + info.name + '(' + info.params.join(", ") +
                    ')' + (info.isConst ? " const" : "") + (info.isVirtual ? " = 0" : ""));
}

QString formatFuncImplementInfo(const FuncImplementInfo& info)
{
    return QString(info.templatePrefix + (!info.isDestructor && !info.isConstructor ? info.returnType + ' ' : "") + info.prototype);
}

ParseSessionData::Ptr sessionData(const TestFile& file, ClangIndex* index)
{
    return ParseSessionData::Ptr(new ParseSessionData(file.url(), file.fileContents().toUtf8(), index));
}

struct CompletionItems {
    SimpleCursor position;
    QStringList completions;
};

}
Q_DECLARE_TYPEINFO(CompletionItems, Q_MOVABLE_TYPE);
using CompletionItemsList = QList<CompletionItems>;
Q_DECLARE_METATYPE(CompletionItemsList);

void TestCodeCompletion::testVirtualOverride()
{
    QFETCH(QString, code);
    QFETCH(CompletionItemsList, expectedItems);

    TestFile file(code, "cpp");

    ClangIndex index;
    ParseSession session(sessionData(file, &index));

    foreach(CompletionItems test, expectedItems) {
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

void TestCodeCompletion::testVirtualOverride_data()
{
    QTest::addColumn<QString>("code");
    QTest::addColumn<CompletionItemsList>("expectedItems");

    QTest::newRow("basic")
        <<  "class Foo { virtual void foo(); virtual char foo(char c, int i, double d); };\n"
            "class Bar : Foo \n{\n}"
        << CompletionItemsList{{{3, 1}, {QString("void foo()"), QString("char foo(char c, int i, double d)")}}};

    QTest::newRow("template")
        << "template<class T1, class T2> class Foo { virtual T2 foo(T1 a, T2 b, int i); } ;\n"
           "class Bar : Foo<char, double> \n{\n}"
        << CompletionItemsList{{{3, 1}, {"double foo(char a, double b, int i)"}}};

    QTest::newRow("nested-template")
        << "template<class T1, class T2> class Foo { virtual T2 foo(T1 a, T2 b, int i); } ;\n"
           "template<class T1, class T2> class Baz { };\n"
           "class Bar : Foo<char, Baz<char, double>> \n{\n}"
        << CompletionItemsList{{{4, 1}, {"Baz<char, double> foo(char a, Baz<char, double> b, int i)"}}};

    QTest::newRow("multi")
        << "class Foo { virtual int foo(int i); };\n"
           "class Baz { virtual char baz(char c); };\n"
           "class Bar : Foo, Baz \n{\n}"
        << CompletionItemsList{{{4, 1}, {"int foo(int i)", "char baz(char c)"}}};

    QTest::newRow("deep")
        << "class Foo { virtual int foo(int i); };\n"
           "class Baz : Foo { };\n"
           "class Bar : Baz \n{\n}"
        << CompletionItemsList{{{4, 1}, {"int foo(int i)"}}};

    QTest::newRow("pure")
        << "class Foo { virtual void foo() = 0; foo() {} };\n"
           "class Bar : Foo \n{\n}"
        << CompletionItemsList{{{3, 1}, {"void foo() = 0"}}};

    QTest::newRow("const")
        << "class Foo { virtual void foo(const int b) const; }\n;"
           "class Bar : Foo \n{\n}"
        << CompletionItemsList{{{3, 1}, {"void foo(const int b) const"}}};
}

void TestCodeCompletion::testImplement()
{
    QFETCH(QString, code);
    QFETCH(CompletionItemsList, expectedItems);

    TestFile file(code, "cpp");

    ClangIndex index;
    ParseSession session(sessionData(file, &index));

    foreach(CompletionItems test, expectedItems) {
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

void TestCodeCompletion::testImplement_data()
{
    QTest::addColumn<QString>("code");
    QTest::addColumn<CompletionItemsList>("expectedItems");

    QTest::newRow("basic")
        << "int foo(char c, int i); \n"
        << CompletionItemsList{{{1, 0}, {"int foo(char c, int i)"}}};

    QTest::newRow("class")
        << "class Foo { \n"
           "int bar(char c, int i); \n\n"
           "}; \n"
        << CompletionItemsList{
                {{2, 0}, {}},
                {{4, 0}, {"int Foo::bar(char c, int i)"}}
        };

    QTest::newRow("namespace")
        << "namespace Foo { \n"
           "int bar(char c, int i); \n\n"
           "}; \n"
        << CompletionItemsList{
                {{2, 0}, {"int bar(char c, int i)"}},
                {{4, 0}, {"int Foo::bar(char c, int i)"}}
        };

    QTest::newRow("two-namespace")
        << "namespace Foo { int bar(char c, int i); };\n"
           "namespace Foo {\n"
           "};\n"
        << CompletionItemsList{{{2, 0}, {"int bar(char c, int i)"}}};

    QTest::newRow("destructor")
        << "class Foo { ~Foo(); }\n"
        << CompletionItemsList{{{1, 0}, {"Foo::~Foo()"}}};

    QTest::newRow("constructor")
        << "class Foo { \n"
                 "Foo(int i); \n"
                 "}; \n"
        << CompletionItemsList{{{3, 1}, {"Foo::Foo(int i)"}}};

    QTest::newRow("template")
        << "template<typename T> class Foo { T bar(T t); };\n"
        << CompletionItemsList{{{1, 1}, {"template<typename T> T Foo<T>::bar(T t)"}}};

    QTest::newRow("specialized-template")
        << "template<typename T> class Foo { \n"
           "T bar(T t); \n"
           "}; \n"
           "template<typename T> T Foo<T>::bar(T t){} \n"
           "template<> class Foo<int> { \n"
           "int bar(int t); \n"
           "}\n"
        << CompletionItemsList{{{6, 1}, {"int Foo<int>::bar(int t)"}}};

    QTest::newRow("nested-class")
        << "class Foo { \n"
           "class Bar { \n"
           "int baz(char c, int i); \n\n"
           "}; \n\n"
           "}; \n\n"
        << CompletionItemsList{
            {{3, 1}, {}},
            {{5, 1}, {}},
            {{7, 1}, {"int Foo::Bar::baz(char c, int i)"}}
        };

    QTest::newRow("nested-namespace")
        << "namespace Foo { \n"
           "namespace Bar { \n"
           "int baz(char c, int i); \n\n"
           "}; \n\n"
           "}; \n\n"
        << CompletionItemsList{
            {{3, 1}, {"int baz(char c, int i)"}},
            {{5, 1}, {"int Bar::baz(char c, int i)"}},
            {{7, 1}, {"int Foo::Bar::baz(char c, int i)"}}
        };

    QTest::newRow("partial-template")
        << "template<typename T> class Foo { \n"
           "template<typename U> class Bar;\n"
           "template<typename U> class Bar<U*> { void baz(T t, U u); }\n"
           "}\n"
        << CompletionItemsList{{{5,1}, {"template<typename T> template<typename U> void Foo<T>::Bar<U *>::baz(T t, U u)"}}};

    QTest::newRow("variadic")
        << "int foo(...); int bar(int i, ...); \n"
        << CompletionItemsList{{{1, 1}, {"int foo(...)", "int bar(int i, ...)"}}};

    QTest::newRow("const")
        << "class Foo { int bar() const; };"
        << CompletionItemsList{{{3, 1}, {"int Foo::bar() const"}}};
}

#include "test_codecompletion.moc"
