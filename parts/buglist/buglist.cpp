/***************************************************************************
                          buglist.cpp  -  description
                             -------------------
    begin                : Sat Nov 11 22:19:31 GMT 2000
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

#include "buglist.h"
#include "bugedit.h"
#include <qdatetime.h>
#include <qdom.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qsortedlist.h>
#include <qxml.h>
#include <fstream.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>

// Column identifiers for the listbox.
#define LST_ID          0
#define LST_DESC        1
#define LST_SEVERITY    2
#define LST_PRIORITY    3
#define LST_AGE         4
#define LST_PACKAGE     5
#define LST_ASSIGNEDTO  6


BugList::BugList(QWidget *parent, const char *name, QString FileName,
                 QString Initials, QString UserName, QString UserEMail)
: QWidget(parent, name)
{
    // Grab our members from the constructor.
    m_FileName = FileName;
    m_Initials = Initials;
    m_UserName = UserName;
    m_UserEMail = UserEMail;

    // Don't let it get too small.
    this->setMinimumSize (500,300);

    // Create a bounding box for the listbox
    pBugListBox = new QHBox(this);
    pBugListBox->setMinimumSize (300,200);

    // Create the list for displaying bugs.
    pMainBugList = new QListView (pBugListBox, "mainbuglist");
    pMainBugList->addColumn (i18n("Bug ID"),72);
    pMainBugList->addColumn (i18n("Description"),190);
    pMainBugList->addColumn (i18n("Severity"),60);
    pMainBugList->addColumn (i18n("Priority"),60);
    pMainBugList->addColumn (i18n("Days Old"),60);
    pMainBugList->addColumn (i18n("Package"),120);
    pMainBugList->addColumn (i18n("Assigned To"),100);
//    pMainBugList->setColumnAlignment (3,Qt::AlignRight);

    // Make the list use all columns for showing focus.
    pMainBugList->setAllColumnsShowFocus (TRUE);

    // Double click on listnox is same as pressing edit buttton.
    connect (pMainBugList, SIGNAL(doubleClicked (QListViewItem * )), this, SLOT(slotListDoubleClicked (QListViewItem *)));

    // Create a bounding box for the maintenance controls
    pMaintenanceBox = new QVBox(this);
    pMaintenanceBox->setMinimumSize (100,200);
    pMaintenanceBox->setMaximumSize (100,200);
    pMaintenanceBox->setSpacing (10);
    pMaintenanceBox->setMargin (0);

    // Create the add button.
    QPushButton *Add = new QPushButton( i18n("Add"), pMaintenanceBox, "add" );
    connect (Add, SIGNAL(clicked()), this, SLOT(slotAddClicked()));
    Add->setMinimumSize (100,25);
    Add->setMaximumSize (100,25);

    // Create the edit button.
    QPushButton *Edit = new QPushButton( i18n("Edit"), pMaintenanceBox, "edit");
    connect (Edit, SIGNAL(clicked()), this, SLOT(slotEditClicked()));
    Edit->setMinimumSize (100,25);
    Edit->setMaximumSize (100,25);

    // Create the delete button.
    QPushButton *Delete = new QPushButton( i18n("Delete"), pMaintenanceBox, "delete" );
    connect (Delete, SIGNAL(clicked()), this, SLOT(slotRemoveClicked()));
    Delete->setMinimumSize (100,25);
    Delete->setMaximumSize (100,25);

    // Create the completed button.
    QPushButton *Completed = new QPushButton( i18n("Completed"), pMaintenanceBox, "completed");
    connect (Completed, SIGNAL(clicked()), this, SLOT(slotCompletedClicked()));
    Completed->setMinimumSize (100,25);
    Completed->setMaximumSize (100,25);

    // Create the Filter button.
    QPushButton *Filter = new QPushButton( i18n("Filter"), pMaintenanceBox, "filter");
    connect (Filter, SIGNAL(clicked()), this, SLOT(slotFilter ()));
    Filter->setMinimumSize (100,25);
    Filter->setMaximumSize (100,25);

    // Create the Ownership button.
    QPushButton *Ownership = new QPushButton( i18n("My Bug"), pMaintenanceBox, "Ownership");
    connect (Ownership, SIGNAL(clicked()), this, SLOT(slotOwnership ()));
    Ownership->setMinimumSize (100,25);
    Ownership->setMaximumSize (100,25);

    // Create a bounding box for the exit controls
    pExitBox = new QVBox(this);
    pExitBox->setMinimumSize (100,60);
    pExitBox->setMaximumSize (100,60);
    pExitBox->setSpacing (10);
    pExitBox->setMargin (0);

    // Create the cancel button.
    QPushButton *Cancel = new KPushButton( KStdGuiItem::cancel(), pExitBox, "cancel" );
    connect (Cancel, SIGNAL(clicked()), this, SLOT(slotCancelClicked()));
    Cancel->setMinimumSize (100,25);
    Cancel->setMaximumSize (100,25);

    // Create the close button.
    QPushButton *Close = new KPushButton( KStdGuiItem::close(), pExitBox, "close");
    connect (Close, SIGNAL(clicked()), this, SLOT(slotCloseClicked()));
    Close->setMinimumSize (100,25);
    Close->setMaximumSize (100,25);

    // Can't be dirty until we change something.
    Dirty = FALSE;

    // Get the dictionary object to clear out the entries on deletion.
    BugDictionary.setAutoDelete (TRUE);

    // Get the dictionary object to clear out the entries on deletion.
    Developers.setAutoDelete (TRUE);

    // Parse the contents of the XML file into memory.
    ParseFile ();

    // Make sure at least we are in there.
    if (!Developers [m_Initials])
    {
        BugCounter *    pBugCounter;

        // Add the current developer settings into the dictionary.
        pBugCounter = new BugCounter;
        pBugCounter->Initials = m_Initials;
        pBugCounter->LastBugNumber = 0;
        Developers.insert (pBugCounter->Initials, pBugCounter);
    }
}


BugList::~BugList()
{
}


void BugList::ParseFile ()
{
    QFile           file (m_FileName);
    QDomDocument    xmlDocument;
    uint            Count;

    // Try to open our bug tracking file.
    if (!file.open (IO_ReadOnly))
    {
        QMessageBox (NULL, i18n("Unable to read the bug tracking file %1").arg(m_FileName));
        return;
    }

    // Try to associate the file with the XML document.
    if (!xmlDocument.setContent(&file))
    {
        QMessageBox (NULL, i18n("The file %1 does not contain valid XML").arg(m_FileName));
        return;
    }

    // Get the main space of the document.
    kdDebug(9040) << "BugList::Parsing the bug list" << endl;
    QDomElement elmMain = xmlDocument.documentElement();

    // Get the list of developers.
    QDomNodeList nodeDevelopers = xmlDocument.elementsByTagName("Developer");
    for (Count = 0;Count < nodeDevelopers.count ();Count++)
    {
        BugCounter *    pBugCounter;

        // Retrieve the developer element.
        QDomElement elmDeveloper = nodeDevelopers.item(Count).toElement();

        // Add the current developer settings into the dictionary.
        pBugCounter = new BugCounter;
        pBugCounter->Initials = elmDeveloper.attribute("Initials");
        pBugCounter->LastBugNumber = elmDeveloper.attribute("Counter").toInt ();
        Developers.insert (pBugCounter->Initials, pBugCounter);
    }

    // Get the list of bugs.
    QDomNodeList nodeBugs = xmlDocument.elementsByTagName("Bug");
    for (Count = 0;Count < nodeBugs.count ();Count++)
    {
        Bug *   pBug;

        // Retrieve the developer element.
        QDomElement elmBug = nodeBugs.item(Count).toElement();

        // Grab the basic attributes.
        pBug = new Bug;
        pBug->BugID = elmBug.attribute("BugID");
        pBug->Description = elmBug.attribute("Description");

        // Work through all the elements.
        QDomNode    nodeChild = elmBug.firstChild ();
        while (!nodeChild.isNull ())
        {
            // Turn the node into an element.
            QDomElement elmSubBug = nodeChild.toElement();

            // Severity
            if (elmSubBug.nodeName () == "Severity")
                pBug->Severity = elmSubBug.text ();

            // BugClass
            if (elmSubBug.nodeName () == "BugClass")
                pBug->BugClass = elmSubBug.text ();

            // Location
            if (elmSubBug.nodeName () == "Location")
                pBug->Location = elmSubBug.text ();

            // AssignedTo
            if (elmSubBug.nodeName () == "AssignedTo")
                pBug->AssignedTo = elmSubBug.text ();

            // AssignedEMail
            if (elmSubBug.nodeName () == "AssignedEMail")
                pBug->AssignedEMail = elmSubBug.text ();

            // ReportUserName
            if (elmSubBug.nodeName () == "ReportUserName")
                pBug->ReportUserName = elmSubBug.text ();

            // ReportEMail
            if (elmSubBug.nodeName () == "ReportEMail")
                pBug->ReportEMail = elmSubBug.text ();

            // Package
            if (elmSubBug.nodeName () == "Package")
                pBug->Package = elmSubBug.text ();

            // VersionNo
            if (elmSubBug.nodeName () == "VersionNo")
                pBug->VersionNo = elmSubBug.text ();

            // Notes
            if (elmSubBug.nodeName () == "Notes")
                pBug->Notes = elmSubBug.text ();

            // Workaround
            if (elmSubBug.nodeName () == "Workaround")
                pBug->Workaround = elmSubBug.text ();

            // SysInfo
            if (elmSubBug.nodeName () == "SysInfo")
                pBug->SysInfo = elmSubBug.text ();

            // Priority
            if (elmSubBug.nodeName () == "Priority")
                pBug->Priority = elmSubBug.text ();

            // Repeat
            if (elmSubBug.nodeName () == "Repeat")
                pBug->Repeat = elmSubBug.text ();

            // AssignedDate
            if (elmSubBug.nodeName () == "AssignedDate")
                pBug->AssignedDate.setYMD (elmSubBug.text ().right (4).toInt (),elmSubBug.text ().mid (elmSubBug.text ().find ('/') + 1,elmSubBug.text ().findRev ('/') - elmSubBug.text ().find ('/') - 1).toInt (),elmSubBug.text ().left (elmSubBug.text ().find ('/')).toInt ());

            // ReportDate
            if (elmSubBug.nodeName () == "ReportDate")
                pBug->ReportDate.setYMD (elmSubBug.text ().right (4).toInt (),elmSubBug.text ().mid (elmSubBug.text ().find ('/') + 1,elmSubBug.text ().findRev ('/') - elmSubBug.text ().find ('/') - 1).toInt (),elmSubBug.text ().left (elmSubBug.text ().find ('/')).toInt ());

            // FixScheduled
            if (elmSubBug.nodeName () == "FixScheduled")
                pBug->FixScheduled.setYMD (elmSubBug.text ().right (4).toInt (),elmSubBug.text ().mid (elmSubBug.text ().find ('/') + 1,elmSubBug.text ().findRev ('/') - elmSubBug.text ().find ('/') - 1).toInt (),elmSubBug.text ().left (elmSubBug.text ().find ('/')).toInt ());

            //pBug->BugClass = elmBug.attribute("BugClass");
            nodeChild = nodeChild.nextSibling ();
        }

        InsertBug (pBug);
    }
}


/**
    Write out an XML file from our dictionary object. Because the dictionary objects
    aren't sorted, I have to do a sort on their indexes before writing out the data.
    The reason for this is to give CVS a fighting chance of merging the file changes
    everyone makes.
*/

