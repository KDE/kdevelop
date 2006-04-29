/* -*- C++ -*-

   This file implements the state handling in ThreadWeaver.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: State.h 32 2005-08-17 08:38:01Z mirko $
*/

#ifndef THREADWEAVER_STATE_H
#define THREADWEAVER_STATE_H

#include <QString>

namespace ThreadWeaver {

    class Job;
    class Thread;
    class WeaverImpl;

    /** All weaver objects maintain a state of operation which can be
        queried by the application. See the threadweaver documentation on
        how the different states are related.
    */
    enum StateId {
        /** The object is in the state of construction and has not yet
            started to process jobs. */
        InConstruction = 0,
        /** Jobs are being processed. */
        WorkingHard,
        /** Job processing is suspended, but some jobs which where already
            in progress are not finished yet. */
        Suspending,
        /** Job processing is suspended, and no jobs are being
            processed. */
        Suspended,
        /** The object is being destructed. Jobs might still be processed,
            the destructor will wait for all threads to exit and then
            end. */
        ShuttingDown,
        /** The object is being destructed, and all threads have
            exited. No jobs are handled anymore. */
        Destructed,
        /** Not a state, but a sentinel for the number of defined states. */
        NoOfStates
    };

    extern const QString StateNames[];

    /** We use a State pattern to handle the system state in ThreadWeaver.
     */
    class State
    {
    public:
        /** Default constructor. */
        State( WeaverImpl *weaver,  const StateId id = InConstruction );

	/** Destructor. */
        virtual ~State();

        /** The ID of the current state.
            @see StateNames, StateID
        */
        const QString& stateName() const;
        /** The state Id. */
        const StateId stateId() const;
        /** Suspend job processing. */
        virtual void suspend() = 0;
        /** Resume job processing. */
        virtual void resume() = 0;
        /** Assign a job to an idle thread.
            @param th the thread to give a new Job to
            @param previous the job this thread finished before calling
         * */
        virtual Job* applyForWork ( Thread *th,  Job* previous ) = 0;
        /** Wait (by suspending the calling thread) until a job becomes available. */
        virtual void waitForAvailableJob ( Thread *th ) = 0;
        /** The state has been changed so that this object is responsible for
         * state handling. */
        virtual void activated();
    protected:
        /** Id of this state.
            Set in the constructor.
        */
        StateId m_id;
        /** The Weaver we relate to. */
        WeaverImpl *m_weaver;
    };
}

#endif // THREADWEAVER_STATE_H
