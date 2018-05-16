/*
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

#include "test_clangutils.h"

#include "../util/clangutils.h"
#include "../util/clangtypes.h"
#include "../util/clangdebug.h"

#include <language/editor/documentrange.h>
#include <tests/testcore.h>
#include <tests/testhelpers.h>
#include <tests/autotestshell.h>

#include <clang-c/Index.h>

#include <QTemporaryFile>

#include <QDebug>
#include <QTest>

#include <memory>

QTEST_MAIN(TestClangUtils)

using namespace KDevelop;

namespace {

struct CursorCollectorVisitor
{
    QVector<CXCursor> cursors;

    static CXChildVisitResult visit(CXCursor cursor, CXCursor /*parent*/, CXClientData d)
    {
        CursorCollectorVisitor* thisPtr = static_cast<CursorCollectorVisitor*>(d);
        thisPtr->cursors << cursor;

        return CXChildVisit_Recurse;
    };
};

CXSourceRange toCXRange(CXTranslationUnit unit, const DocumentRange& range)
{
    auto file = clang_getFile(unit, QString::fromUtf8(range.document.c_str(), range.document.length()).toUtf8());
    auto begin = clang_getLocation(unit, file, range.start().line()+1, range.start().column()+1);
    auto end = clang_getLocation(unit, file, range.end().line()+1, range.end().column()+1);
    return clang_getRange(begin, end);
}

void parse(const QByteArray& code, CXTranslationUnit* unit, QString* fileName = nullptr)
{
    Q_ASSERT(unit);

    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    tempFile.write(code);
    tempFile.flush();

    if (fileName) {
        *fileName = tempFile.fileName();
    }

    std::unique_ptr<void, void(*)(CXIndex)> index(clang_createIndex(1, 1), clang_disposeIndex);
    const QVector<const char*> args = {"-std=c++11", "-xc++", "-Wall", "-nostdinc", "-nostdinc++"};
    *unit = clang_parseTranslationUnit(
        index.get(), qPrintable(tempFile.fileName()),
        args.data(), args.size(),
        nullptr, 0,
        CXTranslationUnit_None
    );
    QVERIFY(*unit);
}

template<typename Visitor>
void runVisitor(const QByteArray& code, Visitor& visitor)
{
    runVisitor(code, &Visitor::visit, &visitor);
}

void runVisitor(const QByteArray& code, CXCursorVisitor visitor, CXClientData data)
{
    CXTranslationUnit unit;
    parse(code, &unit);
    const auto startCursor = clang_getTranslationUnitCursor(unit);
    QVERIFY(!clang_Cursor_isNull(startCursor));
    QVERIFY(clang_visitChildren(startCursor, visitor, data) == 0);
}

}

void TestClangUtils::initTestCase()
{
    QLoggingCategory::setFilterRules(QStringLiteral("*.debug=false\ndefault.debug=true\nkdevelop.plugins.clang.debug=true\n"));
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
}

void TestClangUtils::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestClangUtils::testGetScope()
{
    QFETCH(QByteArray, code);
    QFETCH(int, cursorIndex);
    QFETCH(QString, expectedScope);

    CursorCollectorVisitor visitor;
    runVisitor(code, visitor);
    QVERIFY(cursorIndex < visitor.cursors.size());
    const auto cursor = visitor.cursors[cursorIndex];
    clangDebug() << "Found decl:" << clang_getCursorSpelling(cursor) << "| range:" << ClangRange(clang_getCursorExtent(cursor)).toRange();
    const QString scope = ClangUtils::getScope(cursor);
    QCOMPARE(scope, expectedScope);

    QVERIFY(!visitor.cursors.isEmpty());
}

void TestClangUtils::testGetScope_data()
{
    QTest::addColumn<QByteArray>("code");
    QTest::addColumn<int>("cursorIndex");
    QTest::addColumn<QString>("expectedScope");

    QTest::newRow("func-decl-inside-ns")
        << QByteArray("namespace ns1 { void foo(); } ns1::foo() {}")
        << 2
        << "ns1";
    QTest::newRow("fwd-decl-inside-ns")
        << QByteArray("namespace ns1 { struct foo; } struct ns1::foo {};")
        << 2
        << "ns1";
    QTest::newRow("fwd-decl-inside-ns-inside-ns")
        << QByteArray("namespace ns1 { namespace ns2 { struct foo; } } struct ns1::ns2::foo {};")
        << 3
        << "ns1::ns2";
    QTest::newRow("fwd-decl-inside-ns-inside-ns")
        << QByteArray("namespace ns1 { namespace ns2 { struct foo; } } struct ns1::ns2::foo {};")
        << 3
        << "ns1::ns2";
    QTest::newRow("using-namespace")
        << QByteArray("namespace ns1 { struct klass { struct foo; }; } using namespace ns1; struct klass::foo {};")
        << 5
        << "ns1::klass";
    QTest::newRow("fwd-decl-def-inside-namespace")
        << QByteArray("namespace ns1 { struct klass { struct foo; }; } namespace ns1 { struct klass::foo {}; }")
        << 4
        << "klass";
}

