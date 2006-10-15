/*
 * This file is part of KDevelop
 *
 * Copyright (C) 2006 Hamish Rodda <rodda@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "parsesession.h"

#include "kdev-pg-memory-pool.h"
#include "kdev-pg-token-stream.h"


namespace ruby
{

ParseSession::ParseSession()
        : memory_pool( new parser::memory_pool_type )
        , token_stream( new parser::token_stream_type )
{
}

ParseSession::~ParseSession()
{
    delete memory_pool;
    delete token_stream;
}

void ParseSession::positionAt( std::size_t offset, int *line, int *column ) const
{
    token_stream->location_table()->position_at( offset, line, column );
}

std::size_t ParseSession::size() const
{
    return m_contents.size();
}

const char *ParseSession::contents() const
{
    return m_contents.constData();
}

void ParseSession::setContents( const QByteArray & contents )
{
    m_contents = contents;
}

} // end of namespace ruby
