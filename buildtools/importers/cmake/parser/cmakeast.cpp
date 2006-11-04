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

#include "cmakeast.h"

#include <QtCore/QString>
#include "astfactory.h"

#define CMAKE_REGISTER_AST( klassName, astId ) namespace {                 \
        CMakeAst* Create##klassName() { return new klassName; }            \
        bool registered = AstFactory::self()->registerAst( QLatin1String( #astId ), Create##klassName ); }

#define CMAKE_BEGIN_AST_CLASS( klassName ) class klassName : public CMakeAst {  \
    public:                                                  \
        klassName();                                         \
        ~klassName();                                        \
                                                             \
        virtual void writeBack( const QString& buffer );

#define CMAKE_ADD_AST_MEMBER( returnType, setterType, name ) \
    public:                                              \
        returnType name() const;                         \
        void set##name( setterType );                    \
    private:                                             \
        returnType m_##name;

#define CMAKE_ADD_AST_FUNCTION( function ) \
    public:                                \
       function;                           \

#define CMAKE_END_AST_CLASS( klassName ) };


CMAKE_BEGIN_AST_CLASS( SetAst )
CMAKE_END_AST_CLASS( SetAst )
CMAKE_REGISTER_AST( SetAst, set )


void CMakeAst::writeBack(QString& buffer)
{

}

SetAst::SetAst()
{
}

SetAst::~SetAst()
{
}

void SetAst::writeBack( const QString& buffer )
{
}
