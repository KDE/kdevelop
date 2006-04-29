#ifndef JOBSEQUENCE_H
#define JOBSEQUENCE_H

#include "JobCollection.h"

namespace ThreadWeaver {

    /** A JobSequence is a vector of Jobs that will be executed in a sequence.

        It is implemented by automatically creating the necessary dependencies
        between the Jobs in the sequence.

        JobSequence provides a handy cleanup and unwind mechanism: the stop()
        slot. If it is called, the processing of the sequence will stop, and
        all it's remaining Jobs will be dequeued.
    */
    class JobSequence : public JobCollection
    {
        Q_OBJECT

    public:
        explicit JobSequence ( QObject *parent );

    protected:
        /** Overload to queue the sequence. */
        void aboutToBeQueued ( WeaverInterface *weaver );
    };

}

#endif
