/***************************************************************************
                          bug.h  -  description
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

#ifndef BUG_H
#define BUG_H

#include <qdatetime.h>
#include <qstring.h>

/**
    The Bug class is a simple structure used to contain information about
    bugs within the project.
*/


class Bug
{
    public:
        /** construtor/destructor */
        Bug ();
        ~Bug ();

    public:
        QString     BugID;
        QString     Description;
        QString     Severity;
        QString     BugClass;
        QString     Location;
        QString     AssignedTo;
        QDate       AssignedDate;
        QString     AssignedEMail;
        QString     ReportUserName;
        QString     ReportEMail;
        QDate       ReportDate;
        QString     Package;
        QString     VersionNo;
        QString     Notes;
        QString     Workaround;
        QDate       FixScheduled;
        QString     SysInfo;
        QString     Priority;
        QString     Repeat;
};

#endif

