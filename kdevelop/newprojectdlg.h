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



/** displays all available projecttypes/projectspaces in different categories,
    user can choise if KDevelop should generate a new ProjectSpace + Project
    or add the selected project to the current ProjectSpace
    starts the application wizard (AppWizard)for application generation
    on left side there are projectspaces,top right: available projects (appwizard plugins),
    bottom right: "new projectspace","current projectspace"
  @author Sandy Meier
  */

class NewProjectDlg : public KDialogBase  {
Q_OBJECT
public: 
	NewProjectDlg(QWidget *parent=0, const char *name=0,bool modal=true);
	virtual	~NewProjectDlg();
	void initDialog();

	protected slots:
	void slotProjectSpaceSelected (QListViewItem *item );
	void slotAppwizardSelected (QIconViewItem* item);
	virtual void slotOk(); // from the KDialogBase
	void slotProjectSpaceDirClicked();
	void slotProjectSpaceNameEdit(const QString& text);
	void slotProjectSpaceLocationEdit(const QString & text);
	void slotProjectNameEdit(const QString& text);
	void slotProjectLocationEdit(const QString& text);
	
	protected:
	
	// widgets member
	KIconView *appwizard_iconview;
	KSeparator *m_seperator;
	QLineEdit *m_prjspace_location_linedit;
	QPushButton *m_prjspace_dir_button;
	QButtonGroup *button_group;
	QRadioButton *current_radio_button;
	QRadioButton *new_radio_button;
	QWidget *preview_widget;
	QLabel *description_label;
	KListView *prjspace_listview;
	QLineEdit *m_prjspace_name_linedit;
	QLabel *m_prjspace_location_label;
	QLabel *m_prjspace_name_label;
	QLabel *m_prj_name_label;
	QLineEdit *m_prjname_linedit;
	QLabel *m_prj_location_label;
	QLineEdit *m_prjlocation_linedit;
	QPushButton *m_project_dir_button;
		
	// data members
	QList<ProjectSpace>* m_prjspace_list;
	QList<AppWizard>* m_appwizard_list;
	AppWizard* m_current_appwizard_plugin;
	QString m_current_prjspace_name;
	QPixmap* m_pixmap;
	
	// others
	bool m_project_name_modified;
	bool m_project_location_modified;
	bool m_projectspace_location_modified;
	QString m_default_location;
};


#endif
