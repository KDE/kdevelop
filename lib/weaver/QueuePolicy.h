#ifndef QUEUEPOLICY_H
#define QUEUEPOLICY_H

namespace ThreadWeaver {

    class Job;

/** QueuePolicy is an interface for customizations of the queueing behaviour
    of sets of jobs.

    A job can have a queueing policy assigned. In that case, the job is only
    executed when the method canRun() of the assigned policy returns true. For
    every call to canRun() that returns true, it is guaranteed that the method
    free() or the method release() is called. Calling free() means the job has
    been executed, while calling release() means the job was not executed for
    external reasons, and will be retried lateron.

    As an example, dependencies can be implemented using a QueuePolicy:
    canRun() returns true when the job has no unresolved dependencies. free()
    and release() are empty.

    A job can have multiple queue policies assigned, and will only be executed
    if all of them return true from canRun() within the same execution attempt.
    Jobs only keep a reference to the QueuePolicy. Therefore, the same object
    can be assigned to multiple jobs and this way control the way all those
    jobs are executed. Jobs never assume ownership of their assigned queue
    policies.
*/
    class QueuePolicy
    {
    public:
        virtual ~QueuePolicy() {}

        /** canRun() is called before the job is executed.
            The job will only be executed if canRun() returns true.
        */
        virtual bool canRun( Job* ) = 0;

        /** free() is called after the job has been executed.
            It is guaranteed that release is called only after canRun()
            returned true at an earlier time.
        */
        virtual void free( Job* ) = 0;

        /** release() is called if canRun() returned true, but the job has not
            been executed for external reasons. For example, a second
            QueuePolicy could have returned false from canRun() for the same
            job.
        */
        virtual void release( Job* ) = 0;

        /** destructing() is called when a Job that has this queue policy
            assigned gets destructed.
        */
        virtual void destructed ( Job* ) = 0;
    };

}

#endif
