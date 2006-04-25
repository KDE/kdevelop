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

#include "backgroundparser.h"

#include "kdevcodemodel.h"
#include "kdevcodeaggregate_p.h"
#include "kdevdocumentcontroller.h"

#include "parsejob.h"
#include "parser/dumptree.h"
#include "parser/memorypool.h"
#include "parser/codemodel.h"
#include "cpplanguagesupport.h"
#include "cpphighlighting.h"

#include <QList>
#include <QTimer>
#include <QMutexLocker>

#include <kdebug.h>

#include <ktexteditor/smartrange.h>
#include <ktexteditor/smartinterface.h>
#include <ktexteditor/document.h>

#include <ThreadWeaver.h>
#include <JobCollection.h>

BackgroundParser::BackgroundParser( CppLanguageSupport* cppSupport )
        : QObject( cppSupport ),
        m_cppSupport( cppSupport ),
        m_suspend( false )
{
    m_memoryPool = new pool;
    m_timer = new QTimer( this );
    m_timer->setSingleShot( true );
    connect( m_timer, SIGNAL( timeout() ), this, SLOT( parseDocuments() ) );
    m_timer->start( 500 );
}

BackgroundParser::~BackgroundParser()
{
    Weaver::instance() ->finish();
    delete m_memoryPool;
}

void BackgroundParser::addDocument( const KUrl &url, KDevDocument* document )
{
    if (document)
        m_openDocuments.insert(url, document);

    if ( !m_documents.contains( url ) )
    {
        m_documents.insert( url, true );

        parseDocuments();

    }

    if (document && document->textDocument())
        connect( document->textDocument(), SIGNAL( textChanged( KTextEditor::Document* ) ),
            SLOT( documentChanged( KTextEditor::Document* ) ) );
}

void BackgroundParser::removeDocument( const KUrl &url )
{
    m_documents.remove( url );
    if (m_openDocuments.contains(url))
        m_openDocuments.remove(url);
}

void BackgroundParser::parseDocuments()
{
    JobCollection* collection = new JobCollection(this);
    for ( QMap<KUrl, bool>::Iterator it = m_documents.begin();
            it != m_documents.end(); ++it )
    {
        KUrl url = it.key();
        bool &p = it.value();
        if ( p )
        {
            ParseJob * parse = 0L;
            KDevDocument* document = m_openDocuments[url];

            if (document)
            {
                KTextEditor::SmartRange* highlight = 0L;
                if (KTextEditor::SmartInterface* smart = dynamic_cast<KTextEditor::SmartInterface*>(document->textDocument()))
                {
                    if (smart->documentHighlights().count())
                    {
                        highlight = smart->documentHighlights().first();
                        highlight->deleteChildRanges();

                    } else {
                        highlight = smart->newSmartRange(document->textDocument()->documentRange(), 0L, KTextEditor::SmartRange::ExpandLeft | KTextEditor::SmartRange::ExpandRight);
                        smart->addHighlightToDocument(highlight, true);
                    }
                }

                parse = new ParseJob( document, m_memoryPool, this, highlight );

            } else {
                parse = new ParseJob( url, m_memoryPool, this );
            }

            p = false;

            if ( url == m_cppSupport->documentController() ->activeDocumentUrl() )
            {
                KDevDocument* document = m_cppSupport->documentController() ->documentForUrl( url );
                Q_ASSERT(document->textDocument());

                parse->setContents( document->textDocument()->text().toAscii() );
            }
            connect( parse, SIGNAL( done( Job* ) ),
                     this, SLOT( parseComplete( Job* ) ) );

            collection->addJob( parse );
        }
    }
    Weaver::instance() ->enqueue( collection );
}

void BackgroundParser::parseComplete( Job *job )
{
    QMutexLocker locker( &m_mutex );

    if (JobCollection* collection = qobject_cast<JobCollection*>( job ))
        return collection->deleteLater();

    if (ParseJob * parseJob = qobject_cast<ParseJob*>( job )) {
        if (!parseJob->wasSuccessful())
            // TODO get it to the UI?
            return;

        m_cppSupport->codeHighlighting()->highlightModel(parseJob->codeModel());

        m_cppSupport->codeProxy() ->insertModel( parseJob->document(),
                parseJob->codeModel() );
        m_url2unit.insert( parseJob->document(), parseJob->translationUnit() );

        parseJob->deleteLater();
    }
}

void BackgroundParser::documentChanged( KTextEditor::Document * document )
{
    Q_ASSERT( m_documents.contains( document->url() ) );
    m_documents.insert( document->url(), true );
    if ( !m_timer->isActive() && !m_suspend )
        m_timer->start( 500 );
}

void BackgroundParser::suspend()
{
    m_suspend = true;
    m_timer->stop();
}

void BackgroundParser::resume()
{
    m_suspend = false;
    m_timer->start( 500 );
}

void BackgroundParser::removeDocumentFile( KDevDocument * document )
{
    m_openDocuments.remove(document->url());
}

#include "backgroundparser.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
