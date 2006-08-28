/* -*- C++ -*-

This file implements the WeaverImpl class.


$ Author: Mirko Boehm $
$ Copyright: (C) 2005, 2006 Mirko Boehm $
$ Contact: mirko@kde.org
http://www.kde.org
http://www.hackerbuero.org $
$ License: LGPL with the following explicit clarification:
This code may be linked against any version of the Qt toolkit
from Trolltech, Norway. $

$Id: WeaverImpl.cpp 30 2005-08-16 16:16:04Z mirko $

*/

#include <QObject>
#include <QMutex>
#include <QtDebug>

#include "Job.h"
#include "State.h"
#include "Thread.h"
#include "WeaverImpl.h"
#include "ThreadWeaver.h"
#include "DebuggingAids.h"
#include "WeaverObserver.h"
#include "SuspendedState.h"
#include "SuspendingState.h"
#include "DestructedState.h"
#include "WorkingHardState.h"
#include "ShuttingDownState.h"
#include "InConstructionState.h"

using namespace ThreadWeaver;

WeaverImpl::WeaverImpl(QObject* parent, int inventoryMax)
    : WeaverInterface(parent)
    , m_active(0)
    , m_inventoryMax(inventoryMax)
    , m_mutex ( new QMutex( QMutex::Recursive ) )
    , m_finishMutex( new QMutex )
    , m_jobAvailableMutex ( new QMutex )
    , m_state (0)
{
    // initialize state objects:
    m_states[InConstruction] = new InConstructionState( this );
    setState ( InConstruction );
    m_states[WorkingHard] = new WorkingHardState( this );
    m_states[Suspending] = new SuspendingState( this );
    m_states[Suspended] = new SuspendedState( this );
    m_states[ShuttingDown] = new ShuttingDownState( this );
    m_states[Destructed] = new DestructedState( this );

    connect ( this, SIGNAL ( asyncThreadSuspended( Thread* ) ), SIGNAL ( threadSuspended( Thread* ) ),
              Qt::QueuedConnection );
    setState(  WorkingHard );
}

WeaverImpl::~WeaverImpl()
{   // the constructor may only be called from the thread that owns this
    // object (everything else would be what we professionals call "insane")
    REQUIRE( QThread::currentThread() == thread() );
    debug ( 3, "WeaverImpl dtor: destroying inventory.\n" );
    setState ( ShuttingDown );

    m_jobAvailable.wakeAll();

    // problem: Some threads might not be asleep yet, just finding
    // out if a job is available. Those threads will suspend
    // waiting for their next job (a rare case, but not impossible).
    // Therefore, if we encounter a thread that has not exited, we
    // have to wake it again (which we do in the following for
    // loop).

    while (!m_inventory.isEmpty())
    {
        Thread* th=m_inventory.takeFirst();
        if ( !th->isFinished() )
	{
            for ( ;; )
	    {
                m_jobAvailable.wakeAll();
                if ( th->wait( 100 ) ) break;
                debug ( 1,  "WeaverImpl::~WeaverImpl: thread %i did not exit as expected, "
                        "retrying.\n", th->id() );
	    }
	}
        emit ( threadExited ( th ) );
        delete th;
    }

    m_inventory.clear();
    delete m_mutex;
    delete m_finishMutex;
    delete m_jobAvailableMutex;
    debug ( 3, "WeaverImpl dtor: done\n" );
    setState ( Destructed ); // m_state = Halted;
    // FIXME: delete state objects. what sense does DestructedState make then?
    // FIXME: make state objects static, since they are
}

void WeaverImpl::setState ( StateId id )
{
    if ( m_state==0 || m_state->stateId() != id )
    {
        m_state = m_states[id];
        debug ( 2, "WeaverImpl::setState: state changed to \"%s\".\n",
                m_state->stateName().toAscii().constData() );
        if ( id == Suspended )
	{
            emit ( suspended() );
	}

        m_state->activated();

        emit ( stateChanged ( m_state ) );
    }
}

const State& WeaverImpl::state() const
{
    return *m_state;
}

