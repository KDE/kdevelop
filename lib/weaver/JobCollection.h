#ifndef JOBCOLLECTION_H
#define JOBCOLLECTION_H

#include "Job.h"

namespace ThreadWeaver {

    class Thread;

    /** A JobCollection is a vector of Jobs that will be queued together.
     *
     * In a JobCollection, the order of execution of the elements is not guaranteed.
     *
     * It is intended that the collection is set up first and then
     * queued. After queuing, no further jobs should be added to the collection.
     */
    class KDEVWEAVER_EXPORT JobCollection : public Job
    {
        Q_OBJECT

    public:
        explicit JobCollection ( QObject *parent );
        ~JobCollection ();
        /** Append a job to the collection.

	To use JobCollection, create the Job objects first, add them to the
	collection, and then queue it. After the collection has been queued, no
	further Jobs are supposed to be added.
        */
        virtual void addJob ( Job* );

        /** Overload to manage recursive sets. */
        bool hasUnresolvedDependencies();


    public slots:
        /** Stop processing, dequeue all remaining Jobs.
            job is supposed to be an element of the collection.
            FIXME the job parameter is not necessary anymore
            */
        void stop ( Job *job );

    protected:
        /** Overload to queue the collection. */
        void aboutToBeQueued ( WeaverInterface *weaver );

        /** Return a reference to the job in the job list at position i. */
        Job* jobAt( int i );

        /** Return the number of jobs in the joblist. */
        const int jobListLength();

    private:
        /** Overload the execute method. */
        void execute ( Thread * );

        /** Overload run().
            We have to. */
        void run() {}

        /** The elements of the collection. */
        class JobList;
        JobList* m_elements;

        /** True if this collection has been queued in the Job queue of a
            Weaver. */
        bool m_queued;

        /** A guard job used to manage recursive dependencies. */
        Job* m_guard;
        /** The Weaver interface this collection is queued in. */
        WeaverInterface *m_weaver;
    };

}

#endif