void BugList::WriteXMLFile ()
{
    QSortedList     <QString> SortedBugList;
    QSortedList     <QString> SortedDeveloperList;
    QDictIterator   <Bug> BugIterator (BugDictionary);
    QDictIterator   <BugCounter> DevIterator (Developers);
    QString *       pEntry;
    Bug *           pBug;
    BugCounter *    pBugCounter;
    QString         TheDate;

    // We make a sorted list of the bug ID's - why don't QT provide a sort on
    // their dictionary object?
    while (BugIterator.current())
    {
        SortedBugList.append (&BugIterator.current ()->BugID);
        ++BugIterator;
    }
    SortedBugList.sort ();

    // And a sorted list of developers too.
    while (DevIterator.current())
    {
        SortedDeveloperList.append (&DevIterator.current ()->Initials);
        ++DevIterator;
    }
    SortedDeveloperList.sort ();

    QDomDocument    xmlDocument ("BugTracking");
    xmlDocument.appendChild (xmlDocument.createProcessingInstruction ("xml", "version=\"1.0\" encoding=\"UTF-8\""));

    // The main document space.
    QDomElement elmMain = xmlDocument.appendChild (xmlDocument.createElement("BugData")).toElement();

    // Write out the developer list.
    QDomElement elmDevelopers = elmMain.appendChild (xmlDocument.createElement("Developers")).toElement();
    for (pEntry = SortedDeveloperList.first(); pEntry != NULL; pEntry = SortedDeveloperList.next ())
    {
        // Get a pointer to our object.
        pBugCounter = Developers [*pEntry];

        // Pump it out.
        QDomElement elmDeveloper = elmDevelopers.appendChild (xmlDocument.createElement("Developer")).toElement();
        elmDeveloper.setAttribute ("Initials",pBugCounter->Initials);
        elmDeveloper.setAttribute ("Counter",pBugCounter->LastBugNumber);
    }

    // Output the bug list.
    QDomElement elmBugs = elmMain.appendChild(xmlDocument.createElement("Bugs")).toElement();
    for (pEntry = SortedBugList.first(); pEntry != NULL; pEntry = SortedBugList.next ())
    {
        // Get a pointer to our object.
        pBug = BugDictionary [*pEntry];
        QDomElement elmBug = elmBugs.appendChild (xmlDocument.createElement("Bug")).toElement();

        // BugID
        if (pBug->BugID)
            elmBug.setAttribute ("BugID",pBug->BugID);

        // Description
        if (pBug->Description)
            elmBug.setAttribute ("Description",pBug->Description);

         // Severity
        if (pBug->Severity)
        {
            QDomElement elmSeverity = elmBug.appendChild (xmlDocument.createElement("Severity")).toElement();
            elmSeverity.appendChild (xmlDocument.createTextNode(pBug->Severity)).toElement();
        }

        // Priority
        if (pBug->Priority)
        {
            QDomElement elmPriority = elmBug.appendChild (xmlDocument.createElement("Priority")).toElement();
            elmPriority.appendChild (xmlDocument.createTextNode(pBug->Priority)).toElement();
        }

        // BugClass
        if (pBug->BugClass)
        {
            QDomElement elmBugClass = elmBug.appendChild (xmlDocument.createElement("BugClass")).toElement();
            elmBugClass.appendChild (xmlDocument.createTextNode(pBug->BugClass)).toElement();
        }

        // Location
        if (pBug->Location)
        {
            QDomElement elmLocation = elmBug.appendChild (xmlDocument.createElement("Location")).toElement();
            elmLocation.appendChild (xmlDocument.createTextNode(pBug->Location)).toElement();
        }

        // AssignedTo
        if (pBug->AssignedTo)
        {
            QDomElement elmAssignedTo = elmBug.appendChild (xmlDocument.createElement("AssignedTo")).toElement();
            elmAssignedTo.appendChild (xmlDocument.createTextNode(pBug->AssignedTo)).toElement();
        }

        // AssignedEMail
        if (pBug->AssignedEMail)
        {
            QDomElement elmAssignedEMail = elmBug.appendChild (xmlDocument.createElement("AssignedEMail")).toElement();
            elmAssignedEMail.appendChild (xmlDocument.createTextNode(pBug->AssignedEMail)).toElement();
        }

        // ReportUserName
        if (pBug->ReportUserName)
        {
            QDomElement elmReportUserName = elmBug.appendChild (xmlDocument.createElement("ReportUserName")).toElement();
            elmReportUserName.appendChild (xmlDocument.createTextNode(pBug->ReportUserName)).toElement();
        }

        // ReportEMail
        if (pBug->ReportEMail)
        {
            QDomElement elmReportEMail = elmBug.appendChild (xmlDocument.createElement("ReportEMail")).toElement();
            elmReportEMail.appendChild (xmlDocument.createTextNode(pBug->ReportEMail)).toElement();
        }

        // Package
        if (pBug->Package)
        {
            QDomElement elmPackage = elmBug.appendChild (xmlDocument.createElement("Package")).toElement();
            elmPackage.appendChild (xmlDocument.createTextNode(pBug->Package)).toElement();
        }

        // VersionNo
        if (pBug->VersionNo)
        {
            QDomElement elmVersionNo = elmBug.appendChild (xmlDocument.createElement("VersionNo")).toElement();
            elmVersionNo.appendChild (xmlDocument.createTextNode(pBug->VersionNo)).toElement();
        }

        // Notes
        if (pBug->Notes)
        {
            QDomElement elmNotes = elmBug.appendChild (xmlDocument.createElement("Notes")).toElement();
            elmNotes.appendChild (xmlDocument.createTextNode(pBug->Notes)).toElement();
        }

        // Workaround
        if (pBug->Workaround)
        {
            QDomElement elmWorkaround = elmBug.appendChild (xmlDocument.createElement("Workaround")).toElement();
            elmWorkaround.appendChild (xmlDocument.createTextNode(pBug->Workaround)).toElement();
        }

        // Repeat
        if (pBug->Repeat)
        {
            QDomElement elmRepeat = elmBug.appendChild (xmlDocument.createElement("Repeat")).toElement();
            elmRepeat.appendChild (xmlDocument.createTextNode(pBug->Repeat)).toElement();
        }

        // SysInfo
        if (pBug->SysInfo)
        {
            QDomElement elmSysInfo = elmBug.appendChild (xmlDocument.createElement("SysInfo")).toElement();
            elmSysInfo.appendChild (xmlDocument.createTextNode(pBug->SysInfo)).toElement();
        }

        // AssignedDate
        if (!pBug->AssignedDate.isNull ())
        {
            QDomElement elmAssignedDate = elmBug.appendChild (xmlDocument.createElement("AssignedDate")).toElement();
            TheDate.sprintf ("%i/%i/%i",pBug->AssignedDate.day (), pBug->AssignedDate.month (), pBug->AssignedDate.year ());
            elmAssignedDate.appendChild (xmlDocument.createTextNode(TheDate)).toElement();
        }

        // ReportDate
        if (!pBug->ReportDate.isNull ())
        {
            QDomElement elmReportDate = elmBug.appendChild (xmlDocument.createElement("ReportDate")).toElement();
            TheDate.sprintf ("%i/%i/%i",pBug->ReportDate.day (), pBug->ReportDate.month (), pBug->ReportDate.year ());
            elmReportDate.appendChild (xmlDocument.createTextNode(TheDate)).toElement();
        }

        // FixScheduled
        if (!pBug->FixScheduled.isNull ())
        {
            QDomElement elmFixScheduled = elmBug.appendChild (xmlDocument.createElement("FixScheduled")).toElement();
            TheDate.sprintf ("%i/%i/%i",pBug->FixScheduled.day (), pBug->FixScheduled.month (), pBug->FixScheduled.year ());
            elmFixScheduled.appendChild (xmlDocument.createTextNode(TheDate)).toElement();
        }

        ++BugIterator;
    }

    // Save the document to a file.
    QFile file (m_FileName);
    if (!file.open (IO_WriteOnly))
    {
        QMessageBox (NULL, i18n("Unable to save the bug tracking file"));
        return;
    }
    QTextStream s(&file);
    xmlDocument.save (s,4);
    file.close();
}


