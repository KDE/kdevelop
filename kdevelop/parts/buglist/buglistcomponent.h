/***************************************************************************
                          buglistcomponent.h  -  description
                             -------------------
    begin                : Sun Dec 10 2000
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

#ifndef BUGLISTCOMPONENT_H
#define BUGLISTCOMPONENT_H

#include "buglist.h"
#include <kdevcomponent.h>

/**The main component for the buglist class.
  *@author Ivan Hawkes
  */

class BugListWidget;

class BugListComponent : public KDevComponent
{
Q_OBJECT

public:
   	BugListComponent (QObject *parent=0, const char *name=0);
    ~BugListComponent();

protected:
    virtual void setupGUI();
    virtual void configWidgetRequested(KDialogBase *dlg);
    virtual void stopButtonClicked();
    virtual void projectSpaceOpened();

private:
    BugList     *m_pBugList;

public slots:
    /**
        Call this slot to let the widget know to create the buglist editor.
    */
	void slotActivate ();

    /**
        Call this slot to let the widget know that the widget is closing itself.
    */
	void slotWidgetClosed ();
};


#endif
