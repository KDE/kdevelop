/*
 * This file is part of KDevelop
 *
 * Copyright (c) 2006 Adam Treat <treat@kde.org>
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

#include "parsejob.h"

#include <cassert>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <QByteArray>

#include <kdebug.h>

#include <kdevdocumentcontroller.h>

#include "parser/binder.h"
#include "parser/parser.h"
#include "parser/control.h"
#include "parser/dumptree.h"
#include "parser/rpp/preprocessor.h"

ParseJob::ParseJob( const KUrl &url,
                    pool *memoryPool,
                    QObject* parent )
        : ThreadWeaver::Job( parent ),
        m_document( url ),
        m_memoryPool( memoryPool ),
        m_translationUnit( 0 ),
        m_model( 0 )
{
    m_contents = QByteArray();
}

ParseJob::~ParseJob()
{}

KUrl ParseJob::document() const
{
    return m_document;
}

TranslationUnitAST *ParseJob::translationUnit() const
{
    Q_ASSERT ( isFinished () && m_translationUnit );
    return m_translationUnit;
}

CodeModel *ParseJob::codeModel() const
{
    Q_ASSERT ( isFinished () && m_model );
    return m_model;
}

void ParseJob::run()
{
    bool readFromDisk = m_contents.isNull();
    char *contents;
    std::size_t size;

    if ( !readFromDisk )
    {
        contents = m_contents.data();
        size = m_contents.length() + 1;
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

    kDebug() << "===-- PARSING --===> "
    << m_document.fileName()
    << " <== readFromDisk: " << readFromDisk
    << " size: " << size
    << endl;

    Preprocessor preprocessor;
    QStringList includes;
    //     if ( true )
    //     {
    //         includes.append ("/usr/include");
    //         includes.append ("/usr/lib/gcc/i586-suse-linux/4.0.2/include");
    //         includes.append ("/usr/include/c++/4.0.2");
    //         includes.append ("/usr/include/c++/4.0.2/i586-suse-linux");
    //     }
    includes.append ( "." );
    preprocessor.addIncludePaths( includes );

    preprocessor.processString( contents );
    QByteArray preprocessed = preprocessor.result();
    std::size_t pre_size = preprocessed.length() + 1;

    Parser parser( new Control() );
    m_translationUnit = parser.parse( preprocessed, pre_size, m_memoryPool );

    m_model = new CodeModel;
    Binder binder( m_model, &parser.token_stream, &parser.lexer );
    binder.run( m_document, m_translationUnit );

    //     DumpTree dumpTree;
    //     dumpTree.dump( m_translationUnit );

    if ( readFromDisk )
        munmap( contents, size );

    m_memoryPool = 0;
}

#include "parsejob.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
