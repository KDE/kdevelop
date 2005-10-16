/*
* KDevelop C++ Parse Job
*
* Copyright (c) 2005 Adam Treat <treat@kde.org>
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

#include <cassert>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <kdebug.h>

#include "parser/parser.h"
#include "parsejob.h"

ParseJob::ParseJob( const KURL &url, Parser *parser, QObject* parent )
        : ThreadWeaver::Job( parent ),
        m_document( url ),
        m_parser( parser )
{}

ParseJob::~ParseJob()
{}

void ParseJob::run()
{
    kdDebug() << "=====----- PARSING ----=======> "
    << m_document.fileName() << endl;

    const char* fileName = m_document.fileName().toLatin1();
    char *contents;
    std::size_t size;

    struct stat st;
    stat( fileName, &st );
    size = st.st_size + 1;

    int fd = open( fileName, O_RDONLY );
    assert( fd != -1 );

    contents = ( char * ) mmap( 0, size, PROT_READ, MAP_SHARED, fd, 0 );
    assert( contents != ( void* ) - 1 );
    close( fd );

    pool _p;
    m_parser->parse( contents, size, &_p );
    munmap(contents, size);
    m_parser = 0;
}

#include "parsejob.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
