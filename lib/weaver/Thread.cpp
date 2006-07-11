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

namespace ThreadWeaver {

    class ThreadRunHelper : public QObject
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
//                     {
//                         QMutexLocker l ( &sm_mutex );
                    m_job = job;
//                     }

                    /*
                      QThread* originalOwner = job->QObject::thread();

                      if ( originalOwner != this )
                      {
                      job->moveToThread ( this );
                      }
                    */
                    emit ( jobStarted ( th,  m_job ) );
                    m_job->execute (th);
                    emit ( jobDone ( m_job ) );
                    /*
                      if ( originalOwner != this )
                      {
                      job->moveToThread ( originalOwner );
                      }
                    */
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

//             if ( m_job )
//             {
//                 QMutexLocker l ( &sm_mutex );
//                 if ( m_job )
//                 {
//                     m_job->requestAbort();
//                 }
//             }
        }
    };

    unsigned int Thread::sm_Id;
    QMutex Thread::sm_mutex;

    Thread::Thread (WeaverImpl *parent)
        : QThread (),
          m_parent ( parent ),
          m_runhelper ( 0 ),
//          m_job ( 0 ),
          m_id ( makeId() )
    {
    }

    Thread::~Thread()
    {
    }

    unsigned int Thread::makeId()
    {
        QMutexLocker l (&sm_mutex);
        return ++sm_Id;
    }

    const unsigned int Thread::id()
    {
        return m_id;
    }

    void Thread::run()
    {
        Q_ASSERT ( thread() != this ); // this is created and owned by the main thread
        debug ( 3, "Thread::run [%u]: running.\n", id() );

        ThreadRunHelper helper;
        m_runhelper = &helper;

        connect ( &helper, SIGNAL ( started ( Thread* ) ),
                  SIGNAL ( started ( Thread* ) ) );
        connect ( &helper, SIGNAL ( jobStarted ( Thread*, Job* ) ),
                  SIGNAL ( jobStarted ( Thread*, Job* ) ) );
        connect ( &helper, SIGNAL ( jobDone ( Job* ) ),
                  SIGNAL ( jobDone ( Job* ) ) );
        helper.run( m_parent,  this );

        m_runhelper = 0;
        debug ( 3, "Thread::run [%u]: exiting.\n", id() );
    }

    void Thread::msleep(unsigned long msec)
    {
        QThread::msleep(msec);
    }


    void Thread::requestAbort ()
    {
        if ( m_runhelper )
        {
            m_runhelper->requestAbort();
        } else {
            qDebug ( "Thread::requestAbort: not running." );
        }
    }

}

#include "Thread.moc"
