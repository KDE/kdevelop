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

#include "tag.h"
#include <QDataStream>

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
    : data(source.data)
{
}

Tag::~Tag()
{
}

Tag& Tag::operator = ( const Tag& source )
{
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
        << data.constData()->id
	<< data.constData()->kind
	<< data.constData()->flags
	<< data.constData()->name
	<< data.constData()->scope
	<< data.constData()->fileName
	<< data.constData()->startLine
	<< data.constData()->startColumn
	<< data.constData()->endLine
	<< data.constData()->endColumn
	<< data.constData()->attributes;
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

