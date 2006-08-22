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
#include "kdevmainwindow.h"
#include "kdevlanguagesupport.h"
#include "kdevdocumentcontroller.h"
#include "kdevcodehighlighting.h"

#include "kdevast.h"
#include "kdevparsejob.h"
#include "kdevpersistenthash.h"
#include "kdevprojectcontroller.h"

#include <QList>
#include <QFile>
#include <QTimer>
#include <QProgressBar>

#include <kdebug.h>
#include <kstatusbar.h>

#include <ktexteditor/smartrange.h>
#include <ktexteditor/smartinterface.h>
#include <ktexteditor/document.h>

#include <weaver/ThreadWeaver.h>
#include <weaver/JobCollection.h>
#include <weaver/DebuggingAids.h>

KDevBackgroundParser::KDevBackgroundParser( QObject* parent )
        : QObject( parent ),
        m_suspend( true ),
        m_modelsToCache( 0 ),
        m_progressBar( new QProgressBar ),
        m_weaver( new Weaver( this, 2 ) )
{
    //ThreadWeaver::setDebugLevel(true, 5);
    m_weaver->suspend();

    m_timer = new QTimer( this );
    m_timer->setSingleShot( true );
    connect( m_timer, SIGNAL( timeout() ), this, SLOT( parseDocuments() ) );
    m_timer->start( 500 );

    connect(KDevCore::projectController(), SIGNAL(projectOpened()), SLOT(resume()));
    connect(KDevCore::projectController(), SIGNAL(projectClosed()), SLOT(suspend()));
}

KDevBackgroundParser::~KDevBackgroundParser()
{
    m_weaver->finish();
}

void KDevBackgroundParser::init()
{
    m_progressBar->setMinimumWidth( 150 );
    KDevCore::mainWindow()->statusBar()->addPermanentWidget( m_progressBar );
    m_progressBar->hide();
}

void KDevBackgroundParser::cacheModels( uint modelsToCache )
{
    m_modelsToCache = modelsToCache;
    m_progressBar->reset();
    m_progressBar->setMinimum( 0 );
    m_progressBar->setMaximum( modelsToCache );
    m_progressBar->show();
}

void KDevBackgroundParser::addDocument( const KUrl &url )
{
    Q_ASSERT( url.isValid() );

    if ( !m_documents.contains( url ) )
    {
        m_documents.insert( url, true );
        parseDocuments();
    }
}

void KDevBackgroundParser::addDocument( KDevDocument* document )
{
    Q_ASSERT( document && document->textDocument() );

    m_openDocuments.insert( document->url(), document );

    connect( document->textDocument(), SIGNAL( textChanged( KTextEditor::Document* ) ),
                this, SLOT( documentChanged( KTextEditor::Document* ) ) );

    addDocument( document->url() );
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
    if ( !url.isValid() ) {
        kWarning() << k_funcinfo << "Invalid url " << url << endl;
        return;
    }

    m_documents.remove( url );
    if ( m_openDocuments.contains( url ) )
        m_openDocuments.remove( url );
}

void KDevBackgroundParser::removeDocument( KDevDocument* document )
{
    Q_ASSERT( document && document->textDocument() );

    removeDocument( document->url() );
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

            connect( parse, SIGNAL( done( Job* ) ),
                     this, SLOT( parseComplete( Job* ) ), Qt::QueuedConnection );

            p = false; //Don't parse for next time

            collection->addJob( parse );
        }
    }
    m_weaver ->enqueue( collection );
}

void KDevBackgroundParser::parseComplete( Job *job )
{
    KDevLanguageSupport * langSupport = KDevCore::activeLanguage();
    if ( !langSupport )
        return ;

    if ( KDevParseJob * parseJob = qobject_cast<KDevParseJob*>( job ) )
    {
        if ( m_modelsToCache )
        {
            if ( parseJob->codeModel() )
            m_modelCache.append( qMakePair( parseJob->document(),
                                            parseJob->codeModel() ) );

            m_modelsToCache--; //decrement
            m_progressBar->setValue( m_progressBar->value() + 1 );

            if ( !m_modelsToCache /*&& !m_modelCache.isEmpty()*/ )
            {
                if ( parseJob->codeModel() )
                langSupport->codeProxy() ->insertModelCache( m_modelCache );
                m_modelCache.clear();

                //FIXME Stub for now, but eventually save persistent parser info
                //whatever that may entail.
                if (KDevCore::activeProject())
                    KDevCore::activeProject()->persistentHash()->save();

                m_progressBar->hide();
            }
        }
        else
        {
            if ( parseJob->codeModel() )
            langSupport->codeProxy() ->insertModel( parseJob->document(),
                                                    parseJob->codeModel() );
        }

        //FIXME Stub for now, but eventually save persistent parser info
        //whatever that may entail.
        if (KDevCore::activeProject())
            KDevCore::activeProject()->persistentHash()->insertAST( parseJob->document(), parseJob->AST() );
        else
            parseJob->AST()->release();

        // FIXME hack, threadweaver doesn't let us know when we can delete, so just pick an arbitrary time...
        // (awaiting reply from Mirko on this one)
        QTimer::singleShot(500, parseJob, SLOT(deleteLater()));
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

    m_weaver->suspend();
}

void KDevBackgroundParser::resume()
{
    m_suspend = false;
    m_timer->start( 500 );

    m_weaver->resume();
}

#include "kdevbackgroundparser.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
