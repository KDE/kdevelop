/* -*- C++ -*-

   This file implements the ShuttingDownState class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, 2006 Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: ShuttingDownState.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include "ShuttingDownState.h"

using namespace ThreadWeaver;

void ShuttingDownState::suspend()
{
    // ignored: when shutting down, we do not return to the suspended state
}

void ShuttingDownState::resume()
{
    // ignored: when shutting down, we do not return to the suspended state
}

Job* ShuttingDownState::applyForWork ( Thread*,  Job* )
{
    return 0;  // tell threads to exit
}

void ShuttingDownState::waitForAvailableJob ( Thread*)
{
    // immidiately return here
}

StateId ShuttingDownState::stateId() const
{
    return ShuttingDown;
}

