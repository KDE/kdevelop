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

class BackgroundParserPrivate
{
public:
    BackgroundParserPrivate(BackgroundParser* parser )
        : m_parser(parser)
    {}

    // Non-mutex guarded functions, only call with m_mutex acquired.
    void parseDocumentsInternal()
    {
        kDebug() << "BackgroundParser::parseDocumentsInternal" << endl;
        // First create the jobs, then enqueue them, because they may
        // need to access each other for generating dependencies.
        QList<ParseJob*> jobs;

        for ( QMap<KUrl, bool>::Iterator it = m_documents.begin();
                it != m_documents.end(); ++it )
        {
            kDebug() << "adding document " << it.key() << endl;
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

                parse->setBackgroundParser(m_parser);

                QObject::connect( parse, SIGNAL( done( ThreadWeaver::Job* ) ),
                         m_parser, SLOT( parseComplete( ThreadWeaver::Job* ) ) );

                QObject::connect( parse, SIGNAL( failed( ThreadWeaver::Job* ) ),
                         m_parser, SLOT( parseComplete( ThreadWeaver::Job* ) ) );

                p = false; //Don't parse for next time

                m_parseJobs.insert(url, parse);
                jobs.append(parse);
            }
        }

        // Ok, enqueueing is fine because m_parseJobs contains all of the jobs now

        foreach (ParseJob* parse, jobs) {
            kDebug() << k_funcinfo << "Enqueue " << parse << endl;
            m_weaver ->enqueue( parse );
        }
    }
    void acceptAddDocument(const QUrl& url)
    {
        m_parser->addDocument(KUrl(url));
    }

//     void cacheModelsInternal( uint modelsToCache );
    BackgroundParser* m_parser;
    ILanguageSupport *m_languageSupport;

    QTimer *m_timer;
    int m_delay;
    int m_threads;
    uint m_modelsToCache;

    // A list of known documents, and whether they are due to be parsed or not
    QMap<KUrl, bool> m_documents;
    // A list of open documents
//     QMap<KUrl, Document*> m_openDocuments;
    // Current parse jobs
    QHash<KUrl, ParseJob*> m_parseJobs;

    QPointer<QProgressBar> m_progressBar;

    ThreadWeaver::Weaver* m_weaver;

    ParserDependencyPolicy* m_dependencyPolicy;

    QMutex* m_mutex;
    QWaitCondition* m_waitForJobCreation;
};

BackgroundParser::BackgroundParser(ILanguageSupport *languageSupport,  QObject* parent )
        : QObject( parent ), d(new BackgroundParserPrivate(this))
{
    d->m_languageSupport = languageSupport;
    d->m_delay =  500;
    d->m_threads =  1;
    d->m_modelsToCache =  0;
    d->m_progressBar =  new QProgressBar;
    d->m_weaver =  new Weaver( this );
    d->m_dependencyPolicy =  new ParserDependencyPolicy;
    d->m_mutex = new QMutex();
    d->m_waitForJobCreation = new QWaitCondition;

    ThreadWeaver::setDebugLevel(true, 1);

    d->m_timer = new QTimer( this );
    d->m_timer->setSingleShot( true );
    connect( d->m_timer, SIGNAL( timeout() ), this, SLOT( parseDocuments() ) );

    loadSettings(false); //start the weaver

    // Signal to allow other threads to request document addition.
    connect(this, SIGNAL(requestAddDocument(const QUrl&)),
        this, SLOT(acceptAddDocument(const QUrl&)), Qt::QueuedConnection);
}

BackgroundParser::~BackgroundParser()
{
    suspend();
    d->m_weaver->dequeue();
    d->m_weaver->requestAbort();
    d->m_weaver->finish();

    // Release dequeued jobs
    QHashIterator<KUrl, ParseJob*> it = d->m_parseJobs;
    while (it.hasNext()) {
        it.next();
        it.value()->setBackgroundParser(0);
        delete it.value();
    }

    delete d->m_dependencyPolicy;
    delete d->m_waitForJobCreation;
}

void BackgroundParser::clear(QObject* parent)
{
    QMutexLocker lock(d->m_mutex);

    QHashIterator<KUrl, ParseJob*> it = d->m_parseJobs;
    while (it.hasNext()) {
        it.next();
        if (it.value()->parent() == parent) {
            it.value()->requestAbort();
        }
    }
}

void BackgroundParser::loadSettings( bool projectIsLoaded )
{
    Q_UNUSED( projectIsLoaded )
    KConfigGroup config(KGlobal::config(), "Background Parser");
    bool enabled = config.readEntry( "Enabled", true );
    d->m_delay = config.readEntry( "Delay", 500 );
    d->m_threads = config.readEntry( "Number of Threads", 1 );

    if ( enabled )
        resume();
    else
        suspend();
}

