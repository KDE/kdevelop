#include <WeaverInterface.h>
#include <DebuggingAids.h>

#include <QList>
#include <QObject>
#include <QPointer>

#include "JobCollection.h"
#include "JobCollection_p.h"

namespace ThreadWeaver {

    JobCollectionJobRunner::JobCollectionJobRunner ( Job* guard, Job* payload, QObject* parent )
        : Job( parent )
        , m_guard ( guard )
        , m_payload( payload )
    {
        Q_ASSERT ( payload ); // will not accept zero jobs
        Q_ASSERT ( guard );
        if ( ! m_payload->objectName().isEmpty() )
        {   // this is most usefull for debugging...
            setObjectName ( tr( "JobRunner executing " ) + m_payload->objectName() );
        }
    }

    bool JobCollectionJobRunner::hasUnresolvedDependencies()
    {   // the JobCollectionJobRunner object never have any dependencies:
        return m_payload->hasUnresolvedDependencies()
            || m_guard->hasUnresolvedDependencies();
    }

    Job* JobCollectionJobRunner::payload ()
    {
        return m_payload;
    }

    void JobCollectionJobRunner::aboutToBeQueued ( WeaverInterface *weaver )
    {
        m_payload->aboutToBeQueued( weaver );
    }

    void JobCollectionJobRunner::execute ( Thread *t )
    {
        if ( m_payload )
        {
            m_payload->execute ( t );
        } else {
            debug ( 1, "JobCollection: job in collection has been deleted." );
        }
        Job::execute ( t );
    }

    void JobCollectionJobRunner::run ()
    {
    }

    JobCollectionNullJob::JobCollectionNullJob( QObject* parent )
        : Job( parent )
    {
    }

    void JobCollectionNullJob::run()
    {
    }

    class JobCollection::JobList : public QList <JobCollectionJobRunner*> {};

    JobCollection::JobCollection ( QObject *parent )
        : Job ( parent )
        , m_elements ( new JobList() )
        , m_queued ( false )
        , m_guard ( new JobCollectionNullJob( this) )
        , m_weaver ( 0 )
    {
    }

    JobCollection::~JobCollection()
    {
        // dequeue all remaining jobs:
        if ( m_weaver )
        {
            for ( int i = 1; i < m_elements->size(); ++i )
            {
                if ( m_elements->at( i ) ) // ... a QPointer
                {
                    if ( ! m_elements->at( i )->isFinished() )
                    {
                        m_weaver->dequeue ( m_elements->at( i ) );
                    }
                }
            }
        }

        delete m_elements;
        // QObject cleanup takes care of the job runners
    }

    void JobCollection::addJob ( Job *j )
    {
        P_ASSERT ( m_queued == false );

        m_elements->prepend ( new JobCollectionJobRunner( m_guard, j, this ) );
    }

    void JobCollection::stop( Job *job )
    {
        Q_UNUSED( job );
        P_ASSERT ( m_queued == true ); // should only be stopped once started,
                                       // maybe a bit strict...

        // dequeue everything:
        for ( int index = 1; index < m_elements->size(); ++index )
        {
            if ( ! m_elements->at( index )->isFinished() )
            {
                m_weaver->dequeue ( m_elements->at( index ) );
            }
        }
        m_weaver->dequeue( this );
    }

    void JobCollection::aboutToBeQueued ( WeaverInterface *weaver )
    {
        Q_ASSERT ( m_queued == false ); // never queue twice

        int i;

        m_weaver = weaver;

        if ( m_elements->size() > 0 )
        {
            // set up a dummy job that has the same dependencies as this, but will not be queued:
            QList<Job*> dependencies = getDependencies();

            for ( int index = 0; index < dependencies.size(); ++index )
            {
                m_guard->addDependency ( dependencies.at( index ) );
            }

            // set up the dependencies:
            for ( i = 1; i < m_elements->size(); ++i )
            {
                Job* job = m_elements->at( i );
                addDependency( job );
                m_weaver->enqueue( job );
            }

            m_elements->at( 0 )->aboutToBeQueued( weaver );
        }

        m_queued = true;
    }

    void JobCollection::execute ( Thread *t )
    {
        if ( ! m_elements->isEmpty() )
        {   // this is a hack (but a good one): instead of queueing (this), we
            // execute the last job, to avoid to have (this) wait for an
            // available thread (the last operation does not get queued in
            // aboutToBeQueued() )
            m_elements->at( 0 )->execute ( t );
        }
        Job::execute ( t ); // run() is empty
    }

    Job* JobCollection::jobAt( int i )
    {
        return m_elements->at( i )->payload();
    }

    const int JobCollection::jobListLength()
    {
        return m_elements->size();
    }

    bool JobCollection::hasUnresolvedDependencies()
    {
        bool hasInheritedDependencies;

        if ( m_elements->size() > 0 )
        {
            hasInheritedDependencies = m_elements->at( 0 )->payload()->hasUnresolvedDependencies();
        } else {
            hasInheritedDependencies = false;
        }
        return Job::hasUnresolvedDependencies()
            || hasInheritedDependencies;
    }

}

#include "JobCollection.moc"
#include "JobCollection_p.moc"
