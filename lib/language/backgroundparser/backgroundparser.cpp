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

#include <QList>
#include <QFile>
#include <QTimer>
#include <QProgressBar>
#include <QMutex>
#include <QWaitCondition>
#include <QMutexLocker>
#include <QThread>

#include <kdebug.h>
#include <kglobal.h>
#include <kstatusbar.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include <ktexteditor/smartrange.h>
#include <ktexteditor/smartinterface.h>
#include <ktexteditor/document.h>

//ThreadWeaver includes
#include <State.h>
#include <ThreadWeaver.h>
#include <JobCollection.h>
#include <DebuggingAids.h>

#include "parsejob.h"
#include "ilanguagesupport.h"
#include "parserdependencypolicy.h"

namespace KDevelop
{
BackgroundParser::BackgroundParser(ILanguageSupport *languageSupport,  QObject* parent )
        : QObject( parent ),
        m_languageSupport(languageSupport),
        m_delay( 500 ),
        m_threads( 1 ),
        m_modelsToCache( 0 ),
        m_progressBar( new QProgressBar ),
        m_weaver( new Weaver( this ) ),
        m_dependencyPolicy( new ParserDependencyPolicy ),
        m_mutex(new QMutex()),
        m_waitForJobCreation(new QWaitCondition)
{
    ThreadWeaver::setDebugLevel(true, 1);

    m_timer = new QTimer( this );
    m_timer->setSingleShot( true );
    connect( m_timer, SIGNAL( timeout() ), this, SLOT( parseDocuments() ) );

    suspend(); //Don't start the weaver until after project file has been read

    // Signal to allow other threads to request document addition.
    connect(this, SIGNAL(requestAddDocument(const QUrl&)),
        this, SLOT(acceptAddDocument(const QUrl&)), Qt::QueuedConnection);
}

BackgroundParser::~BackgroundParser()
{
    suspend();
    m_weaver->dequeue();
    m_weaver->requestAbort();
    m_weaver->finish();

    // Release dequeued jobs
    QHashIterator<KUrl, ParseJob*> it = m_parseJobs;
    while (it.hasNext()) {
        it.next();
        it.value()->setBackgroundParser(0);
        delete it.value();
    }

    delete m_dependencyPolicy;
    delete m_waitForJobCreation;
}

void BackgroundParser::clear(QObject* parent)
{
    QMutexLocker lock(m_mutex);

    QHashIterator<KUrl, ParseJob*> it = m_parseJobs;
    while (it.hasNext()) {
        it.next();
        if (it.value()->parent() == parent) {
            it.value()->requestAbort();
        }
    }
}

void BackgroundParser::loadSettings( bool projectIsLoaded )
{
    KConfigGroup config(KGlobal::config(), "Background Parser");
    bool enabled = config.readEntry( "Enabled", true );
    m_delay = config.readEntry( "Delay", 500 );
    m_threads = config.readEntry( "Number of Threads", 1 );

    if ( enabled )
        resume();
    else
        suspend();
}

void BackgroundParser::saveSettings( bool projectIsLoaded )
{
}

void BackgroundParser::initialize()
{
    QMutexLocker lock(m_mutex);

/*    m_progressBar->setMinimumWidth( 150 );
    Core::mainWindow()->statusBar()->addPermanentWidget( m_progressBar );
    m_progressBar->hide();*/
}

void BackgroundParser::cleanup()
{
}

void BackgroundParser::acceptAddDocument(const QUrl& url)
{
    addDocument(KUrl(url));
}

void BackgroundParser::addDocument( const KUrl &url )
{
    kDebug() << "BackgroundParser::addDocument" << endl;
    QMutexLocker lock(m_mutex);
    {

    if (thread() != QThread::currentThread()) {
        // We're being called from a worker thread... transfer this request to the main thread
        // and wait for the result ( it will want to be able to retrieve the job)
        emit requestAddDocument(url);

        bool foundParseJob = false;
        int count = 0;

        do {
            // Only wait for one second, or five wakeups, as this can deadlock when closing the project
            // FIXME: detect when project is closing
            m_waitForJobCreation->wait(m_mutex, 200);

            foundParseJob = m_parseJobs.contains(url);
            ++count;

        } while (!foundParseJob && count < 5);

        return;
    }

    Q_ASSERT( url.isValid() );

    if ( !m_documents.contains( url ) )
    {
        m_documents.insert( url, true );
        parseDocumentsInternal();
    }

    }

    // Wake up waiting threads
    m_waitForJobCreation->wakeAll();
}

/*
void BackgroundParser::addDocument( Document* document )
{
    Q_ASSERT(thread() == QThread::currentThread());

    Q_ASSERT( document && document->textDocument() );

    m_openDocuments.insert( document->url(), document );

    connect( document->textDocument(), SIGNAL( textChanged( KTextEditor::Document* ) ),
                this, SLOT( documentChanged( KTextEditor::Document* ) ) );

    addDocument( document->url() );
}
*/

void BackgroundParser::addDocumentList( const KUrl::List &urls )
{
    QMutexLocker lock(m_mutex);

    uint i = 0;
    foreach( KUrl url, urls )
    {
        if ( !m_documents.contains( url ) )
        {
            i++;
            m_documents.insert( url, true );
        }
    }

//     cacheModelsInternal( i );
    parseDocumentsInternal();
}

void BackgroundParser::removeDocument( const KUrl &url )
{
    QMutexLocker lock(m_mutex);

    if ( !url.isValid() ) {
        kWarning() << k_funcinfo << "Invalid url " << url << endl;
        return;
    }

    m_documents.remove( url );
//     if ( m_openDocuments.contains( url ) )
//         m_openDocuments.remove( url );
}

/*
void BackgroundParser::removeDocument( Document* document )
{
    Q_ASSERT(thread() == QThread::currentThread());

    Q_ASSERT( document && document->textDocument() );

    removeDocument( document->url() );
}
*/

void BackgroundParser::parseDocuments()
{
    QMutexLocker lock(m_mutex);

    parseDocumentsInternal();
}

void BackgroundParser::parseDocumentsInternal()
{
    // First create the jobs, then enqueue them, because they may
    // need to access each other for generating dependencies.
    QList<ParseJob*> jobs;

    for ( QMap<KUrl, bool>::Iterator it = m_documents.begin();
            it != m_documents.end(); ++it )
    {
        KUrl url = it.key();
        bool &p = it.value();
        if ( p )
        {
            ParseJob * parse = 0L;
//             Document* document = m_openDocuments[ url ];

//             if ( document )
//                 parse = langSupport->createParseJob( document );
//             else
                parse = m_languageSupport->createParseJob( url );

            if ( !parse )
                return ; //Language part did not produce a valid ParseJob

            parse->setBackgroundParser(this);

            connect( parse, SIGNAL( done( Job* ) ),
                     this, SLOT( parseComplete( Job* ) ) );

            connect( parse, SIGNAL( failed( Job* ) ),
                     this, SLOT( parseComplete( Job* ) ) );

            p = false; //Don't parse for next time

            m_parseJobs.insert(url, parse);
            jobs.append(parse);
        }
    }

    // Ok, enqueueing is fine because m_parseJobs contains all of the jobs now

    foreach (ParseJob* parse, jobs) {
        //kDebug() << k_funcinfo << "Enqueue " << parse << endl;
        m_weaver ->enqueue( parse );
    }
}

void BackgroundParser::parseComplete( Job *job )
{
    QMutexLocker lock(m_mutex);

    //kDebug() << k_funcinfo << "Complete " << job << " success? " << job->success() << endl;

    if ( ParseJob * parseJob = qobject_cast<ParseJob*>( job ) )
    {
        m_parseJobs.remove(parseJob->document());

        // FIXME hack, threadweaver doesn't let us know when we can delete, so just pick an arbitrary time...
        // (awaiting reply from Mirko on this one)
        parseJob->setBackgroundParser(0);
        QTimer::singleShot(500, parseJob, SLOT(deleteLater()));
        //delete parseJob;
    }
}

/*
void BackgroundParser::documentChanged( KTextEditor::Document * document )
{
    QMutexLocker lock(m_mutex);

    Q_ASSERT( m_documents.contains( document->url() ) );
    m_documents.insert( document->url(), true );

    bool s = m_weaver->state().stateId() == ThreadWeaver::Suspended
            || m_weaver->state().stateId() ==  ThreadWeaver::Suspending;

    if ( !m_timer->isActive() && !s )
        m_timer->start( m_delay );
}
*/

void BackgroundParser::suspend()
{
    bool s = m_weaver->state().stateId() == ThreadWeaver::Suspended
            || m_weaver->state().stateId() ==  ThreadWeaver::Suspending;

    if ( s ) //Already suspending
        return;

    m_timer->stop();

    m_weaver->suspend();

    if (m_progressBar)
        m_progressBar->hide();
}

void BackgroundParser::resume()
{
    bool s = m_weaver->state().stateId() == ThreadWeaver::Suspended
            || m_weaver->state().stateId() ==  ThreadWeaver::Suspending;

    if ( m_timer->isActive() && !s ) //Not suspending
        return;

    m_timer->start( m_delay );

    m_weaver->setMaximumNumberOfThreads( m_threads );
    m_weaver->resume();

    if (m_weaver->queueLength() && m_progressBar)
        m_progressBar->show();
}

void BackgroundParser::setDelay( int msec )
{
    QMutexLocker lock(m_mutex);

    m_delay = msec;
}

void BackgroundParser::setThreads( int threads )
{
    QMutexLocker lock(m_mutex);

    m_threads = threads;
}

ParserDependencyPolicy* BackgroundParser::dependencyPolicy() const
{
    return m_dependencyPolicy;
}

ParseJob* BackgroundParser::parseJobForDocument(const KUrl& document) const
{
    QMutexLocker lock(m_mutex);

    if (m_parseJobs.contains(document))
        return m_parseJobs[document];

    return 0;
}

}
#include "backgroundparser.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
