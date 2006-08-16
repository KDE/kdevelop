/* -*- C++ -*-

   This file declares the StateIMplementation class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, 2006 Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id$
*/

#ifndef STATEIMPLEMENTATION_H
#define STATEIMPLEMENTATION_H

#ifndef THREADWEAVER_PRIVATE_API
#define THREADWEAVER_PRIVATE_API
#endif

#include "State.h"

namespace ThreadWeaver {

    class WeaverImpl;

    class StateImplementation : public State
    {
    public:
        StateImplementation( WeaverInterface* weaver )
            : State ( weaver )
        {}

    protected:
        WeaverImpl* weaver();
    };

}

#endif
