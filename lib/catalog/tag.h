/* This file is part of KDevelop
    Copyright (C) 2003 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef TAG_H
#define TAG_H

#include <qmap.h>
#include <qvariant.h>

class QDataStream;

class Tag
{
public:
    enum Kind
    {
	Kind_Unknown,

	Kind_Typedef = 1000,
        Kind_Namespace,
        Kind_UsingDirective,
	Kind_Base_class,
	Kind_Enum,
	Kind_Enumerator,
	Kind_Class,
	Kind_Struct,
	Kind_Union,
	Kind_VariableDeclaration,
	Kind_Variable,
	Kind_FunctionDeclaration,
	Kind_Function,
	// ...

	Kind_Custom = 2000
    };

public:
    Tag();
    Tag( const Tag& source );
    ~Tag();

    Tag& operator = ( const Tag& source );

    int kind() const
    {
        return attribute( "kind" ).toInt();
    }

    void setKind( int kind )
    {
        setAttribute( "kind", kind );
    }

    QString fileName() const
    {
        return attribute( "fileName" ).toString();
    }

    void setFileName( const QString& fileName )
    {
        setAttribute( "fileName", fileName );
    }

    QString path( const QString& sep = QString::fromLatin1("::") ) const
    {
        QString s = scope().join( sep );
        if( s.isNull() )
            return name();
	return s + sep + name();
    }

    QString name() const
    {
        return attribute( "name" ).toString();
    }

    void setName( const QString& name )
    {
        setAttribute( "name", name );
    }

    QStringList scope() const
    {
        return attribute( "scope" ).toStringList();
    }

    void setScope( const QStringList& scope )
    {
        setAttribute( "scope", scope );
    }

    void getStartPosition( int* line, int* column ) const
    {
	if( line ) *line = attribute( "startLine" ).toInt();
	if( column ) *column = attribute( "startColumn" ).toInt();
    }

    void setStartPosition( int line, int column )
    {
	setAttribute( "startLine", line );
	setAttribute( "startColumn", column );
    }

    void getEndPosition( int* line, int* column ) const
    {
	if( line ) *line = attribute( "endLine" ).toInt();
	if( column ) *column = attribute( "endColumn" ).toInt();
    }

    void setEndPosition( int line, int column )
    {
	setAttribute( "endLine", line );
	setAttribute( "endColumn", column );
    }

    bool hasAttribute( const QString& name ) const
    {
        return m_attributes.contains( name );
    }

    QVariant attribute( const QString& name ) const
    {
        return m_attributes[ name ];
    }

    void setAttribute( const QString& name, const QVariant& value )
    {
        m_attributes[ name ] = value;
    }

    void load( QDataStream& stream );
    void store( QDataStream& stream ) const;

private:
    QMap<QString, QVariant> m_attributes;
};

#endif
