/* -*- C++ -*-

   This file declares the Thread class.

   Thread is not a part of the public interface of the ThreadWeaver library.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2004, 2005, 2006 Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Troll Tech, Norway. $

   $Id: Thread.h 32 2005-08-17 08:38:01Z mirko $
*/

#ifndef THREADWEAVER_THREAD_H
#define THREADWEAVER_THREAD_H

#include <QMutex>
#include <QThread>

namespace ThreadWeaver {

    class Job;
    class WeaverImpl;
    class ThreadRunHelper;

    /** The class Thread is used to represent the worker threads in
        the weaver's inventory. It is not meant to be overloaded. */
    class Thread : public QThread
    {
        Q_OBJECT

    public:
	/** Create a thread.
	    These thread objects are only used inside the Weaver parent
	    object.

            @param parent the parent WeaverImpl
        */
        explicit Thread ( WeaverImpl *parent = 0 );

	/** The destructor. */
        ~Thread();

        /** Overload to execute the assigned jobs.
	    Whenever the thread is idle, it will ask it's Weaver parent for a
	    Job to do. The Weaver will either return a Job or a Nil
	    pointer. When a Nil pointer is returned, it tells the thread to
	    exit.
	*/
        void run();

	/** Provide the msleep() method (protected in QThread) to be
	    available  for executed jobs.
	    @param msec amount of milliseconds
        */
	void msleep ( unsigned long msec );

	/** Returns the thread id.
	    This id marks the respective Thread object, and must
	    therefore not be confused with, e.g., the pthread thread
	    ID. */
	const unsigned int id();

        /** Request the abortion of the current job.
            If there is no current job, this method will do nothing, but can
            safely be called.
            It forwards the request to the current Job.
         */
        void requestAbort();

    signals:
        /** The thread has been started. */
        void started ( Thread* );
        /** The thread started to process a job. */
        void jobStarted ( Thread*,  Job* );
        /** The thread finished to execute a job. */
        void jobDone ( Job* );

    private:
        class Private;
        Private* d;
    };

}

#endif

