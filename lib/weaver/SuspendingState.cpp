/* -*- C++ -*-

   This file implements the SuspendingState class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, 2006 Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: SuspendingState.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include "State.h"
#include "WeaverImpl.h"
#include "ThreadWeaver.h"
#include "SuspendingState.h"

using namespace ThreadWeaver;

void SuspendingState::suspend()
{
    // this request is not handled in Suspending state (we are already
    // suspending...)
}

void SuspendingState::resume()
{
    weaver()->setState ( WorkingHard );
}

void SuspendingState::activated()
{
    if ( weaver()->activeThreadCount() == 0 )
    {
        weaver()->setState( Suspended );
    }
}

Job* SuspendingState::applyForWork ( Thread *th,  Job* previous )
{
    if ( weaver()->activeThreadCount() == 0 )
    {
        weaver()->setState ( Suspended );
    }
    weaver()->waitForAvailableJob ( th );
    return weaver()->applyForWork ( th,  previous );
}

void SuspendingState::waitForAvailableJob ( Thread *th )
{
    weaver()->blockThreadUntilJobsAreBeingAssigned( th );
}

StateId SuspendingState::stateId() const
{
    return Suspending;
}
