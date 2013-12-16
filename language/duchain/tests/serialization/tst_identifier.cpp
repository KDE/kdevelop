/*
 * This file is part of KDevelop
 * Copyright 2012-2013 Milian Wolff <mail@milianw.de>
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

#include "tst_identifier.h"

#include <language/duchain/duchainlock.h>
#include <language/duchain/identifier.h>

#include <qtest_kde.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>

QTEST_KDEMAIN(TestIdentifier, NoGUI);

using namespace KDevelop;

void TestIdentifier::initTestCase()
{
  AutoTestShell::init();
  TestCore::initialize(Core::NoUi);
}

void TestIdentifier::cleanupTestCase()
{
  TestCore::shutdown();
}

void TestIdentifier::benchIdentifierCopyConstant()
{
  QBENCHMARK {
    Identifier identifier("Asdf");
    identifier.index();
    Identifier copy(identifier);
  }
}

void TestIdentifier::benchIdentifierCopyDynamic()
{
  QBENCHMARK {
    Identifier identifier("Asdf");
    Identifier copy(identifier);
  }
}

void TestIdentifier::benchQidCopyPush()
{
  QBENCHMARK {
    Identifier id("foo");
    QualifiedIdentifier base(id);
    QualifiedIdentifier copy(base);
    copy.push(id);
  }
}
