/***************************************************************************
 *   Copyright (C) 2002 by Jakob Simon-Gaarde                              *
 *   jsgaarde@tdcspace.dk                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "projectconfigurationdlg.h"
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qmessagebox.h>
#include <kfiledialog.h>
#include <klineedit.h>
#include <klistview.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qvalidator.h>

ProjectConfigurationDlg::ProjectConfigurationDlg(ProjectConfiguration *conf,QWidget* parent, const char* name, bool modal, WFlags fl)
: ProjectConfigurationDlgBase(parent,name,modal,fl)
//=================================================
{
  m_projectConfiguration = conf;
  m_targetLibraryVersion->setValidator(new QRegExpValidator(
    QRegExp("\\d+(\\.\\d+)?(\\.\\d+)"), this));
  buildProjectInstallTree(install_list);
  UpdateControls();
  install_list->hide();
}


ProjectConfigurationDlg::~ProjectConfigurationDlg()
//==============================================
{
}

void ProjectConfigurationDlg::buildProjectInstallTree(KListView *listviewControl)
{/*
//  if (item->configuration.m_template == QTMP_SUBDIRS)
//    return;

  // Insert all GroupItems and all of their children into the view
  if (listviewControl)
  {
//    QPtrListIterator<SubprojectItem> it1(item->scopes);
//    for (; it1.current(); ++it1)
//    {
//      listviewControl->insertItem(*it1);
//      buildProjectDetailTree(*it1,NULL);
//    }
    QPtrListIterator<GroupItem> it2(item->groups);
    for (; it2.current(); ++it2)
    {
        listviewControl->insertItem(*it2);
        QPtrListIterator<FileItem> it3((*it2)->files);
        for (; it3.current(); ++it3)
            (*it2)->insertItem(*it3);
        (*it2)->setOpen(true);
    }
  }
  else
  {
//    QPtrListIterator<SubprojectItem> it1(item->scopes);
//    for (; it1.current(); ++it1)
//    {
//      item->insertItem(*it1);
//      buildProjectDetailTree(*it1,NULL);
//    }
    QPtrListIterator<GroupItem> it2(item->groups);
    for (; it2.current(); ++it2)
    {
        item->insertItem(*it2);
        QPtrListIterator<FileItem> it3((*it2)->files);
        for (; it3.current(); ++it3)
            (*it2)->insertItem(*it3);
        (*it2)->setOpen(true);
    }
  }*/
}


void ProjectConfigurationDlg::radioLibrarytoggled(bool on)
//=============================================
{
	groupLibrary->setEnabled(on);
	//radioShared->setChecked(true);
	m_targetLibraryVersion->setEnabled(on);
}

void ProjectConfigurationDlg::browseTargetPath()
//==============================================
{
  m_targetPath->setText(KFileDialog::getExistingDirectory());

}

void ProjectConfigurationDlg::updateProjectConfiguration()
//=======================================================
{
  // Template
  if (radioApplication->isChecked())
    m_projectConfiguration->m_template = QTMP_APPLICATION;
  else if (radioLibrary->isChecked()) {
    m_projectConfiguration->m_template = QTMP_LIBRARY;
    m_projectConfiguration->m_libraryversion = 
      m_targetLibraryVersion->text();
  }
  else if (radioSubdirs->isChecked())
    m_projectConfiguration->m_template = QTMP_SUBDIRS;

  // Buildmode
  if (radioDebugMode->isChecked())
    m_projectConfiguration->m_buildMode = QBM_DEBUG;
  if (radioReleaseMode->isChecked())
    m_projectConfiguration->m_buildMode = QBM_RELEASE;

  // requirements
  m_projectConfiguration->m_requirements = 0;
  if (checkQt->isChecked())
    m_projectConfiguration->m_requirements += QD_QT;
  if (checkOpenGL->isChecked())
    m_projectConfiguration->m_requirements += QD_OPENGL;
  if (checkThread->isChecked())
    m_projectConfiguration->m_requirements += QD_THREAD;
  if (checkX11->isChecked())
    m_projectConfiguration->m_requirements += QD_X11;
  
  // Lib mode
  if (radioStatic->isChecked())
    m_projectConfiguration->m_lib = QL_STATIC;
  else if (radioPlugin->isChecked())
      m_projectConfiguration->m_lib = QL_PLUGIN;
  else
      m_projectConfiguration->m_lib = QL_SHARED;
  
  // Warnings
  m_projectConfiguration->m_warnings = QWARN_OFF;
  if (checkWarning->isChecked())
    m_projectConfiguration->m_warnings = QWARN_ON;

  m_projectConfiguration->m_target = "";
/*  if ((m_targetPath->text().simplifyWhiteSpace()!="" ||
      m_targetOutputFile->text().simplifyWhiteSpace()!="") &&
      !radioSubdirs->isChecked())
  {
    QString outputFile = m_targetOutputFile->text();
    if (outputFile.simplifyWhiteSpace() == "")
      outputFile = m_projectConfiguration->m_subdirName;
    m_projectConfiguration->m_target = m_targetPath->text() + "/" + outputFile;
  }*/
  m_projectConfiguration->m_target = m_targetOutputFile->text();
  m_projectConfiguration->m_destdir = m_targetPath->text();
  m_projectConfiguration->m_includepath = QStringList::split(" ",m_includePath->text());
  m_projectConfiguration->m_defines = QStringList::split(" ",m_defines->text());
  m_projectConfiguration->m_cxxflags_debug = QStringList::split(" ",m_debugFlags->text());
  m_projectConfiguration->m_cxxflags_release = QStringList::split(" ",m_releaseFlags->text());
  m_projectConfiguration->m_lflags_debug = QStringList::split(" ",m_debugFlagsLink->text());
  m_projectConfiguration->m_lflags_release = QStringList::split(" ",m_releaseFlagsLink->text());
  m_projectConfiguration->m_librarypath = QStringList::split(" ",m_libraryPath->text());
  m_projectConfiguration->m_objectpath = m_objectPath->text();
  m_projectConfiguration->m_uipath = m_uiPath->text();
  m_projectConfiguration->m_mocpath = m_mocPath->text();
  m_projectConfiguration->m_libs = m_Libs->text();
  m_projectConfiguration->m_installtargetpath = m_InstallTargetPath->text();
  if (checkInstallTarget->isChecked())
    m_projectConfiguration->m_installtarget = true;
  else
    m_projectConfiguration->m_installtarget = false;
  if (checkDontInheritConfig->isChecked())
    m_projectConfiguration->m_inheritconfig = false;
  else
    m_projectConfiguration->m_inheritconfig = true;

  QDialog::accept();
}


