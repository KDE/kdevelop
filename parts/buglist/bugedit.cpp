/***************************************************************************
                          bugedit.cpp  -  description
                             -------------------
    begin                : Mon Nov 13 2000
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

#include "bugedit.h"
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qtabbar.h>
#include <klocale.h>


// Strings used for the severity of the bug.
// Make this international when I figure out how.
// Might even read this from the XML file in time.

const char * severity_desc [] =
{
    "critical",
    "grave",
    "crash",
    "normal",
    "low",
    "wishlist",
    0
};


// Strings used for the priority of the bug.
// Make this international when I figure out how.
// Might even read this from the XML file in time.

const char * priority_desc [] =
{
    "critical",
    "high",
    "normal",
    "low",
    0
};


// Convert a date to a string for editing. This is not properly internationalised, sorry guys.
// I haven't got a grip on the best way to handle localisation yet :)

const QString BugEdit::DateToQString (QDate InputDate)
{
    QString     OutputString;

    OutputString.sprintf ("%02i/%02i/%04i", InputDate.day (), InputDate.month (), InputDate.year ());

    return OutputString;
}


// Convert a QString to a date - again, this is not localised. Sorry, coming soonish :)

const QDate BugEdit::QStringToDate (QString InputString)
{
    QDate   OutputDate;
    int     FirstSlash;
    int     LastSlash;
    int     Day;
    int     Month;
    int     Year;

    // Avoid empty strings.
    if (InputString.length () > 0)
    {
        // Work out the position of the delimiters.
        FirstSlash = InputString.find ('/');
        LastSlash = InputString.findRev ('/');

        // Date is read from file in format dd/mm/yyyy
        Day = InputString.left (FirstSlash).toInt ();
        Month = InputString.mid (FirstSlash + 1, LastSlash - FirstSlash - 1).toInt ();
        Year = InputString.right (InputString.length () - LastSlash - 1).toInt ();

        // Handle those damn two digit dates.
        if (Year < 100)
        {
            // Basic, cheap-ass, date windowing system.
            if (Year < 70)
                Year += 2000;
            else
                Year += 1900;
        }

        // Attempt to get a sensible date.
        OutputDate.setYMD (Year, Month, Day);
    }

    return OutputDate;
}

BugEdit::BugEdit (QWidget * parent, const char * name, Bug * pBug, bool AddItem, bool modal, WFlags f)
: QTabDialog (parent, name, modal)
{
    int     Count;

    // Let them know what we are doing.
    setCaption (i18n ("Add/Edit Bug Record"));

    // Remember which bug we are meant to edit.
    m_pBug = pBug;
    m_AddItem = AddItem;

    // Create the tab sheets.
    pSheetGeneral = new QHBox (this,"shtGeneral");
    pSheetNotes = new QHBox (this,"shtNotes");
    pSheetRepeat = new QHBox (this,"shtRepeat");
    pSheetWorkaround = new QHBox (this,"shtWorkaround");
    pSheetSysInfo = new QHBox (this,"shtSysInfo");

    // Place a grid on the frame.
    GeneralLayout = new QGrid (2,pSheetGeneral, "gridGeneral");
    GeneralLayout->setMargin (10);
    GeneralLayout->setSpacing (4);

    // Add in our tabs.
    addTab (pSheetGeneral,"General");
    addTab (pSheetNotes,"Notes");
    addTab (pSheetRepeat,"Repeat");
    addTab (pSheetWorkaround,"Workaround");
    addTab (pSheetSysInfo,"SysInfo");

    // LABEL: BugID
    QLabel *lblBugID = new QLabel (i18n ("Bug ID:"),GeneralLayout);

    // LINEEDIT: BugID
    editBugID = new QLineEdit(GeneralLayout);
    lblBugID->setBuddy (editBugID);
    editBugID->setGeometry (10,10,80,20);
    editBugID->setText (pBug->BugID);
    QWhatsThis::add (editBugID,
        i18n("This is used to uniquely identify the bug\n"
              "across the lifespan of its existence."));

    // LABEL: Description
    QLabel *lblDescription = new QLabel (i18n ("Description:"),GeneralLayout);

    // LINEEDIT: Description
    editDescription = new QLineEdit(GeneralLayout);
    lblDescription->setBuddy (editDescription);
    editDescription->setText (pBug->Description);
    QWhatsThis::add (editDescription,
	    i18n("Enter a brief, one line,\n"
			 "description of the bug."));

    // LABEL: Severity
    QLabel *lblSeverity = new QLabel (i18n ("Severity:"), GeneralLayout);

    // LINEEDIT: Severity
    cboSeverity = new QComboBox(false, GeneralLayout);
    lblSeverity->setBuddy (cboSeverity);
    cboSeverity->insertStrList (severity_desc);
    QWhatsThis::add (cboSeverity,
	    i18n("CRITICAL:\n"
             "Makes unrelated software on the system (or the whole system) "
             "break, or causes serious data loss, or introduces a security hole on "
             "systems where you install the package.\n\n"
             "GRAVE:\n"
             "Makes the package in question unusable or mostly so, or causes "
             "data loss, or introduces a security hole allowing access to the "
             "accounts of users who use the package.\n\n"
             "CRASH:\n"
             "Indicates drkonqi reported bugs. These bugs have their own severity "
             "as they are very often duplicates or have no reported way to reproduce "
             "them. They have to be reviewed in any case.\n\n"
             "NORMAL:\n"
             "The default value, for normal bugs.\n\n"
             "LOW:\n"
             "Trivial feature fix which does not impact usage.\n\n"
             "WISHLIST:\n"
             "For any feature request, and also for any bugs that are very difficult "
             "to fix due to major design considerations."));

    // Select the correct list item for severity.
    cboSeverity->setCurrentItem (3);
    for (Count = 0;Count < cboSeverity->count ();Count++)
    {
        if (cboSeverity->text (Count) == pBug->Severity)
            cboSeverity->setCurrentItem (Count);
    }

    // LABEL: Priority
    QLabel *lblPriority = new QLabel (i18n ("Priority:"), GeneralLayout);

    // LINEEDIT: Priority
    cboPriority = new QComboBox(false, GeneralLayout);
    lblPriority->setBuddy (cboPriority);
    cboPriority->insertStrList (priority_desc);
    QWhatsThis::add (cboPriority,
	    i18n("CRITICAL:\n"
             "This bug must be fixed in the near future.\n\n"
             "HIGH:\n"
             "This bug is serious, but not crippling\n"
             "to the system. It should be fixed at the\n"
             "first reasonable opportunity.\n\n"
             "NORMAL:\n"
             "The default value, for normal bugs.\n\n"
             "LOW:\n"
             "Trivial feature fix which does not impact usage.\n\n"));

    // Select the correct list item for severity.
    cboPriority->setCurrentItem (2);
    for (Count = 0;Count < cboPriority->count ();Count++)
    {
        if (cboPriority->text (Count) == pBug->Priority)
            cboPriority->setCurrentItem (Count);
    }

    // LABEL: Package
    QLabel *lblPackage = new QLabel (i18n ("Package:"), GeneralLayout);

    // LINEEDIT: Package
    editPackage = new QLineEdit(GeneralLayout);
    lblPackage->setBuddy (editPackage);
    editPackage->setText (pBug->Package);
    QWhatsThis::add (editPackage,
	    i18n("Enter the name of the package this\n"
			 "bug occurred in. Please do not enter\n"
			 "the version number at this time."));

    // LABEL: VersionNo
    QLabel *lblVersionNo = new QLabel (i18n ("Version no:"), GeneralLayout);

    // LINEEDIT: VersionNo
    editVersionNo = new QLineEdit(GeneralLayout);
    lblVersionNo->setBuddy (editVersionNo);
    editVersionNo->setText (pBug->VersionNo);
    QWhatsThis::add (editVersionNo,
	    i18n("Enter the version number of the package\n"
			 "the bug is found in."));

    // LABEL: Class
    QLabel *lblBugClass = new QLabel (i18n ("Class:"), GeneralLayout);

    // LINEEDIT: Class
    editBugClass = new QLineEdit(GeneralLayout);
    lblBugClass->setBuddy (editBugClass);
    editBugClass->setText (pBug->BugClass);
    QWhatsThis::add (editBugClass,
	    i18n("The class of bug."));

    // LABEL: Location
    QLabel *lblLocation = new QLabel (i18n ("Location:"), GeneralLayout);

    // LINEEDIT: Location
    editLocation = new QLineEdit(GeneralLayout);
    lblLocation->setBuddy (editLocation);
    editLocation->setText (pBug->Location);
    QWhatsThis::add (editLocation,
	    i18n("Enter the location within the source code\n"
			 "where the bug may be found (if known)."));

    // SPACER!!!
    new QWidget (GeneralLayout);
    new QWidget (GeneralLayout);

    // LABEL: ReportDate
    QLabel *lblReportDate = new QLabel (i18n ("Reported on:"), GeneralLayout);

    // LINEEDIT: ReportDate
    editReportDate = new QLineEdit(GeneralLayout);
    lblReportDate->setBuddy (editReportDate);
    if (pBug->ReportDate.isValid ())
        editReportDate->setText (DateToQString (pBug->ReportDate));
    QWhatsThis::add (editReportDate,
	    i18n("Enter the date this bug was reported."));

    // LABEL: ReportUserName
    QLabel *lblReportUserName = new QLabel (i18n ("Reported by:"), GeneralLayout);

    // LINEEDIT: ReportUserName
    editReportUserName = new QLineEdit(GeneralLayout);
    lblReportUserName->setBuddy (editReportUserName);
    editReportUserName->setText (pBug->ReportUserName);
    QWhatsThis::add (editReportUserName,
	    i18n("Enter the name of the person\n"
			 "who reported this bug."));

    // LABEL: ReportEMail
    QLabel *lblReportEMail = new QLabel (i18n ("Reported by (Email):"), GeneralLayout);

    // LINEEDIT: ReportEMail
    editReportEMail = new QLineEdit(GeneralLayout);
    lblReportEMail->setBuddy (editReportEMail);
    editReportEMail->setText (pBug->ReportEMail);
    QWhatsThis::add (editReportEMail,
	    i18n("Enter the email address of the\n"
			 "person who reported this bug."));

    // SPACER!!!
    new QWidget (GeneralLayout);
    new QWidget (GeneralLayout);

    // LABEL: AssignedDate
    QLabel *lblAssignedDate = new QLabel (i18n ("Assigned on:"), GeneralLayout);

    // LINEEDIT: AssignedDate
    editAssignedDate = new QLineEdit(GeneralLayout);
    lblAssignedDate->setBuddy (editAssignedDate);
    if (pBug->AssignedDate.isValid ())
        editAssignedDate->setText (DateToQString (pBug->AssignedDate));
    QWhatsThis::add (editAssignedDate,
	    i18n("Enter the date this bug was\n"
			 "assigned to a person for repair."));

    // LABEL: AssignedTo
    QLabel *lblAssignedTo = new QLabel (i18n ("Assigned to:"), GeneralLayout);

    // LINEEDIT: AssignedTo
    editAssignedTo = new QLineEdit(GeneralLayout);
    lblAssignedTo->setBuddy (editAssignedTo);
    editAssignedTo->setText (pBug->AssignedTo);
    QWhatsThis::add (editAssignedTo,
	    i18n("Enter the name of the person you\n"
			 "wish to assign this bug to."));

    // LABEL: AssignedEMail
    QLabel *lblAssignedEMail = new QLabel (i18n ("Assigned to (Email):"), GeneralLayout);

    // LINEEDIT: AssignedEMail
    editAssignedEMail = new QLineEdit(GeneralLayout);
    lblAssignedEMail->setBuddy (editAssignedEMail);
    editAssignedEMail->setText (pBug->AssignedEMail);
    QWhatsThis::add (editAssignedEMail,
	    i18n("Enter the email address of the person\n"
			 "you wish to assign this bug to."));

    // SPACER!!!
    new QWidget (GeneralLayout);
    new QWidget (GeneralLayout);

    // LINEEDIT: Notes.
    editNotes = new QMultiLineEdit (pSheetNotes,"Notes");
    editNotes->setText (pBug->Notes);
    QWhatsThis::add (editNotes,
	    i18n("Enter a full description of the\n"
			 "nature of the bug."));

    // LINEEDIT: How to repeat
    editRepeat = new QMultiLineEdit(pSheetRepeat,"Repeat");
    editRepeat->setText (pBug->Repeat);
    QWhatsThis::add (editRepeat,
	    i18n("Enter the method used to repeat this\n"
			 "error, if there is one."));

    // LINEEDIT: Workaround
    editWorkaround = new QMultiLineEdit(pSheetWorkaround,"Workaround");
    editWorkaround->setText (pBug->Workaround);
    QWhatsThis::add (editWorkaround,
	    i18n("Enter a workaround for the bug,\n"
			 "if there is one."));

    // LINEEDIT: SysInfo
    editSysInfo = new QMultiLineEdit (pSheetSysInfo,"SysInfo");
    editSysInfo->setText (pBug->SysInfo);
    QWhatsThis::add (editSysInfo,
	    i18n("Enter a full description of your OS, including\n"
			 "distribution, Qt version, compilers, etc."));

    // Need a cancel button too.
    setOkButton (i18n("OK"));
    setCancelButton (i18n("Cancel"));
	
    // Get those main buttons hooked up.
    connect (this, SIGNAL(cancelButtonPressed()), this, SLOT(cancelClicked()));
    connect (this, SIGNAL(applyButtonPressed()), this, SLOT(closeClicked()));

    // Set the focus to the most likely entry to start with.
    editBugID->setFocus ();

    // Can't be dirty until we change something.
    Dirty = FALSE;
}


BugEdit::~BugEdit()
{
}


// Updates the contents of the Bug record from the screen.

void BugEdit::updateRecord()
{
    m_pBug->BugID = editBugID->text ();
    m_pBug->Description = editDescription->text ();
    m_pBug->Severity = cboSeverity->currentText ();
    m_pBug->Priority = cboPriority->currentText ();
    m_pBug->BugClass = editBugClass->text ();
    m_pBug->Location = editLocation->text ();
    m_pBug->AssignedTo = editAssignedTo->text ();
    m_pBug->AssignedEMail = editAssignedEMail->text ();
    m_pBug->ReportUserName = editReportUserName->text ();
    m_pBug->ReportEMail = editReportEMail->text ();
    m_pBug->Package = editPackage->text ();
    m_pBug->VersionNo = editVersionNo->text ();
    m_pBug->Notes = editNotes->text ();
    m_pBug->Repeat = editRepeat->text ();
    m_pBug->Workaround = editWorkaround->text ();
    m_pBug->SysInfo = editSysInfo->text ();

    m_pBug->AssignedDate = QStringToDate (editAssignedDate->text ());
    m_pBug->ReportDate = QStringToDate (editReportDate->text ());
}


// They pressed the 'Close' button. We should exit and update the
// file if it has changed.

void BugEdit::closeClicked()
{
    // Check to see if the file has changed.
//    if (Dirty)
    {
        // Write out the changed record.
        updateRecord ();

        // Get the buglist to add/update as needed.
        if (m_AddItem)
            emit sigAddBug (m_pBug);
        else
            emit sigUpdateBug (m_pBug);
    }

    // Close this widget.
    close ();
}


// They pressed the 'Cancel' button. We should exit without saving, checking
// first to ensure they want to lose all changes.

void BugEdit::cancelClicked()
{
    if (Dirty)
    {
        QMessageBox     MB;

        // Give them a chance to abort if the list has changed.
        if (MB.warning (this,
            i18n ("Record Changed"),
            i18n ("The record has been changed.\n"
                  "If you exit now you will lose all your changes.\n"
                  "Are you sure you want to exit?"),
            QMessageBox::Yes,QMessageBox::Cancel | QMessageBox::Default) == QMessageBox::Cancel)
        {
            // Ignore the button press.
            return;
        }
    }

    // Close this widget.
    close ();
}

void BugEdit::resizeEvent (QResizeEvent * QEvent)
{
    // Pass the event down to our subclass.
    QWidget::resizeEvent (QEvent);
}
#include "bugedit.moc"
