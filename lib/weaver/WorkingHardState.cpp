/* -*- C++ -*-

This file implements the WorkingHardState class.

$ Author: Mirko Boehm $
$ Copyright: (C) 2005, 2006 Mirko Boehm $
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

#include "Job.h"
#include "Thread.h"
#include "WeaverImpl.h"
#include "ThreadWeaver.h"
#include "DebuggingAids.h"
#include "WorkingHardState.h"


using namespace ThreadWeaver;

void WorkingHardState::activated()
{
    weaver()->assignJobs();
}

void WorkingHardState::suspend()
{
    weaver()->setState ( Suspending );
}

void WorkingHardState::resume()
{
}

Job* WorkingHardState::applyForWork ( Thread *th,  Job* )
{   // beware: this code is executed in the applying thread!
    debug ( 2, "WorkingHardState::applyForWork: thread %i applies for work "
            "in %s state.\n", th->id(),
            qPrintable ( weaver()->state().stateName() ) );

    Job *next = weaver()->takeFirstAvailableJob();

    if ( next )
    {
        weaver()->incActiveThreadCount();
        return next;
    } else {
        debug ( 2, "WorkingHardState::applyForWork: no work for thread %i, "
                "blocking it.\n", th->id() );
        weaver()->waitForAvailableJob( th );
        // this is no infinite recursion: the state may have changed
        // meanwhile, or jobs may have come available:
        return weaver()->applyForWork( th,  0 );
    }
}

void WorkingHardState::waitForAvailableJob ( Thread *th )
{
    weaver()->blockThreadUntilJobsAreBeingAssigned ( th );
}

StateId WorkingHardState::stateId() const
{
    return WorkingHard;
}
