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

#include "astfactory.h"
#include <map>
#include <QtCore/QString>
#include <kstaticdeleter.h>
#include "cmakeast.h"

class AstFactory::Private
{
public:
    static KStaticDeleter<AstFactory> deleter;

    CallbackMap callbacks;
};

KStaticDeleter<AstFactory> AstFactory::Private::deleter;
AstFactory* AstFactory::s_self = 0;

AstFactory* AstFactory::self()
{
    if ( !s_self )
        Private::deleter.setObject( s_self, new AstFactory() );

    return s_self;
}


AstFactory::AstFactory()
{
    d = new Private;
}

bool AstFactory::registerAst( const QString& astId,
                              CreateAstCallback createFn )
{
    if ( d->callbacks.contains( astId.toLower() ) )
         return false;

    d->callbacks.insert( astId.toLower(), createFn );
    return true;
}


bool AstFactory::unregisterAst( const QString& astId )
{
    return d->callbacks.remove( astId.toLower() ) == 1;
}


CMakeAst* AstFactory::createAst( const QString& astId )
{
    CallbackMap::const_iterator it = d->callbacks.find( astId.toLower() );
    if ( it == d->callbacks.end() )
        return 0;

    return ( it.value() )();

}
