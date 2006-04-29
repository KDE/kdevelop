/* -*- C++ -*-

   This file declares the Job class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2004, 2005 Mirko Boehm $
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

#include <QList>
#include <QObject>
#include "kdevexport.h"

class QMutex;
class QWaitCondition;

namespace ThreadWeaver {

    class Thread;
    class WeaverInterface;
    class JobRunHelper;
    class JobMultiMap;

    /** A Job is a simple abstraction of an action that is to be
        executed in a thread context.
	It is essential for the ThreadWeaver library that as a kind of
        convention, the different creators of Job objects do not touch the
        protected data members of the Job until somehow notified by the
        Job.

	Also, please note that Jobs may not be executed twice. Create two
	different objects to perform two consecutive or parallel runs.

	Jobs may declare dependencies. If Job B depends on Job A, B may not be
	executed before A is finished.

    */

    class KDEVWEAVER_EXPORT Job : public QObject
    {
        Q_OBJECT
    public:
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

            @param weaver the Weaver object the job will be queued in
            */
        virtual void aboutToBeQueued ( WeaverInterface *weaver );

        /** Returns true if the jobs's execute method finished. */
        bool isFinished() const { return m_finished; }

	/** Process events related to this job (created by the processing
	    thread or the weaver or whoever). */
	// virtual void processEvent ( Event* );

        /** Add a dependency.
            The object given will be added as a dependency. The Job will not
            be executed until all dependencies have been processed.
            The job is automatically added to the dependency as a dependent.
            @param dependency: the other job this job depends on
        */
        void addDependency (Job* dependency);

        /** Remove dependency.
            The given dependency will be removed. If none are left, the job
            will be executed as soon as a waiting thread is available.
            The job will automatically be removed as a dependent of dep.

            Returns false if the given object is not dependency of this job.

	    This function is inefficient, and should be used only to abort
	    execution of a job.

	    @param dep the dependency that will be removed
        */
        bool removeDependency (Job *dep);

        /** Retrieve a list of dependencies of this job. */
        QList<Job*> getDependencies() const;

        /** Query whether the job has an unresolved dependency.
            In case it does, it will not be processed by a thread trying to
            request a job. */
        virtual bool hasUnresolvedDependencies();

    signals:
	/** This signal is emitted when this job is being processed by a
	    thread. */
	void started ( Job* );
	/** This signal is emitted when the job has been finished. */
	void done ( Job* );

    protected:
        friend class JobRunHelper;

        /** The method that actually performs the job. It is called from
            execute(). This method is the one to overload it with the
            job's task. */
        virtual void run () = 0;
	/** Return the thread that executes this job.
	    Returns zero of the job is not currently executed.

	    Do not confuse with QObject::thread() const !
	    //  @TODO rename to executingThread()
	    */
	inline Thread *thread() { return m_thread; }
	/** Call with status = true to mark this job as done. */
	inline void setFinished ( bool status ) { m_finished = status; }
        /** Resolve all dependencies.
            This method is called after the Job has been finished, or
            when it is deleted without being executed (performed by the
            destructor).
            The method will remove all entries stating that another Job
            depends on this one.
        */
        virtual void resolveDependencies();

        Thread * m_thread;

//         QMutex *m_wcmutex;
// 	QWaitCondition *m_wc;

	/** A container to keep track of Job dependencies.
	    For each dependency A->B, which means Job B depends on Job A and
	    may only be executed after A has been finished, an entry will be
	    added with key A and value B. When A is finished, the entry will
	    be removed. */
        static JobMultiMap* sm_dep();
//         static QMultiMap<Job*, Job*> *sm_dep();
	static QMutex *sm_mutex;
    private:
	QMutex *m_mutex;
	/** m_finished is set to true when the Job has been executed. */
        bool m_finished;

    public:
        /** This method should be useful for debugging purposes. */
        static void DumpJobDependencies();
    };

}

#endif // THREADWEAVER_JOB_H

