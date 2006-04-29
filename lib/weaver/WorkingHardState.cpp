/* -*- C++ -*-

   This file implements the WorkingHardState class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: WorkingHardState.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include <QString>
#include <QByteArray>

#include "ThreadWeaver.h"
#include "Job.h"
#include "Thread.h"
#include "WeaverImpl.h"
#include "DebuggingAids.h"
#include "WorkingHardState.h"


namespace ThreadWeaver {

    void WorkingHardState::activated()
    {
        m_weaver->assignJobs();
    }

    void WorkingHardState::suspend()
    {
        m_weaver->setState ( Suspending );
    }

    void WorkingHardState::resume()
    {

    }

    Job* WorkingHardState::applyForWork ( Thread *th,  Job* )
    {   // beware: this code is executed in the applying thread!
        debug ( 2, "WorkingHardState::applyForWork: thread %i applies for work "
                "in %s state.\n", th->id(),
                qPrintable ( m_weaver->state().stateName() ) );

        Job *next = m_weaver->takeFirstAvailableJob();

        if ( next )
        {
            m_weaver->incActiveThreadCount();
            return next;
        } else {
            debug ( 2, "WorkingHardState::applyForWork: no work for thread %i, "
                    "blocking it.\n", th->id() );
            m_weaver->waitForAvailableJob( th );
            // this is no infinite recursion: the state may have changed
            // meanwhile, or jobs may have come available:
            return m_weaver->applyForWork( th,  0 );
        }
    }

    void WorkingHardState::waitForAvailableJob ( Thread *th )
    {
        m_weaver->blockThreadUntilJobsAreBeingAssigned ( th );
    }

}
