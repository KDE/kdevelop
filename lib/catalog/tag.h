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
        return m_kind;
    }

    void setKind( int kind )
    {
        m_kind = kind;
    }

    QString fileName() const
    {
        return m_fileName;
    }

    void setFileName( const QString& fileName )
    {
        m_fileName = fileName;
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
        return m_name;
    }

    void setName( const QString& name )
    {
        m_name = name;
    }

    QStringList scope() const
    {
        return m_scope;
    }

    void setScope( const QStringList& scope )
    {
        m_scope = scope;
    }

    void getStartPosition( int* line, int* column ) const
    {
	if( line ) *line = m_startLine;
	if( column ) *column = m_startColumn;
    }

    void setStartPosition( int line, int column )
    {
	m_startLine = line;
	m_startColumn = column;
    }

    void getEndPosition( int* line, int* column ) const
    {
	if( line ) *line = m_endLine;
	if( column ) *column = m_endColumn;
    }

    void setEndPosition( int line, int column )
    {
	m_endLine = line;
	m_endColumn = column;
    }

    bool hasAttribute( const QCString& name ) const
    {
	if( name == "kind" ||
	    name == "name" ||
	    name == "scope" ||
	    name == "fileName" ||
	    name == "startLine" ||
	    name == "startColumn" ||
	    name == "endLine" ||
	    name == "endColumn" )
	    return true;
        return m_attributes.contains( name );
    }

    QVariant attribute( const QCString& name ) const
    {
	if( name == "kind" )
	    return m_kind;
	else if( name == "name" )
	    return m_name;
	else if( name == "scope" )
	    return m_scope;
	else if( name == "fileName" )
	    return m_fileName;
	else if( name == "startLine" )
	    return m_startLine;
	else if( name == "startColumn" )
	    return m_startColumn;
	else if( name == "endLine" )
	    return m_endLine;
	else if( name == "endColumn" )	
	    return m_endColumn;
        return m_attributes[ name ];
    }

    void setAttribute( const QCString& name, const QVariant& value )
    {
	if( name == "kind" )
	    m_kind = value.toInt();
	else if( name == "name" )
	    m_name = value.toString();
	else if( name == "scope" )
	    m_scope = value.toStringList();
	else if( name == "fileName" )
	    m_fileName = value.toString();
	else if( name == "startLine" )
	    m_startLine = value.toInt();
	else if( name == "startColumn" )
	    m_startColumn = value.toInt();
	else if( name == "endLine" )
	    m_endLine = value.toInt();
	else if( name == "endColumn" )	
	    m_endColumn = value.toInt();
	else
	    m_attributes[ name ] = value;
    }

    void load( QDataStream& stream );
    void store( QDataStream& stream ) const;

private:
    int m_kind;
    QString m_name;
    QStringList m_scope;
    QString m_fileName;
    int m_startLine, m_startColumn;
    int m_endLine, m_endColumn;
    QMap<QCString, QVariant> m_attributes;
};

#endif
