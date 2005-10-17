/*
 * KDevelop C++ Background Parser
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

#include "backgroundparser.h"

#include <QList>
#include <QTimer>

#include <kdebug.h>

#include <ktexteditor/document.h>

#include "kdevdocumentcontroller.h"

#include <ThreadWeaver.h>
#include "parsejob.h"

#include "parser/preprocessor.h"
#include "parser/control.h"
#include "parser/parser.h"
#include "parser/memorypool.h"
#include "parser/dumptree.h"

#include "cpplanguagesupport.h"
#include <kdevdocumentcontroller.h>

#include "backgroundparser.h"

BackgroundParser::BackgroundParser( CppLanguageSupport* cppSupport )
        : QObject( cppSupport ),
        m_cppSupport( cppSupport )
{
    m_weaver = Weaver::instance();
    m_preprocessor = new Preprocessor( this );
    m_control = new Control();
    m_parser = new Parser( m_control );
    m_memoryPool = new pool();
    m_timer = new QTimer( this );
    m_timer->setSingleShot( true );
    connect( m_timer, SIGNAL( timeout() ), this, SLOT( parseDocuments() ) );
    m_timer->start( 500 );
}

BackgroundParser::~BackgroundParser()
{
    delete m_control;
    delete m_parser;
}

void BackgroundParser::addDocument( const KURL &url )
{
    if ( !m_documents.contains( url ) )
    {
        m_documents.insert( url, true );

        if ( KTextEditor::Document * doc =
                    m_cppSupport->documentController() ->documentForURL( url ) )
            connect( doc, SIGNAL( textChanged( KTextEditor::Document* ) ),
                     SLOT( documentChanged( KTextEditor::Document* ) ) );

        parseDocuments();
    }
}

void BackgroundParser::removeDocument( const KURL &url )
{
    m_documents.remove( url );
}

void BackgroundParser::parseDocuments()
{
    QList< Job* > jobs;
    for ( QMap<KURL, bool>::Iterator it = m_documents.begin();
            it != m_documents.end(); ++it )
    {
        KURL url = it.key();
        bool &p = it.value();
        if ( p )
        {
            ParseJob * parse = new ParseJob( url, m_preprocessor,
                                             m_parser, m_memoryPool, this );
            p = false;

            if ( url == m_cppSupport->documentController() ->activeDocument() )
            {
                KTextEditor::Document * doc =
                    m_cppSupport->documentController() ->documentForURL( url );

                parse->setContents( doc->text().toAscii() );
            }
            connect( parse, SIGNAL( done( Job* ) ),
                     this, SLOT( parseComplete( Job* ) ) );
            jobs.append( parse );
        }
    }
    m_weaver->enqueue( jobs );
}

void BackgroundParser::parseComplete( Job *job )
{
    ParseJob * parseJob = dynamic_cast<ParseJob*>( job );
    m_url2unit[ parseJob->document() ] = parseJob->translationUnit();
}

void BackgroundParser::documentChanged( KTextEditor::Document * document )
{
    Q_ASSERT( m_documents.contains( document->url() ) );
    m_documents.insert( document->url(), true );
    if ( !m_timer->isActive() )
        m_timer->start( 500 );
}

#include "backgroundparser.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
