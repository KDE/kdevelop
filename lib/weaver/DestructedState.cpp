/* -*- C++ -*-

This file implements the DestructedState class.

$ Author: Mirko Boehm $
$ Copyright: (C) 2005, 2006 Mirko Boehm $
$ Contact: mirko@kde.org
http://www.kde.org
http://www.hackerbuero.org $
$ License: LGPL with the following explicit clarification:
This code may be linked against any version of the Qt toolkit
from Trolltech, Norway. $

$Id: DestructedState.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include "DestructedState.h"

using namespace ThreadWeaver;

void DestructedState::suspend()
{
}

void DestructedState::resume()
{
}

Job* DestructedState::applyForWork ( Thread*,  Job*)
{
    return 0;
}

void DestructedState::waitForAvailableJob ( Thread * )
{
}

StateId DestructedState::stateId() const
{
    return Destructed;
}