// Add the bug passed as a parameter to our listbox and dictionary.

void BugList::InsertBug (Bug * pBug)
{
    QString     Age;
    char        tmp [10];

    // Work out the age of the request and zero pad it so it sorts in the list.
    if (!pBug->ReportDate.isNull ())
    {
        sprintf (tmp,"%03i",pBug->ReportDate.daysTo (QDate::currentDate ()));
        Age = tmp;
    }
    else
    {
        // Age unknown...
        Age = " - ";
    }

    // Add the bug to the dictionary.
    BugDictionary.insert (pBug->BugID,pBug);

    // Add the current bug item to the list.
    (void) new QListViewItem (pMainBugList, pBug->BugID, pBug->Description, pBug->Severity, pBug->Priority, Age, pBug->Package, pBug->AssignedTo);
}


// Add the bug passed as a parameter to our listbox and dictionary.
// NOTE: This does not mark the buffer as dirty because I use this method
// from the structure parser to load the listbox on startup.

void BugList::slotAddBug (Bug * pBug)
{
    // Do the insertion.
    InsertBug (pBug);

    // Mark the buffer as being dirty now.
    Dirty = TRUE;
}


void BugList::slotUpdateBug (Bug * pBug)
{
    QListViewItem *     pItem;
    QString             Age;
    char                tmp [10];

    // Work out the age of the request and zero pad it so it sorts in the list.
    if (!pBug->ReportDate.isNull ())
    {
        sprintf (tmp,"%03i",pBug->ReportDate.daysTo (QDate::currentDate ()));
        Age = tmp;
    }
    else
    {
        // Age unknown...
        Age = " - ";
    }

    pItem = pMainBugList->currentItem ();
    if (pItem != NULL)
    {
        // Update the text in the listbox.
        pItem->setText (LST_DESC,pBug->Description);
        pItem->setText (LST_SEVERITY,pBug->Severity);
        pItem->setText (LST_AGE,Age);
        pItem->setText (LST_PACKAGE,pBug->Package);
        pItem->setText (LST_ASSIGNEDTO,pBug->AssignedTo);
        pItem->setText (LST_PRIORITY,pBug->Priority);

        // Let them know when they exit that changes have occured.
        Dirty = TRUE;
    }
}


