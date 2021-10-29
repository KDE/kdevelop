/*
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "testsuite.h"

#include <language/duchain/declaration.h>

using namespace KDevelop;

TestSuite<Declaration*>& KDevelop::declarationTestSuite()
{
    static TestSuite<Declaration*> suite;
    return suite;
}

TestSuite<DUContext*>& KDevelop::contextTestSuite()
{
    static TestSuite<DUContext*> suite;
    return suite;
}

TestSuite<AbstractType::Ptr>& KDevelop::typeTestSuite()
{
    static TestSuite<AbstractType::Ptr> suite;
    return suite;
}
