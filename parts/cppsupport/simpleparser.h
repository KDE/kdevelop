/* $Id$
 *
 *  Copyright (C) 2002 Roberto Raggi (roberto@kdevelop.org)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#ifndef simpleparser_h
#define simpleparser_h

#include <qstring.h>
#include <qvaluelist.h>

class SimpleVariable{
public:
    SimpleVariable()
        : name( QString::null ), type( QString::null ){}
    SimpleVariable( const SimpleVariable& source )
        : name( source.name ), type( source.type ) {}
    ~SimpleVariable(){}

    SimpleVariable& operator = ( const SimpleVariable& source ){
        name = source.name;
        type = source.type;
        return *this;
    }

    QString name;
    QString type;
};

class SimpleContext{
public:
    SimpleContext( SimpleContext* prev=0 )
        : m_prev( prev ) {}

    virtual ~SimpleContext()
        {
            if( m_prev ){
                delete( m_prev );
                m_prev = 0;
            }
        }

    SimpleContext* prev() const
        { return m_prev; }

    void attach( SimpleContext* ctx )
        { m_prev = ctx; }

    void detach()
        { m_prev = 0; }

    const QValueList<SimpleVariable>& vars() const
        { return m_vars; }

    void add( const SimpleVariable& v )
        { m_vars.append( v ); }

    void add( const QValueList<SimpleVariable>& vars )
        { m_vars += vars; }

    SimpleVariable findVariable( const QString& varname )
        {
            SimpleContext* ctx = this;
            while( ctx ){
                const QValueList<SimpleVariable>& vars = ctx->vars();
                for( int i=vars.count() - 1; i>=0; --i ){
                    SimpleVariable v = vars[ i ];
                    if( v.name == varname )
                        return v;
                }
                ctx = ctx->prev();
            }
            return SimpleVariable();
        }

private:
    QValueList<SimpleVariable> m_vars;
    SimpleContext* m_prev;
};

class SimpleParser{
public:
    static SimpleContext* localVariables( QString contents );
};

#endif
