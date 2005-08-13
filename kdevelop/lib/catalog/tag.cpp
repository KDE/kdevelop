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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "tag.h"
#include <qdatastream.h>

Tag::Tag()
{
    data = new TagData();
    data->kind = 0;
    data->flags = 0;
    data->startLine = 0;
    data->startColumn = 0;
    data->endLine = 0;
    data->endColumn = 0;
}

Tag::Tag( const Tag& source )
{
    data = source.data;
    data->ref();
}

Tag::~Tag()
{
    if( data->deref() ){
	delete( data );
	data = 0;
    }
}

void Tag::detach()
{
    if( data->count != 1 )
	*this = copy();
}

Tag Tag::copy()
{
    Tag t;
    
    t.data->id = data->id;
    t.data->kind = data->kind;
    t.data->flags = data->flags;
    t.data->name = data->name;
    t.data->scope = data->scope;
    t.data->fileName = data->fileName;
    t.data->startLine = data->startLine;
    t.data->startColumn = data->startColumn;
    t.data->endLine = data->endLine;
    t.data->endColumn = data->endColumn;
    t.data->attributes = data->attributes;
    
    return t;
}

Tag& Tag::operator = ( const Tag& source )
{
    source.data->ref();
    if ( data->deref() ){
	delete data;
    }
    data = source.data;
    
    return( *this );
}

void Tag::load( QDataStream& stream )
{
    stream
        >> data->id
	>> data->kind
	>> data->flags
	>> data->name
	>> data->scope
	>> data->fileName
	>> data->startLine
	>> data->startColumn
	>> data->endLine
	>> data->endColumn
	>> data->attributes;
}

void Tag::store( QDataStream& stream ) const
{
    stream
        << data->id
	<< data->kind
	<< data->flags
	<< data->name
	<< data->scope
	<< data->fileName
	<< data->startLine
	<< data->startColumn
	<< data->endLine
	<< data->endColumn
	<< data->attributes;
}

QDataStream& operator << ( QDataStream& s, const Tag& t)
{
  t.store( s );
  return s;
}

QDataStream& operator >> ( QDataStream& s, Tag& t )
{
  t.load( s );
  return s;
}

