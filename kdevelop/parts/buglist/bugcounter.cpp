/***************************************************************************
                          bugcounter.cpp  -  description
                             -------------------
    begin                : Sun Dec 3 2000
    copyright            : (C) 2000 by Ivan Hawkes
    email                : linuxgroupie@ivanhawkes.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "bugcounter.h"


const QString BugCounter::GetNextID ()
{
    QString     NextID;

    LastBugNumber++;
    NextID.sprintf ("ILH%05i", LastBugNumber);

    return NextID;
}
