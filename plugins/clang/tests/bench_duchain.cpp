/*
    SPDX-FileCopyrightText: 2016 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

QTEST_MAIN(BenchDUChain)

#include "moc_bench_duchain.cpp"
