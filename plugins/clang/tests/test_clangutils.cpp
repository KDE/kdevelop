/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
        auto* thisPtr = static_cast<CursorCollectorVisitor*>(d);
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

struct DisposeTranslationUnit {
    void operator()(CXTranslationUnit unit) { clang_disposeTranslationUnit(unit); }
};
using TranslationUnit = std::unique_ptr<CXTranslationUnitImpl, DisposeTranslationUnit>;

TranslationUnit parse(const QByteArray& code, QString* fileName = nullptr)
{
    QTemporaryFile tempFile;
    QVERIFY_RETURN(tempFile.open(), {});
    tempFile.write(code);
    tempFile.flush();

    if (fileName) {
        *fileName = tempFile.fileName();
    }

    std::unique_ptr<void, void(*)(CXIndex)> index(clang_createIndex(1, 1), clang_disposeIndex);
    const QList<const char*> args = {"-std=c++20", "-xc++", "-Wall", "-nostdinc", "-nostdinc++"};
    auto unit = TranslationUnit(clang_parseTranslationUnit(index.get(), qPrintable(tempFile.fileName()), args.data(),
                                                           args.size(), nullptr, 0, CXTranslationUnit_None));
    QVERIFY_RETURN(unit, {});
    return unit;
}

TranslationUnit runVisitor(const QByteArray& code, CXCursorVisitor visitor, CXClientData data)
{
    auto unit = parse(code);
    const auto startCursor = clang_getTranslationUnitCursor(unit.get());
    QVERIFY_RETURN(!clang_Cursor_isNull(startCursor), {});
    QVERIFY_RETURN(clang_visitChildren(startCursor, visitor, data) == 0, {});
    return unit;
}

template <typename Visitor>
TranslationUnit runVisitor(const QByteArray& code, Visitor& visitor)
{
    return runVisitor(code, &Visitor::visit, &visitor);
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
    auto unit = runVisitor(code, visitor);
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
    auto unit = runVisitor(code, visitor);
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

    // Partially specialize one
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

    // Fully specialize 3 args
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

    QString fileName;
    auto unit = parse(code, &fileName);

    DocumentRange documentRange{IndexedString(fileName), range};
    auto cxRange = toCXRange(unit.get(), documentRange);
    const QString contents = ClangUtils::getRawContents(unit.get(), cxRange);
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

void TestClangUtils::testGetCursorSignature()
{
    QFETCH(QByteArray, code);
    QFETCH(QString, expectedSignature);

    QString fileName;
    auto unit = parse(code, &fileName);
    CXCursor functionCursor = clang_getNullCursor();
    const auto startCursor = clang_getTranslationUnitCursor(unit.get());
    ClangUtils::visitChildren(startCursor, [&](CXCursor cursor, CXCursor){
        switch (clang_getCursorKind(cursor))
        {
            case CXCursor_FunctionDecl:
            case CXCursor_CXXMethod:
            case CXCursor_FunctionTemplate:
                functionCursor = cursor;
                return CXChildVisit_Break;
            default:
                return CXChildVisit_Recurse;
        }
    });
    QVERIFY2(!clang_Cursor_isNull(functionCursor), "function not found");
    auto scope = ClangUtils::getScope(functionCursor, startCursor);
    auto signature = ClangUtils::getCursorSignature(functionCursor, scope, {});
    QCOMPARE(signature, expectedSignature);
}

void TestClangUtils::testGetCursorSignature_data()
{
    QTest::addColumn<QByteArray>("code");
    QTest::addColumn<QString>("expectedSignature");

    QTest::newRow("global-less")
        << QByteArray("class klass {}; bool operator < (const klass&, const klass&);")
        << "bool operator<(const klass&, const klass&)";
    QTest::newRow("member-less")
        << QByteArray("class klass { bool operator < (const klass&); };")
        << "bool klass::operator<(const klass&)";
    QTest::newRow("template-member-less")
        << QByteArray("class klass { template<typename T> bool operator < (const T&); };")
        << "bool klass::operator<(const T&)";
}

#include "moc_test_clangutils.cpp"
