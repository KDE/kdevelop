/* -*- C++ -*-

   This file implements the StateImplementation class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, 2006Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: InConstructionState.cpp 30 2005-08-16 16:16:04Z mirko $
*/

#include "StateImplementation.h"

#include "WeaverImpl.h"

using namespace ThreadWeaver;

WeaverImpl* StateImplementation::weaver()
{
    Q_ASSERT ( dynamic_cast<WeaverImpl*> ( State::weaver() ) );
    return static_cast<WeaverImpl*> ( State::weaver() );
}