void ProjectConfigurationDlg::UpdateControls()
//============================================
{
  QRadioButton *activateRadiobutton=NULL;
  // Project template
  switch (m_projectConfiguration->m_template)
  {
    case QTMP_APPLICATION:
      activateRadiobutton = radioApplication;
      break;
    case QTMP_LIBRARY:
      activateRadiobutton = radioLibrary;
      m_targetLibraryVersion->setText(m_projectConfiguration->m_libraryversion);
      
      if (m_projectConfiguration->m_lib == QL_STATIC)
	  radioStatic->setChecked(true);
      else if (m_projectConfiguration->m_lib == QL_PLUGIN)
	  radioPlugin->setChecked(true);
      else
	  radioShared->setChecked(true);
      
      break;
    case QTMP_SUBDIRS:
      activateRadiobutton = radioSubdirs;
      break;
  }
  // Buildmode
  if (activateRadiobutton)
    activateRadiobutton->setChecked(true);

  switch (m_projectConfiguration->m_buildMode)
  {
    case QBM_DEBUG:
      activateRadiobutton = radioDebugMode;
      break;
    case QBM_RELEASE:
      activateRadiobutton = radioReleaseMode;
      break;
  }
  if (activateRadiobutton)
    activateRadiobutton->setChecked(true);

  // Requirements
  if (m_projectConfiguration->m_requirements & QD_QT)
    checkQt->setChecked(true);
  if (m_projectConfiguration->m_requirements & QD_OPENGL)
    checkOpenGL->setChecked(true);
  if (m_projectConfiguration->m_requirements & QD_THREAD)
    checkThread->setChecked(true);
  if (m_projectConfiguration->m_requirements & QD_X11)
    checkX11->setChecked(true);
  
  // Warnings
  if (m_projectConfiguration->m_warnings == QWARN_ON)
  {
    checkWarning->setChecked(true);
  }
  
  // Target
/*  QString targetString = m_projectConfiguration->m_target;
  int slashPos = targetString.findRev('/');

  if (slashPos>=0)
  {
    m_targetPath->setText(targetString.left(slashPos));
    m_targetOutputFile->setText(targetString.right(targetString.length()-slashPos-1));
  }
  else
    m_targetOutputFile->setText(targetString);
*/
  m_targetOutputFile->setText(m_projectConfiguration->m_target);
  m_targetPath->setText(m_projectConfiguration->m_destdir);
  clickSubdirsTemplate();

  // Includepath
  m_includePath->setText(m_projectConfiguration->m_includepath.join(" "));      
  m_defines->setText(m_projectConfiguration->m_defines.join(" "));
  m_debugFlags->setText(m_projectConfiguration->m_cxxflags_debug.join(" "));
  m_releaseFlags->setText(m_projectConfiguration->m_cxxflags_release.join(" "));
  m_debugFlagsLink->setText(m_projectConfiguration->m_lflags_debug.join(" "));
  m_releaseFlagsLink->setText(m_projectConfiguration->m_lflags_release.join(" "));
  m_libraryPath->setText(m_projectConfiguration->m_librarypath.join(" "));
  m_objectPath->setText(m_projectConfiguration->m_objectpath);
  m_uiPath->setText(m_projectConfiguration->m_uipath);
  m_mocPath->setText(m_projectConfiguration->m_mocpath);
  m_Libs->setText(m_projectConfiguration->m_libs);
  m_InstallTargetPath->setText(m_projectConfiguration->m_installtargetpath);
  if (m_projectConfiguration->m_installtarget == true)
    checkInstallTarget->setChecked(true);
  if (m_projectConfiguration->m_inheritconfig == false)
    checkDontInheritConfig->setChecked(true);

}

void ProjectConfigurationDlg::clickSubdirsTemplate()
{
  if (radioSubdirs->isChecked())
  {
    m_targetPath->setEnabled(false);
    m_targetOutputFile->setEnabled(false);
    Browse->setEnabled(false);
  }
  else
  {
    m_targetPath->setEnabled(true);
    m_targetOutputFile->setEnabled(true);
    Browse->setEnabled(true);
  }
}
