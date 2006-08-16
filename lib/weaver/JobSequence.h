/* -*- C++ -*-

   This file declares the JobSequence class.

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
        explicit JobSequence ( QObject *parent = 0 );

    protected:
        /** Overload to queue the sequence. */
        void aboutToBeQueued ( WeaverInterface *weaver );

        /** reimplemented */
        void internalJobDone( Job* );
    };

}

#endif
