/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "cmakeastfactorytest.h"
#include <QtCore/QString>

#include "astfactory.h"
#include "cmakeast.h"


QTEST_MAIN( CMakeAstFactoryTest )

class FooAst : public CMakeAst
{
public:
    static CMakeAst* createFooAst() {
        return new FooAst;
    }
    virtual bool parseFunctionInfo(const CMakeFunctionDesc& ) { return false; }
};


CMakeAstFactoryTest::CMakeAstFactoryTest()
{
}

CMakeAstFactoryTest::~CMakeAstFactoryTest()
{
}


void CMakeAstFactoryTest::testNonRegisteredObject()
{
    CMakeAst* a = AstFactory::self()->createAst( "foo" );
    QVERIFY(a); //It's a MacroCallAst
    delete a;
}

void CMakeAstFactoryTest::testRegisteredObject()
{
    const QString fooType = "foo";
    bool registered = AstFactory::self()->registerAst( fooType,
                                                         &FooAst::createFooAst );
    AstFactory::self()->unregisterAst( fooType );
    QVERIFY( registered );
}

void CMakeAstFactoryTest::testCaseSensitivity()
{
    const QString fooType = "Foo";
    bool registered = AstFactory::self()->registerAst( "Foo", &FooAst::createFooAst );
    bool notRegistered = AstFactory::self()->registerAst( "FOO", &FooAst::createFooAst );
    AstFactory::self()->unregisterAst( fooType );
    QVERIFY( registered == true );
    QVERIFY( notRegistered == false );
}

void CMakeAstFactoryTest::testUnregisterObject()
{
    bool registered = AstFactory::self()->registerAst( "Foo", &FooAst::createFooAst );
    bool unregistered = AstFactory::self()->unregisterAst( "Foo" );

    QVERIFY( registered );
    QVERIFY( unregistered );
}

void CMakeAstFactoryTest::testCreateObject()
{
    bool registered = AstFactory::self()->registerAst( "Foo",  &FooAst::createFooAst );
    QVERIFY( registered );

    CMakeAst* ast = AstFactory::self()->createAst( "foo" );
    QVERIFY( ast != 0 );
    delete ast;
}
