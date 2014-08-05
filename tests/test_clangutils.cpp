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

#include <language/editor/simplerange.h>

#include <clang-c/Index.h>

#include <KDebug>
#include <KTemporaryFile>

#include <qtest_kde.h>
#include <QtTest>

#include <memory>

QTEST_KDEMAIN(TestClangUtils, GUI)

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

template<typename Visitor>
void runVisitor(const QByteArray& code, Visitor& visitor)
{
    runVisitor(code, &Visitor::visit, &visitor);
}

void runVisitor(const QByteArray& code, CXCursorVisitor visitor, CXClientData data)
{
    KTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    tempFile.write(code);
    tempFile.flush();

    std::unique_ptr<void, void(*)(CXIndex)> index(clang_createIndex(1, 1), clang_disposeIndex);
    const QVector<const char*> args = {"-std=c++11", "-xc++", "-Wall", "-nostdinc", "-nostdinc++"};
    CXTranslationUnit unit = clang_parseTranslationUnit(
        index.get(), qPrintable(tempFile.fileName()),
        args.data(), args.size(),
        nullptr, 0,
        CXTranslationUnit_None
    );

    const auto startCursor = clang_getTranslationUnitCursor(unit);
    QVERIFY(!clang_Cursor_isNull(startCursor));
    QVERIFY(clang_visitChildren(startCursor, visitor, data) == 0);
}

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
    qDebug() << "Found decl:" << ClangString(clang_getCursorSpelling(cursor))
        << "| range:" << ClangRange(clang_getCursorExtent(cursor)).toSimpleRange();
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

#include "test_clangutils.moc"
