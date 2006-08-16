/* -*- C++ -*-

   This file implements the JobSequence class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2004, 2005, 2006 Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: DebuggingAids.h 30 2005-08-16 16:16:04Z mirko $
*/

#include <WeaverInterface.h>
#include <DebuggingAids.h>

#include "JobSequence.h"
#include "DependencyPolicy.h"

using namespace ThreadWeaver;

JobSequence::JobSequence ( QObject *parent )
    : JobCollection ( parent )
{
}

void JobSequence::aboutToBeQueued ( WeaverInterface *weaver )
{
    REQUIRE (weaver != 0);

    if ( jobListLength() > 1 )
    {
        // set up the dependencies:
        for ( int i = 1; i < jobListLength(); ++i )
        {
            Job* jobA = jobAt(i);
            Job* jobB = jobAt(i-1);
            P_ASSERT ( jobA != 0 );
            P_ASSERT ( jobB != 0 );
            DependencyPolicy::instance().addDependency ( jobA, jobB );
        }
    }

    JobCollection::aboutToBeQueued( weaver );
}

void JobSequence::internalJobDone( Job* job)
{
    REQUIRE ( job != 0 );
    JobCollection::internalJobDone(job);
    if ( ! job->success() )
    {
        stop( job );
    }
}

#ifdef USE_CMAKE
#include "JobSequence.moc"
#endif
