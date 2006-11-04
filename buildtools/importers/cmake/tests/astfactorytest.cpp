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

#include "astfactorytest.h"
#include <kstaticdeleter.h>
#include <QtCore/QString>
#include <map>

class Ast;

class AstFactory
{
public:
    static AstFactory* self();
    typedef Ast* ( *CreateAstCallback )();
public:
    //Returns true if registration was successful
    bool registerAst( const QString& astId,
                      CreateAstCallback createFn )
    {
        return m_callbacks.insert( CallbackMap::value_type( astId,
                                                            createFn ) ).second;
    }

    //Returns true if the AstId was successful
    bool unregisterAst( const QString& astId )
    {
        return m_callbacks.erase( astId ) == 1;
    }

    Ast* createAst( const QString& astId )
    {
        CallbackMap::const_iterator it = m_callbacks.find( astId );
        if ( it == m_callbacks.end() )
            return 0;

        return ( it->second )();
    }

private:
    static AstFactory* s_self;
    static KStaticDeleter<AstFactory> astDeleter;
    AstFactory() {}
    typedef std::map<QString,  CreateAstCallback> CallbackMap;
    CallbackMap m_callbacks;

};

KStaticDeleter<AstFactory> AstFactory::astDeleter;
AstFactory* AstFactory::s_self = 0;

AstFactory* AstFactory::self()
{
    if ( !s_self )
       astDeleter.setObject( s_self, new AstFactory() );

    return s_self;
}

class Ast { int foo; };

QTEST_MAIN( AstFactoryTest );


AstFactoryTest::AstFactoryTest()
{
}

AstFactoryTest::~AstFactoryTest()
{
}


void AstFactoryTest::testNoRegisteredObjects()
{
    Ast* a = AstFactory::self()->createAst( "foo" );
    QVERIFY( a == 0 );
}

void AstFactoryTest::testRegisteredObjects()
{
    const QString fooType = "foo";

    class FooAst : public Ast
    {
    public:
        static Ast* createFooAst() {
            return new FooAst;
        }

    };

    bool registered = AstFactory::self()->registerAst( fooType,
                                                         &FooAst::createFooAst );
    Ast* foo = AstFactory::self()->createAst( QLatin1String( "foo" ) );

    QVERIFY( foo != 0 );
}

#include "astfactorytest.moc"
