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

#include <QFile>
#include <QByteArray>

#include <kdebug.h>
#include <klocale.h>

ParseJob::ParseJob( const KUrl &url,
                    QObject *parent )
        : KDevParseJob( url, parent ),
        m_AST( 0 ),
        m_model( 0 )
{}

ParseJob::ParseJob( KDevDocument *document,
                    KTextEditor::SmartRange *highlight,
                    QObject *parent )
        : KDevParseJob( document, highlight, parent ),
        m_AST( 0 ),
        m_model( 0 )
{}

ParseJob::~ParseJob()
{}

KDevAST *ParseJob::AST() const
{
    return 0;
//     Q_ASSERT ( isFinished () && m_AST );
//     return m_AST;
}

KDevCodeModel *ParseJob::codeModel() const
{
    return 0;
//     Q_ASSERT ( isFinished () && m_model );
//     return new KDevCodeModel;
}

void ParseJob::run()
{
    bool readFromDisk = m_contents.isNull();
    std::size_t size;

    char *contents;

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

        QByteArray fileData = file.readAll();
        QString qcontents = QString::fromUtf8( fileData.constData() );
        contents = fileData.data();
        size = fileData.size();
        assert( !qcontents.isEmpty() );
        file.close();
    }
    else
    {
        // FIXME: jpetso says: why is this here if we don't use it?
        // qcontents = QString::fromUtf8( m_contents.constData() );
        size = m_contents.size();
        contents = m_contents.data();
    }

    kDebug() << "===-- PARSING --===> "
    << m_document.fileName()
    << " <== readFromDisk: " << readFromDisk
    << " size: " << size
    << endl;

    parser::csharp_compatibility_mode compatibility_mode = parser::csharp10_compatibility;

    parser::token_stream_type token_stream;
    parser::memory_pool_type memory_pool;

  // 0) setup
    csharp::parser csharp_parser;
    csharp_parser.set_compatibility_mode(compatibility_mode);
    csharp_parser.set_token_stream(&token_stream);
    csharp_parser.set_memory_pool(&memory_pool);

  // 1) tokenize
    csharp_parser.tokenize(contents);

  // 2) parse
    compilation_unit_ast *ast = 0;
    bool matched = csharp_parser.parse_compilation_unit(&ast);
    if (matched)
    {
        default_visitor v;
        v.visit_node(ast);
    }
    else
    {
        csharp_parser.yy_expected_symbol(ast_node::Kind_compilation_unit, "compilation_unit"); // ### remove me
    }
}

#include "parsejob.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
