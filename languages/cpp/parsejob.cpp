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

#include <kdevdocumentcontroller.h>

#include "parser/parser.h"
#include "parser/dumptree.h"
#include "parsejob.h"

ParseJob::ParseJob( const KURL &url, Parser *parser, QObject* parent )
        : ThreadWeaver::Job( parent ),
        m_document( url ),
        m_parser( parser )
{
    m_size = -1;
}

ParseJob::~ParseJob()
{}

KURL ParseJob::document() const
{
    return m_document;
}

TranslationUnitAST *ParseJob::translationUnit() const
{
    return m_translationUnit;
}

void ParseJob::run()
{
    bool readFromDisk = ( m_size == -1 );
    char *contents;
    std::size_t size;

    if ( !readFromDisk )
    {
        contents = qstrdup( m_contents );
        size = m_size + 1;
    }
    else
    {
        const char* fileName = m_document.fileName().toLatin1();
        struct stat st;
        stat( fileName, &st );
        size = st.st_size + 1;

        int fd = open( fileName, O_RDONLY );
        if ( fd == -1 )
            return ;

        contents = ( char * ) mmap( 0, size, PROT_READ, MAP_SHARED, fd, 0 );
        assert( contents != ( void* ) - 1 );
        close( fd );
    }

    kdDebug() << "===-- PARSING --===> "
    << m_document.fileName()
    << " <== readFromDisk: " << readFromDisk
    << " size: " << size
    << endl;

    pool _p;
    m_translationUnit = m_parser->parse( contents, size, &_p );

    //     DumpTree dumpTree;
    //     dumpTree.dump( m_translationUnit );

    munmap( contents, size );
    m_parser = 0;
}

#include "parsejob.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
