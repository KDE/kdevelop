/***************************************************************************
                          newprojectdlg.h  -  description
                             -------------------
    begin                : Sat May 13 2000
    copyright            : (C) 2000 by Sandy Meier
    email                : smeier@kdevelop.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef NEWPROJECTDLG_H
#define NEWPROJECTDLG_H

#include <kdialogbase.h>
#include <klocale.h>
#include <kseparator.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <kiconview.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qlist.h>
#include <klistbox.h>
#include <klistview.h>

#include "projectspace.h"
#include "appwizard.h"
#include "newprojectdlgbase.h"

/** displays all available projecttypes/projectspaces in different categories,
    user can choise if KDevelop should generate a new ProjectSpace + Project
    or add the selected project to the current ProjectSpace
    starts the application wizard (AppWizard)for application generation
    on left side there are projectspaces,top right: available projects (appwizard plugins),
    bottom right: "new projectspace","current projectspace"
  @author Sandy Meier
  */

class NewProjectDlg : public NewProjectDlgBase  {
Q_OBJECT
public: 
	NewProjectDlg(ProjectSpace* projectSpace=0,QWidget *parent=0, const char *name=0,bool modal=true);
	virtual	~NewProjectDlg();
	void initDialog();
	bool newProjectSpaceCreated();
	QString projectSpaceFile();

	protected slots:
	void slotProjectSpaceSelected (QListViewItem *item );
	void slotAppwizardSelected (QIconViewItem* item);
	virtual void slotOk(); // from the KDialogBase
	virtual void slotAddToCurrentProjectSpaceClicked();
	virtual void slotNewProjectSpaceClicked();
	void slotProjectSpaceDirClicked();
	void slotProjectSpaceNameEdit(const QString& text);
	void slotProjectSpaceLocationEdit(const QString & text);
	void slotProjectNameEdit(const QString& text);
	void slotProjectLocationEdit(const QString& text);
	void slotProjectDirClicked();

	
	protected:
	
		
	// data members
	QList<ProjectSpace>* m_prjspace_list;
	QList<AppWizard>* m_appwizard_list;
	AppWizard* m_current_appwizard_plugin;
	QString m_current_prjspace_name;
	QPixmap* m_pixmap;
	ProjectSpace* m_pProjectSpace;
	ProjectSpace* m_pSelectedProjectSpace;
	
	
	// others
	bool m_project_name_modified;
	bool m_project_location_modified;
	bool m_projectspace_location_modified;
	QString m_default_location;
	bool m_newProjectSpace;
};


#endif