// They pressed the 'Add' button.

void BugList::slotAddClicked()
{
    Bug *           pBug;
    BugCounter *    pBugCounter;

    // Create a bug for the editor to work on.
    pBug = new Bug;

    // Try to grab a developer's bug counter.
    pBugCounter = Developers [m_Initials];

    // They didn't have a matching set of initials. We better add it now.
    if (!pBugCounter)
    {
        BugCounter *    pBugCounter;

        // Add the current developer settings into the dictionary.
        pBugCounter = new BugCounter;
        pBugCounter->Initials = m_Initials;
        pBugCounter->LastBugNumber = 0;
        Developers.insert (pBugCounter->Initials, pBugCounter);

        // And get that counter again.
        pBugCounter = Developers [m_Initials];
    }

    // Get a suitable ID.
    pBug->BugID = pBugCounter->GetNextID ();

    // Fire up the bug editor on the new bug we created.
    BugEdit * pBugEdit = new BugEdit (this,"bugedit",pBug,TRUE,TRUE);
    pBugEdit->setGeometry (0,0,400,400);

    // Allow the editor to signal when we should add an item to our list.
    connect (pBugEdit, SIGNAL(sigAddBug (Bug *)), this, SLOT(slotAddBug (Bug *)));

    // Show the edit screen.
    pBugEdit->show();
}


