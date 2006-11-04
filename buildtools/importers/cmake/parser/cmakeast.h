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

#ifndef CMAKEAST_H
#define CMAKEAST_H

#include <QtCore/QList>

class CMakeFunctionDesc;

class CMakeAst
{
public:
    CMakeAst() { }
    CMakeAst( const CMakeAst& ast ) : m_children( ast.m_children ) {}
    virtual ~CMakeAst() { qDeleteAll( m_children ); }

    /**
     * Adds a child Ast Node to this node. This will only have uses in cases
     * where custom macros are used since CMakeLists.txt files generally have
     * a pretty flat tree structure
     */
    virtual void addChildAst( CMakeAst* node ) { m_children.append( node ); }

    /**
     * Indicates if the Ast has children
     * @return true if the Ast has children
     * @return false if the Ast has no children
     */
    bool hasChildren() const { return m_children.isEmpty(); }

    /**
     * Get the children of this ast
     * @return the list of this ast's children
     */
    QList<CMakeAst*> children() const  { return m_children; }

    /**
     * Writes the information stored in the Ast into the @p buffer.
     * All Asts that are a child of this Ast are written back as well.
     */
    virtual void writeBack(QString& buffer);

    virtual bool parseFunctionInfo( const CMakeFunctionDesc& function ) { return false; }

protected:
    QList<CMakeAst*> m_children;

};

#define CMAKE_REGISTER_AST( klassName, astId ) namespace {                 \
        CMakeAst* Create##klassName() { return new klassName; }            \
        bool astId = AstFactory::self()->registerAst( QLatin1String( #astId ), Create##klassName ); }

#define CMAKE_BEGIN_AST_CLASS( klassName ) class klassName : public CMakeAst {  \
    public:                                                  \
        klassName() {}                                       \
        ~klassName() {}                                      \
                                                             \
        virtual void writeBack( const QString& buffer );     \
        virtual bool parseFunctionInfo( const CMakeFunctionDesc& );

#define CMAKE_ADD_AST_MEMBER( returnType, setterType, name ) \
    public:                                              \
        returnType name() const;                         \
        void set##name( setterType );                    \
    private:                                             \
        returnType m_##name;

#define CMAKE_ADD_AST_FUNCTION( function ) \
    public:                                \
       function;

#define CMAKE_END_AST_CLASS( klassName ) };



#endif
