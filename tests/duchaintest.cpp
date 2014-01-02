/*
 * Copyright 2014  Milian Wolff <mail@milianw.de>
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

#include "duchaintest.h"

#include <qtest_kde.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/testfile.h>
#include <language/duchain/duchainlock.h>

QTEST_KDEMAIN(DUChainTest, NoGUI);

using namespace KDevelop;

void DUChainTest::initTestCase()
{
    QVERIFY(qputenv("KDEV_DISABLE_PLUGINS", "kdevcppsupport"));
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
}

void DUChainTest::cleanupTestCase()
{
    TestCore::shutdown();
}

void DUChainTest::testInclude()
{
    TestFile header("int foo() { return 42; }\n", "h");
    header.parse(TopDUContext::AllDeclarationsContextsAndUses);

    TestFile impl("#include \"" + header.url().byteArray() + "\"\n"
                  "int main() { return foo(); }", "cpp", &header);
    impl.parse(TopDUContext::AllDeclarationsContextsAndUses);

    auto headerCtx = header.topContext();
    QVERIFY(headerCtx);

    auto implCtx = impl.topContext();
    QVERIFY(implCtx);

    DUChainReadLocker lock;

    QCOMPARE(headerCtx->localDeclarations().size(), 1);
    QCOMPARE(implCtx->localDeclarations().size(), 1);
}

void DUChainTest::testReparse()
{
    TestFile file("int main() { return 42; }", "cpp");
    file.parse(TopDUContext::AllDeclarationsContextsAndUses);

    for (int i = 0; i < 2; ++i) {
        QVERIFY(file.waitForParsed(500));
        DUChainReadLocker lock;
        QCOMPARE(file.topContext()->localDeclarations().size(), 1);
        file.parse(TopDUContext::Features(TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::ForceUpdateRecursive));
    }
}

#include "duchaintest.moc"
