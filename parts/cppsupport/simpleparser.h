/* $Id$
 *
 *  Copyright (C) 2002 Roberto Raggi (raggi@cli.di.unipi.it)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#ifndef simpleparser_h
#define simpleparser_h

#include <qstring.h>
#include <qvaluelist.h>
#include <qstringlist.h>

class SimpleVariable{
public:
    SimpleVariable()
        : scope( 0 ), name( QString::null ), type( QString::null ){}
    SimpleVariable( const SimpleVariable& source )
        : scope( source.scope ), name( source.name ), type( source.type ) {}
    ~SimpleVariable(){}

    SimpleVariable& operator = ( const SimpleVariable& source ){
        scope = source.scope;
        name = source.name;
        type = source.type;
        return *this;
    }

    int scope;
    QString name;
    QString type;
};

class SimpleParser{
public:
    static SimpleVariable findVariable( const QValueList<SimpleVariable>&, const QString& );
    static QValueList<SimpleVariable> localVariables( QString contents );
    static QValueList<SimpleVariable> parseFile( const QString& filename );
};

#endif
