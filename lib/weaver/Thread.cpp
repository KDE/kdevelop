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

#include "ThreadWeaver.h"
#include "WeaverImpl.h"
#include "Thread.h"
#include "Job.h"
#include "DebuggingAids.h"

namespace ThreadWeaver {

    unsigned int Thread::sm_Id;
    QMutex Thread::sm_mutex;

    Thread::Thread (WeaverImpl *parent)
        : QThread (),
          m_parent ( parent ),
          m_job ( 0 ),
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
        debug ( 3, "Thread::run [%u]: running.\n", id() );
	emit ( started ( this) );

        while (true)
        {
	    debug ( 3, "Thread::run [%u]: trying to execute the next job.\n", id() );

            // this is the *only* assignment to m_job  in the Thread class!
            Job* job = m_parent->applyForWork ( this, m_job );

            if (job == 0)
            {
                break;
            } else {
                {
                    QMutexLocker l ( &sm_mutex );
                    m_job = job;
                }

                emit ( jobStarted ( this,  m_job ) );
                m_job->execute (this);
                emit ( jobDone ( m_job ) );
            }
        }
        debug ( 3, "Thread::run [%u]: exiting.\n", id() );
    }

    void Thread::msleep(unsigned long msec)
    {
        QThread::msleep(msec);
    }


    void Thread::requestAbort ()
    {
        if ( m_job )
        {
            QMutexLocker l ( &sm_mutex );
            if ( m_job )
            {
                m_job->requestAbort();
            }
        }
    }

}