// They pressed the 'Edit' button

void BugList::slotEditClicked()
{
        Bug *           pBug;

    // Make sure there is an entry selected and then delete.
    if (pMainBugList->currentItem () != NULL)
    {
        // Get a pointer to the entry they want to edit.
        pBug = BugDictionary [pMainBugList->currentItem ()->text (0)];

        // Fire up the bug editor on this existing bug.
        BugEdit * pBugEdit = new BugEdit (this,"bugedit",pBug,FALSE,TRUE);
        pBugEdit->setGeometry (0,0,400,400);

        // Allow the editor to signal when we should update our item.
        connect (pBugEdit, SIGNAL(sigUpdateBug (Bug *)), this, SLOT(slotUpdateBug (Bug *)));

        // Show the edit screen.
        pBugEdit->show();
    }
}


// They double clicked the listbox. Same effect as clicking the edit button.

void BugList::slotListDoubleClicked (QListViewItem *)
{
    slotEditClicked ();
}


// They pressed the 'Remove' button.

void BugList::slotRemoveClicked()
{
    QMessageBox     MB;

    // Make sure there is an entry selected and then delete.
    if (pMainBugList->currentItem () != NULL)
    {
        // Give them a chance to abort if the list has changed.
        if (MB.warning (this,
            i18n ("Delete Bug Report"),
            i18n ("When you delete a bug instead of closing\n"
                  "it, there will no notification sent to\n"
                  "the person who reported the bug or to\n"
                  "any relevant newsgroups.\n\n"
                  "Are you sure you want to do this?"),
            QMessageBox::Yes,QMessageBox::Cancel | QMessageBox::Default) == QMessageBox::Yes)
        {
            // Delete the current item from the list and dictionary.
            BugDictionary.remove (pMainBugList->currentItem ()->text (0));
            delete pMainBugList->currentItem ();

            // Let them know when they exit that changes have occured.
            Dirty = TRUE;
        }
    }
}


