/* -*- C++ -*-

This file implements the DependencyPolicy class.

$ Author: Mirko Boehm $
$ Copyright: (C) 2004, 2005, 2006 Mirko Boehm $
$ Contact: mirko@kde.org
http://www.kde.org
http://www.hackerbuero.org $
$ License: LGPL with the following explicit clarification:
This code may be linked against any version of the Qt toolkit
from Trolltech, Norway. $

$Id: DebuggingAids.cpp 20 2005-08-08 21:02:51Z mirko $
*/

#include <QMutex>
#include <QtDebug>

#include "Job.h"
#include "DependencyPolicy.h"
#include "DebuggingAids.h"

using namespace ThreadWeaver;

typedef QMultiMap<Job*, Job*> JobMultiMap;

class DependencyPolicy::Private
{
public:
    /** A container to keep track of Job dependencies.
        For each dependency A->B, which means Job B depends on Job A and
        may only be executed after A has been finished, an entry will be
        added with key A and value B. When A is finished, the entry will
        be removed. */
    JobMultiMap& dependencies()
    {
        static JobMultiMap depMap;
        return depMap;
    }

    QMutex& mutex()
    {
        static QMutex s_mutex;
        return s_mutex;
    }

};

DependencyPolicy::DependencyPolicy()
    : QueuePolicy()
    , d ( new Private() )
{
}

DependencyPolicy::~DependencyPolicy()
{
    delete d;
}

void DependencyPolicy::addDependency( Job* jobA, Job* jobB )
{
    // jobA depends on jobB
    REQUIRE ( jobA != 0 && jobB != 0 && jobA != jobB );
    jobA->assignQueuePolicy( this );
    jobB->assignQueuePolicy( this );
    QMutexLocker l( & d->mutex() );
    d->dependencies().insert( jobA, jobB );

    ENSURE ( d->dependencies().contains (jobA));
}

bool DependencyPolicy::removeDependency( Job* jobA, Job* jobB )
{
    REQUIRE (jobA != 0 && jobB != 0);
    bool result = false;
    QMutexLocker l( & d->mutex() );

    // there may be only one (!) occurence of [this, dep]:
    QMutableMapIterator<Job*, Job*> it( d->dependencies () );
    while ( it.hasNext() )
    {
        it.next();
        if ( it.key()==jobA && it.value()==jobB )
        {
            it.remove();
            result = true;
            break;
        }
    }

    ENSURE ( ! d->dependencies().keys(jobB).contains(jobA) );
    return result;
}

void DependencyPolicy::resolveDependencies( Job* job )
{
    if ( job->success() )
    {
        QMutexLocker l( & d->mutex() );
        QMutableMapIterator<Job*, Job*> it( d->dependencies() );
        // there has to be a better way to do this: (?)
        while ( it.hasNext() )
        {   // we remove all entries where jobs depend on *this* :
            it.next();
            if ( it.value()==job )
            {
                it.remove();
            }
        }
    }
}

QList<Job*> DependencyPolicy::getDependencies( Job* job ) const
{
    REQUIRE (job != 0);
    QList<Job*> result;
    JobMultiMap::const_iterator it;
    QMutexLocker l( & d->mutex() );

    for ( it = d->dependencies().begin(); it != d->dependencies().end(); ++it )
    {
        if ( it.key() == job )
        {
            result.append( it.value() );
        }
    }
    return result;
}

bool DependencyPolicy::hasUnresolvedDependencies( Job* job )
{
    REQUIRE (job != 0);
    QMutexLocker l( & d->mutex() );
    return d->dependencies().contains( job );
}

DependencyPolicy& DependencyPolicy::instance ()
{
    static DependencyPolicy policy;
    return policy;
}

bool DependencyPolicy::canRun( Job* job )
{
    REQUIRE (job != 0);
    return ! hasUnresolvedDependencies( job );
}

void DependencyPolicy::free( Job* job )
{
    REQUIRE (job != 0);
    if ( job->success() )
    {
        resolveDependencies( job );
        debug( 3, "DependencyPolicy::free: dependencies resolved for job %p.\n", job);
    } else {
        debug( 3, "DependencyPolicy::free: not resolving dependencies for %p (execution not successful).\n",
               job);
    }
    ENSURE ( ( ! hasUnresolvedDependencies( job ) && job->success() ) || ! job->success() );
}

void DependencyPolicy::release( Job* job )
{
    REQUIRE (job != 0);
}

void DependencyPolicy::destructed( Job* job )
{
    REQUIRE (job != 0);
    resolveDependencies ( job );
}

void DependencyPolicy::dumpJobDependencies()
{
    QMutexLocker l( & d->mutex() );

    debug ( 0, "Job Dependencies (left depends on right side):\n" );
    for ( JobMultiMap::const_iterator it = d->dependencies().begin(); it != d->dependencies().end(); ++it )
    {
        debug( 0, "  : %p (%s%s) <-- %p (%s%s)\n",
               it.key(),
               it.key()->objectName().isEmpty() ? "" : qPrintable ( it.key()->objectName() + QObject::tr ( " of type " ) ),
               it.key()->metaObject()->className(),
               it.value(),
               it.value()->objectName().isEmpty() ? "" : qPrintable ( it.value()->objectName() + QObject::tr ( " of type " ) ),
               it.value()->metaObject()->className() );
    }
    debug ( 0, "-----------------\n" );
}

