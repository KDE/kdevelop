/* -*- C++ -*-

   This file declares the ShuttingDownState class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: ShuttingDownState.h 32 2005-08-17 08:38:01Z mirko $
*/

#ifndef ShuttingDownState_H
#define ShuttingDownState_H

#include "State.h"

namespace ThreadWeaver {

    /** ShuttingDownState is enabled when the Weaver destructor is entered. It
        prevents threads from still accessing queue management methods.
    */
    class ShuttingDownState : public State
    {
    public:
	ShuttingDownState( WeaverImpl *weaver)
	    : State (weaver, ShuttingDown)
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

#endif // ShuttingDownState_H
