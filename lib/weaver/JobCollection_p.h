#ifndef JOBCOLLECTION_P_H
#define JOBCOLLECTION_P_H

#include "JobCollection.h"

namespace ThreadWeaver {

    class JobCollectionNullJob : public Job
    {
        Q_OBJECT
    public:
        JobCollectionNullJob( QObject* parent );

    private:
        void run();
    };

    /* QPointers are used internally to be able to dequeue jobs at destruction
       time. The owner of the jobs could have deleted them in the meantime.
       We use a class instead of a typedef to be able to forward-declare the
       class in the declaration.
    */
    class JobCollectionJobRunner : public Job
    {
        Q_OBJECT

    public:
        JobCollectionJobRunner ( Job* guard, Job* payload, QObject* parent );

        bool hasUnresolvedDependencies();

        Job* payload ();

        void aboutToBeQueued ( WeaverInterface *weaver );

        void execute ( Thread *t );

    private:
        void run ();

        Job* m_guard;
        QPointer<Job> m_payload;
    };

}

#endif
