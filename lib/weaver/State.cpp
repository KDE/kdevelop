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

   $Id: State.cpp 20 2005-08-08 21:02:51Z mirko $
*/

#include <QString>

#include <State.h>

namespace ThreadWeaver {

    const QString StateNames[NoOfStates] = {
        "InConstruction",
        "WorkingHard",
        "Suspending",
        "Suspended",
        "ShuttingDown",
        "Destructed"
    };

    State::State ( WeaverImpl *weaver,  const StateId id )
        : m_id ( id ),
          m_weaver ( weaver )
    {
    }

    State::~State()
    {
    }

    const QString& State::stateName () const
    {
        return StateNames[m_id];
    }

    const StateId State::stateId() const
    {
        return m_id;
    }

    void State::activated()
    {
    }
}

