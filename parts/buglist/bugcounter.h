/***************************************************************************
                          bugcounter.h  -  description
                             -------------------
    begin                : Sun Dec 3 2000
    copyright            : (C) 2000 by Ivan Hawkes
    email                : blackhawk@ivanhawkes.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef BUGCOUNTER_H
#define BUGCOUNTER_H

#include <qstring.h>


/**
    This class is used to provide entries in a list, of all the counters
    for each developer in the team. This enables us to use a sequential
    numbering system that doesn't create conflicting ID's even when a
    project's developers are spread over the internet.
*/

class BugCounter
{
    public:
        QString     Initials;
        int         LastBugNumber;

    public:
        const QString GetNextID ();
};


#endif

