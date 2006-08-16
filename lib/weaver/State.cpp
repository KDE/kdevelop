/* -*- C++ -*-

This file implements the state handling in ThreadWeaver.

$ Author: Mirko Boehm $
$ Copyright: (C) 2005, 2006 Mirko Boehm $
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

using namespace ThreadWeaver;

const QString StateNames[NoOfStates] = {
    "InConstruction",
    "WorkingHard",
    "Suspending",
    "Suspended",
    "ShuttingDown",
    "Destructed"
};

class State::Private
{
public:
    Private ( WeaverInterface* theWeaver )
        : weaver( theWeaver )
    {
        Q_ASSERT_X( sizeof StateNames / sizeof StateNames[0] == NoOfStates, "State::Private ctor",
                    "Make sure to keep StateId and StateNames in sync!" );
    }

    /** The Weaver we relate to. */
    WeaverInterface *weaver;
};


State::State ( WeaverInterface *weaver )
    : d  ( new Private ( weaver ) )
{
}

State::~State()
{
    delete d; d = 0;
}

const QString& State::stateName () const
{
    return StateNames[ stateId() ];
}

void State::activated()
{
}

WeaverInterface* State::weaver()
{
    return d->weaver;
}
