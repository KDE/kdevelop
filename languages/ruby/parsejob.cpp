/*****************************************************************************
 * Copyright (c) 2006 Alexander Dymo <adymo@kdevelop.org>                    *
 *                                                                           *
 * Permission is hereby granted, free of charge, to any person obtaining     *
 * a copy of this software and associated documentation files (the           *
 * "Software"), to deal in the Software without restriction, including       *
 * without limitation the rights to use, copy, modify, merge, publish,       *
 * distribute, sublicense, and/or sell copies of the Software, and to        *
 * permit persons to whom the Software is furnished to do so, subject to     *
 * the following conditions:                                                 *
 *                                                                           *
 * The above copyright notice and this permission notice shall be            *
 * included in all copies or substantial portions of the Software.           *
 *                                                                           *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,           *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF        *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                     *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE    *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION    *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION     *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.           *
 *****************************************************************************/

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

#include "parser/parsesession.h"
#include "parser/ruby_parser.h"

#include "rubylanguagesupport.h"

namespace ruby
{

ParseJob::ParseJob( const KUrl &url, RubyLanguageSupport *parent )
        : KDevParseJob( url, parent )
        , m_session( new ParseSession )
        , m_AST( 0 )
        , m_model( 0 )
        , m_readFromDisk( false )
{}

ParseJob::ParseJob( KDevDocument *document, RubyLanguageSupport *parent )
        : KDevParseJob( document, parent )
        , m_session( new ParseSession )
        , m_AST( 0 )
        , m_model( 0 )
        , m_readFromDisk( false )
{}

ParseJob::~ParseJob()
{}

RubyLanguageSupport *ParseJob::ruby() const
{
    return static_cast<RubyLanguageSupport*>(const_cast<QObject*>(parent()));
}

KDevAST *ParseJob::AST() const
{
    Q_ASSERT(isFinished() && m_AST);
    return m_AST;
}

KDevCodeModel *ParseJob::codeModel() const
{
    Q_ASSERT(isFinished() && m_model);
    return m_model;
}

ParseSession *ParseJob::parseSession() const
{
    return m_session;
}

bool ParseJob::wasReadFromDisk() const
{
    return m_readFromDisk;
}

void ParseJob::run()
{
    if ( abortRequested() )
        return abortJob();

    QMutexLocker lock(ruby()->parseMutex(thread()));

    m_readFromDisk = !contentsAvailableFromEditor();

    if ( m_readFromDisk )
    {
        QFile file( m_document.path() );
        if ( !file.open( QIODevice::ReadOnly ) )
        {
            m_errorMessage = i18n( "Could not open file '%1'", m_document.path() );
            kWarning( 9007 ) << k_funcinfo << "Could not open file " << m_document
                             << " (path " << m_document.path() << ")" << endl;
            return ;
        }

        m_session->setContents( file.readAll() );
        Q_ASSERT ( m_session->size() > 0 );
        file.close();
    }
    else
    {
        m_session->setContents( contentsFromEditor().toAscii() );
    }

    kDebug() << "===-- PARSING --===> "
             << m_document.fileName()
             << " <== readFromDisk: " << m_readFromDisk
             << " size: " << m_session->size()
             << endl;

    if ( abortRequested() )
        return abortJob();

    // 0) setup
    parser ruby_parser;
    ruby_parser.set_token_stream( m_session->token_stream );
    ruby_parser.set_memory_pool( m_session->memory_pool );

    // 1) tokenize
    ruby_parser.tokenize( (char*) m_session->contents() );

    if ( abortRequested() )
        return abortJob();

    // 2) parse
    bool matched = ruby_parser.parse_program( &m_AST );
    m_AST->language = ruby();
    m_model = new CodeModel;

    if ( abortRequested() )
        return abortJob();

    if ( matched )
    {
        //TODO: bind declarations to the code model
    }
    else
    {
        ruby_parser.yy_expected_symbol(ast_node::Kind_program, "program");
    }
}

} // end of namespace ruby

#include "parsejob.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
