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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef TAG_H
#define TAG_H

#include <QtCore/QMap>
#include <QtCore/QVariant>
#include <QtCore/QByteArray>
#include <QtCore/QSharedData>
#include <QtCore/QSharedDataPointer>
#include <QtCore/QStringList>

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
        return data.constData()->id;
    }

    void setId( const QByteArray& id )
    {
        data->id = id;
    }

    int kind() const
    {
        return data.constData()->kind;
    }

    void setKind( int kind )
    {
        data->kind = kind;
    }

    quint64 flags() const
    {
        return data.constData()->flags;
    }

    void setFlags( quint64 flags )
    {
        data->flags = flags;
    }

    QString fileName() const
    {
        return data.constData()->fileName;
    }

    void setFileName( const QString& fileName )
    {
        data->fileName = fileName;
    }

    QString path( const QString& sep = QLatin1String("::") ) const
    {
        QString s = scope().join( sep );
        if( s.isNull() )
            return name();
	return s + sep + name();
    }

    QString name() const
    {
        return data.constData()->name;
    }

    void setName( const QString& name )
    {
        data->name = name;
    }
    
    QStringList scope() const
    {
        return data.constData()->scope;
    }

    void setScope( const QStringList& scope )
    {
        data->scope = scope;
    }

    void getStartPosition( int* line, int* column ) const
    {
	if( line ) *line = data.constData()->startLine;
	if( column ) *column = data.constData()->startColumn;
    }

    void setStartPosition( int line, int column )
    {
	data->startLine = line;
	data->startColumn = column;
    }

    void getEndPosition( int* line, int* column ) const
    {
	if( line ) *line = data.constData()->endLine;
	if( column ) *column = data.constData()->endColumn;
    }

    void setEndPosition( int line, int column )
    {
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
        return data.constData()->attributes.contains( name );
    }

    QVariant attribute( const QByteArray& name ) const
    {
	if( name == "id" )
	    return data.constData()->id;
	else if( name == "kind" )
	    return data.constData()->kind;
	else if( name == "name" )
	    return data.constData()->name;
	else if( name == "scope" )
	    return data.constData()->scope;
	else if( name == "fileName" )
	    return data.constData()->fileName;
	else if( name == "startLine" )
	    return data.constData()->startLine;
	else if( name == "startColumn" )
	    return data.constData()->startColumn;
	else if( name == "endLine" )
	    return data.constData()->endLine;
	else if( name == "endColumn" )
	    return data.constData()->endColumn;
	else if( name == "prefix" )
	    return data.constData()->name.left( 2 );
        return data.constData()->attributes[ name ];
    }

    void setAttribute( const QByteArray& name, const QVariant& value )
    {
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
    struct TagData: public QSharedData
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
    };
    
    QSharedDataPointer<TagData> data;
};

QDataStream& operator << ( QDataStream&, const Tag& );
QDataStream& operator >> ( QDataStream&, Tag& );

#endif