void BackgroundParser::saveSettings( bool projectIsLoaded )
{
    Q_UNUSED( projectIsLoaded )
}

void BackgroundParser::initialize()
{
    QMutexLocker lock(d->m_mutex);

/*    m_progressBar->setMinimumWidth( 150 );
    Core::mainWindow()->statusBar()->addPermanentWidget( m_progressBar );
    m_progressBar->hide();*/
}

void BackgroundParser::cleanup()
{
}

void BackgroundParser::addDocument( const KUrl &url )
{
    kDebug() << "BackgroundParser::addDocument" << endl;
    QMutexLocker lock(d->m_mutex);
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
            d->m_waitForJobCreation->wait(d->m_mutex, 200);

            foundParseJob = d->m_parseJobs.contains(url);
            ++count;

        } while (!foundParseJob && count < 5);

        return;
    }

    Q_ASSERT( url.isValid() );

    if ( !d->m_documents.contains( url ) )
    {
        d->m_documents.insert( url, true );
        d->parseDocumentsInternal();
    }

    }

    // Wake up waiting threads
    d->m_waitForJobCreation->wakeAll();
}

/*
void BackgroundParser::addDocument( Document* document )
{
    Q_ASSERT(thread() == QThread::currentThread());

    Q_ASSERT( document && document->textDocument() );

    d->m_openDocuments.insert( document->url(), document );

    connect( document->textDocument(), SIGNAL( textChanged( KTextEditor::Document* ) ),
                this, SLOT( documentChanged( KTextEditor::Document* ) ) );

    addDocument( document->url() );
}
*/

void BackgroundParser::addDocumentList( const KUrl::List &urls )
{
    QMutexLocker lock(d->m_mutex);

    uint i = 0;
    foreach( KUrl url, urls )
    {
        if ( !d->m_documents.contains( url ) )
        {
            i++;
            d->m_documents.insert( url, true );
        }
    }

//     cacheModelsInternal( i );
    d->parseDocumentsInternal();
}

void BackgroundParser::removeDocument( const KUrl &url )
{
    QMutexLocker lock(d->m_mutex);

    if ( !url.isValid() ) {
        kWarning() << k_funcinfo << "Invalid url " << url << endl;
        return;
    }

    d->m_documents.remove( url );
//     if ( d->m_openDocuments.contains( url ) )
//         d->m_openDocuments.remove( url );
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
    QMutexLocker lock(d->m_mutex);

    d->parseDocumentsInternal();
}

void BackgroundParser::parseComplete( Job *job )
{
    QMutexLocker lock(d->m_mutex);

    //kDebug() << k_funcinfo << "Complete " << job << " success? " << job->success() << endl;

    if ( ParseJob * parseJob = qobject_cast<ParseJob*>( job ) )
    {
        d->m_parseJobs.remove(parseJob->document());

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

    Q_ASSERT( d->m_documents.contains( document->url() ) );
    d->m_documents.insert( document->url(), true );

    bool s = d->m_weaver->state().stateId() == ThreadWeaver::Suspended
            || d->m_weaver->state().stateId() ==  ThreadWeaver::Suspending;

    if ( !d->m_timer->isActive() && !s )
        d->m_timer->start( m_delay );
}
*/

void BackgroundParser::suspend()
{
    bool s = d->m_weaver->state().stateId() == ThreadWeaver::Suspended
            || d->m_weaver->state().stateId() ==  ThreadWeaver::Suspending;

    if ( s ) //Already suspending
        return;

    d->m_timer->stop();

    d->m_weaver->suspend();

    if (d->m_progressBar)
        d->m_progressBar->hide();
}

void BackgroundParser::resume()
{
    bool s = d->m_weaver->state().stateId() == ThreadWeaver::Suspended
            || d->m_weaver->state().stateId() ==  ThreadWeaver::Suspending;

    if ( d->m_timer->isActive() && !s ) //Not suspending
        return;

    d->m_timer->start( d->m_delay );

    d->m_weaver->setMaximumNumberOfThreads( d->m_threads );
    d->m_weaver->resume();

    if (d->m_weaver->queueLength() && d->m_progressBar)
        d->m_progressBar->show();
}

void BackgroundParser::setDelay( int msec )
{
    QMutexLocker lock(d->m_mutex);

    d->m_delay = msec;
}

void BackgroundParser::setThreads( int threads )
{
    QMutexLocker lock(d->m_mutex);

    d->m_threads = threads;
}

ParserDependencyPolicy* BackgroundParser::dependencyPolicy() const
{
    return d->m_dependencyPolicy;
}

ParseJob* BackgroundParser::parseJobForDocument(const KUrl& document) const
{
    QMutexLocker lock(d->m_mutex);

    if (d->m_parseJobs.contains(document))
        return d->m_parseJobs[document];

    return 0;
}

}
#include "backgroundparser.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
