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

#ifndef ASTFACTORY_H
#define ASTFACTORY_H

#include <QtCore/QMap>


class QString;
class CMakeAst;

class AstFactory
{
public:
    //We are a singleton. Fuckin' act like it
    static AstFactory* self();

    ///Alias for the AST creation callback
    typedef CMakeAst* ( *CreateAstCallback )();
public:
    ///Returns true if registration was successful
    bool registerAst( const QString& astId,
                      CreateAstCallback createFn );

    ///Returns true if the AstId was successfully found and unregistered
    bool unregisterAst( const QString& astId );

    ///Create the AST specified by @p astId
    ///Return either a valid AST object or zero.
    CMakeAst* createAst( const QString& astId );

private:
    typedef QMap<QString, CreateAstCallback> CallbackMap;
    AstFactory();
    class Private;
    Private* d;
    static AstFactory* s_self;
};

#endif
