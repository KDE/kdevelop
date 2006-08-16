/* -*- C++ -*-

   This file declares the JobCollection class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2004, 2005, 2006 Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: DebuggingAids.h 30 2005-08-16 16:16:04Z mirko $
*/

#ifndef JOBCOLLECTION_H
#define JOBCOLLECTION_H

#include "Job.h"

namespace ThreadWeaver {

    class Thread;
    class JobCollectionJobRunner;

    /** A JobCollection is a vector of Jobs that will be queued together.
     *
     * In a JobCollection, the order of execution of the elements is not guaranteed.
     *
     * It is intended that the collection is set up first and then
     * queued. After queuing, no further jobs should be added to the collection.
     */
    class JobCollection : public Job
    {
        friend class JobCollectionJobRunner;
        Q_OBJECT

    public:
        explicit JobCollection ( QObject *parent = 0 );
        ~JobCollection ();
        /** Append a job to the collection.

	To use JobCollection, create the Job objects first, add them to the
	collection, and then queue it. After the collection has been queued, no
	further Jobs are supposed to be added.
        */
        virtual void addJob ( Job* );

        /** Overload to manage recursive sets. */
        bool canBeExecuted();

    public slots:
        /** Stop processing, dequeue all remaining Jobs.
            job is supposed to be an element of the collection.
            */
        void stop ( Job *job );

    protected:
        /** Overload to queue the collection. */
        void aboutToBeQueued ( WeaverInterface *weaver );

        /** Overload to dequeue the collection. */
        void aboutToBeDequeued ( WeaverInterface *weaver );

        /** Return a reference to the job in the job list at position i. */
        Job* jobAt( int i );

        /** Return the number of jobs in the joblist. */
        const int jobListLength();

        /** Callback method for done jobs.
        */
        virtual void internalJobDone( Job* );

	/** Perform the task usually done when one individual job is
	    finished, but in our case only when the whole collection
	    is finished or partly dequeued.
	*/
	void finalCleanup();

    private:
        /** Overload the execute method. */
        void execute ( Thread * );

        /** Overload run().
            We have to. */
        void run() {}

        /** Dequeue all elements of the collection.
            Note: This will not dequeue the collection itself.
        */
        void dequeueElements();
      
      class Private;
      Private* d;

    };

}

#endif
