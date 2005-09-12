/* -*- C++ -*-

   This file declares the WeaverInterface class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: WeaverImpl.h 29 2005-08-14 19:04:30Z mirko $
*/

#ifndef WeaverInterface_H
#define WeaverInterface_H

#include <QObject>
#include <QList>

#include "State.h"

namespace ThreadWeaver {

    class WeaverObserver;
    class Job;

    /** WeaverInterface provides a common interface for weaver
        implementations, to be used for example in adapters and
        decorators. The member documentation is provided in the Weaver and
        WeaverImpl classes.
    */

    class WeaverInterface : public QObject {
        Q_OBJECT
    public:
        WeaverInterface ( QObject* parent = 0 );
        virtual ~WeaverInterface() {}
        /** Return the state of the weaver object. */
        virtual const State& state() const = 0;
        /** Register an observer.

            Observers provides signals on different weaver events that are
            otherwise only available through objects of different classes
            (threads, jobs). Usually, access to the signals of those objects
            is not provided through the weaver API. Use an observer to reveice
            notice, for example, on thread activity.

            To unregister, simply delete the observer.
         */
        virtual void registerObserver ( WeaverObserver* ) = 0;
        /** Add a job to be executed.

            It depends on the state if execution of the job will be attempted
            immidiately. In suspended state, jobs can be added to the queue,
            but the threads remain suspended. In WorkongHard state, an idle
            thread may immidiately execute the job, or it might be queued if
            all threads are busy.
        */
        virtual void enqueue ( Job* ) = 0;
        /** Enqueue all jobs in the given list.
	    This is an atomic operation, no jobs will start
	    before all jobs in the list are enqueued.
	    If you need a couple of jobs done and want to receive the
	    finished () signal afterwards, use this method to queue
	    them. Otherwise, when enqueueing your jobs
	    individually, there is a chance that you receive more than
	    one finished signal. */
        virtual void enqueue (const QList<Job*>& jobs) = 0;
        /** Remove a job from the queue.
            If the job was queued but not started so far, it is simple
            removed from the queue. For now, it is unsupported to
            dequeue a job once its execution has started.
	    For that case, you will have to provide a method to interrupt your
	    job's execution (and receive the done signal).
            Returns true if the job has been dequeued, false if the
            job has already been started or is not found in the
            queue. */
        virtual bool dequeue ( Job* ) = 0;
        /** Remove all queued jobs.
            Please note that this will not kill the threads, therefore
            all jobs that are being processed will be continued. */
        virtual void dequeue () = 0;
	/** Finish all queued operations, then return.

	    This method is used in imperative (not event driven) programs that
	    cannot react on events to have the controlling (main) thread wait
	    wait for the jobs to finish. The call will block the calling
	    thread and return when all queued jobs have been processed.

	    Warning: This will suspend your thread!
	    Warning: If one of your jobs enters an infinite loop, this
	             will never return! */
        virtual void finish () = 0;
        /** Suspend job execution.
            When suspending, all threads are allowed to finish the
            currently assigned job but will not receive a new
            assignment.
            When all threads are done processing the assigned job, the
            signal suspended will() be emitted.
            If you call suspend() and there are no jobs left to
            be done, you will immidiately receive the suspended()
            signal. */
        virtual void suspend () = 0;
        /** Resume job queueing.
            @see suspend
        */
        virtual void resume () = 0;
        /** Is the queue empty?
	    The queue is empty if no more jobs are queued. */
        virtual bool isEmpty () = 0;
	/** Is the weaver idle?
	    The weaver is idle if no jobs are queued and no jobs are processed
            by the threads (m_active is zero). */
        virtual bool isIdle () = 0;
	/** Returns the number of pending jobs.
            This will return the number of queued jobs. Jobs that are
	    currently being executed are not part of the queue. All jobs in
	    the queue are waiting to be executed.
        */
        virtual int queueLength () = 0;
	/** Returns the current number of threads in the inventory. */
        virtual int noOfThreads () = 0;
        /** Request aborts of the currently executed jobs.
            It is important to understand that aborts are requested, but
	    cannot be guaranteed, as not all Job classes support it. It is up
	    to the application to decide if and how job aborts are
	    necessary. */
        virtual void requestAbort() = 0;
    signals:
	/** This signal is emitted when the Weaver has finished ALL currently
	    queued jobs.
	    If a number of jobs is enqueued sequentially, this signal might be
	    emitted a couple of times (what happens is that all already queued
	    jobs have been processed while you still add new ones). This is
	    not a bug, but the intended behaviour. */
        void finished ();
        /** Thread queueing has been suspended.
            When suspend is called with state = true, all threads are
            allowed to finish their job. When the last thread
            finished, this signal is emitted. */
        void suspended ();
	/** This signal is emitted when a job is done.
	    The Job object emits a signal jobDone(), too, when the individual
	    job has finished.

	    This signal here is emitted by the Weaver object for
	    each single job that is finished.

	    It is up to the
	    programmer to decide if this signal or the done signal of the job
	    is more handy. */
        void jobDone (Job*);
    };

}

#endif
