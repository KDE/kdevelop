/* -*- C++ -*-

   This file implements debugging aids for multithreaded applications.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2004, 2005 Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: DebuggingAids.cpp 20 2005-08-08 21:02:51Z mirko $
*/

#include <QMutex>

#include "DebuggingAids.h"

/** A global mutex for the ThreadWeaver objects.
    Generally, you should not use it in your own code. */
QMutex ThreadWeaver::GlobalMutex;
bool ThreadWeaver::Debug = true;
int ThreadWeaver::DebugLevel = 01;
