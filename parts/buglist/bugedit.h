/***************************************************************************
                          bugedit.h  -  description
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

#ifndef BUGEDIT_H
#define BUGEDIT_H

#include "buglist.h"
#include <qtabdialog.h>
#include <qdialog.h>
#include <qhbox.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qframe.h>
#include <qlayout.h>
#include <qgrid.h>


/**This class is used to edit the individual bug entried in the list.
  *@author Ivan Hawkes
  */


class BugEdit : public QTabDialog
{
Q_OBJECT

public:
    // Constructor and destructor.
    BugEdit (QWidget * parent=0,
             const char * name=0,
             Bug * pBug = 0,
             bool AddItem = TRUE,
             bool modal=FALSE,
             WFlags f=0);
	  ~BugEdit ();
	
    // Resize method.
    void resizeEvent (QResizeEvent *);

signals:
    void sigAddBug (Bug *);
    void sigUpdateBug (Bug *);

private:
    void updateRecord();
    const QString DateToQString (QDate InputDate);
    const QDate QStringToDate (QString InputString);

private slots:
    void closeClicked();
    void cancelClicked();

private:
    // Need to keep a pointer to the edit controls so we know what values they entered.
    QLineEdit *         editBugID;
    QLineEdit *         editDescription;
    QComboBox *         cboSeverity;
    QComboBox *         cboPriority;
    QLineEdit *         editBugClass;
    QLineEdit *         editLocation;
    QLineEdit *         editAssignedTo;
    QLineEdit *         editAssignedDate;
    QLineEdit *         editAssignedEMail;
    QLineEdit *         editReportUserName;
    QLineEdit *         editReportEMail;
    QLineEdit *         editReportDate;
    QLineEdit *         editPackage;
    QLineEdit *         editVersionNo;
    QMultiLineEdit *    editNotes;
    QMultiLineEdit *    editRepeat;
    QMultiLineEdit *    editWorkaround;
    QMultiLineEdit *    editSysInfo;
    QLineEdit *         editFixScheduled;

private:
    QFrame *        pMainFrame;
    bool            Dirty;
    Bug *           m_pBug;
    bool            m_AddItem;

    // Tab sheets.
    QHBox *         pSheetGeneral;
    QHBox *         pSheetNotes;
    QHBox *         pSheetRepeat;
    QHBox *         pSheetWorkaround;
    QHBox *         pSheetSysInfo;

    // Layouts.
    QGrid *         GeneralLayout;
};

#endif

