/* -*- C++ -*-

This file declares the Job class.

$ Author: Mirko Boehm $
$ Copyright: (C) 2004, 2005, 2006 Mirko Boehm $
$ Contact: mirko@kde.org
http://www.kde.org
http://www.hackerbuero.org $
$ License: LGPL with the following explicit clarification:
This code may be linked against any version of the Qt toolkit
from Troll Tech, Norway. $

$Id: Job.h 32 2005-08-17 08:38:01Z mirko $
*/

#ifndef THREADWEAVER_JOB_H
#define THREADWEAVER_JOB_H

#include <QObject>

class QMutex;
class QWaitCondition;

namespace ThreadWeaver {

    class Thread;
    class QueuePolicy;
    class JobRunHelper;
    class WeaverInterface;
    class QueuePolicyList;

    /** A Job is a simple abstraction of an action that is to be
        executed in a thread context.
	It is essential for the ThreadWeaver library that as a kind of
        convention, the different creators of Job objects do not touch the
        protected data members of the Job until somehow notified by the
        Job.

	Also, please note that Jobs may not be executed twice. Create two
	different objects to perform two consecutive or parallel runs.

	Jobs may declare dependencies. If Job B depends on Job A, B may not be
	executed before A is finished. To learn about dependencies, see
	DependencyPolicy.
    */

    class Job : public QObject
    {
        Q_OBJECT

    public:
        friend class JobRunHelper;

        /** Construct a Job.

        @param parent the parent QObject
        */
        explicit Job ( QObject* parent = 0 );

	/** Destructor. */
        virtual ~Job();

	/** Perform the job. The thread in which this job is executed
	    is given as a parameter.
            Do not overload this method to create your own Job
            implementation, overload run(). */
        virtual void execute(Thread*);

        /** The queueing priority of the job.
            Jobs will be sorted by their queueing priority when
            enqueued. A higher queueing priority will place the job in
            front of all lower-priority jobs in the queue.

            Note: A higher or lower priority does not influence queue
            policies. For example, a high-priority job that has an
            unresolved dependency will not be executed, which means an
            available lower-priority job will take precedence.

            The default implementation returns zero. Only if this method
            is overloaded for some job classes, priorities will
            influence the execution order of jobs.
        */
        virtual int priority() const;

        /** Return whether the Job finished successfully or not.
            The default implementation simply returns true. Overload in
            derived classes if the derived Job class can fail.

            If a job fails (success() returns false), it will *NOT* resolve
            it's dependencies when it finishes. This will make sure that Jobs
            that depend on the failed job will not be started.

            There is an important gotcha: When a Job object is deleted, it
            will always resolve it's dependencies. If dependent jobs should
            not be executed after a failure, it is important to dequeue those
            befor deleting the failed Job.

            A JobSequence may be helpful for that purpose.
        */
        virtual bool success () const { return true; }

        /** Abort the execution of the job.
            Call this method to ask the Job to abort if it is currently executed.
            Please note that the default implementation of the method does
            nothing (!). This is due to the fact that there is no generic
            method to abort a Job processing. Not even a default boolean flag
            makes sense, as Job could, for example, be in an event loop and
            will need to create an exit event.
            You have to reimplement the method to actually initiate an abort
            action.
            The method is not pure virtual because users are not supposed to
            be forced to always implement requestAbort().
            Also, this method is supposed to return immidiately, not after the
            abort has completed. It requests the abort, the Job has to act on
            the request. */
        virtual void requestAbort () {}

        /** The job is about to be added to the weaver's job queue.
            The job will be added right after this method finished. The
            default implementation does nothing.
            Use this method to, for example, queue sub-operations as jobs
            before the job itself is queued.

            Note: When this method is called, the associated Weaver object's
            thread holds a lock on the weaver's queue. Therefore, it is save
            to assume that recursive queueing is atomic from the queues
            perspective.

            @param weaver the Weaver object the job will be queued in
        */
        virtual void aboutToBeQueued ( WeaverInterface *weaver );

        /** This Job is about the be dequeued from the weaver's job queue.
            The job will be removed from the queue right after this method
            returns.
            Use this method to dequeue, if necessary,  sub-operations (joobs) that this job
            has enqueued.

            Note: When this method is called, the associated Weaver object's
            thread does hold a lock on the weaver's queue.

            Note: The default implementation does nothing.

            @param weaver the Weaver object from which the job will be dequeued
        */
        virtual void aboutToBeDequeued ( WeaverInterface *weaver );

        /** canBeExecuted() returns true if all the jobs queue policies agree to it.
            If it returns true, it expects that the job is executed right
            after that. The done() methods of the queue policies will be
            automatically called when the job is finished.

            If it returns false, all queue policy resources have been freed,
            and the method can be called again at a later time.
        */
        virtual bool canBeExecuted();

        /** Returns true if the jobs's execute method finished. */
        bool isFinished() const;

        /** Assign a queue policy.
            Queue Policies customize the queueing (running) behaviour of sets
            of jobs. Examples for queue policies are dependencies and resource
            restrictions.
            Every queue policy object can only be assigned once to a job,
            multiple assignments will be IGNORED.
        */
        void assignQueuePolicy ( QueuePolicy* );

        /** Remove a queue policy from this job.
         */
        void removeQueuePolicy ( QueuePolicy* );

    signals:
	/** This signal is emitted when this job is being processed by a
	    thread. */
	void started ( Job* );
	/** This signal is emitted when the job has been finished. */
	void done ( Job* );

        /** This job has failed.
            This signal is emitted when success() returns false after the job
            is executed.
        */
        void failed( Job* );

    protected:
        class Private;
        Private* d;

        /** Free the queue policies acquired before this job has been
            executed. */
        void freeQueuePolicyResources();

        /** The method that actually performs the job. It is called from
            execute(). This method is the one to overload it with the
            job's task. */
        virtual void run () = 0;
	/** Return the thread that executes this job.
	    Returns zero of the job is not currently executed.

	    Do not confuse with QObject::thread() const !
	    //  @todo rename to executingThread()
	    */
        Thread *thread();

	/** Call with status = true to mark this job as done. */
        void setFinished ( bool status );

        /** The mutex used to protect this job. */
        // QMutex& mutex();

    };
}

#endif // THREADWEAVER_JOB_H

