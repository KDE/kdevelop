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
#include <kglobal.h>
#include "cmakeast.h"

class AstFactory::Private
{
public:
    CallbackMap callbacks;
};

AstFactory* AstFactory::self()
{
    K_GLOBAL_STATIC(AstFactory, s_self);
    return s_self;
}


AstFactory::AstFactory()
{
    d = new Private;
}

AstFactory::~AstFactory()
{
    delete d;
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
    CallbackMap::const_iterator it = d->callbacks.constFind( astId.toLower() );
    if ( it == d->callbacks.constEnd() )
        return new MacroCallAst;

    return ( it.value() )();

}

QStringList AstFactory::commands() const
{
    return d->callbacks.uniqueKeys();
}

bool AstFactory::contains(const QString& name) const
{
    return d->callbacks.contains(name);
}
