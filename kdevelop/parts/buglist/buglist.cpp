/***************************************************************************
                          buglist.cpp  -  description
                             -------------------
    begin                : Sat Nov 11 22:19:31 GMT 2000
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

#include "buglist.h"
#include "bugedit.h"
#include <qdatetime.h>
#include <qmessagebox.h>
#include <qsortedlist.h>
#include <fstream.h>
#include <klocale.h>

// Column identifiers for the listbox.
#define LST_ID          0
#define LST_DESC        1
#define LST_SEVERITY    2
#define LST_PRIORITY    3
#define LST_AGE         4
#define LST_PACKAGE     5
#define LST_ASSIGNEDTO  6

BugList::BugList(QWidget *parent, const char *name)
: QWidget(parent, name)
{
    // Don't let it get too small.
    this->setMinimumSize (500,300);

    // Create a bounding box for the listbox
    pBugListBox = new QHBox(this);
    pBugListBox->setMinimumSize (300,200);

    // Create the list for displaying bugs.
    pMainBugList = new QListView (pBugListBox, "mainbuglist");
    pMainBugList->addColumn ("Bug ID",72);
    pMainBugList->addColumn ("Description",190);
    pMainBugList->addColumn ("Severity",60);
    pMainBugList->addColumn ("Priority",60);
    pMainBugList->addColumn ("Days Old",60);
    pMainBugList->addColumn ("Package",120);
    pMainBugList->addColumn ("Assigned To",100);
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
    QPushButton *Add = new QPushButton( "Add", pMaintenanceBox, "add" );
    connect (Add, SIGNAL(clicked()), this, SLOT(slotAddClicked()));
    Add->setMinimumSize (100,25);
    Add->setMaximumSize (100,25);

    // Create the edit button.
    QPushButton *Edit = new QPushButton( "Edit", pMaintenanceBox, "edit");
    connect (Edit, SIGNAL(clicked()), this, SLOT(slotEditClicked()));
    Edit->setMinimumSize (100,25);
    Edit->setMaximumSize (100,25);

    // Create the delete button.
    QPushButton *Delete = new QPushButton( "Delete", pMaintenanceBox, "delete" );
    connect (Delete, SIGNAL(clicked()), this, SLOT(slotRemoveClicked()));
    Delete->setMinimumSize (100,25);
    Delete->setMaximumSize (100,25);

    // Create the completed button.
    QPushButton *Completed = new QPushButton( "Completed", pMaintenanceBox, "completed");
    connect (Completed, SIGNAL(clicked()), this, SLOT(slotCompletedClicked()));
    Completed->setMinimumSize (100,25);
    Completed->setMaximumSize (100,25);

    // Create the Filter button.
    QPushButton *Filter = new QPushButton( "Filter", pMaintenanceBox, "filter");
    connect (Filter, SIGNAL(clicked()), this, SLOT(slotFilter ()));
    Filter->setMinimumSize (100,25);
    Filter->setMaximumSize (100,25);

    // Create the Ownership button.
    QPushButton *Ownership = new QPushButton( "My Bug", pMaintenanceBox, "Ownership");
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
    QPushButton *Cancel = new QPushButton( "Cancel", pExitBox, "cancel" );
    connect (Cancel, SIGNAL(clicked()), this, SLOT(slotCancelClicked()));
    Cancel->setMinimumSize (100,25);
    Cancel->setMaximumSize (100,25);

    // Create the close button.
    QPushButton *Close = new QPushButton( "Close", pExitBox, "close");
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
    FileName = "/home/ivan/buglist/bugs.xml";
    ParseFile ();

    // Make sure at least we are in there.
    if (Developers.isEmpty ())
    {
        BugCounter *    pBugCounter;

        // Add the current developer settings into the dictionary.
        pBugCounter = new BugCounter;
        pBugCounter->Initials = "ILH";
        pBugCounter->LastBugNumber = 1;
        Developers.insert (pBugCounter->Initials, pBugCounter);
    }
}


BugList::~BugList()
{
}


void BugList::ParseFile ()
{
    // Create a handler.
    StructureParser     handler (this);

    // Use the simple reader to parse our file.
    QFile               xmlFile (FileName);
    QXmlInputSource     source (xmlFile);
    QXmlSimpleReader    reader;

    // Begin to parse the XML file.
    reader.setFeature ("http://trolltech.com/xml/features/report-whitespace-only-CharData",FALSE);
    reader.setContentHandler (&handler);
    reader.parse (source);
}


/**
    Write out an XML file from our dictionary object. Because the dictionary objects
    aren't sorted, I have to do a sort on their indexes before writing out the data.
    The reason for this is to give CVS a fighting chance of merging the file changes
    everyone makes.
*/

