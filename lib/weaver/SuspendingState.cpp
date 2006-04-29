/* -*- C++ -*-

   This file implements the SuspendingState class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: SuspendingState.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include "ThreadWeaver.h"
#include "WeaverImpl.h"
#include "SuspendingState.h"

namespace ThreadWeaver {

    void SuspendingState::suspend()
    {
        // this request is not handled in Suspending state (we are already
        // suspending...)
    }

    void SuspendingState::resume()
    {
        m_weaver->setState ( WorkingHard );
    }

    void SuspendingState::activated()
    {
        if ( m_weaver->activeThreadCount() == 0 )
        {
            m_weaver->setState( Suspended );
        }
    }

    Job* SuspendingState::applyForWork ( Thread *th,  Job* previous )
    {
        if ( m_weaver->activeThreadCount() == 0 )
        {
            m_weaver->setState ( Suspended );
        }
        m_weaver->waitForAvailableJob ( th );
        return m_weaver->applyForWork ( th,  previous );
    }

    void SuspendingState::waitForAvailableJob ( Thread *th )
    {
        m_weaver->blockThreadUntilJobsAreBeingAssigned( th );
    }

}
