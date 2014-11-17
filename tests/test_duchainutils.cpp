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

#include "test_duchainutils.h"

#include <language/duchain/duchainlock.h>
#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testfile.h>
#include <tests/testhelpers.h>

#include "../duchain/duchainutils.h"

#include <QtTest>

using namespace KDevelop;

QTEST_GUILESS_MAIN(TestDUChainUtils);

Q_DECLARE_METATYPE(KTextEditor::Range);

void TestDUChainUtils::initTestCase()
{
    QVERIFY(qputenv("KDEV_DISABLE_PLUGINS", "kdevcppsupport"));
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
        TestFile file(code, "cpp");
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
