/* -*- C++ -*-

   This file implements the WeaverInterface class.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: WeaverImpl.h 29 2005-08-14 19:04:30Z mirko $
*/

#include "WeaverInterface.h"

namespace ThreadWeaver {

    WeaverInterface:: WeaverInterface ( QObject* parent )
        : QObject ( parent )
    {
    }

}
