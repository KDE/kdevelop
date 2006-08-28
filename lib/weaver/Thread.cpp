/* -*- C++ -*-

   This file implements the Thread class.

   Thread is not a part of the public interface of the ThreadWeaver library.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2004, 2005 Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Troll Tech, Norway. $

   $Id: Thread.cpp 25 2005-08-14 12:41:38Z mirko $
*/

#include <QMutex>
#include <QtDebug>

#include "ThreadWeaver.h"
#include "WeaverImpl.h"
#include "Thread.h"
#include "Job.h"
#include "DebuggingAids.h"

using namespace ThreadWeaver;

class Thread::Private
{
public:
    explicit Private ( WeaverImpl* theParent )
        : parent ( theParent )
        , runhelper ( 0 )
        , id ( makeId() )
    {}

    WeaverImpl *parent;

    ThreadRunHelper* runhelper;

    const unsigned int id;

    static unsigned int makeId()
    {
        static unsigned int s_id;
        static QMutex sm_mutex;
        QMutexLocker l (&sm_mutex);
        return ++s_id;
    }
};


class ThreadWeaver::ThreadRunHelper : public QObject
{
    Q_OBJECT
public:
    explicit ThreadRunHelper ()
        : QObject ( 0 )
        , m_job( 0 )
    {
    }

signals: // see Thread:

    /** The thread has been started. */
    void started ( Thread* );
    /** The thread started to process a job. */
    void jobStarted ( Thread*,  Job* );
    /** The thread finished to execute a job. */
    void jobDone ( Job* );

private:
    Job* m_job;

public:
    void run ( WeaverImpl *parent, Thread* th )
    {
        Q_ASSERT ( thread() == th );
        emit ( started ( th) );

        while (true)
        {
            debug ( 3, "Thread::run [%u]: trying to execute the next job.\n", th->id() );

            // this is the *only* assignment to m_job  in the Thread class!
            Job* job = parent->applyForWork ( th, m_job );

            if (job == 0)
            {
                break;
            } else {
                m_job = job;
                emit ( jobStarted ( th,  m_job ) );
                m_job->execute (th);
                emit ( jobDone ( m_job ) );
                m_job = 0;
            }
        }
    }

    void requestAbort()
    {
        Job* job = m_job;
        if ( job )
        {
            job->requestAbort();
        }
    }
};

Thread::Thread (WeaverImpl *parent)
    : QThread () // no parent, because the QObject hierarchy of this thread
                 // does not have a parent (see QObject::pushToThread)
    , d ( new Private ( parent ) )
{
}

Thread::~Thread()
{
    delete d; d = 0;
}

const unsigned int Thread::id()
{
    return d->id;
}

void Thread::run()
{
// disabled while testing movetothread...
//    Q_ASSERT ( thread() != this ); // this is created and owned by the main thread
    debug ( 3, "Thread::run [%u]: running.\n", id() );

    ThreadRunHelper helper;
    d->runhelper = &helper;

    connect ( &helper, SIGNAL ( started ( Thread* ) ),
              SIGNAL ( started ( Thread* ) ) );
    connect ( &helper, SIGNAL ( jobStarted ( Thread*, Job* ) ),
              SIGNAL ( jobStarted ( Thread*, Job* ) ) );
    connect ( &helper, SIGNAL ( jobDone ( Job* ) ),
              SIGNAL ( jobDone ( Job* ) ) );
    helper.run( d->parent,  this );

    d->runhelper = 0;
    debug ( 3, "Thread::run [%u]: exiting.\n", id() );
}

void Thread::msleep(unsigned long msec)
{
    QThread::msleep(msec);
}


void Thread::requestAbort ()
{
    if ( d->runhelper )
    {
        d->runhelper->requestAbort();
    } else {
        qDebug ( "Thread::requestAbort: not running." );
    }
}

#include "Thread.moc"
#ifdef USE_CMAKE
#include "Thread_moc.cpp"
#endif
