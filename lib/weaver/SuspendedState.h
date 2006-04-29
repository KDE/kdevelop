/* -*- C++ -*-

   This file declares the SuspendedState class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: SuspendedState.h 32 2005-08-17 08:38:01Z mirko $
*/

#ifndef SuspendedState_H
#define SuspendedState_H

#include "State.h"

namespace ThreadWeaver {

    /** In SuspendedState, jobs are queued, but will not be executed. All
        thread remains blocked.  */
    class SuspendedState : public State
    {
    public:
	explicit SuspendedState( WeaverImpl *weaver)
	    : State (weaver, Suspended)
	    {
	    }
	/** Suspend job processing. */
        virtual void suspend();
        /** Resume job processing. */
        virtual void resume();
        /** Assign a job to an idle thread. */
        virtual Job* applyForWork ( Thread *th,  Job* previous );
        /** Wait (by suspending the calling thread) until a job becomes available. */
        virtual void waitForAvailableJob ( Thread *th );
    };

}

#endif // SuspendedState_H
