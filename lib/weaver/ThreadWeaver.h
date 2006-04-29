/* -*- C++ -*-

   This file implements the public interfaces of the Weaver and the Job class.
   It should be the only include file necessary to use the ThreadWeaver
   library.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: ThreadWeaver.h 32 2005-08-17 08:38:01Z mirko $
*/
#ifndef THREADWEAVER_H
#define THREADWEAVER_H

#include <QObject>

#include <Job.h>
#include <State.h>
#include <DebuggingAids.h>
#include "WeaverInterface.h"

namespace ThreadWeaver {

    class WeaverObserver;

    /** The Weaver class acts as a facade to the WeaverImpl class.

        It creates and destroys WeaverImpl objects. Also, it provides a
        factory method for this purpose that can be overloaded to create
        derived WeaverImpl objects.

        Weaver provides a static instance that can be used to perform jobs in
        threads without managing a weaver object. The static instance will
        only be created when it is first accessed. Also, Weaver objects will
        create the threads only when the first jobs are queued. Therefore, the
        creation of a Weaver object is a rather cheap operation.

        The WeaverImpl class provides two parts of API - one for the threads
        that are handled by it, and one for the ThreadWeaver users
        (application developers). To separate those two different API parts,
        Weaver only provides the interface supposed to be used by developers
        of multithreaded applications. */
    class Weaver : public WeaverInterface
    {
        Q_OBJECT
    public:
	/** Construct a Weaver object. */
        Weaver (QObject* parent=0,
                int inventoryMin = 4, // minimal number of provided threads
                int inventoryMax = 32); // maximum number of provided threads
	/** Destruct a Weaver object. */
        virtual ~Weaver ();
	const State& state() const;
        void registerObserver ( WeaverObserver* );
	/** Return the global Weaver instance.
	    In some cases, a global Weaver object per application is
	    sufficient for the applications purpose. If this is the case,
	    query instance() to a pointer to a global instance.
	    If instance is never called, a global Weaver object will not be
	    created.
	*/
	static ThreadWeaver::Weaver* instance();
        virtual void enqueue (Job*);
	virtual void enqueue (const QList<Job*>& jobs);
        virtual bool dequeue (Job*);
        virtual void dequeue ();
	virtual void finish();
        virtual void suspend( );
        virtual void resume();
        bool isEmpty ();
	bool isIdle ();
	int queueLength ();
	int noOfThreads ();
        void requestAbort();
    protected:
        WeaverInterface *m_weaverinterface;
	/** The application-global Weaver instance.

	    This  instance will only be created if this method is actually called
	    in the lifetime of the application. */
	static Weaver *m_instance;
        /** The factory method to create the actual Weaver implementation.
            Overload this method to use a different or adapted implementation.
            */
        virtual WeaverInterface* makeWeaverImpl(int inventoryMin, int inventoryMax );
    };
}

#endif // THREADWEAVER_H
