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

#include "kdevcodemodel.h"
#include "kdevcodeaggregate_p.h"
#include "kdevdocumentcontroller.h"
#include "kdevlanguagesupport.h"
#include "kdevcore.h"
#include "kdevproject.h"

#include "kdevast.h"
#include "kdevparsejob.h"

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
        m_weaver( new Weaver( this, 1, 1 ) )
{
    m_timer = new QTimer( this );
    m_timer->setSingleShot( true );
    connect( m_timer, SIGNAL( timeout() ), this, SLOT( parseDocuments() ) );
    m_timer->start( 500 );
    KDevApi::self()->setBackgroundParser( this );
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
        connect( document->textDocument(), SIGNAL( textChanged( KTextEditor::Document* ) ),
                 SLOT( documentChanged( KTextEditor::Document* ) ) );
}

void KDevBackgroundParser::addDocumentList( const KUrl::List &urls )
{
    foreach( KUrl url, urls )
    if ( !m_documents.contains( url ) )
        m_documents.insert( url, true );

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
    KDevLanguageSupport * langSupport = KDevApi::self() ->languageSupport();
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
            {
                KTextEditor::SmartRange * highlight = 0L;
                if ( KTextEditor::SmartInterface * smart =
                            dynamic_cast<KTextEditor::SmartInterface*>( document->textDocument() )
                   )
                {
                    if ( smart->documentHighlights().count() )
                    {
                        highlight = smart->documentHighlights().first();
                        highlight->deleteChildRanges();

                    }
                    else
                    {
                        highlight =
                            smart->newSmartRange( document->textDocument() ->documentRange(),
                                                  0L,
                                                  KTextEditor::SmartRange::ExpandLeft
                                                  | KTextEditor::SmartRange::ExpandRight );
                        smart->addHighlightToDocument( highlight, false );
                    }
                }

                parse = langSupport->createParseJob( document, highlight );

            }
            else
            {
                parse = langSupport->createParseJob( url );
            }

            if ( !parse )
                return; //Language part did not produce a valid KDevParseJob

            p = false;

            if ( url == KDevApi::self() ->documentController() ->activeDocumentUrl() )
            {
                KDevDocument * document =
                    KDevApi::self() ->documentController() ->documentForUrl( url );
                Q_ASSERT( document->textDocument() );

                parse->setContents( document->textDocument() ->text().toAscii() );
            }
            connect( parse, SIGNAL( done( Job* ) ),
                     this, SLOT( parseComplete( Job* ) ) );

            collection->addJob( parse );
        }
    }
    m_weaver ->enqueue( collection );
}

void KDevBackgroundParser::parseComplete( Job *job )
{
    QMutexLocker locker( &m_mutex );

    KDevLanguageSupport * langSupport = KDevApi::self() ->languageSupport();
    if ( !langSupport )
        return ;

    if (JobCollection* collection = qobject_cast<JobCollection*>( job ))
        return collection->deleteLater();

    if (KDevParseJob * parseJob = qobject_cast<KDevParseJob*>( job )) {
        if (!parseJob->wasSuccessful())
            // TODO get it to the UI?
            return;

        //FIXME abstract out codehighlingting in kdevlanguagesupport
        //langSupport->codeHighlighting()->highlightModel(parseJob->codeModel());
        //langSupport->codeHighlighting()->highlightTree(parseJob->highlight());

        langSupport->codeProxy() ->insertModel(
                parseJob->document(),
                parseJob->codeModel() );
        m_url2unit.insert( parseJob->document(), parseJob->AST() );

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
