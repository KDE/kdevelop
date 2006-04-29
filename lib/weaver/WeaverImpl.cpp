/* -*- C++ -*-

   This file implements the WeaverImpl class.


   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
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

#include "ThreadWeaver.h"
#include "WeaverObserver.h"
#include "WeaverImpl.h"
#include "Thread.h"
#include "Job.h"
#include "DebuggingAids.h"
#include "State.h"
#include "DestructedState.h"
#include "InConstructionState.h"
#include "ShuttingDownState.h"
#include "SuspendedState.h"
#include "SuspendingState.h"
#include "WorkingHardState.h"

namespace ThreadWeaver {

    WeaverImpl::WeaverImpl(QObject* parent, int inventoryMin, int inventoryMax)
        : WeaverInterface(parent),
          m_active(0),
          m_inventoryMin(inventoryMin),
          m_inventoryMax(inventoryMax),
          m_mutex ( new QMutex( QMutex::Recursive ) ),
          m_state (0)
    {
        // initialize state objects:
        m_states[InConstruction] = new InConstructionState( this );
        setState ( InConstruction );
        m_states[WorkingHard] = new WorkingHardState( this );
        m_states[Suspending] = new SuspendingState( this );
        m_states[Suspended] = new SuspendedState( this );
        m_states[ShuttingDown] = new ShuttingDownState( this );
        m_states[Destructed] = new DestructedState( this );

	setState(  WorkingHard );
    }

    WeaverImpl::~WeaverImpl()
    {
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
	debug ( 3, "WeaverImpl dtor: done\n" );
	setState ( Destructed ); // m_state = Halted;
        // @TODO: delete state objects. what sense does DestructedState make then?
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

    void WeaverImpl::lock()
    {
        m_mutex->lock();
    }

    void WeaverImpl::unlock()
    {
        m_mutex->unlock();
    }

    int WeaverImpl::noOfThreads ()
    {
        QMutexLocker l (m_mutex);
        return m_inventory.count ();
    }

    void WeaverImpl::enqueue(Job* job)
    {
        adjustInventory ( 1 );
	if (job)
	{
            job->aboutToBeQueued ( this );
	    {
		QMutexLocker l (m_mutex);
		m_assignments.append(job);
	    }
	    assignJobs();
	}
    }

    void WeaverImpl::enqueue (const QList <Job*>& jobs)
    {
        adjustInventory ( jobs.size() );
	if (!jobs.isEmpty())
	{
            for ( int i = 0; i < jobs.size(); ++i )
            {
                jobs[i]->aboutToBeQueued ( this );
            }

	    {
		QMutexLocker l (m_mutex);
		m_assignments << jobs;
	    }
	    assignJobs();
	}
    }

    void WeaverImpl::adjustInventory ( int noOfNewJobs )
    {
        // no of threads that can be created:
        const int reserve = m_inventoryMin - noOfThreads();
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

                th->start ();
                debug ( 2, "WeaverImpl::adjustInventory: thread created, "
                        "%i threads in inventory.\n", noOfThreads() );
            }
        }
    }

    Thread* WeaverImpl::createThread()
    {
        return new Thread ( this );
    }

    bool WeaverImpl::dequeue ( Job* job )
    {
        QMutexLocker l (m_mutex);
        if ( int i = m_assignments.indexOf ( job ) )
        {
            m_assignments.removeAt( i );
            return true;
        } else {
            return false;
        }
    }

    void WeaverImpl::dequeue ()
    {
        QMutexLocker l (m_mutex);
        m_assignments.clear();
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

    bool WeaverImpl::isEmpty()
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
        m_jobFinished.wakeOne();
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
            if ( ! m_assignments.at(index)->hasUnresolvedDependencies() )
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
        debug ( 3,  "WeaverImpl::blockThread...: thread %i blocked.\n", th->id());
        emit ( threadSuspended ( th ) );
	QMutex mutex;
	mutex.lock();
	m_jobAvailable.wait( &mutex );
	mutex.unlock();
        debug ( 3,  "WeaverImpl::blockThread...: thread %i resumed.\n", th->id());
    }

    int WeaverImpl::queueLength()
    {
        QMutexLocker l (m_mutex);
        return m_assignments.count();
    }

    bool WeaverImpl::isIdle ()
    {
        QMutexLocker l (m_mutex);
        return isEmpty() && m_active == 0;
    }

    void WeaverImpl::finish()
    {
        while ( !isIdle() )
        {
            debug (2, "WeaverImpl::finish: not done, waiting.\n" );
            QMutex mutex;
            mutex.lock();
            m_jobFinished.wait( &mutex );
            mutex.unlock();
        }
	debug (1, "WeaverImpl::finish: done.\n\n\n" );
    }

    void WeaverImpl::requestAbort()
    {
        for ( int i = 0; i<m_inventory.size(); ++i )
        {
            m_inventory[i]->requestAbort();
        }
    }

}