void BugList::WriteXMLFile ()
{
    ofstream        Out (FileName,ios::trunc | ios::out);
    QSortedList     <QString> SortedBugList;
    QSortedList     <QString> SortedDeveloperList;
    QDictIterator   <Bug> BugIterator (BugDictionary);
    QDictIterator   <BugCounter> DevIterator (Developers);
    QString *       pEntry;
    Bug *           pBug;
    BugCounter *    pBugCounter;

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

    // Write the header.
    Out << "<?xml version='1.0'?>" << endl;
    Out << "<Bugs>" << endl;

    // Out with the developer list.
    Out << "  <Developers>" << endl;
    for (pEntry = SortedDeveloperList.first(); pEntry != NULL; pEntry = SortedDeveloperList.next ())
    {
        // Get a pointer to our object.
        pBugCounter = Developers [*pEntry];

        // Pump it out.
        Out << "    <Developer>" << endl;
        if (pBugCounter->Initials)
            Out << "      <Initials>" << pBugCounter->Initials << "</Initials>" << endl;
        Out << "      <Counter>" << pBugCounter->LastBugNumber << "</Counter>" << endl;
        Out << "    </Developer>" << endl;
    }
    Out << "  </Developers>" << endl;

    // Output the bug list.
    for (pEntry = SortedBugList.first(); pEntry != NULL; pEntry = SortedBugList.next ())
    {
        // Get a pointer to our object.
        pBug = BugDictionary [*pEntry];

        // Iterate the dictionary.
        Out << "  <Bug>" << endl;

        // BugID
        if (pBug->BugID)
            Out << "    <BugID>" << pBug->BugID << "</BugID>" << endl;

        // Description
        if (pBug->Description)
            Out << "    <Description>" << pBug->Description << "</Description>" << endl;

        // Severity
        if (pBug->Severity)
            Out << "    <Severity>" << pBug->Severity << "</Severity>" << endl;

        // Priority
        if (pBug->Priority)
            Out << "    <Priority>" << pBug->Priority << "</Priority>" << endl;

        // BugClass
        if (pBug->BugClass)
            Out << "    <Class>" << pBug->BugClass << "</Class>" << endl;

        // Location
        if (pBug->Location)
            Out << "    <Location>" << pBug->Location << "</Location>" << endl;

        // AssignedTo
        if (pBug->AssignedTo)
            Out << "    <AssignedTo>" << pBug->AssignedTo << "</AssignedTo>" << endl;

        // AssignedDate
        if (!pBug->AssignedDate.isNull ())
        {
            Out << "    <AssignedDate>";
            Out << pBug->AssignedDate.day () << "/";
            Out << pBug->AssignedDate.month () << "/";
            Out << pBug->AssignedDate.year ();
            Out << "</AssignedDate>" << endl;
        }

        // AssignedEMail
        if (pBug->AssignedEMail)
            Out << "    <AssignedEMail>" << pBug->AssignedEMail << "</AssignedEMail>" << endl;

        // ReportUserName
        if (pBug->ReportUserName)
            Out << "    <ReportUserName>" << pBug->ReportUserName << "</ReportUserName>" << endl;

        // ReportEMail
        if (pBug->ReportEMail)
            Out << "    <ReportEMail>" << pBug->ReportEMail << "</ReportEMail>" << endl;

        // ReportDate
        if (!pBug->ReportDate.isNull ())
        {
            Out << "    <ReportDate>";
            Out << pBug->ReportDate.day () << "/";
            Out << pBug->ReportDate.month () << "/";
            Out << pBug->ReportDate.year ();
            Out << "</ReportDate>" << endl;
        }

        // Fixed
        Out << "    <Fixed>" << pBug->Fixed << "</Fixed>" << endl;

        // FixDate
        if (!pBug->FixDate.isNull ())
        {
            Out << "    <FixDate>";
            Out << pBug->FixDate.day () << "/";
            Out << pBug->FixDate.month () << "/";
            Out << pBug->FixDate.year ();
            Out << "</FixDate>" << endl;
        }

        // Package
        if (pBug->Package)
            Out << "    <Package>" << pBug->Package << "</Package>" << endl;

        // VersionNo
        if (pBug->VersionNo)
            Out << "    <VersionNo>" << pBug->VersionNo << "</VersionNo>" << endl;

        // Notes
        if (pBug->Notes)
            Out << "    <Notes>" << pBug->Notes << "</Notes>" << endl;

        // Workaround
        if (pBug->Workaround)
            Out << "    <Workaround>" << pBug->Workaround << "</Workaround>" << endl;

        // FixScheduled
        if (!pBug->FixScheduled.isNull ())
        {
            Out << "    <FixScheduled>";
            Out << pBug->FixScheduled.day () << "/";
            Out << pBug->FixScheduled.month () << "/";
            Out << pBug->FixScheduled.year ();
            Out << "</FixScheduled>" << endl;
        }

        // Repeat
        if (pBug->Repeat)
            Out << "    <Repeat>" << pBug->Repeat << "</Repeat>" << endl;

        // SysInfo
        if (pBug->SysInfo)
            Out << "    <SysInfo>" << pBug->SysInfo << "</SysInfo>" << endl;

        // Next dictionary item now.
        Out << "  </Bug>" << endl;

        ++BugIterator;
    }

    // Close off the XML.
    Out << "</Bugs>" << endl;
}


// Add the bug passed as a parameter to our listbox and dictionary.

void BugList::slotAddBug (Bug * pBug)
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
    }
}


// They pressed the 'Add' button.

void BugList::slotAddClicked()
{
    Bug *           pBug;
    BugCounter *    pBugCounter;

    // Create a bug for the editor to work on.
    pBug = new Bug;

    // Get a new bug id.
    pBugCounter = Developers ["ILH"];
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

        // Let them know when they exit that changes have occured.
        Dirty = TRUE;
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
            i18n ("The bug list has been changed,\n"
                  "do you want to save your changes?"),
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

        if ((pBug->AssignedTo != "Ivan Hawkes") && (pBug->AssignedTo.length () > 0))
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
        pBug->AssignedTo = "Ivan Hawkes";
        pBug->AssignedDate = QDateTime::currentDateTime ().date ();
        pBug->AssignedEMail = "linuxgroupie@ivanhawkes.com";

        // Update the list.
        slotUpdateBug (pBug);

        // Let them know when they exit that changes have occured.
        Dirty = TRUE;
    }

}
