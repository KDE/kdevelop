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

#include "parser/java_parser.h"
#include "parser/java_default_visitor.h"

ParseJob::ParseJob( const KUrl &url,
                    QObject *parent )
        : KDevParseJob( url, parent ),
        m_AST( 0 ),
        m_model( 0 )
{}

ParseJob::ParseJob( KDevDocument *document,
                    QObject *parent )
        : KDevParseJob( document, parent ),
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
    bool readFromDisk = !openDocument();

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
        assert( !qcontents.isEmpty() );
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

    parser::java_compatibility_mode compatibility_mode = parser::java15_compatibility;

    parser::token_stream_type token_stream;
    parser::memory_pool_type memory_pool;

  // 0) setup
    java::parser java_parser;
    java_parser.set_compatibility_mode(compatibility_mode);
    java_parser.set_token_stream(&token_stream);
    java_parser.set_memory_pool(&memory_pool);

  // 1) tokenize
    java_parser.tokenize(contents);

  // 2) parse
    compilation_unit_ast *ast = 0;
    bool matched = java_parser.parse_compilation_unit(&ast);
    if (matched)
    {
        default_visitor v;
        v.visit_node(ast);
    }
    else
    {
        java_parser.yy_expected_symbol(ast_node::Kind_compilation_unit, "compilation_unit"); // ### remove me
    }
}

#include "parsejob.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
