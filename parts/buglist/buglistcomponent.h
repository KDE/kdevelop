/***************************************************************************
                          buglistcomponent.h  -  description
                             -------------------
    begin                : Sun Dec 10 2000
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

#ifndef BUGLISTCOMPONENT_H
#define BUGLISTCOMPONENT_H

#include <qguardedptr.h>
#include <kgenericfactory.h>
#include "kdevplugin.h"
#include "buglist.h"
#include <kaction.h>
//#include <kdevcomponent.h>
//#include <projectspace.h>

/**The main component for the buglist class.
  *@author Ivan Hawkes
  */

class BugListWidget;
//class KDevLanguageSupport;
//class KDevEditorManager;
//class ProjectSpace;

class BugListComponent : public KDevPlugin
{
Q_OBJECT

public:
    BugListComponent (QObject *parent, const char *name, const QStringList &args);
//   	BugListComponent (QObject *parent=0, const char *name=0);
    ~BugListComponent();
//		KDevLanguageSupport* languageSupport();
//		KDevEditorManager* editorManager();
//		ProjectSpace* projectSpace();
		
public slots:	
	void slotProjectChanged();
	void slotProjectSpaceOpened();
	void slotProjectSpaceClosed();
	void slotStopButtonClicked();

  // Call this slot to let the widget know to create the buglist editor.
	void slotActivate ();

  // Call this slot to let the widget know that the widget is closing itself.
	void slotWidgetClosed ();

protected:
    virtual void setupGUI();

private:
    BugList         *m_pBugList;        // The main buglist object.
//    ProjectSpace    *m_pProjectSpace;   // Pointer to the project space.
    void             *m_pProjectSpace;   // Pointer to the project space.
    KAction         *m_pMenuAction;     // The menu item we add.

    // Configuration info needed by the buglist component.
    QString         m_FileName;
    QString         m_Initials;
    QString         m_UserName;
    QString         m_UserEMail;
};

typedef KGenericFactory<BugListComponent> BugListFactory;

#endif
