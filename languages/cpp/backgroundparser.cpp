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

#include "kdevcodemodel.h"
#include "kdevdocumentcontroller.h"

#include <ThreadWeaver.h>
#include "parsejob.h"

#include "parser/control.h"
#include "parser/dumptree.h"
#include "parser/codemodel.h"
#include "parser/memorypool.h"

#include "cpplanguagesupport.h"
#include <kdevdocumentcontroller.h>

#include "backgroundparser.h"

BackgroundParser::BackgroundParser( CppLanguageSupport* cppSupport )
        : QObject( cppSupport ),
        m_cppSupport( cppSupport )
{
    m_weaver = Weaver::instance();
    m_control = new Control();
    m_memoryPool = new pool();
    m_timer = new QTimer( this );
    m_timer->setSingleShot( true );
    connect( m_timer, SIGNAL( timeout() ), this, SLOT( parseDocuments() ) );
    m_timer->start( 500 );
}

BackgroundParser::~BackgroundParser()
{
    delete m_control;
    delete m_memoryPool;
}

void BackgroundParser::addDocument( const KURL &url )
{
    if ( !m_documents.contains( url ) )
    {
        m_documents.insert( url, true );

        if ( KTextEditor::Document * doc =
                    m_cppSupport->documentController() ->textPartForURL( url ) )
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
            ParseJob * parse = new ParseJob( url, m_control,
                                             m_memoryPool, this );
            p = false;

            if ( url == m_cppSupport->documentController() ->activeDocument() )
            {
                KTextEditor::Document * doc =
                    m_cppSupport->documentController() ->textPartForURL( url );

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

    KDevCodeModel *cm = m_cppSupport->codeModel();
    FileModelItem file = parseJob->fileModelItem();

    // This of course can be made vastly more efficient and it should
    // be broken out into it's own class/method, but something quick
    // for testing now
    foreach ( NamespaceModelItem _namespace, file->namespaces() )
    {
        KDevCodeNamespaceItem *n =
            new KDevCodeNamespaceItem( _namespace->name() );
        cm->appendItem( n );

        foreach ( ClassModelItem _class, _namespace->classes() )
        {
            KDevCodeClassItem *c =
                new KDevCodeClassItem( _class->name() );
            cm->appendItem( c, n );

            foreach ( FunctionModelItem _function, _class->functions() )
            {
                KDevCodeFunctionItem *f =
                    new KDevCodeFunctionItem( _function->name() );
                cm->appendItem( f, c );
            }
            foreach ( VariableModelItem _variable, _class->variables() )
            {
                KDevCodeVariableItem *v =
                    new KDevCodeVariableItem( _variable->name() );
                cm->appendItem( v, c );
            }
        }
        foreach ( FunctionModelItem _function, _namespace->functions() )
        {
            KDevCodeFunctionItem *f =
                new KDevCodeFunctionItem( _function->name() );
            cm->appendItem( f, n );
        }
        foreach ( VariableModelItem _variable, _namespace->variables() )
        {
            KDevCodeVariableItem *v =
                new KDevCodeVariableItem( _variable->name() );
            cm->appendItem( v, n );
        }
    }
    foreach ( ClassModelItem _class, file->classes() )
    {
        KDevCodeClassItem *c =
            new KDevCodeClassItem( _class->name() );
        cm->appendItem( c );

        foreach ( FunctionModelItem _function, _class->functions() )
        {
            KDevCodeFunctionItem *f =
                new KDevCodeFunctionItem( _function->name() );
            cm->appendItem( f, c );
        }
        foreach ( VariableModelItem _variable, _class->variables() )
        {
            KDevCodeVariableItem *v =
                new KDevCodeVariableItem( _variable->name() );
            cm->appendItem( v, c );
        }
    }
    foreach ( FunctionModelItem _function, file->functions() )
    {
        KDevCodeFunctionItem *f =
            new KDevCodeFunctionItem( _function->name() );
        cm->appendItem( f );
    }
    foreach ( VariableModelItem _variable, file->variables() )
    {
        KDevCodeVariableItem *v =
            new KDevCodeVariableItem( _variable->name() );
        cm->appendItem( v );
    }
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
