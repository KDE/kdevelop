/***************************************************************************
                          appwizard.cpp  -  description
                             -------------------
    begin                : Sat May 13 2000
    copyright            : (C) 2000 by Sandy Meier
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

#include "appwizard.h"
#include "pluginloader.h"
#include <qfiledialog.h>
#include <iostream.h>
#include <kprocess.h>
#include <kdebug.h>


AppWizard::AppWizard(QWidget* parent, const char* obj_name) : AppWizardBase(parent,obj_name,true){
  // conntects
  m_project=0;
  
}
AppWizard::~AppWizard(){
}

void AppWizard::init(bool new_projectspace,ProjectSpace* projectspace){
  kdDebug(9000) << "enter AppWizard::init" << endl;
  m_new_projectspace = new_projectspace;
  m_projectspace = projectspace;
  getProject();
 
  if(!m_new_projectspace){
    removePage(page(1)); // remove the headerpage
  }
  
}

void AppWizard::accept(){
  kdDebug(9000) << "start generation" << endl;
  if(m_new_projectspace){ // only if a new one was selected
    m_projectspace->generateDefaultFiles();
  }
  m_projectspace->addProject(m_project);
  m_projectspace->writeConfig();
  generateDefaultFiles();
  QWizard::accept();
}

QString  AppWizard::getProjectSpaceName(){
  return m_project_space_name;
}
QString  AppWizard::getPreviewPicture(){
  return m_application_picture;
}
Project* AppWizard::getProject(){
  if(m_project == 0){ // no project instance
    m_project = PluginLoader::getNewProject(m_projecttype_name);
  }
  return m_project;
}

void AppWizard::generateDefaultFiles(){
  KShellProcess proc("/bin/sh");
  
  // create the directories
  proc << "mkdirhier";
  proc << m_project->getAbsolutePath();
  proc.start(KProcess::Block,KProcess::AllOutput);
  
  // untar/unzip the project
  proc.clearArguments();
  QString args = "xzvf " + m_project_template + " -C " + m_project->getAbsolutePath();
  cerr << "AppWizard::generateDefaultFiles():" << args;
  proc << "tar";
  proc << args;
  proc.start(KProcess::Block,KProcess::AllOutput);
}
void AppWizard::slotNewHeader(){
  header_multiedit->clear();
}
void AppWizard::slotLoadHeader(){
}

#include "appwizard.moc"
#include "appwizardbase.moc"
