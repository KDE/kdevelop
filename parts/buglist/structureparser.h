/***************************************************************************
                          structureparser.h  -  description
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

#ifndef STRUCTUREPARSER_H
#define STRUCTUREPARSER_H

#include "buglist.h"
#include <qstring.h>
#include <qxml.h>


// Forward declaration to handle the dependancies.
class BugList;


/**
    Subclassed XML parser which is used to pull the details from our
    XML file and load the data into the dictionary and listbox objects.
*/

class StructureParser : public QXmlDefaultHandler
{
    public:
        StructureParser (BugList * Parent);

        // Document handling code.
        virtual bool startDocument ();
        virtual bool endDocument ();

        // Element handling code.
        virtual bool startElement (const QString & namespaceURI, const QString & localName, const QString & qName, const QXmlAttributes & atts);
        virtual bool endElement (const QString & namespaceURI, const QString & localName, const QString & qName);
        virtual bool characters ( const QString & ch );

        // Add a record to the developer list.
        void AddDeveloper ();

        // We use this to load the listview from our XML handler.
        Bug *           CurrentBug;
        BugList *       m_pParent;

    private:
        // The current state of our FSA.
        typedef enum
        {
            stateUnknown        = 0,
            stateDeveloperID    = 1,
            stateInitials       = 2,
            stateCounter        = 3,
            stateBugID          = 4,
            stateDescription    = 5,
            stateSeverity       = 6,
            stateBugClass       = 7,
            stateReportUserName = 8,
            stateReportEMail    = 9,
            stateReportDate     = 10,
            stateFixed          = 11,
            stateFixDate        = 12,
            statePackage        = 13,
            stateVersionNo      = 14,
            stateNotes          = 15,
            stateLocation       = 16,
            stateWorkaround     = 17,
            stateFixScheduled   = 18,
            stateAssignedTo     = 19,
            stateAssignedDate   = 20,
            stateAssignedEMail  = 21,
            stateSysInfo        = 22,
            statePriority       = 23,
            stateRepeat         = 24
        } StateType;

    private:
        StateType       State;
        int             CurrentCount;
        QString         CurrentInitials;
};


#endif

