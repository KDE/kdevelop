/* -*- C++ -*-

   This file implements the SuspendedState class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
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

namespace ThreadWeaver {

    void SuspendedState::suspend()
    {
        // this request is not handled in Suspended state
    }

    void SuspendedState::resume()
    {
        m_weaver->setState( WorkingHard );
    }

    Job* SuspendedState::applyForWork ( Thread *th,  Job* previous )
    {   // suspend all threads in case they wake up:
        m_weaver->waitForAvailableJob( th );
        return m_weaver->applyForWork ( th,  previous );
    }

    void SuspendedState::waitForAvailableJob ( Thread *th )
    {
        m_weaver->blockThreadUntilJobsAreBeingAssigned ( th );
    }

}
