/* -*- C++ -*-

   This file implements the JobCollection class.

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

#include <QList>
#include <QObject>
#include <QPointer>

#include "JobCollection.h"
#include "DependencyPolicy.h"

using namespace ThreadWeaver;

/* QPointers are used internally to be able to dequeue jobs at destruction
   time. The owner of the jobs could have deleted them in the meantime.
   We use a class instead of a typedef to be able to forward-declare the
   class in the declaration.
*/
class ThreadWeaver::JobCollectionJobRunner : public Job
{
    Q_OBJECT

public:
    JobCollectionJobRunner ( JobCollection* collection, Job* payload, QObject* parent )
        : Job( parent )
        , m_payload( payload )
        , m_collection( collection )
    {
        Q_ASSERT ( payload ); // will not accept zero jobs

        if ( ! m_payload->objectName().isEmpty() )
        {   // this is most useful for debugging...
            setObjectName( tr( "JobRunner executing " ) + m_payload->objectName() );
        } else {
            setObjectName( tr( "JobRunner (unnamed payload)" ) );
        }
    }

    bool canBeExecuted()
    {   // the JobCollectionJobRunner object never have any dependencies:
        return m_payload->canBeExecuted();
    }

    Job* payload ()
    {
        return m_payload;
    }

    void aboutToBeQueued ( WeaverInterface *weaver )
    {
        m_payload->aboutToBeQueued( weaver );
    }

    void aboutToBeDequeued ( WeaverInterface *weaver )
    {
        m_payload->aboutToBeDequeued( weaver );
    }

    void execute ( Thread *t )
    {
        if ( m_payload )
        {
            m_payload->execute ( t );
            m_collection->internalJobDone ( m_payload);
        } else {
            debug ( 1, "JobCollection: job in collection has been deleted." );
        }
        Job::execute ( t );
    }

    int priority () const
    {
        return m_payload->priority();
    }

private:
    void run () {}

    QPointer<Job> m_payload;
    JobCollection* m_collection;
};

class JobList : public QList <JobCollectionJobRunner*> {};

class JobCollection::Private
{
public:

  Private()
    : elements ( new JobList() )
    , weaver ( 0 )
    , jobCounter (0)
    {}

  ~Private()
    {
      delete elements;
    }

  /* The elements of the collection. */
  JobList* elements;

  // FIXME (solved) unused
  /* True if this collection has been queued in the Job queue of a Weaver. */
  // bool queued;

  // FIXME (solved) still necessary?
  /* A guard job used to manage recursive dependencies. */
  // Job* guard;

  /* The Weaver interface this collection is queued in. */
  WeaverInterface *weaver;

  /* Counter for the finished jobs.
     Set to the number of elements when started.
     When zero, all elements are done.
  */
  int jobCounter;
};

JobCollection::JobCollection ( QObject *parent )
    : Job ( parent )
    , d (new Private)
{
}

JobCollection::~JobCollection()
{   // dequeue all remaining jobs:
    if ( d->weaver != 0 ) // still queued
        dequeueElements();
    // QObject cleanup takes care of the job runners
    delete d; d = 0;
}

void JobCollection::requestAbort()
{
    for (int i = 0; i < jobListLength(); ++i)
        jobAt(i)->requestAbort();
}

void JobCollection::addJob ( Job *job )
{
    REQUIRE( d->weaver == 0 );
    REQUIRE( job != 0);
    d->elements->append ( new JobCollectionJobRunner( this, job, this ) );
}

void JobCollection::stop( Job *job )
{   // this only works if there is an event queue executed by the main
    // thread, and it is not blocked:
    Q_UNUSED( job );
    if ( d->weaver != 0 )
    {
        debug( 4, "JobCollection::stop: dequeueing %p.\n", this);
        d->weaver->dequeue( this );
    }
    // FIXME ENSURE ( d->weaver == 0 ); // verify that aboutToBeDequeued has been called
}