void WeaverImpl::registerObserver ( WeaverObserver *ext )
{
    connect ( this,  SIGNAL ( stateChanged ( State* ) ),
              ext,  SIGNAL ( weaverStateChanged ( State* ) ) );
    connect ( this,  SIGNAL ( threadStarted ( Thread* ) ),
              ext,  SIGNAL ( threadStarted ( Thread* ) ) );
    connect ( this,  SIGNAL ( threadBusy( Thread*,  Job* ) ),
              ext,  SIGNAL ( threadBusy ( Thread*,  Job* ) ) );
    connect ( this,  SIGNAL ( threadSuspended ( Thread* ) ),
              ext,  SIGNAL ( threadSuspended ( Thread* ) ) );
    connect ( this,  SIGNAL ( threadExited ( Thread* ) ) ,
              ext,  SIGNAL ( threadExited ( Thread* ) ) );
}

int WeaverImpl::numberOfThreads () const
{
    QMutexLocker l (m_mutex);
    return m_inventory.count ();
}

void WeaverImpl::enqueue(Job* job)
{
    adjustInventory ( 1 );
    debug ( 3, "WeaverImpl::enqueue: queueing job %p of type %s.\n",
            job, job->metaObject()->className() );
    if (job)
    {
        QMutexLocker l (m_mutex);
        job->aboutToBeQueued ( this );
        // find positiEon for insertion:;
        // FIXME (after 0.6) optimize: factor out queue nanagement into own class,
        // and use binary search for insertion (not done yet because
        // refactoring already planned):
        int i = m_assignments.size();
        if (i > 0)
	{
            while ( i > 0 && m_assignments.at(i - 1)->priority() < job->priority() ) --i;
            m_assignments.insert( i, (job) );
	} else {
            m_assignments.append (job);
	}
        assignJobs();
    }
}

void WeaverImpl::adjustInventory ( int noOfNewJobs )
{
    // no of threads that can be created:
    const int reserve = m_inventoryMax - numberOfThreads();
    if ( reserve > 0 )
    {
        QMutexLocker l (m_mutex);
        for ( int i = 0; i < qMin ( reserve,  noOfNewJobs ); ++i )
	{
            Thread *th = createThread();
            m_inventory.append(th);
            connect ( th,  SIGNAL ( jobStarted ( Thread*,  Job* ) ),
                      SIGNAL ( threadBusy ( Thread*,  Job* ) ) );
            connect ( th,  SIGNAL ( jobDone( Job* ) ),
                      SIGNAL ( jobDone( Job* ) ) );
            connect ( th,  SIGNAL ( started ( Thread* ) ),
                      SIGNAL ( threadStarted ( Thread* ) ) );

            th->moveToThread( th ); // be sane from the start
            th->start ();
            debug ( 2, "WeaverImpl::adjustInventory: thread created, "
                    "%i threads in inventory.\n", numberOfThreads() );
	}
    }
}

Thread* WeaverImpl::createThread()
{
    return new Thread( this );
}

bool WeaverImpl::dequeue ( Job* job )
{
    bool result;
    {
        QMutexLocker l (m_mutex);

        job->aboutToBeDequeued( this );

        int i = m_assignments.indexOf ( job );
        if ( i != -1 )
        {
            m_assignments.removeAt( i );
            result = true;
            debug( 3, "WeaverImpl::dequeue: job %p dequeued, %i jobs left.\n",
                   job, m_assignments.size() );
        } else {
            debug( 3, "WeaverImpl::dequeue: job %p not found in queue.\n", job );
            result = false;
        }
    }

    // from the queues point of view, a job is just as finished if
    // it gets dequeued:
    m_jobFinished.wakeOne();
    return result;
}

void WeaverImpl::dequeue ()
{
    debug( 3, "WeaverImpl::dequeue: dequeueing all jobs.\n" );
    QMutexLocker l (m_mutex);
    for ( int index = 0; index < m_assignments.size(); ++index )
    {
        m_assignments.at( index )->aboutToBeDequeued( this );
    }
    m_assignments.clear();

    Q_ASSERT ( m_assignments.isEmpty() );
}

void WeaverImpl::suspend ()
{
    m_state->suspend();
}

void WeaverImpl::resume ( )
{
    m_state->resume();
}

