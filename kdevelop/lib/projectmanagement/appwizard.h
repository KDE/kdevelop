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

#include <qwizard.h>
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

class AppWizard : public QWizard, public Plugin  {
Q_OBJECT

public: 
 
/**
   @param projectspace pointer to the current ProjectSpace
*/
 
 AppWizard(QWidget* parent=0, const char* name=0);
 virtual ~AppWizard();
 virtual void init(bool new_projectspace=true,ProjectSpace* projectspace=0);
 virtual void initDefaultPages();
 // for which workspace is this plugin
 QString  getProjectSpaceName();
 // the picture for the newProjectDialog, the QString contains the absolute path
 QString getPreviewPicture();
 /* returns a pointer to the Project*/
 Project* getProject();
 
 /** generates default files/app, properties from configwidgets set in AppWizard*/
 virtual void generateDefaultFiles();
 
 protected slots:
  virtual void accept();
 
 protected:
 QLineEdit* emailline;
 QLineEdit* authorline;
  QLineEdit* versionline;
  QLabel* name;
  QLabel* email;
  QLabel* authorname;
  QLabel* versionnumber;
  QLabel* directory;

  QCheckBox* gnufiles;
  QCheckBox* lsmfile;
  
  // second page
  QPushButton*  fnew;
  QPushButton*  fload;
  KEdit* fedit;
  QCheckBox* fheader;
  
  // pages	
  QWidget* m_general_page;
  QWidget* m_fileheader_page;
  
  // others
  bool m_new_projectspace;
  ProjectSpace* m_projectspace;
  Project* m_project;
  
  //plugin infos, static
  QString m_project_space_name;
  QString m_application_picture;
  QString m_project_library;
  
  QString m_project_template;
  QString m_path;
};

#endif
