/***************************************************************************
                          appwizard.h  - base class for application wizards
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

#ifndef APPWIZARD_H
#define APPWIZARD_H

#include "appwizardbase.h"
#include <qwidget.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <klocale.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <keditcl.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

#include "projectspace.h"


/**display some standard pages
  and the project specific widgets,
  if new projectspace was enabled ask about
  author name, email, templates, cvs and so on...)
  baseclass for the all Appwizard plugins,
  *@author Sandy Meier
  */

class AppWizard : public AppWizardBase, public Plugin  {
Q_OBJECT

public: 
 
/**
   @param projectspace pointer to the current ProjectSpace
*/
 
 AppWizard(QWidget* parent=0, const char* name=0);
 virtual ~AppWizard();
 virtual void init(bool new_projectspace=true,ProjectSpace* projectspace=0,
		   QString projectName="",QString absProjectLocation="");
 // for which workspace is this plugin
 QString  getProjectSpaceName();
 // the picture for the newProjectDialog, the QString contains the absolute path
 QString getPreviewPicture();
 /** generates default files/app, properties from configwidgets set in AppWizard*/
 virtual void generateDefaultFiles();

 virtual void generateFile(QString abs_oldpos,QString abs_newpos);

 /** replace |VERSION|, |AUTHOR| and so on...*/
 virtual void setInfosInString(QString& text);
 
 protected slots:
  virtual void accept();
 virtual void slotNewHeader();
 virtual void slotLoadHeader();
 protected:
  
  // others
  bool m_new_projectspace;
  ProjectSpace* m_projectspace;
  Project* m_project;
  QString m_projectName;
  QString m_absProjectLocation;
  
  //plugin infos, static
  QString m_project_space_name;
  QString m_application_picture;
  QString m_projecttype_name;
  
  QString m_project_template;
  QString m_path;
};

#endif
