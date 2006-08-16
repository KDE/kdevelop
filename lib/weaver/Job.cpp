/* -*- C++ -*-

This file implements the Job class.

$ Author: Mirko Boehm $
$ Copyright: (C) 2004, 2005, 2006 Mirko Boehm $
$ Contact: mirko@kde.org
http://www.kde.org
http://www.hackerbuero.org $
$ License: LGPL with the following explicit clarification:
This code may be linked against any version of the Qt toolkit
from Troll Tech, Norway. $

$Id: Job.cpp 20 2005-08-08 21:02:51Z mirko $
*/

#include <QSet>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QMultiMap>
#include <QMetaObject>
#include <QWaitCondition>
#include <DebuggingAids.h>
#include <Thread.h>

#include "Job.h"
#include "QueuePolicy.h"
#include "DependencyPolicy.h"

using namespace ThreadWeaver;

class ThreadWeaver::QueuePolicyList : public QList<QueuePolicy*> {};

class Job::Private
{
public:
    Private ()
        : thread (0)
        , queuePolicies ( new QueuePolicyList )
        , mutex (new QMutex (QMutex::NonRecursive) )
        , finished (false)
    {}

    /* The thread that executes this job. Zero when the job is not executed. */
    Thread * thread;

    /* The list of QueuePolicies assigned to this Job. */
    QueuePolicyList* queuePolicies;

    QMutex *mutex;
    /* d->finished is set to true when the Job has been executed. */
    bool finished;
};

Job::Job ( QObject *parent )
    : QObject (parent)
    , d(new Private())
{
}

Job::~Job()
{
    for ( int index = 0; index < d->queuePolicies->size(); ++index )
    {
        d->queuePolicies->at( index )->destructed( this );
    }

    delete d; d = 0;
}

class ThreadWeaver::JobRunHelper : public QObject
{
    Q_OBJECT
public:
    JobRunHelper()
        : QObject ( 0 )
    {
    }

signals:
    void started ( Job* );
    void done ( Job* );
    void failed( Job* );

public:

    void runTheJob ( Thread* th, Job* job )
    {
        P_ASSERT ( th == thread() );
        job->d->mutex->lock();
        job->d->thread = th;
        job->d->mutex->unlock();

        emit ( started ( job ) );

        job->run();

        job->d->mutex->lock();
        job->d->thread = 0;
        job->setFinished (true);
        job->d->mutex->unlock();
        job->freeQueuePolicyResources();

        if ( ! job->success() )
        {
            emit ( failed( job ) );
        }

        emit ( done( job ) );
    }
};

void Job::execute(Thread *th)
{
//    P_ASSERT (sm_dep()->values(this).isEmpty());
    JobRunHelper helper;
    connect ( &helper,  SIGNAL ( started ( Job* ) ), SIGNAL ( started ( Job* ) ) );
    connect ( &helper,  SIGNAL ( done ( Job* ) ), SIGNAL ( done ( Job* ) ) );
    connect ( &helper, SIGNAL( failed( Job* ) ), SIGNAL( failed( Job* ) ) );

    debug(3, "Job::execute: executing job of type %s %s in thread %i.\n",
          metaObject()->className(), objectName().isEmpty() ? "" : qPrintable( objectName() ), th->id());
    helper.runTheJob( th, this );
    debug(3, "Job::execute: finished execution of job in thread %i.\n", th->id());
}

int Job::priority () const
{
    return 0;
}

void Job::freeQueuePolicyResources()
{
    for ( int index = 0; index < d->queuePolicies->size(); ++index )
    {
        d->queuePolicies->at( index )->free( this );
    }
}

void Job::aboutToBeQueued ( WeaverInterface* )
{
}

void Job::aboutToBeDequeued ( WeaverInterface* )
{
}

bool Job::canBeExecuted()
{
    QueuePolicyList acquired;

    bool success = true;

    if ( d->queuePolicies->size() > 0 )
    {
        debug( 4, "Job::canBeExecuted: acquiring permission from %i queue %s.\n",
               d->queuePolicies->size(), d->queuePolicies->size()==1 ? "policy" : "policies" );
        for ( int index = 0; index < d->queuePolicies->size(); ++index )
        {
            if ( d->queuePolicies->at( index )->canRun( this ) )
            {
                acquired.append( d->queuePolicies->at( index ) );
            } else {
                success = false;
                break;
            }
        }

        debug( 4, "Job::canBeExecuted: queue policies returned %s.\n", success ? "true" : "false" );

        if ( ! success )
        {

            for ( int index = 0; index < acquired.size(); ++index )
            {
                acquired.at( index )->release( this );
            }
        }
    } else {
        debug( 4, "Job::canBeExecuted: no queue policies, this job can be executed.\n" );
    }

    return success;
}

void Job::assignQueuePolicy( QueuePolicy* policy )
{
    if ( ! d->queuePolicies->contains( policy ) )
    {
        d->queuePolicies->append( policy );
    }
}

void Job::removeQueuePolicy( QueuePolicy* policy )
{
    int index = d->queuePolicies->indexOf( policy );
    if ( index != -1 )
    {
        d->queuePolicies->removeAt( index );
    }
}

bool Job::isFinished() const
{
    return d->finished;
}

Thread* Job::thread()
{
    return d->thread;
}

void Job::setFinished ( bool status )
{
    d->finished = status;
}

// QMutex& Job::mutex()
// {
//     return * d->mutex;
// }

#include "Job.moc"
#ifdef USE_CMAKE
#include "Job_moc.cpp"
#endif