void TestClangUtils::testTemplateArgumentTypes()
{
    QFETCH(QByteArray, code);
    QFETCH(int, cursorIndex);
    QFETCH(QStringList, expectedTypes);

    CursorCollectorVisitor visitor;
    runVisitor(code, visitor);
    QVERIFY(cursorIndex < visitor.cursors.size());
    const auto cursor = visitor.cursors[cursorIndex];
    const QStringList types = ClangUtils::templateArgumentTypes(cursor);

    QCOMPARE(types, expectedTypes);
}

void TestClangUtils::testTemplateArgumentTypes_data()
{
    QTest::addColumn<QByteArray>("code");
    QTest::addColumn<int>("cursorIndex");
    QTest::addColumn<QStringList>("expectedTypes");

    QTest::newRow("template-spec")
        << QByteArray(
            "template<typename T>\n"
            "struct is_void\n"
            "{ };\n"
            "template<>\n"
            "struct is_void<void>\n"
            "{ };\n")
        << 2
        << QStringList({"void"});

    // Partially specialise one
    QTest::newRow("template-partial-spec")
        << QByteArray(
            "template<typename T, unsigned N>\n"
            "struct is_void\n"
            "{ };\n"
            "template<typename T>\n"
            "struct is_void<T, 3>\n"
            "{ };\n")
        << 3
        << QStringList({"type-parameter-0-0", ""});

    // Fully specialise 3 args
    QTest::newRow("template-full-spec")
        << QByteArray(
            "template<typename T, typename R, unsigned N>\n"
            "struct is_void\n"
            "{ };\n"
            "template<>\n"
            "struct is_void<unsigned, void, 5>\n"
            "{ };\n")
        << 4
        << QStringList({"unsigned int", "void", ""});
}

void TestClangUtils::testGetRawContents()
{
    QFETCH(QByteArray, code);
    QFETCH(KTextEditor::Range, range);
    QFETCH(QString, expectedContents);

    CXTranslationUnit unit;
    QString fileName;
    parse(code, &unit, &fileName);

    DocumentRange documentRange{IndexedString(fileName), range};
    auto cxRange = toCXRange(unit, documentRange);
    const QString contents = QString::fromLatin1(ClangUtils::getRawContents(unit, cxRange));
    QCOMPARE(contents, expectedContents);
}

void TestClangUtils::testGetRawContents_data()
{
    QTest::addColumn<QByteArray>("code");
    QTest::addColumn<KTextEditor::Range>("range");
    QTest::addColumn<QString>("expectedContents");

    QTest::newRow("complete")
        << QByteArray("void; int foo = 42; void;")
        << KTextEditor::Range(0, 6, 0, 19)
        << "int foo = 42;";
    QTest::newRow("cut-off-at-start")
        << QByteArray("void; int foo = 42; void;")
        << KTextEditor::Range(0, 7, 0, 19)
        << "nt foo = 42;";
    QTest::newRow("cut-off-at-end")
        << QByteArray("void; int foo = 42; void;")
        << KTextEditor::Range(0, 6, 0, 17)
        << "int foo = 4";
    QTest::newRow("whitespace")
        << QByteArray("void; int         ; void;")
        << KTextEditor::Range(0, 5, 0, 18)
        << " int         ";
    QTest::newRow("empty-range")
        << QByteArray("void;")
        << KTextEditor::Range(0, 0, 0, 0)
        << "";
}

void TestClangUtils::testRangeForIncludePathSpec()
{
    QCOMPARE(ClangUtils::rangeForIncludePathSpec("#include <vector>"), KTextEditor::Range(0, 10, 0, 16));
    QCOMPARE(ClangUtils::rangeForIncludePathSpec("# include <vector>"), KTextEditor::Range(0, 11, 0, 17));
    QCOMPARE(ClangUtils::rangeForIncludePathSpec("#\t include <vector>"), KTextEditor::Range(0, 12, 0, 18));
    QCOMPARE(ClangUtils::rangeForIncludePathSpec("#include <foo\\\".h>"), KTextEditor::Range(0, 10, 0, 17));
    QCOMPARE(ClangUtils::rangeForIncludePathSpec("#include \"foo\\\".h\""), KTextEditor::Range(0, 10, 0, 17));
    QCOMPARE(ClangUtils::rangeForIncludePathSpec("#include \"foo<>.h\""), KTextEditor::Range(0, 10, 0, 17));
}