// They pressed the 'Completed' button. This means they want to mark this
// item as being completed. In this implementation, this is the same as
// pressing the delete button. I intend to expand this to send a bug
// completion notification mail in a later version to the group
// and the origninator.

void BugList::slotCompletedClicked()
{
    // Should perhaps check to delete first?

    // Make sure there is an entry selected and then delete.
    if (pMainBugList->currentItem () != NULL)
    {
        // Delete the current item from the list and dictionary.
        BugDictionary.remove (pMainBugList->currentItem ()->text (0));
        delete pMainBugList->currentItem ();

        // Let them know when they exit that changes have occured.
        Dirty = TRUE;
    }
}


// They pressed the 'Close' button. We should exit and update the
// file if it has changed.

void BugList::slotCloseClicked()
{
    // Check to see if the file has changed.
    if (Dirty)
    {
        QMessageBox     MB;
        int             Result;

        Result = MB.warning (this,
            i18n ("The Bug List has Changed"),
            i18n ("The bug list has been changed.\n"
                  "Do you want to save your changes?"),
            QMessageBox::Yes | QMessageBox::Default,QMessageBox::No,QMessageBox::Cancel);
        if (Result == QMessageBox::Yes)
        {
            // Save the contents out to a file.
            WriteXMLFile ();
        }

        // They decided not to exit after all.
        if (Result == QMessageBox::Cancel)
            return;
    }

    // Shut down.
    close ();
    emit signalDeactivate ();
}


