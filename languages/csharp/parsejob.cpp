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

#include <kdevcodemodel.h>

#include "Thread.h"

#include <QFile>
#include <QByteArray>

#include <kdebug.h>
#include <klocale.h>

#include "parser/csharp_parser.h"
#include "parser/csharp_binder.h"

#include "csharplanguagesupport.h"

using namespace csharp;

ParseJob::ParseJob( const KUrl &url,
                    CSharpLanguageSupport *parent )
        : KDevParseJob( url, parent ),
        m_AST( 0 ),
        m_model( 0 )
{}

ParseJob::ParseJob( KDevDocument *document,
                    CSharpLanguageSupport *parent )
        : KDevParseJob( document, parent ),
        m_AST( 0 ),
        m_model( 0 )
{}

ParseJob::~ParseJob()
{}

CSharpLanguageSupport* ParseJob::csharp() const
{
    return static_cast<CSharpLanguageSupport*>(const_cast<QObject*>(parent()));
}

KDevAST *ParseJob::AST() const
{
    Q_ASSERT ( isFinished () && m_AST );
    return m_AST;
}

KDevCodeModel *ParseJob::codeModel() const
{
    Q_ASSERT ( isFinished () && m_model );
    return m_model;
}

void ParseJob::run()
{
    if (abortRequested())
        return abortJob();

    QMutexLocker lock(csharp()->parseMutex(thread()));

    bool readFromDisk = !contentsAvailableFromEditor();

    char *contents;
    QByteArray fileData;

    if ( readFromDisk )
    {
        QFile file( m_document.path() );
        if ( !file.open( QIODevice::ReadOnly ) )
        {
            m_errorMessage = i18n( "Could not open file '%1'", m_document.path() );
            kWarning( 9007 ) << k_funcinfo << "Could not open file " << m_document
                             << " (path " << m_document.path() << ")" << endl;
            return ;
        }

        fileData = file.readAll();
        QString qcontents = QString::fromUtf8( fileData.constData() );
        contents = fileData.data();
        Q_ASSERT ( !qcontents.isEmpty() );
        file.close();
    }
    else
    {
        fileData = contentsFromEditor().toAscii();
        contents = fileData.data();
    }

    kDebug() << "===-- PARSING --===> "
    << m_document.fileName()
    << " <== readFromDisk: " << readFromDisk
    << " size: " << fileData.length()
    << endl;

    if (abortRequested())
        return abortJob();

    parser::csharp_compatibility_mode compatibility_mode = parser::csharp20_compatibility;

    parser::token_stream_type token_stream;
    parser::memory_pool_type memory_pool;

    // 0) setup
    parser csharp_parser;
    csharp_parser.set_compatibility_mode( compatibility_mode );
    csharp_parser.set_token_stream( &token_stream );
    csharp_parser.set_memory_pool( &memory_pool );

    // 1) tokenize
    csharp_parser.tokenize( contents );

    if (abortRequested())
        return abortJob();

    // 2) parse
    bool matched = csharp_parser.parse_compilation_unit( &m_AST );
    m_model = new CodeModel;

    if (abortRequested())
        return abortJob();

    if (matched)
    {
        Binder binder( m_model, csharp_parser.token_stream );
        binder.run( m_document, m_AST );
    }
    else
    {
        csharp_parser.yy_expected_symbol(ast_node::Kind_compilation_unit, "compilation_unit"); // ### remove me
    }
}

#include "parsejob.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
