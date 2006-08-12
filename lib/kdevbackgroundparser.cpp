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

#include "kdevbackgroundparser.h"

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevcodemodel.h"
#include "kdevlanguagesupport.h"
#include "kdevdocumentcontroller.h"

#include "kdevast.h"
#include "kdevparsejob.h"
#include "kdevpersistenthash.h"

#include <QList>
#include <QTimer>
#include <QMutexLocker>
#include <QFile>

#include <kdebug.h>

#include <ktexteditor/smartrange.h>
#include <ktexteditor/smartinterface.h>
#include <ktexteditor/document.h>

#include <weaver/ThreadWeaver.h>
#include <weaver/JobCollection.h>

KDevBackgroundParser::KDevBackgroundParser( QObject* parent )
        : QObject( parent ),
        m_suspend( false ),
        m_modelsToCache( 0 ),
        m_peristentHash( new KDevPersistentHash ),
        m_weaver( new Weaver( this, 1, 1 ) ) //C++ parser can't multi-thread at the moment
{
    //FIXME Stub for now, but eventually load persistent parser info
    //whatever that may entail.
    m_peristentHash->load();

    m_timer = new QTimer( this );
    m_timer->setSingleShot( true );
    connect( m_timer, SIGNAL( timeout() ), this, SLOT( parseDocuments() ) );
    m_timer->start( 500 );
}

KDevBackgroundParser::~KDevBackgroundParser()
{
    m_weaver->finish();
}

void KDevBackgroundParser::addDocument( const KUrl &url, KDevDocument* document )
{
    if ( document )
        m_openDocuments.insert( url, document );

    if ( !m_documents.contains( url ) )
    {
        m_documents.insert( url, true );
        parseDocuments();
    }

    if ( document && document->textDocument() )
    {
        connect( document->textDocument(), SIGNAL( textChanged( KTextEditor::Document* ) ),
                 this, SLOT( documentChanged( KTextEditor::Document* ) ) );
    }
}

void KDevBackgroundParser::addDocumentList( const KUrl::List &urls )
{
    uint i = 0;
    foreach( KUrl url, urls )
    {
        if ( !m_documents.contains( url ) )
        {
            i++;
            m_documents.insert( url, true );
        }
    }

    cacheModels( i );
    parseDocuments();
}

void KDevBackgroundParser::removeDocument( const KUrl &url )
{
    m_documents.remove( url );
    if ( m_openDocuments.contains( url ) )
        m_openDocuments.remove( url );
}

void KDevBackgroundParser::parseDocuments()
{
    KDevLanguageSupport * langSupport = KDevCore::activeLanguage();
    if ( !langSupport )
        return ;

    JobCollection* collection = new JobCollection( this );
    for ( QMap<KUrl, bool>::Iterator it = m_documents.begin();
            it != m_documents.end(); ++it )
    {
        KUrl url = it.key();
        bool &p = it.value();
        if ( p )
        {
            KDevParseJob * parse = 0L;
            KDevDocument* document = m_openDocuments[ url ];

            if ( document )
                parse = langSupport->createParseJob( document );
            else
                parse = langSupport->createParseJob( url );

            if ( !parse )
                return ; //Language part did not produce a valid KDevParseJob

            if ( url == KDevCore::documentController() ->activeDocumentUrl() )
            {
                KDevDocument * document =
                    KDevCore::documentController() ->documentForUrl( url );
                Q_ASSERT( document->textDocument() );

                parse->setContents( document->textDocument() ->text().toAscii() );
            }

            connect( parse, SIGNAL( done( Job* ) ),
                     this, SLOT( parseComplete( Job* ) ) );

            p = false; //Don't parse for next time

            collection->addJob( parse );
        }
    }
    m_weaver ->enqueue( collection );
}

void KDevBackgroundParser::cacheModels( uint modelsToCache )
{
    m_modelsToCache = modelsToCache;
}

void KDevBackgroundParser::parseComplete( Job *job )
{
    KDevLanguageSupport * langSupport = KDevCore::activeLanguage();
    if ( !langSupport )
        return ;

    if ( KDevParseJob * parseJob = qobject_cast<KDevParseJob*>( job ) )
    {
        //FIXME The Java and CSharp parsers don't get past this as they have
        //no current code model.
        if ( !parseJob->wasSuccessful() )
            return ;

        //FIXME abstract out codehighlingting in kdevlanguagesupport
        //langSupport->codeHighlighting()->highlightModel(parseJob->codeModel());
        //langSupport->codeHighlighting()->highlightTree(parseJob->highlight());

        if ( m_modelsToCache )
        {
            m_modelCache.append( qMakePair( parseJob->document(),
                                            parseJob->codeModel() ) );

            m_modelsToCache--; //decrement

            if ( !m_modelsToCache && !m_modelCache.isEmpty() )         //decremented to zero
            {
                langSupport->codeProxy() ->insertModelCache( m_modelCache );
                m_modelCache.clear();

                //FIXME Stub for now, but eventually save persistent parser info
                //whatever that may entail.
                m_peristentHash->save();
            }
        }
        else
        {
            langSupport->codeProxy() ->insertModel( parseJob->document(),
                                                    parseJob->codeModel() );
        }

        //FIXME Stub for now, but eventually save persistent parser info
        //whatever that may entail.
        m_peristentHash->insert( parseJob->document(), parseJob->AST() );

        parseJob->deleteLater();
    }
}

void KDevBackgroundParser::documentChanged( KTextEditor::Document * document )
{
    Q_ASSERT( m_documents.contains( document->url() ) );
    m_documents.insert( document->url(), true );
    if ( !m_timer->isActive() && !m_suspend )
        m_timer->start( 500 );
}

void KDevBackgroundParser::suspend()
{
    m_suspend = true;
    m_timer->stop();
}

void KDevBackgroundParser::resume()
{
    m_suspend = false;
    m_timer->start( 500 );
}

void KDevBackgroundParser::removeDocumentFile( KDevDocument * document )
{
    m_openDocuments.remove( document->url() );
}

#include "kdevbackgroundparser.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
