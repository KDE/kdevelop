/* -*- C++ -*-

This file implements the WeaverObserver class.

$ Author: Mirko Boehm $
$ Copyright: (C) 2005, 2006 Mirko Boehm $
$ Contact: mirko@kde.org
http://www.kde.org
http://www.hackerbuero.org $
$ License: LGPL with the following explicit clarification:
This code may be linked against any version of the Qt toolkit
from Trolltech, Norway. $

$Id: WeaverObserver.cpp 31 2005-08-16 16:21:10Z mirko $
*/

#include "WeaverObserver.h"

using namespace ThreadWeaver;

WeaverObserver::WeaverObserver ( QObject *parent )
    : QObject ( parent )
{
}

WeaverObserver::~WeaverObserver()
{
}

#ifdef USE_CMAKE
#include "WeaverObserver.moc"
#endif
