/* -*- C++ -*-

   This file implements the public interfaces of the WeaverImpl class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: WeaverImpl.h 32 2005-08-17 08:38:01Z mirko $
*/
#ifndef WeaverImpl_H
#define WeaverImpl_H

// #ifndef THREADWEAVER_H
// #error WeaverImpl.h is not supposed to be included directly, use ThreadWeaver.h!
// #endif

#include <QObject>
#include <QWaitCondition>

#include "State.h"
#include "WeaverInterface.h"

namespace ThreadWeaver {

    class Job;
    class Thread;
    class WeaverObserver;

    /** A WeaverImpl is the manager of worker threads (Thread objects) to
        which it assigns jobs from it's queue. It extends the API of
        WeaverInterface to provide additional methods needed by the Thread
        objects. */
    class WeaverImpl : public WeaverInterface
    {
        Q_OBJECT
    public:
	/** Construct a WeaverImpl object. */
        WeaverImpl (QObject* parent=0,
                int inventoryMin = 4, // minimal number of provided threads
                int inventoryMax = 32); // maximum number of provided threads
	/** Destruct a WeaverImpl object. */
        virtual ~WeaverImpl ();
	const State& state() const;
        /** Set the object state. */
        void setState( StateId );
        void registerObserver ( WeaverObserver* );
        virtual void enqueue (Job*);
	virtual void enqueue (const QList<Job*>& jobs);
        virtual bool dequeue (Job*);
        virtual void dequeue ();
	virtual void finish();
        virtual void suspend( );
        virtual void resume();
        bool isEmpty ();
	bool isIdle ();
        int queueLength ();
        /** Assign a job to the calling thread.
	    This is supposed to be called from the Thread objects in
	    the inventory. Do not call this method from your code.

	    Returns 0 if the weaver is shutting down, telling the
	    calling thread to finish and exit.
            If no jobs are available and shut down is not in progress,
            the calling thread is suspended until either condition is
            met.
	    In *previous*, threads give the job they have completed. If this is
	    the first job, previous is zero. */
        virtual Job* applyForWork (Thread *thread, Job *previous);
        /** Blocks the calling thread until some actor calls assignJobs. */
        void blockThreadUntilJobsAreBeingAssigned(Thread *th);
        /** Wait for a job to become available. */
	void waitForAvailableJob(Thread *th);
        /** Increment the count of active threads. */
        void incActiveThreadCount();
        /** Decrement the count of active threads. */
        void decActiveThreadCount();
        /** Returns the number of active threads.
            Threads are active if they process a job.
        */
        const int activeThreadCount();
        /** Take the first available job out of the queue and return it.
            The job will be removed from the queue (therefore, take).
            Only jobs that have no unresolved dependancies are considered
	    available. If only jobs that depened on other, unfinished jobs are
	    in the queue, this method returns a nil pointer. */
        Job* takeFirstAvailableJob();
        /** Schedule enqueued jobs to be executed by idle threads.
            This will try to distribute as many jobs as possible
            to all idle threads. */
        void assignJobs();
	int noOfThreads ();
        void requestAbort();
    signals:
        /** A Thread has been created. */
        void threadStarted ( Thread* );
        /** A thread has exited. */
        void threadExited ( Thread* );
        /** A thread has been suspended. */
        void threadSuspended ( Thread* );
        /** The thread is busy executing job j. */
        void threadBusy ( Thread*,  Job* j);
        /** The Weaver's state has changed. */
        void stateChanged ( State* );
    protected:
        /** Adjust active thread count.
            This is a helper function for incActiveThreadCount and decActiveThreadCount. */
        void adjustActiveThreadCount ( int diff );
        /** Factory method to create the threads.
            Overload in adapted Weaver implementations.
        */
        virtual Thread* createThread();
        /** Adjust the inventory size.

	    This method creates threads on demand. Threads in the inventory
	    are not created upon construction of the WeaverImpl object, but
	    when jobs are queued. This avoids costly delays on the application
	    startup time. Threads are created when the inventory size is under
	    inventoryMin and new jobs are queued.
	    */
        // @TODO: add code to raise inventory size over inventoryMin
        // @TODO: add code to quit unnecessary threads
        void adjustInventory ( int noOfNewJobs );
	/** Lock the mutex for this weaver. The threads in the
	    inventory need to lock the weaver's mutex to synchronize
	    the job management. */
	void lock ();
	/** Unlock. See lock(). */
	void unlock ();
        /** The thread inventory. */
        QList<Thread*> m_inventory;
        /** The job queue. */
        QList<Job*> m_assignments;
	/** The number of jobs that are assigned to the worker
	    threads, but not finished. */
	int m_active;
        /** Stored setting. */
        int m_inventoryMin;
        /** Stored setting . */
        int m_inventoryMax;
        /** Wait condition all idle or done threads wait for. */
        QWaitCondition m_jobAvailable;
	/** Wait for a job to finish. */
	QWaitCondition m_jobFinished;
    private:
	/** Mutex to serialize operations. */
	QMutex *m_mutex;
        // @TODO: make state objects static
	/** The state of the art.
         * @see StateId
	*/
	State*  m_state;
        /** The state objects. */
        State *m_states[NoOfStates];
    };

} // namespace ThreadWeaver

#endif // WeaverImpl_H