void JobCollection::aboutToBeQueued ( WeaverInterface *weaver )
{
    REQUIRE ( d->weaver == 0 ); // never queue twice

    d->weaver = weaver;

    if ( d->elements->size() > 0 )
    {
        d->elements->at( 0 )->aboutToBeQueued( weaver );
    }

    ENSURE(d->weaver != 0);
}

void JobCollection::aboutToBeDequeued( WeaverInterface* weaver )
{   //  Q_ASSERT ( d->weaver != 0 );
    // I thought: "must have been queued first"
    // but the user can queue and dequeue in a suspended Weaver

    if ( d->weaver )
    {
        dequeueElements();

        d->elements->at( 0 )->aboutToBeDequeued( weaver );
    }

    d->weaver = 0;
    ENSURE ( d->weaver == 0 );
}

void JobCollection::execute ( Thread *t )
{
    REQUIRE ( d->weaver != 0);

    // FIXME (after 0.6) make sure this is async:
    emit (started (this));

    if ( ! d->elements->isEmpty() )
    { // d->elements is supposedly constant at this time, since we are
      // already queued
      // set job counter:
        d->jobCounter = d->elements->size();

        // queue elements:
        for (int index = 1; index < d->elements->size(); ++index)
	{
            d->weaver->enqueue (d->elements->at(index));
	}

        // this is a hack (but a good one): instead of queueing (this), we
        // execute the last job, to avoid to have (this) wait for an
        // available thread (the last operation does not get queued in
        // aboutToBeQueued() )
        // NOTE: this also calls internalJobDone()
        d->elements->at( 0 )->execute ( t );
    } else {
        // otherwise, we are just a regular, empty job (sob...):
        Job::execute( t );
    }

    // do not emit done, done is emitted when the last job called
    // internalJobDone()
    // also, do not free the queue policies yet, since not the whole job
    // is done
}

Job* JobCollection::jobAt( int i )
{
    REQUIRE ( i >= 0 && i < d->elements->size() );
    return d->elements->at( i )->payload();
}

const int JobCollection::jobListLength()
{
    return d->elements->size();
}

bool JobCollection::canBeExecuted()
{
    bool inheritedCanRun = true;

    if ( d->elements->size() > 0 )
    {
        inheritedCanRun = d->elements->at( 0 )->canBeExecuted();
    }

    return Job::canBeExecuted() && inheritedCanRun;
}

void JobCollection::internalJobDone ( Job* job )
{
    //REQUIRE (job != 0);

    if ( d->jobCounter == 0 )
    {   // there is a small chance that (this) has been dequeued in the
        // meantime, in this case, there is nothing left to clean up:
        d->weaver = 0;
        return;
    }

    Q_UNUSED (job);
    --d->jobCounter;

    if (d->jobCounter == 0)
    {
        finalCleanup();

        if (! success())
	{
            emit failed(this);
	}
    }
    ENSURE (d->jobCounter >= 0);
}

void JobCollection::finalCleanup()
{
    freeQueuePolicyResources();
    setFinished(true);
    emit done(this);
    d->weaver = 0;
}

void JobCollection::dequeueElements()
{   // dequeue everything:
    if ( d->weaver != 0 )
    {
        for ( int index = 1; index < d->elements->size(); ++index )
	{
            if ( d->elements->at( index ) && ! d->elements->at( index )->isFinished() ) // ... a QPointer
	    {
                debug( 4, "JobCollection::dequeueElements: dequeueing %p.\n",
                       d->elements->at( index ) );
                d->weaver->dequeue ( d->elements->at( index ) );
	    } else {
                debug( 4, "JobCollection::dequeueElements: not dequeueing %p, already finished.\n",
                       d->elements->at( index ) );
	    }
	}

        if (d->jobCounter != 0)
	{ // if jobCounter is not zero, then we where waiting for the
	  // last job to finish before we would have freed our queue
	  // policies, but in this case we have to do it here:
            finalCleanup();
	}
        d->jobCounter = 0;
    }
}

#include "JobCollection.moc"
#ifdef USE_CMAKE
#include "JobCollection_moc.cpp"
#endif
