/***************************************************************************
                          kdesdiappwizardplugin.h  -  description
                             -------------------
    begin                : Sat May 13 2000
    copyright            : (C) 2000 by Sandy Meier (KDevelop Team)
    email                : smeier@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDESDIAPPWIZARDPLUGIN_H
#define KDESDIAPPWIZARDPLUGIN_H

#include "appwizard.h"
#include <qwidget.h>
#include <qlabel.h>
#include <qlist.h>
#include "filepropspage.h"
#include <kaboutdata.h>

/**
  *@author Sandy Meier
  */

class KDESDIAppWizardPlugin : public AppWizard  {
Q_OBJECT
  public: 
 KDESDIAppWizardPlugin(QObject* parent=0,const char* name=0);
 ~KDESDIAppWizardPlugin();
 virtual void init(bool new_projectspace=true,ProjectSpace* projectspace=0,
		   QString projectname="",QString absProjectLocation="");
 
 /** generates default files/app, properties from configwidgets set in AppWizard*/
 virtual void generateDefaultFiles();
 virtual void setInfosInString(QString& text);
 virtual KAboutData* aboutPlugin();

  QLabel* m_text;
  QWidget* m_sdi_general_page;
  FilePropsPage* m_sdi_fileprops_page;

  QString m_appheader;
  QString m_appheader_big;
  QString m_viewheader;
  QString m_viewheader_big;
  QString m_docheader;
  QString m_docheader_big;

  ClassFileProp* m_app_prop;
  ClassFileProp* m_view_prop;
  ClassFileProp* m_doc_prop;
  
 protected:
  KAboutData* m_pAboutData;
};

#endif
