/* -*- C++ -*-

   This file declares the DestructedState class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, 2006 Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: DestructedState.h 32 2005-08-17 08:38:01Z mirko $
*/

#ifndef DestructedState_H
#define DestructedState_H

#ifndef THREADWEAVER_PRIVATE_API
#define THREADWEAVER_PRIVATE_API
#endif

#include "StateImplementation.h"

namespace ThreadWeaver {

    /** DestructedState is only active after the thread have been destroyed by
        the destructor, but before superclass destructors have finished.
    */
    class DestructedState : public StateImplementation
    {
    public:
	explicit DestructedState( WeaverInterface *weaver)
	    : StateImplementation (weaver)
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

        /** reimpl */
        StateId stateId() const;
    };

}

#endif // DestructedState_H
