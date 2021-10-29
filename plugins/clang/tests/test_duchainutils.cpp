/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "test_duchainutils.h"

#include <language/duchain/duchainlock.h>
#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testfile.h>
#include <tests/testhelpers.h>

#include "../duchain/duchainutils.h"

#include <QLoggingCategory>
#include <QTest>

using namespace KDevelop;

QTEST_GUILESS_MAIN(TestDUChainUtils)

void TestDUChainUtils::initTestCase()
{
    QLoggingCategory::setFilterRules(QStringLiteral("*.debug=false\ndefault.debug=true\nkdevelop.plugins.clang.debug=true\n"));
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
}

void TestDUChainUtils::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestDUChainUtils::getFunctionSignatureRange()
{
    QFETCH(QString, code);
    QFETCH(KTextEditor::Range, expectedRange);
    {
        TestFile file(code, QStringLiteral("cpp"));
        file.parse();
        QVERIFY(file.waitForParsed());

        DUChainReadLocker lock;
        QVERIFY(file.topContext());

        const auto functionDecl = file.topContext()->localDeclarations()[0];
        const auto range = ClangIntegration::DUChainUtils::functionSignatureRange(functionDecl);
        QCOMPARE(range, expectedRange);
    }
}

void TestDUChainUtils::getFunctionSignatureRange_data()
{
    QTest::addColumn<QString>("code");
    QTest::addColumn<KTextEditor::Range>("expectedRange");

    QTest::newRow("function-declaration")
        << "void func(\nint a, int b\n);\n"
        << KTextEditor::Range(0, 0, 2, 1);
    QTest::newRow("function-definition")
        << "void func(\nint a, int b\n) {}\n"
        << KTextEditor::Range(0, 0, 2, 2);

}
