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

#include "tag.h"
#include <qdatastream.h>

Tag::Tag()
    : m_kind( 0 ),
      m_startLine( 0 ), m_startColumn( 0 ),
      m_endLine( 0 ), m_endColumn( 0 )
{
}

Tag::Tag( const Tag& source )
    : m_kind( source.m_kind ),
    m_name( source.m_name ),
    m_scope( source.m_scope ),
    m_fileName( source.m_fileName ),
    m_startLine( source.m_startLine ), m_startColumn( source.m_startColumn ),
    m_endLine( source.m_endLine ), m_endColumn( source.m_endColumn ),   
    m_attributes( source.m_attributes )
{
}

Tag::~Tag()
{
}

Tag& Tag::operator = ( const Tag& source )
{
    m_kind = source.m_kind;
    m_name = source.m_name;
    m_scope = source.m_scope;
    m_fileName = source.m_fileName;
    m_startLine = source.m_startLine;
    m_startColumn = source.m_startColumn;
    m_endLine = source.m_endLine;
    m_endColumn = source.m_endColumn;
    m_attributes = source.m_attributes;
    return( *this );
}

void Tag::load( QDataStream& stream )
{
    stream 
	>> m_kind
	>> m_name
	>> m_scope
	>> m_fileName
	>> m_startLine
	>> m_startColumn
	>> m_endLine
	>> m_endColumn
	>> m_attributes;
}

void Tag::store( QDataStream& stream ) const
{
    stream 
	<< m_kind
	<< m_name
	<< m_scope
	<< m_fileName
	<< m_startLine
	<< m_startColumn
	<< m_endLine
	<< m_endColumn
	<< m_attributes;
}
