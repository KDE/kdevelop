/*
 * Copyright 2016 Milian Wolff <mail@milianw.de>
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

#include "bench_duchain.h"

#include <QLoggingCategory>
#include <QTest>

#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <tests/testfile.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>

using namespace KDevelop;

BenchDUChain::BenchDUChain()
{
}

BenchDUChain::~BenchDUChain()
{
}

void BenchDUChain::initTestCase()
{
    QLoggingCategory::setFilterRules(QStringLiteral("*.debug=false\ndefault.debug=true\nkdevelop.plugins.clang.debug=true\n"));
    QVERIFY(qputenv("KDEV_CLANG_DISPLAY_DIAGS", "1"));

    AutoTestShell::init({QStringLiteral("kdevclangsupport")});
    TestCore::initialize(Core::NoUi);
}

void BenchDUChain::cleanupTestCase()
{
    TestCore::shutdown();
}

void BenchDUChain::benchDUChainBuilder()
{
    QBENCHMARK_ONCE {
        TestFile file(
            "#include <vector>\n"
            "#include <map>\n"
            "#include <set>\n"
            "#include <algorithm>\n"
            "#include <functional>\n"
            "#include <limits>\n"
            "#include <bitset>\n"
            "#include <iostream>\n"
            "#include <string>\n"
            "#include <mutex>\n", QStringLiteral("cpp"));
        file.parse(TopDUContext::AllDeclarationsContextsAndUses);
        QVERIFY(file.waitForParsed(60000));

        DUChainReadLocker lock;
        auto top = file.topContext();
        QVERIFY(top);
    }
}

QTEST_MAIN(BenchDUChain);
