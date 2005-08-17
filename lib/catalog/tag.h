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
#include <q3shared.h>
//Added by qt3to4:
#include <QByteArray>
#include <qstringlist.h>

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
    
    QByteArray id() const
    {
        return data->id;
    }

    void setId( const QByteArray& id )
    {
	detach();
        data->id = id;
    }

    int kind() const
    {
        return data->kind;
    }

    void setKind( int kind )
    {
	detach();
        data->kind = kind;
    }

    quint64 flags() const
    {
        return data->flags;
    }

    void setFlags( quint64 flags )
    {
	detach();
        data->flags = flags;
    }

    QString fileName() const
    {
        return data->fileName;
    }

    void setFileName( const QString& fileName )
    {
	detach();
        data->fileName = fileName;
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
        return data->name;
    }

    void setName( const QString& name )
    {
	detach();
        data->name = name;
    }
    
    QStringList scope() const
    {
        return data->scope;
    }

    void setScope( const QStringList& scope )
    {
	detach();
        data->scope = scope;
    }

    void getStartPosition( int* line, int* column ) const
    {
	if( line ) *line = data->startLine;
	if( column ) *column = data->startColumn;
    }

    void setStartPosition( int line, int column )
    {
	detach();
	data->startLine = line;
	data->startColumn = column;
    }

    void getEndPosition( int* line, int* column ) const
    {
	if( line ) *line = data->endLine;
	if( column ) *column = data->endColumn;
    }

    void setEndPosition( int line, int column )
    {
	detach();
	data->endLine = line;
	data->endColumn = column;
    }

    bool hasAttribute( const QByteArray& name ) const
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
        return data->attributes.contains( name );
    }

    QVariant attribute( const QByteArray& name ) const
    {
	if( name == "id" )
	    return data->id;
	else if( name == "kind" )
	    return data->kind;
	else if( name == "name" )
	    return data->name;
	else if( name == "scope" )
	    return data->scope;
	else if( name == "fileName" )
	    return data->fileName;
	else if( name == "startLine" )
	    return data->startLine;
	else if( name == "startColumn" )
	    return data->startColumn;
	else if( name == "endLine" )
	    return data->endLine;
	else if( name == "endColumn" )
	    return data->endColumn;
	else if( name == "prefix" )
	    return data->name.left( 2 );
        return data->attributes[ name ];
    }

    void setAttribute( const QByteArray& name, const QVariant& value )
    {
	detach();
	if( name == "id" )
	    data->id = value.toCString();
	else if( name == "kind" )
	    data->kind = value.toInt();
	else if( name == "name" )
	    data->name = value.toString();
	else if( name == "scope" )
	    data->scope = value.toStringList();
	else if( name == "fileName" )
	    data->fileName = value.toString();
	else if( name == "startLine" )
	    data->startLine = value.toInt();
	else if( name == "startColumn" )
	    data->startColumn = value.toInt();
	else if( name == "endLine" )
	    data->endLine = value.toInt();
	else if( name == "endColumn" )
	    data->endColumn = value.toInt();
	else
	    data->attributes[ name ] = value;
    }
    
    void load( QDataStream& stream );
    void store( QDataStream& stream ) const;

private:
    Tag copy();
    void detach();
    
private:
    struct TagData: public Q3Shared
    {
	QByteArray id;
	int kind;
	quint64 flags;
	QString name;
	QStringList scope;
	QString fileName;
	int startLine, startColumn;
	int endLine, endColumn;
	QMap<QByteArray, QVariant> attributes;
    } *data;
};

QDataStream& operator << ( QDataStream&, const Tag& );
QDataStream& operator >> ( QDataStream&, Tag& );

#endif
