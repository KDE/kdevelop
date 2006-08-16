/* -*- C++ -*-

   This file declares the WeaverObserver class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, 2006 Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: WeaverObserver.h 32 2005-08-17 08:38:01Z mirko $
*/

#ifndef WEAVEROBSERVER_H
#define WEAVEROBSERVER_H

#include <QObject>

namespace ThreadWeaver {

    class Job;
    class State;
    class Thread;

    /** Observers provides signals on some Weaver events that are
        otherwise only available through objects of different classes
        (threads, jobs). Usually, access to the signals of those objects
        is not provided through the ThreadWeaver API. Use an observer to receive
        notice, for example, on thread activity.

        Observers are registered through the WeaverInterface API
        (see WeaverInterface::registerObserver).

        To unregister, simply delete the observer.

        Observers are meant to be used within the controlling thread (usually
        the main thread), that creates the Jobs. If this is not the case, be
        aware not to delete Job objects externally that you reference in a
        WeaverObserver implementation.
    */
    class WeaverObserver : public QObject
    {
        Q_OBJECT
    public:
        explicit WeaverObserver ( QObject *parent = 0 );
        virtual ~WeaverObserver();
    signals:
        void threadStarted ( Thread* );
        void threadBusy( Thread*,  Job* );
        void threadSuspended ( Thread* );
        void threadExited ( Thread* );
        void weaverStateChanged ( State* );
    };

}

#endif
