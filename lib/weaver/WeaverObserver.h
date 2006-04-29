/* -*- C++ -*-

   This file declares the WeaverObserver class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
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

    class Thread;
    class Job;
    class State;

    /** Observers provides signals on different weaver events that are
        otherwise only available through objects of different classes
        (threads, jobs). Usually, access to the signals of those objects
        is not provided through the weaver API. Use an observer to reveice
        notice, for example, on thread activity.

        To unregister, simply delete the observer.

        When using WeaverObservers, it is recommended to create and delete
        Jobs only in the controlling (usually the main) thread. Also, it is
        better to use deleteLater(..), as Job pointers are passed around in
        signals.
    */
    class WeaverObserver : public QObject
    {
        Q_OBJECT
    public:
        WeaverObserver ( QObject *parent = 0 );
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