// They pressed the 'Cancel' button. We should exit without saving, checking
// first to ensure they want to lose all changes.

void BugList::slotCancelClicked()
{
    if (Dirty)
    {
        QMessageBox     MB;

        // Give them a chance to abort if the list has changed.
        if (MB.warning (this,
            i18n ("The Bug List has Changed"),
            i18n ("The bug list has been changed.\n"
                  "If you exit now you will lose all your changes.\n"
                  "Are you sure you want to exit?"),
            QMessageBox::Yes,QMessageBox::Cancel | QMessageBox::Default) == QMessageBox::Cancel)
        {
            // Ignore the button press.
            return;
        }
    }

    // Shut down.
    close ();
    emit signalDeactivate ();
}


void BugList::resizeEvent ( QResizeEvent * QEvent)
{
    // Resize the layout of the screen.
    pBugListBox->setGeometry (10, 10, this->width () - 130, this->height () - 20);
    pMaintenanceBox->setGeometry (this->width () - 110, 10, 100, 200);
    pExitBox->setGeometry (this->width () - 110, this->height () - 70, 100, 60);

    // Proportionally change the width of the columns in the listbox.
//    BugListBox

    // Pass the event down to our subclass.
    QWidget::resizeEvent (QEvent);
}


void BugList::slotFilter ()
{

}

/*  Allows the current user to take ownership of this bug, if it is not
    currently assigned to anyone else. */

void BugList::slotOwnership ()
{
    QMessageBox     MB;
    Bug *           pBug;

    // Make sure there is an entry selected and then delete.
    if (pMainBugList->currentItem () != NULL)
    {
        // Get a pointer to the entry they want to edit.
        pBug = BugDictionary [pMainBugList->currentItem ()->text (0)];

        if ((pBug->AssignedTo != m_UserName) && (pBug->AssignedTo.length () > 0))
        {
            if (MB.warning (this,
                i18n ("Bug Owner Already Set"),
                i18n ("The owner information for this bug\n"
                      "has already been set. Are you\n"
                      "sure you want to change it?"),
                QMessageBox::Yes | QMessageBox::Default,QMessageBox::No) != QMessageBox::Yes)
            {
                return;
            }
        }

        // Update the fields to our settings.
        pBug->AssignedTo = m_UserName;
        pBug->AssignedDate = QDateTime::currentDateTime ().date ();
        pBug->AssignedEMail = m_UserEMail;

        // Update the list.
        slotUpdateBug (pBug);
    }
}
#include "buglist.moc"
