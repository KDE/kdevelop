/***************************************************************************
                          buglist.h  -  description
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

#ifndef BUGLIST_H
#define BUGLIST_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "bug.h"
#include "bugcounter.h"
#include "structureparser.h"
#include <kapp.h>
#include <kdevcomponent.h>
#include <qwidget.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qdict.h>


// Forward declaration to handle the dependancies.
class StructureParser;


class BugList : public QWidget // KDevComponent
{
Q_OBJECT

friend StructureParser;

public:
    /** construtor/destructor */
    BugList(QWidget* parent=0, const char *name=0);
    ~BugList();

    // Resize method.
    void resizeEvent ( QResizeEvent * );

    // Parse the XML file to fill our listbox.
    void ParseFile ();
    void WriteXMLFile ();

signals:
    void signalDeactivate ();

public slots:
    void slotCloseClicked();
    void slotCancelClicked();

private slots:
    void slotAddClicked ();
    void slotEditClicked ();
    void slotListDoubleClicked (QListViewItem *);
    void slotRemoveClicked ();
    void slotCompletedClicked ();

    void slotAddBug (Bug *);
    void slotUpdateBug (Bug *);
    void slotFilter ();
    void slotOwnership ();

protected:
    QListView *         pMainBugList;
    QDict <Bug>         BugDictionary;
    QDict <BugCounter>  Developers;

private:
    QHBox *             pExitBox;
    QHBox *             pMaintenanceBox;
    QHBox *             pBugListBox;
    QString             FileName;
    int                 MyCounter;
    bool                Dirty;                  // True if the file has been chnaged in memory.
};

#endif

