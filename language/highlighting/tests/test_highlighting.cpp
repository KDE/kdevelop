/*
 * This file is part of KDevelop
 *
 * Copyright 2010 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "test_highlighting.h"

#include <QTest>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <language/duchain/duchain.h>
#include <language/codegen/coderepresentation.h>
#include <language/highlighting/codehighlighting.h>
#include <KColorUtils>

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


#include "test_highlighting.moc"