void WeaverImpl::assignJobs()
{
    m_jobAvailable.wakeAll();
}

bool WeaverImpl::isEmpty() const
{
    QMutexLocker l (m_mutex);
    return  m_assignments.isEmpty();
}


void WeaverImpl::incActiveThreadCount()
{
    adjustActiveThreadCount ( 1 );
}

void WeaverImpl::decActiveThreadCount()
{
    adjustActiveThreadCount ( -1 );
    // the done job could have freed another set of jobs, and we do not know how
    // many - therefore we need to wake all threads:
    m_jobFinished.wakeAll();
}

void WeaverImpl::adjustActiveThreadCount( int diff )
{
    QMutexLocker l (m_mutex);
    m_active += diff;
    debug ( 4, "WeaverImpl::adjustActiveThreadCount: %i active threads (%i jobs"
            " in queue).\n", m_active,  queueLength() );

    if ( m_assignments.isEmpty() && m_active == 0)
    {
        P_ASSERT ( diff < 0 ); // cannot reach Zero otherwise
        emit ( finished() );
    }
}

const int WeaverImpl::activeThreadCount()
{
    return m_active;
}

Job* WeaverImpl::takeFirstAvailableJob()
{
    QMutexLocker l (m_mutex);
    Job *next = 0;
    for (int index = 0; index < m_assignments.size(); ++index)
    {
        if ( m_assignments.at(index)->canBeExecuted() )
	{
            next = m_assignments.at(index);
            m_assignments.removeAt (index);
            break;
	}
    }
    return next;
}

Job* WeaverImpl::applyForWork(Thread *th, Job* previous)
{
    if (previous)
    {   // cleanup and send events:
        decActiveThreadCount();
    }
    return m_state->applyForWork ( th,  0 );
}

void WeaverImpl::waitForAvailableJob(Thread* th)
{
    m_state->waitForAvailableJob ( th );
}

void WeaverImpl::blockThreadUntilJobsAreBeingAssigned ( Thread *th )
{
    Q_UNUSED ( th );
    debug ( 4,  "WeaverImpl::blockThread...: thread %i blocked.\n", th->id());
    emit asyncThreadSuspended ( th );
    QMutexLocker l( m_jobAvailableMutex );
    m_jobAvailable.wait( m_jobAvailableMutex );
    debug ( 4,  "WeaverImpl::blockThread...: thread %i resumed.\n", th->id());
}

int WeaverImpl::queueLength() const
{
    QMutexLocker l (m_mutex);
    return m_assignments.count();
}

bool WeaverImpl::isIdle () const
{
    QMutexLocker l (m_mutex);
    return isEmpty() && m_active == 0;
}

void WeaverImpl::finish()
{
#ifdef QT_NO_DEBUG
    const int MaxWaitMilliSeconds = 200;
#else
    const int MaxWaitMilliSeconds = 2000;
#endif

    while ( !isEmpty() )
    {
        debug (2, "WeaverImpl::finish: not done, waiting.\n" );
        QMutexLocker l( m_finishMutex );
        if ( m_jobFinished.wait( m_finishMutex, MaxWaitMilliSeconds ) == false )
	{
            debug ( 2, "WeaverImpl::finish: wait timed out, %i jobs left, waking threads.\n",
                    queueLength() );
            m_jobAvailable.wakeAll();
	}
    }
    debug (2, "WeaverImpl::finish: done.\n\n\n" );
}

void WeaverImpl::requestAbort()
{
    for ( int i = 0; i<m_inventory.size(); ++i )
    {
        m_inventory[i]->requestAbort();
    }
}

void WeaverImpl::dumpJobs()
{
    QMutexLocker l (m_mutex);
    debug( 0, "WeaverImpl::dumpJobs: current jobs:\n" );
    for ( int index = 0; index < m_assignments.size(); ++index )
    {
        debug( 0, "--> %4i: %p %s (priority %i)\n", index, m_assignments.at( index ),
               m_assignments.at( index )->metaObject()->className(),
               m_assignments.at(index)->priority() );
    }
}

#ifdef USE_CMAKE
#include "WeaverImpl.moc"
#endif
