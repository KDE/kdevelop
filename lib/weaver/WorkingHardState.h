/* -*- C++ -*-

   This file declares the WorkingHardState class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: WorkingHardState.h 32 2005-08-17 08:38:01Z mirko $
*/

#ifndef WorkingHardState_H
#define WorkingHardState_H

#include "State.h"

namespace ThreadWeaver {

    /* WorkingHardState handles the state dependent calls in the state where
       jobs are executed.  */
    class WorkingHardState : public State
    {
    public:
	explicit WorkingHardState( WeaverImpl *weaver)
	    : State (weaver, WorkingHard)
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
	/** Overload. */
	void activated();
    };

}

#endif // WorkingHardIState_H
