/***************************************************************************
                          structureparser.cpp  -  description
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

#include "structureparser.h"


StructureParser::StructureParser (BugList * Parent)
{
    m_pParent = Parent;
}


bool StructureParser::startDocument()
{
    // We would need to clear the list down before begining work.
    m_pParent->pMainBugList->clear ();

    return TRUE;
}


bool StructureParser::endDocument()
{
    return TRUE;
}


bool StructureParser::startElement (const QString & namespaceURI, const QString & localName, const QString & qName, const QXmlAttributes & atts)
{
    QString     Tag;

    // Just trying to ensure we recognise the tags, regardless of case.
    Tag = qName.upper ();

    // If this is the main tag, we will clear out the variable used for storing the tag values.
    if (Tag == "BUG")
    {
        //memset (&CurrentBug,sizeof (CurrentBug),0);
        CurrentBug = new Bug;
    }

    // BugID
    if (Tag == "BUGID")
    {
        State = stateBugID;
    }
    else if (Tag == "DESCRIPTION")
    {
        State = stateDescription;
    }
    else if (Tag == "SEVERITY")
    {
        State = stateSeverity;
    }
    else if (Tag == "CLASS")
    {
        State = stateBugClass;
    }
    else if (Tag == "LOCATION")
    {
        State = stateLocation;
    }
    else if (Tag == "REPORTUSERNAME")
    {
        State = stateReportUserName;
    }
    else if (Tag == "REPORTEMAIL")
    {
        State = stateReportEMail;
    }
    else if (Tag == "REPORTDATE")
    {
        State = stateReportDate;
    }
    else if (Tag == "FIXED")
    {
        State = stateFixed;
    }
    else if (Tag == "FIXDATE")
    {
        State = stateFixDate;
    }
    else if (Tag == "PACKAGE")
    {
        State = statePackage;
    }
    else if (Tag == "VERSIONNO")
    {
        State = stateVersionNo;
    }
    else if (Tag == "NOTES")
    {
        State = stateNotes;
    }
    else if (Tag == "WORKAROUND")
    {
        State = stateWorkaround;
    }
    else if (Tag == "FIXSCHEDULED")
    {
        State = stateFixScheduled;
    }
    else if (Tag == "ASSIGNEDTO")
    {
        State = stateAssignedTo;
    }
    else if (Tag == "ASSIGNEDDATE")
    {
        State = stateAssignedDate;
    }
    else if (Tag == "ASSIGNEDEMAIL")
    {
        State = stateAssignedEMail;
    }
    else if (Tag == "REPEAT")
    {
        State = stateRepeat;
    }
    else if (Tag == "SYSINFO")
    {
        State = stateSysInfo;
    }
    else if (Tag == "PRIORITY")
    {
        State = statePriority;
    }
    else if (Tag == "DEVELOPER")
    {
        State = stateDeveloperID;
    }
    else if (Tag == "INITIALS")
    {
        State = stateInitials;
    }
    else if (Tag == "COUNTER")
    {
        State = stateCounter;
    }
    else
    {
        State = stateUnknown;
    }

    return TRUE;
}


bool StructureParser::characters ( const QString & ch )
{
    switch (State)
    {
        case stateBugID:
            CurrentBug->BugID = ch;
            break;

        case stateDescription:
            CurrentBug->Description = ch;
            break;

        case stateSeverity:
            CurrentBug->Severity = ch;
            break;

        case stateBugClass:
            CurrentBug->BugClass = ch;
            break;

        case stateLocation:
            CurrentBug->Location = ch;
            break;

        case stateReportUserName:
            CurrentBug->ReportUserName = ch;
            break;

        case stateReportEMail:
            CurrentBug->ReportEMail = ch;
            break;

        case statePackage:
            CurrentBug->Package = ch;
            break;

        case stateVersionNo:
            CurrentBug->VersionNo = ch;
            break;

        case stateNotes:
            CurrentBug->Notes = ch;
            break;

        case stateWorkaround:
            CurrentBug->Workaround = ch;
            break;

        case stateAssignedTo:
            CurrentBug->AssignedTo = ch;
            break;

        case stateAssignedEMail:
            CurrentBug->AssignedEMail = ch;
            break;

        case stateRepeat:
            CurrentBug->Repeat = ch;
            break;

        case stateSysInfo:
            CurrentBug->SysInfo = ch;
            break;

        case statePriority:
            CurrentBug->Priority = ch;
            break;

        case stateDeveloperID:
            // Just in case.
            CurrentInitials = "";
            CurrentCount = 1;
            break;

        case stateInitials:
            CurrentInitials = ch;
            break;

        case stateCounter:
            CurrentCount = ch.toInt ();
            break;

        case stateFixed:
            if (ch.toInt () == 0)
                CurrentBug->Fixed = FALSE;
            else
                CurrentBug->Fixed = TRUE;
            break;

        case stateReportDate:
            // Date is read from file in format dd/mm/yyyy
            CurrentBug->ReportDate.setYMD (ch.right (4).toInt (),ch.mid (ch.find ('/') + 1,ch.findRev ('/') - ch.find ('/') - 1).toInt (),ch.left (ch.find ('/')).toInt ());//ch;
            break;

        case stateFixDate:
            // Date is read from file in format dd/mm/yyyy
            CurrentBug->FixDate.setYMD (ch.right (4).toInt (),ch.mid (ch.find ('/') + 1,ch.findRev ('/') - ch.find ('/') - 1).toInt (),ch.left (ch.find ('/')).toInt ());//ch;
            break;

        case stateAssignedDate:
            // Date is read from file in format dd/mm/yyyy
            CurrentBug->AssignedDate.setYMD (ch.right (4).toInt (),ch.mid (ch.find ('/') + 1,ch.findRev ('/') - ch.find ('/') - 1).toInt (),ch.left (ch.find ('/')).toInt ());//ch;
            break;

        case stateFixScheduled:
            // Date is read from file in format dd/mm/yyyy
            CurrentBug->FixScheduled.setYMD (ch.right (4).toInt (),ch.mid (ch.find ('/') + 1,ch.findRev ('/') - ch.find ('/') - 1).toInt (),ch.left (ch.find ('/')).toInt ());//ch;
            break;

        case stateUnknown:
            // This is an error condition.
            break;
    }

    return TRUE;
}


bool StructureParser::endElement (const QString & namespaceURI, const QString & localName, const QString & qName)
{
    QString     Tag;

    // Just trying to ensure we recognise the tags, regardless of case.
    Tag = qName.upper ();

    // When a bug tag is closed we should add the bug to the listbox.
    if (Tag == "BUG")
    {
        m_pParent->InsertBug (CurrentBug);
    }
    else if (Tag == "DEVELOPER")
    {
        AddDeveloper ();
    }

    return TRUE;
}


void StructureParser::AddDeveloper ()
{
    BugCounter *    pBugCounter;

    // Add the current developer settings into the dictionary.
    pBugCounter = new BugCounter;
    pBugCounter->Initials = CurrentInitials;
    pBugCounter->LastBugNumber = CurrentCount;
    m_pParent->Developers.insert (pBugCounter->Initials, pBugCounter);
}
