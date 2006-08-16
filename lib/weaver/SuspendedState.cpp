/* -*- C++ -*-

This file implements the SuspendedState class.

$ Author: Mirko Boehm $
$ Copyright: (C) 2005, 2006 Mirko Boehm $
$ Contact: mirko@kde.org
http://www.kde.org
http://www.hackerbuero.org $
$ License: LGPL with the following explicit clarification:
This code may be linked against any version of the Qt toolkit
from Trolltech, Norway. $

$Id: SuspendedState.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include "ThreadWeaver.h"
#include "WeaverImpl.h"
#include "SuspendedState.h"

using namespace ThreadWeaver;

void SuspendedState::suspend()
{
    // this request is not handled in Suspended state
}

void SuspendedState::resume()
{
    weaver()->setState( WorkingHard );
}

Job* SuspendedState::applyForWork ( Thread *th,  Job* previous )
{   // suspend all threads in case they wake up:
    weaver()->waitForAvailableJob( th );
    return weaver()->applyForWork ( th,  previous );
}

void SuspendedState::waitForAvailableJob ( Thread *th )
{
    weaver()->blockThreadUntilJobsAreBeingAssigned ( th );
}

StateId SuspendedState::stateId() const
{
    return Suspended;
}
