/*
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "test_highlighting.h"

#include <QTest>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <language/duchain/duchain.h>
#include <language/codegen/coderepresentation.h>
#include <language/highlighting/codehighlighting.h>

QTEST_MAIN(TestHighlighting)

using namespace KDevelop;

void TestHighlighting::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);

    DUChain::self()->disablePersistentStorage();
    CodeRepresentation::setDiskChangesForbidden(true);
}

void TestHighlighting::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestHighlighting::testInitialization()
{
    CodeHighlighting highlighting(this);
    QVERIFY(highlighting.attributeForDepth(0));
}
