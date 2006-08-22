/***************************************************************************
 *   Copyright (C) 2002 by Jakob Simon-Gaarde                              *
 *   jsgaarde@tdcspace.dk                                                  *
 *   Copyright (C) 2002-2003 by Alexander Dymo                             *
 *   cloudtemple@mksat.net                                                 *
 *   Copyright (C) 2003 by Thomas Hasart                                   *
 *   thasart@gmx.de                                                        *
 *   Copyright (C) 2006 by Andreas Pakulat                                 *
 *   apaku@gmx.de                                                          *
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
#include <knotifyclient.h>
#include <klineedit.h>
#include <kinputdialog.h>
#include <kurlrequesterdlg.h>
#include <kurlrequester.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klistview.h>
#include <kmessagebox.h>

#include <qdialog.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qlistview.h>
#include <qlabel.h>
#include <iostream>
#include <qregexp.h>
#include <qvalidator.h>
#include <qtabwidget.h>
#include <pathutil.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>
#include <klocale.h>

#include "trollprojectwidget.h"
#include "trollprojectpart.h"

ProjectConfigurationDlg::ProjectConfigurationDlg(QListView *_prjList, TrollProjectWidget* _prjWidget, QWidget* parent, const char* name, bool modal, WFlags fl)
: ProjectConfigurationDlgBase(parent,name,modal,fl | Qt::WStyle_Tool), prjWidget(_prjWidget), myProjectItem(0)
//=================================================
{
  prjList=_prjList;
  m_targetLibraryVersion->setValidator(new QRegExpValidator(
    QRegExp("\\d+(\\.\\d+)?(\\.\\d+)"), this));
}

void ProjectConfigurationDlg::updateSubproject(SubqmakeprojectItem* _item)
{
  if( myProjectItem )
  {
  kdDebug(9024) << "changing subproject, behave: " << prjWidget->dialogSaveBehaviour() << endl;
    switch( prjWidget->dialogSaveBehaviour() )
    {
      case TrollProjectWidget::AlwaysSave:
        apply();
        break;
      case TrollProjectWidget::NeverSave:
        break;
      case TrollProjectWidget::Ask:
        if( !buttonApply->isEnabled() )
          break;
        if( KMessageBox::questionYesNo(0, i18n("Save the current subprojects configuration?"),
                                i18n("Save Configuration?")) == KMessageBox::Yes )
          apply();
        break;
    }
  }
  myProjectItem=_item;
//  m_projectConfiguration = conf;
  updateControls();
  if ( prjWidget->m_part->isQt4Project() )
  {
    radioDebugReleaseMode->setEnabled( true );
    checkBuildAll->setEnabled(true);
    qt4Group->setEnabled( myProjectItem->configuration.m_requirements & QD_QT );
    rccdir_url->setEnabled( true );
    rccdir_label->setEnabled( true );
  }
  buttonApply->setEnabled(false);
}


ProjectConfigurationDlg::~ProjectConfigurationDlg()
//==============================================
{
}

//void ProjectConfigurationDlg::radioLibrarytoggled(bool on)
////=============================================
//{
//	if(!on) checkStatic->setChecked(false);
//	checkStatic->setEnabled(on);
//	m_targetLibraryVersion->setEnabled(on);
//}

/*
void ProjectConfigurationDlg::radioLibrarytoggled(bool on)
//=============================================
{
	groupLibrary->setEnabled(on);
	//radioShared->setChecked(true);
	m_targetLibraryVersion->setEnabled(on);
}
*/
void ProjectConfigurationDlg::browseTargetPath()
//==============================================
{
  m_targetPath->setText(getRelativePath(myProjectItem->path,KFileDialog::getExistingDirectory()));
  buttonApply->setEnabled(true);
}


void ProjectConfigurationDlg::updateProjectConfiguration()
//=======================================================
{
  // Template
  myProjectItem->configuration.m_requirements = 0;
  if( !myProjectItem->isScope )
  {
	if (radioApplication->isChecked())
	{
		myProjectItem->configuration.m_template = QTMP_APPLICATION;
		myProjectItem->setPixmap(0,SmallIcon("qmake_app"));
	}
	else if (radioLibrary->isChecked())
	{
		myProjectItem->configuration.m_template = QTMP_LIBRARY;
		if (staticRadio->isOn())
			myProjectItem->configuration.m_requirements += QD_STATIC;
		else if (sharedRadio->isOn()){
			myProjectItem->configuration.m_requirements += QD_SHARED;
			myProjectItem->configuration.m_libraryversion = m_targetLibraryVersion->text();
		}
		if (checkPlugin->isOn())
		myProjectItem->configuration.m_requirements += QD_PLUGIN;
		if ( checkDesigner->isChecked() )
			myProjectItem->configuration.m_requirements += QD_DESIGNER;
		myProjectItem->setPixmap(0,SmallIcon("qmake_lib"));
	}
	else if (radioSubdirs->isChecked())
	{
	myProjectItem->configuration.m_template = QTMP_SUBDIRS;
	myProjectItem->setPixmap(0,SmallIcon("qmake_sub"));
	}
  }
  // Buildmode
  if (radioDebugMode->isChecked())
    myProjectItem->configuration.m_buildMode = QBM_DEBUG;
  if (radioReleaseMode->isChecked())
    myProjectItem->configuration.m_buildMode = QBM_RELEASE;
  if ( radioDebugReleaseMode->isChecked() )
    myProjectItem->configuration.m_buildMode = QBM_DEBUG_AND_RELEASE;

  // requirements
  if (exceptionCheck->isChecked())
    myProjectItem->configuration.m_requirements += QD_EXCEPTIONS;
  if (stlCheck->isChecked())
    myProjectItem->configuration.m_requirements += QD_STL;
  if (rttiCheck->isChecked())
    myProjectItem->configuration.m_requirements += QD_RTTI;
  if (checkQt->isChecked())
    myProjectItem->configuration.m_requirements += QD_QT;
  if (checkOpenGL->isChecked())
    myProjectItem->configuration.m_requirements += QD_OPENGL;
  if (checkThread->isChecked())
    myProjectItem->configuration.m_requirements += QD_THREAD;
  if (checkX11->isChecked())
    myProjectItem->configuration.m_requirements += QD_X11;
  if (checkOrdered->isChecked())
    myProjectItem->configuration.m_requirements += QD_ORDERED;
  if (checkLibtool->isChecked())
  {
    myProjectItem->configuration.m_requirements += QD_LIBTOOL;
  }
  if( checkPkgconf->isChecked())
  {
    myProjectItem->configuration.m_requirements += QD_PKGCONF;
  }
  if (checkConsole->isChecked() )
  {
    myProjectItem->configuration.m_requirements += QD_CONSOLE;
  }
  if (checkPCH->isChecked() )
    myProjectItem->configuration.m_requirements += QD_PCH;
  // Warnings
  myProjectItem->configuration.m_warnings = QWARN_OFF;
  if (checkWarning->isChecked())
    myProjectItem->configuration.m_warnings = QWARN_ON;

  //Qt4 libs
  if ( prjWidget->m_part->isQt4Project() )
  {
    if ( checkTestlib->isChecked() )
      myProjectItem->configuration.m_requirements += QD_TESTLIB;
    if ( checkAssistant->isChecked() )
      myProjectItem->configuration.m_requirements += QD_ASSISTANT;
    if ( checkUiTools->isChecked() )
      myProjectItem->configuration.m_requirements += QD_UITOOLS;
    if ( checkQDBus->isChecked() )
      myProjectItem->configuration.m_requirements += QD_DBUS;
    if ( checkBuildAll->isChecked() )
      myProjectItem->configuration.m_requirements += QD_BUILDALL;

    myProjectItem->configuration.m_qt4libs = 0;
    if ( checkQt4Core->isChecked() )
      myProjectItem->configuration.m_qt4libs += Q4L_CORE;
    if ( checkQt4Gui->isChecked() )
      myProjectItem->configuration.m_qt4libs += Q4L_GUI;
    if ( checkQt4SQL->isChecked() )
      myProjectItem->configuration.m_qt4libs += Q4L_SQL;
    if ( checkQt4SVG->isChecked() )
      myProjectItem->configuration.m_qt4libs += Q4L_SVG;
    if ( checkQt4XML->isChecked() )
      myProjectItem->configuration.m_qt4libs += Q4L_XML;
    if ( checkQt4Network->isChecked() )
      myProjectItem->configuration.m_qt4libs += Q4L_NETWORK;
    if ( checkQt3Support->isChecked() )
      myProjectItem->configuration.m_qt4libs += Q4L_QT3;
    if ( checkQt4OpenGL->isChecked() )
      myProjectItem->configuration.m_qt4libs += Q4L_OPENGL;
  }

  myProjectItem->configuration.m_target = "";
/*  if ((m_targetPath->text().simplifyWhiteSpace()!="" ||
      m_targetOutputFile->text().simplifyWhiteSpace()!="") &&
      !radioSubdirs->isChecked())
  {
    QString outputFile = m_targetOutputFile->text();
    if (outputFile.simplifyWhiteSpace() == "")
      outputFile = myProjectItem->configuration.m_subdirName;
    myProjectItem->configuration.m_target = m_targetPath->text() + "/" + outputFile;
  }*/
  myProjectItem->configuration.m_target = m_targetOutputFile->text();
  myProjectItem->configuration.m_destdir = m_targetPath->text();
//  myProjectItem->configuration.m_includepath = m_includePath->text();
  myProjectItem->configuration.m_defines = QStringList::split(" ",m_defines->text());
  myProjectItem->configuration.m_cxxflags_debug = QStringList::split(" ",m_debugFlags->text());
  myProjectItem->configuration.m_cxxflags_release = QStringList::split(" ",m_releaseFlags->text());
//  myProjectItem->configuration.m_lflags_debug = QStringList::split(" ",m_debugFlagsLink->text());
// myProjectItem->configuration.m_lflags_release = QStringList::split(" ",m_releaseFlagsLink->text());
//  myProjectItem->configuration.m_librarypath = QStringList::split(" ",m_libraryPath->text());
  if (checkDontInheritConfig->isChecked())
    myProjectItem->configuration.m_inheritconfig = false;
  else
    myProjectItem->configuration.m_inheritconfig = true;

  //add selected includes
  myProjectItem->configuration.m_incadd.clear();

  InsideCheckListItem *insideItem=(InsideCheckListItem *)insideinc_listview->firstChild();
  while(insideItem)
  {
    if(insideItem->isOn()){
      QString tmpInc=insideItem->prjItem->getIncAddPath(myProjectItem->getDownDirs());
      tmpInc=QDir::cleanDirPath(tmpInc);
      myProjectItem->configuration.m_incadd.append(tmpInc);

    }
    insideItem=(InsideCheckListItem*)insideItem->itemBelow();
  }

  QCheckListItem *outsideItem=(QCheckListItem *)outsideinc_listview->firstChild();
  while(outsideItem)
  {
    myProjectItem->configuration.m_incadd.append(outsideItem->text(0));
    outsideItem=(QCheckListItem*)outsideItem->itemBelow();
  }

  //target.install
  if (checkInstallTarget->isChecked() == true)
  {
    myProjectItem->configuration.m_target_install = true;
    myProjectItem->configuration.m_target_install_path = m_InstallTargetPath->text();
  }
  else
  {
    myProjectItem->configuration.m_target_install = false;
  }

  //makefile
  myProjectItem->configuration.m_makefile = makefile_url->url();

  //add libs to link
  myProjectItem->configuration.m_libadd.clear();
  myProjectItem->configuration.m_librarypath.clear();

  //inside libs to link
//  myProjectItem->configuration.m_prjdeps.clear();
  insideItem=(InsideCheckListItem *)insidelib_listview->firstChild();
  while(insideItem)
  {
    if(insideItem->isOn()){

      QString tmpLib=insideItem->prjItem->getLibAddObject(myProjectItem->getDownDirs());
      if(insideItem->prjItem->configuration.m_requirements & QD_SHARED)
      {
        tmpLib=/*"-l"+*/tmpLib;

        //add path if shared lib is linked
        QString tmpPath=insideItem->prjItem->getLibAddPath(myProjectItem->getDownDirs());
        if(tmpPath!=""){
          myProjectItem->configuration.m_librarypath.append(/*"-L"+*/tmpPath);
        }
      }
      myProjectItem->configuration.m_libadd.append(tmpLib);

    }
    insideItem=(InsideCheckListItem*)insideItem->itemBelow();
  }

  //outside libs to link
  outsideItem=(QCheckListItem *)outsidelib_listview->firstChild();
  while(outsideItem)
  {
    myProjectItem->configuration.m_libadd.append(outsideItem->text(0));
    outsideItem=(QCheckListItem*)outsideItem->itemBelow();
  }

  //external project dependencies
  myProjectItem->configuration.m_prjdeps.clear();
  QListViewItem *depItem=extDeps_view->firstChild();
  while(depItem)
  {
    myProjectItem->configuration.m_prjdeps.append(depItem->text(0));
    depItem=depItem->itemBelow();
  }
  //internal project dependencies
  insideItem=dynamic_cast<InsideCheckListItem *>(intDeps_view->firstChild());
  while(insideItem)
  {
      if (insideItem->isOn())
      {
        if(insideItem->prjItem->configuration.m_requirements & QD_STATIC)
        {
            QString tmpLib=insideItem->prjItem->getLibAddObject(myProjectItem->getDownDirs());
            myProjectItem->configuration.m_prjdeps.append(tmpLib);
        }
        else if(insideItem->prjItem->configuration.m_requirements & QD_SHARED)
        {
            QString tmpLib=insideItem->prjItem->getSharedLibAddObject(myProjectItem->getDownDirs());
            myProjectItem->configuration.m_prjdeps.append(tmpLib);
        }
        else
        {
            QString tmpApp=insideItem->prjItem->getApplicationObject(myProjectItem->getDownDirs());
            myProjectItem->configuration.m_prjdeps.append(tmpApp);
        }
      }
      insideItem=dynamic_cast<InsideCheckListItem *>(insideItem->itemBelow());
  }

  //extra lib paths
  QListViewItem *lvItem=outsidelibdir_listview->firstChild();
  while(lvItem)
  {
    myProjectItem->configuration.m_librarypath.append(lvItem->text(0));
    lvItem=lvItem->itemBelow();
  }

  //change build order
  lvItem=buildorder_listview->firstChild();
  myProjectItem->subdirs.clear();
  while(lvItem)
  {
    myProjectItem->subdirs.append(lvItem->text(0));
    lvItem=lvItem->itemBelow();
  }

  // intermediate locations
  myProjectItem->configuration.m_objectpath = objdir_url->url();
  myProjectItem->configuration.m_uipath = uidir_url->url();
  myProjectItem->configuration.m_mocpath = mocdir_url->url();

  //CORBA
  myProjectItem->configuration.idl_compiler=idlCmdEdit->url();
  myProjectItem->configuration.idl_options=idlCmdOptionsEdit->text();

  // custom vars
  QListViewItem *item = customVariables->firstChild();
  for( ; item; item = item->nextSibling() )
	myProjectItem->configuration.m_variables[item->text(0)] = item->text(1);

}

void ProjectConfigurationDlg::accept()
{
  apply();
  myProjectItem=0;
  QDialog::accept();
}

void ProjectConfigurationDlg::reject()
{
  myProjectItem=0;
  QDialog::reject();
}

void ProjectConfigurationDlg::updateControls()
//============================================
{
  QRadioButton *activateRadiobutton=NULL;
  // Project template
  libGroup->setEnabled(false);
  if( !myProjectItem->isScope )
  {
	switch (myProjectItem->configuration.m_template)
	{
	case QTMP_APPLICATION:
	activateRadiobutton = radioApplication;
	if (myProjectItem->configuration.m_requirements & QD_CONSOLE )
		checkConsole->setChecked(true);
	break;
	case QTMP_LIBRARY:
	libGroup->setEnabled(true);

	activateRadiobutton = radioLibrary;
	staticRadio->setChecked(true); //default
	if (myProjectItem->configuration.m_requirements & QD_STATIC){
		staticRadio->setChecked(true);
	}
	if (myProjectItem->configuration.m_requirements & QD_SHARED){
		sharedRadio->setChecked(true);
		m_targetLibraryVersion->setText(myProjectItem->configuration.m_libraryversion);
	}

	if (myProjectItem->configuration.m_requirements & QD_PLUGIN)
		checkPlugin->setChecked(true);
	if ( myProjectItem->configuration.m_requirements & QD_DESIGNER )
		checkDesigner->setChecked( true );
	if (myProjectItem->configuration.m_requirements & QD_LIBTOOL )
		checkLibtool->setChecked(true);
	if (myProjectItem->configuration.m_requirements & QD_PKGCONF )
		checkPkgconf->setChecked(true);
	break;
	case QTMP_SUBDIRS:
	activateRadiobutton = radioSubdirs;
	break;
	}
  }
  // Buildmode
  if (activateRadiobutton)
    activateRadiobutton->setChecked(true);
  switch (myProjectItem->configuration.m_buildMode)
  {
    case QBM_DEBUG:
      activateRadiobutton = radioDebugMode;
      break;
    case QBM_RELEASE:
      activateRadiobutton = radioReleaseMode;
      break;
    case QBM_DEBUG_AND_RELEASE:
      activateRadiobutton = radioDebugReleaseMode;
      break;
  }
  if (activateRadiobutton)
    activateRadiobutton->setChecked(true);

  // Requirements
  if (myProjectItem->configuration.m_requirements & QD_QT)
    checkQt->setChecked(true);
  if (myProjectItem->configuration.m_requirements & QD_OPENGL)
    checkOpenGL->setChecked(true);
  if (myProjectItem->configuration.m_requirements & QD_THREAD)
    checkThread->setChecked(true);
  if (myProjectItem->configuration.m_requirements & QD_X11)
    checkX11->setChecked(true);
  if (myProjectItem->configuration.m_requirements & QD_ORDERED)
    checkOrdered->setChecked(true);
  if (myProjectItem->configuration.m_requirements & QD_EXCEPTIONS)
    exceptionCheck->setChecked(true);
  if (myProjectItem->configuration.m_requirements & QD_STL)
    stlCheck->setChecked(true);
  if (myProjectItem->configuration.m_requirements & QD_RTTI)
    rttiCheck->setChecked(true);
  if (myProjectItem->configuration.m_requirements & QD_PCH)
    checkPCH->setChecked(true);
  // Warnings
  if (myProjectItem->configuration.m_warnings == QWARN_ON)
  {
    checkWarning->setChecked(true);
  }

  //Qt4 libs
  if ( prjWidget->m_part->isQt4Project() )
  {

    if ( myProjectItem->configuration.m_requirements & QD_ASSISTANT )
      checkAssistant->setChecked( true );
    if ( myProjectItem->configuration.m_requirements & QD_TESTLIB )
      checkTestlib->setChecked( true );
    if ( myProjectItem->configuration.m_requirements & QD_UITOOLS )
      checkUiTools->setChecked( true );
    if ( myProjectItem->configuration.m_requirements & QD_DBUS )
      checkQDBus->setChecked( true );
    if ( myProjectItem->configuration.m_requirements & QD_BUILDALL )
      checkBuildAll->setChecked( true );

    if ( myProjectItem->configuration.m_qt4libs & Q4L_CORE )
      checkQt4Core->setChecked( true );
    else
      checkQt4Core->setChecked( false );
    if ( myProjectItem->configuration.m_qt4libs & Q4L_GUI )
      checkQt4Gui->setChecked( true );
    else
      checkQt4Gui->setChecked( false );
    if ( myProjectItem->configuration.m_qt4libs & Q4L_SQL )
      checkQt4SQL->setChecked( true );
    if ( myProjectItem->configuration.m_qt4libs & Q4L_XML )
      checkQt4XML->setChecked( true );
    if ( myProjectItem->configuration.m_qt4libs & Q4L_NETWORK )
      checkQt4Network->setChecked( true );
    if ( myProjectItem->configuration.m_qt4libs & Q4L_SVG )
      checkQt4SVG->setChecked( true );
    if ( myProjectItem->configuration.m_qt4libs & Q4L_OPENGL )
      checkQt4OpenGL->setChecked( true );
    if ( myProjectItem->configuration.m_qt4libs & Q4L_QT3 )
      checkQt3Support->setChecked( true );
  }

  //makefile
  makefile_url->setURL(myProjectItem->configuration.m_makefile);

  // Target
/*  QString targetString = myProjectItem->configuration.m_target;
  int slashPos = targetString.findRev('/');

  if (slashPos>=0)
  {
    m_targetPath->setText(targetString.left(slashPos));
    m_targetOutputFile->setText(targetString.right(targetString.length()-slashPos-1));
  }
  else
    m_targetOutputFile->setText(targetString);
*/
  //target.path = path
  //INSTALLS += target
  if (myProjectItem->configuration.m_target_install == true)
  {
    checkInstallTarget->setChecked(true);
    m_InstallTargetPath->setEnabled(true);
  }
  else
  {
    checkInstallTarget->setChecked(false);
    m_InstallTargetPath->setEnabled(false);
  }
  m_InstallTargetPath->setText(myProjectItem->configuration.m_target_install_path);



  m_targetOutputFile->setText(myProjectItem->configuration.m_target);
  m_targetPath->setText(myProjectItem->configuration.m_destdir);
  clickSubdirsTemplate();

  m_defines->setText(myProjectItem->configuration.m_defines.join(" "));
  m_debugFlags->setText(myProjectItem->configuration.m_cxxflags_debug.join(" "));
  m_releaseFlags->setText(myProjectItem->configuration.m_cxxflags_release.join(" "));

  if (myProjectItem->configuration.m_inheritconfig == false)
    checkDontInheritConfig->setChecked(true);
  else
    checkDontInheritConfig->setChecked(false);

  updateIncludeControl();
  updateLibaddControl();
  updateLibDirAddControl();
  updateBuildOrderControl();
  updateDependenciesControl();

  objdir_url->setURL(myProjectItem->configuration.m_objectpath);
  uidir_url->setURL(myProjectItem->configuration.m_uipath);
  mocdir_url->setURL(myProjectItem->configuration.m_mocpath);


  customVariables->clear();
  QMap<QString,QString>::Iterator idx = myProjectItem->configuration.m_variables.begin();
  for( ; idx != myProjectItem->configuration.m_variables.end(); ++idx )
  	new KListViewItem(customVariables,idx.key(),idx.data());
}

QPtrList <qProjectItem> ProjectConfigurationDlg::getAllProjects()
{
  QPtrList <qProjectItem> tmpPrjList;
  qProjectItem *item=(qProjectItem *)prjList->firstChild();
  while(item)
  {
    tmpPrjList.append(item);
    getAllSubProjects(item,&tmpPrjList);
    item=(qProjectItem *)item->nextSibling();
  }
  return(tmpPrjList);
}

void ProjectConfigurationDlg::getAllSubProjects(qProjectItem *item, QPtrList <qProjectItem> *itemList)
{

  qProjectItem *subItem=(qProjectItem *)item->firstChild();
  while(subItem)
  {
    itemList->append(subItem);
    getAllSubProjects(subItem,itemList);
    subItem=(qProjectItem *)subItem->nextSibling();
  }
}



void ProjectConfigurationDlg::updateIncludeControl()
{
  insideinc_listview->setSorting(-1,false);
  outsideinc_listview->setSorting(-1,false);


  QStringList incList=myProjectItem->configuration.m_incadd;
  QPtrList <qProjectItem> itemList=getAllProjects();
  qProjectItem *item=itemList.first();


  while(item){
    if(item->type() == qProjectItem::Subproject)
    {

      //includes
      //temp strlist
      if( ((SubqmakeprojectItem*)item)->configuration.m_template==QTMP_LIBRARY ||
          ((SubqmakeprojectItem*)item)->configuration.m_template==QTMP_APPLICATION){
          SubqmakeprojectItem *sItem=(SubqmakeprojectItem*)item;
         //if(sItem!=myProjectItem) //needed own path for other libs
         if (!sItem->isScope)
         {
              QString tmpInc=sItem->getIncAddPath(myProjectItem->getDownDirs());
              tmpInc=QDir::cleanDirPath(tmpInc);
              InsideCheckListItem *newItem=new InsideCheckListItem(insideinc_listview,insideinc_listview->lastItem(),sItem, this);
              QStringList::Iterator it=incList.begin();

          for(;it!=incList.end();++it)
          {
             // we need an exact, case sensitive match here
            if ((*it).contains(tmpInc, true) && ((*it).length() == tmpInc.length()))
            {
              incList.remove(it);
              newItem->setOn(true);
              it=incList.begin();
            }
          }
        }
      }
    }
//    item=(ProjectItem*)item->itemBelow();
    item=itemList.next();
  }

  //all other in incList are outside incs
  QStringList::Iterator it1=incList.begin();
  for(;it1!=incList.end();++it1)
  {
      new QListViewItem(outsideinc_listview,outsideinc_listview->lastItem(),(*it1));
  }

}
void ProjectConfigurationDlg::updateLibaddControl()
{

  QPtrList <qProjectItem> itemList=getAllProjects();

  insidelib_listview->setSorting(-1,false);
  outsidelib_listview->setSorting(-1,false);

  //update librarys
  //temp strlist
  QStringList libList=myProjectItem->configuration.m_libadd;


  QStringList::Iterator it=libList.begin();
  for(;it!=libList.end();++it){
        qProjectItem *item=itemList.first();
        while(item){
          if(item->type()==qProjectItem::Subproject)
          {

            //librarys
            if( ((SubqmakeprojectItem*)item)->configuration.m_template==QTMP_LIBRARY ){
              SubqmakeprojectItem *sItem=(SubqmakeprojectItem*)item;


              if(sItem!=myProjectItem)
              {
                // create lib string
                  QString tmpLib=sItem->getLibAddObject(myProjectItem->getDownDirs());
                  if(sItem->configuration.m_requirements & QD_SHARED) tmpLib=/*"-l"+*/tmpLib;
                  if(tmpLib==(*it)){
                    InsideCheckListItem *newItem=new InsideCheckListItem(insidelib_listview,insidelib_listview->lastItem(),sItem, this);
                    libList.remove(it);
                    it=libList.begin();
                    newItem->setOn(true);
                    itemList.remove(item);
                    item=itemList.first();
                  }

              }
            }
          }
          item=itemList.next();
        }
    }

  //add all other prj in itemList unchecked
  qProjectItem *item=itemList.first();
  while(item)
  {
    if(item->type()==qProjectItem::Subproject)
    {
      //librarys
      if( ((SubqmakeprojectItem*)item)->configuration.m_template==QTMP_LIBRARY ){
        SubqmakeprojectItem *sItem=(SubqmakeprojectItem*)item;
        if(sItem!=myProjectItem)
        {
          // create lib string
          QString tmpLib=sItem->getLibAddObject(myProjectItem->getDownDirs());
          if(sItem->configuration.m_requirements & QD_SHARED) tmpLib=/*"-l"+*/tmpLib;
          InsideCheckListItem *newItem=new InsideCheckListItem(insidelib_listview,insidelib_listview->lastItem(),sItem,this);
          newItem->setOn(false);
        }
      }
    }
    item=itemList.next();
  }

  //all other in libList are outside libs
  QStringList::Iterator it1=libList.begin();
  for(;it1!=libList.end();++it1)
  {
      new QListViewItem(outsidelib_listview,outsidelib_listview->lastItem(),(*it1));
  }
}

void ProjectConfigurationDlg::updateDependenciesControl( )
{
  QPtrList <qProjectItem> itemList=getAllProjects();

  intDeps_view->setSorting(-1,false);
  extDeps_view->setSorting(-1,false);

  QStringList depsList=myProjectItem->configuration.m_prjdeps;

  QStringList::Iterator it=depsList.begin();
  for(;it!=depsList.end();++it){
        qProjectItem *item=itemList.first();
        while(item){
          if(item->type()==qProjectItem::Subproject)
          {
            if( (((SubqmakeprojectItem*)item)->configuration.m_template==QTMP_LIBRARY )
                || (((SubqmakeprojectItem*)item)->configuration.m_template==QTMP_APPLICATION ) ){
              SubqmakeprojectItem *sItem=(SubqmakeprojectItem*)item;
              if ((!sItem->isScope) && (sItem!=myProjectItem))
              {
                  QString tmpLib;
                  if(sItem->configuration.m_requirements & QD_SHARED)
                    tmpLib = sItem->getSharedLibAddObject(myProjectItem->getDownDirs());
                  else if(sItem->configuration.m_requirements & QD_STATIC)
                    tmpLib=sItem->getLibAddObject(myProjectItem->getDownDirs());
                  else
                    tmpLib=sItem->getApplicationObject(myProjectItem->getDownDirs());
                  if(tmpLib==(*it)){
                    InsideCheckListItem *newItem=new InsideCheckListItem(intDeps_view,intDeps_view->lastItem(),sItem,this);
                    depsList.remove(it);
                    it=depsList.begin();
                    newItem->setOn(true);
                    itemList.remove(item);
                    item=itemList.first();
                  }

              }
            }
          }
          item=itemList.next();
        }
    }

  //add all other prj in itemList unchecked
  qProjectItem *item=itemList.first();
  while(item)
  {
    if(item->type()==qProjectItem::Subproject)
    {
      if( (((SubqmakeprojectItem*)item)->configuration.m_template==QTMP_LIBRARY )
        || (((SubqmakeprojectItem*)item)->configuration.m_template==QTMP_APPLICATION ) ){
        SubqmakeprojectItem *sItem=(SubqmakeprojectItem*)item;
        if ((!sItem->isScope) && (sItem!=myProjectItem))
        {
          QString tmpLib;
          if(sItem->configuration.m_requirements & QD_SHARED)
            tmpLib = sItem->getSharedLibAddObject(myProjectItem->getDownDirs());
          else if(sItem->configuration.m_requirements & QD_STATIC)
            tmpLib=sItem->getLibAddObject(myProjectItem->getDownDirs());
          else
            tmpLib=sItem->getApplicationObject(myProjectItem->getDownDirs());
          InsideCheckListItem *newItem=new InsideCheckListItem(intDeps_view,intDeps_view->lastItem(),sItem,this);
          newItem->setOn(false);
        }
      }
    }
    item=itemList.next();
  }

  QStringList::Iterator it1=depsList.begin();
  for(;it1!=depsList.end();++it1)
  {
      new QListViewItem(extDeps_view,extDeps_view->lastItem(),(*it1));
  }
}


void ProjectConfigurationDlg::updateBuildOrderControl()
{
  //sort build order only if subdirs
  if(myProjectItem->configuration.m_template==QTMP_SUBDIRS)
  {

      QPtrList <qProjectItem> itemList;

      qProjectItem *item=(qProjectItem *)myProjectItem->firstChild();
      while(item){
        itemList.append(item);
        item=(qProjectItem*)item->nextSibling();
      }



    incaddTab->setEnabled(false);
    buildorder_listview->setSorting(-1,false);
    QStringList buildList=myProjectItem->subdirs;
    QStringList::Iterator it1=buildList.begin();
    for(;it1!=buildList.end();++it1){
      item=itemList.first();
      while(item){
        if(item->type()==qProjectItem::Subproject)
        {
          if(item->text(0)==(*it1)){
            new QListViewItem(buildorder_listview,buildorder_listview->lastItem(),item->text(0));
            itemList.take();
            item=itemList.first();
          }
        }
        item=itemList.next();;
      }
    }
    //add the rest
      item=itemList.first();
      while(item){
        if(item->type()==qProjectItem::Subproject)
        {
            new QListViewItem(buildorder_listview,buildorder_listview->lastItem(),item->text(0));
        }
        item=itemList.next();;
      }

  }
}

void ProjectConfigurationDlg::updateLibDirAddControl()
{
  //update librarys
  //temp strlist
  QStringList libDirList=myProjectItem->configuration.m_librarypath;

  QPtrList <qProjectItem> itemList=getAllProjects();
  outsidelibdir_listview->setSorting(-1,false);

  qProjectItem *item=itemList.first();

  while(item){
    if(item->type()==qProjectItem::Subproject)
    {

      //librarys
      if( ((SubqmakeprojectItem*)item)->configuration.m_template==QTMP_LIBRARY ){
        SubqmakeprojectItem *sItem=(SubqmakeprojectItem*)item;

        if(sItem!=myProjectItem)
        {
          // create lib string
          QString tmpLibDir=sItem->getLibAddPath(myProjectItem->getDownDirs());

          QStringList::Iterator it=libDirList.begin();
          for(;it!=libDirList.end();++it)
          {

            if((*it).find(tmpLibDir)>=0 && tmpLibDir!="" && !tmpLibDir.isEmpty()){
              libDirList.remove(it);
              it=libDirList.begin();
            }
          }
        }
      }
    }
    item=itemList.next();
  }

  //all other in libList are outside libs
  QStringList::Iterator it1=libDirList.begin();
  for(;it1!=libDirList.end();++it1)
  {
      new QListViewItem(outsidelibdir_listview,outsidelibdir_listview->lastItem(),(*it1));
  }

}


void ProjectConfigurationDlg::slotStaticLibClicked(int)
{
/*  dymo: static libs can be linked with other libs too
    so please don't uncomment following*/
/*if (staticRadio->isChecked())
  {
    libAddTab->setEnabled(false);
    libPathTab->setEnabled(false);
  } else {
    libAddTab->setEnabled(true);
    libPathTab->setEnabled(true);
  }*/
  if ( staticRadio->isChecked() )
  {
    checkPlugin->setEnabled( false );
    checkDesigner->setEnabled( false );
  }
  buttonApply->setEnabled(true);
}

void ProjectConfigurationDlg::sharedLibChanged()
{
  if ( sharedRadio->isChecked() )
  {
    checkPlugin->setEnabled( true );
    checkDesigner->setEnabled( checkPlugin->isChecked() );
  }
  buttonApply->setEnabled(true);
}

void ProjectConfigurationDlg::templateLibraryClicked(int)
{
  if (radioLibrary->isChecked())
  {
//    staticRadio->setChecked(true);
    TabBuild->setTabEnabled(custVarsTab, true);
    TabBuild->setTabEnabled(depTab,true);
    TabBuild->setTabEnabled(libAddTab,true);
    TabBuild->setTabEnabled(incaddTab,true);
    TabBuild->setTabEnabled(buildOptsTab,true);
    TabBuild->setTabEnabled(configTab,true);
    libGroup->setEnabled(true);
  } else {
    libGroup->setEnabled(false);
  }
  buttonApply->setEnabled(true);
}
void ProjectConfigurationDlg::clickSubdirsTemplate()
{
  if (radioSubdirs->isChecked())
  {
    TabBuild->setTabEnabled(custVarsTab, true);
    TabBuild->setTabEnabled(depTab,false);
    TabBuild->setTabEnabled(libAddTab,false);
    TabBuild->setTabEnabled(incaddTab,false);
    TabBuild->setTabEnabled(buildOptsTab,false);
    TabBuild->setTabEnabled(configTab,false);
  }
  else
  {
    TabBuild->setTabEnabled(custVarsTab, true);
    TabBuild->setTabEnabled(depTab,true);
    TabBuild->setTabEnabled(libAddTab,true);
    TabBuild->setTabEnabled(incaddTab,true);
    TabBuild->setTabEnabled(buildOptsTab,true);
    TabBuild->setTabEnabled(configTab,true);

    if (radioLibrary->isChecked()) libGroup->setEnabled(true);
    else libGroup->setEnabled(false);
    //70corbaTab->setEnabled(true);
  }
  buttonApply->setEnabled(true);
}



//build order buttons
void ProjectConfigurationDlg::buildorderMoveUpClicked()
{
    if (buildorder_listview->currentItem() == buildorder_listview->firstChild()) {
        KNotifyClient::beep();
        return;
    }

    QListViewItem *item = buildorder_listview->firstChild();
    while (item->nextSibling() != buildorder_listview->currentItem())
        item = item->nextSibling();
    item->moveItem(buildorder_listview->currentItem());
    buttonApply->setEnabled(true);
}


void ProjectConfigurationDlg::buildorderMoveDownClicked()
{
   if (buildorder_listview->currentItem() == 0 || buildorder_listview->currentItem()->nextSibling() == 0) {
        KNotifyClient::beep();
        return;
   }

   buildorder_listview->currentItem()->moveItem(buildorder_listview->currentItem()->nextSibling());
  buttonApply->setEnabled(true);

}







//Include dir buttons
void ProjectConfigurationDlg::insideIncMoveUpClicked()
{
    if (insideinc_listview->currentItem() == insideinc_listview->firstChild()) {
        KNotifyClient::beep();
        return;
    }

    QListViewItem *item = insideinc_listview->firstChild();
    while (item->nextSibling() != insideinc_listview->currentItem())
        item = item->nextSibling();
    item->moveItem(insideinc_listview->currentItem());
  buttonApply->setEnabled(true);

}


void ProjectConfigurationDlg::insideIncMoveDownClicked()
{
   if (insideinc_listview->currentItem() == 0 || insideinc_listview->currentItem()->nextSibling() == 0) {
        KNotifyClient::beep();
        return;
   }

   insideinc_listview->currentItem()->moveItem(insideinc_listview->currentItem()->nextSibling());
  buttonApply->setEnabled(true);

}


void ProjectConfigurationDlg::outsideIncMoveUpClicked()
{
    if (outsideinc_listview->currentItem() == outsideinc_listview->firstChild()) {
        KNotifyClient::beep();
        return;
    }

    QListViewItem *item = outsideinc_listview->firstChild();
    while (item->nextSibling() != outsideinc_listview->currentItem())
        item = item->nextSibling();
    item->moveItem(outsideinc_listview->currentItem());
  buttonApply->setEnabled(true);

}


void ProjectConfigurationDlg::outsideIncMoveDownClicked()
{
   if (outsideinc_listview->currentItem() == 0 || outsideinc_listview->currentItem()->nextSibling() == 0) {
        KNotifyClient::beep();
        return;
   }

   outsideinc_listview->currentItem()->moveItem(outsideinc_listview->currentItem()->nextSibling());
  buttonApply->setEnabled(true);

}


void ProjectConfigurationDlg::outsideIncAddClicked()
{
    KURLRequesterDlg dialog("", i18n("Add include directory:"), 0, 0);
    dialog.urlRequester()->setMode(KFile::Directory);
    if (dialog.exec() != QDialog::Accepted)
        return;
    QString dir = dialog.urlRequester()->url();
    if (!dir.isEmpty())
        new QListViewItem(outsideinc_listview, dir);
  buttonApply->setEnabled(true);

}


void ProjectConfigurationDlg::outsideIncRemoveClicked()
{
    delete outsideinc_listview->currentItem();
  buttonApply->setEnabled(true);

}










//libadd buttons
void ProjectConfigurationDlg::insideLibMoveUpClicked()
{
    if (insidelib_listview->currentItem() == insidelib_listview->firstChild()) {
        KNotifyClient::beep();
        return;
    }

    QListViewItem *item = insidelib_listview->firstChild();
    while (item->nextSibling() != insidelib_listview->currentItem())
        item = item->nextSibling();
    item->moveItem(insidelib_listview->currentItem());
  buttonApply->setEnabled(true);

}


void ProjectConfigurationDlg::insideLibMoveDownClicked()
{
   if (insidelib_listview->currentItem() == 0 || insidelib_listview->currentItem()->nextSibling() == 0) {
        KNotifyClient::beep();
        return;
   }

   insidelib_listview->currentItem()->moveItem(insidelib_listview->currentItem()->nextSibling());
  buttonApply->setEnabled(true);

}


void ProjectConfigurationDlg::outsideLibMoveUpClicked()
{
    if (outsidelib_listview->currentItem() == outsidelib_listview->firstChild()) {
        KNotifyClient::beep();
        return;
    }

    QListViewItem *item = outsidelib_listview->firstChild();
    while (item->nextSibling() != outsidelib_listview->currentItem())
        item = item->nextSibling();
    item->moveItem(outsidelib_listview->currentItem());
  buttonApply->setEnabled(true);

}


void ProjectConfigurationDlg::outsideLibMoveDownClicked()
{
   if (outsidelib_listview->currentItem() == 0 || outsidelib_listview->currentItem()->nextSibling() == 0) {
        KNotifyClient::beep();
        return;
   }

   outsidelib_listview->currentItem()->moveItem(outsidelib_listview->currentItem()->nextSibling());
  buttonApply->setEnabled(true);

}


void ProjectConfigurationDlg::outsideLibAddClicked()
{
    bool ok;
    QString dir = KInputDialog::getText(i18n("Add Library"), i18n("Add library to link:"), "-l", &ok, 0);
    if (ok && !dir.isEmpty() && dir != "-I")
        new QListViewItem(outsidelib_listview, dir);
  buttonApply->setEnabled(true);

}


void ProjectConfigurationDlg::outsideLibRemoveClicked()
{
    delete outsidelib_listview->currentItem();
  buttonApply->setEnabled(true);
}



//lib paths buttons


void ProjectConfigurationDlg::outsideLibDirMoveUpClicked()
{
    if (outsidelibdir_listview->currentItem() == outsidelibdir_listview->firstChild()) {
        KNotifyClient::beep();
        return;
    }

    QListViewItem *item = outsidelibdir_listview->firstChild();
    while (item->nextSibling() != outsidelibdir_listview->currentItem())
        item = item->nextSibling();
    item->moveItem(outsidelibdir_listview->currentItem());
  buttonApply->setEnabled(true);
}


void ProjectConfigurationDlg::outsideLibDirMoveDownClicked()
{
   if (outsidelibdir_listview->currentItem() == 0 || outsidelibdir_listview->currentItem()->nextSibling() == 0) {
        KNotifyClient::beep();
        return;
   }

   outsidelibdir_listview->currentItem()->moveItem(outsidelibdir_listview->currentItem()->nextSibling());
  buttonApply->setEnabled(true);
}


void ProjectConfigurationDlg::outsideLibDirAddClicked()
{
    KURLRequesterDlg dialog("", i18n("Add library directory:"), 0, 0);
    dialog.urlRequester()->setMode(KFile::Directory);
    if (dialog.exec() != QDialog::Accepted)
        return;
    QString dir = dialog.urlRequester()->url();
    if (!dir.isEmpty())
        new QListViewItem(outsidelibdir_listview, dir);
  buttonApply->setEnabled(true);
}


void ProjectConfigurationDlg::outsideLibDirRemoveClicked()
{
    delete outsidelibdir_listview->currentItem();
  buttonApply->setEnabled(true);
}

void ProjectConfigurationDlg::outsideIncEditClicked()
{
    QListViewItem *item=outsideinc_listview->currentItem();
    if(item==NULL)return;
    QString text=item->text(0);

    KURLRequesterDlg dialog(text, i18n("Change include directory:"), 0, 0);
    dialog.urlRequester()->setMode(KFile::Directory);
    if (dialog.exec() != QDialog::Accepted)
        return;
    QString dir = dialog.urlRequester()->url();
    if (!dir.isEmpty())
        item->setText(0,dir);
  buttonApply->setEnabled(true);
}

void ProjectConfigurationDlg::outsideLibEditClicked()
{
    bool ok;
    QListViewItem *item=outsidelib_listview->currentItem();
    if(item==NULL)return;
    QString text=item->text(0);

    QString dir = KInputDialog::getText(i18n("Change Library"), i18n("Change library to link:"), text, &ok, 0);
    if (ok && !dir.isEmpty() && dir != "-l")
        item->setText(0,dir);

  buttonApply->setEnabled(true);
}

void ProjectConfigurationDlg::outsideLibDirEditClicked()
{
    QListViewItem *item=outsidelibdir_listview->currentItem();
    if(item==NULL)return;
    QString text=item->text(0);

    KURLRequesterDlg dialog(text, i18n("Change library directory:"), 0, 0);
    dialog.urlRequester()->setMode(KFile::Directory);
    if (dialog.exec() != QDialog::Accepted)
        return;
    QString dir = dialog.urlRequester()->url();
    if (!dir.isEmpty())
        item->setText(0,dir);
  buttonApply->setEnabled(true);
}

void ProjectConfigurationDlg::slotInstallTargetClicked()
{
  if (checkInstallTarget->isChecked() == true)
    m_InstallTargetPath->setEnabled(true);
  else
    m_InstallTargetPath->setEnabled(false);
  buttonApply->setEnabled(true);
}


void ProjectConfigurationDlg::extAdd_button_clicked( )
{
    KURLRequesterDlg dialog("", i18n("Add target:"), 0, 0);
    dialog.urlRequester()->setMode(KFile::File);
    if (dialog.exec() != QDialog::Accepted)
        return;
    QString path = dialog.urlRequester()->url();
    if (!path.isEmpty())
        new QListViewItem(extDeps_view, path);
  buttonApply->setEnabled(true);
}

void ProjectConfigurationDlg::extEdit_button_clicked( )
{
    QListViewItem *item=extDeps_view->currentItem();
    if(item==NULL)return;
    QString text=item->text(0);

    KURLRequesterDlg dialog(text, i18n("Change target:"), 0, 0);
    dialog.urlRequester()->setMode(KFile::File);
    if (dialog.exec() != QDialog::Accepted)
        return;
    QString path = dialog.urlRequester()->url();
    if (!path.isEmpty())
        item->setText(0, path);
  buttonApply->setEnabled(true);
}

void ProjectConfigurationDlg::extMoveDown_button_clicked( )
{
   if (extDeps_view->currentItem() == 0 || extDeps_view->currentItem()->nextSibling() == 0) {
        KNotifyClient::beep();
        return;
   }

   extDeps_view->currentItem()->moveItem(extDeps_view->currentItem()->nextSibling());
  buttonApply->setEnabled(true);
}

void ProjectConfigurationDlg::extMoveUp_button_clicked( )
{
    if (extDeps_view->currentItem() == extDeps_view->firstChild()) {
        KNotifyClient::beep();
        return;
    }

    QListViewItem *item = extDeps_view->firstChild();
    while (item->nextSibling() != extDeps_view->currentItem())
        item = item->nextSibling();
    item->moveItem(extDeps_view->currentItem());
  buttonApply->setEnabled(true);
}

void ProjectConfigurationDlg::extRemove_button_clicked( )
{
    delete extDeps_view->currentItem();
  buttonApply->setEnabled(true);
}

void ProjectConfigurationDlg::intMoveDown_button_clicked( )
{
   if (intDeps_view->currentItem() == 0 || intDeps_view->currentItem()->nextSibling() == 0) {
        KNotifyClient::beep();
        return;
   }

   intDeps_view->currentItem()->moveItem(intDeps_view->currentItem()->nextSibling());
  buttonApply->setEnabled(true);
}

void ProjectConfigurationDlg::intMoveUp_button_clicked( )
{
    if (intDeps_view->currentItem() == intDeps_view->firstChild()) {
        KNotifyClient::beep();
        return;
    }

    QListViewItem *item = intDeps_view->firstChild();
    while (item->nextSibling() != intDeps_view->currentItem())
        item = item->nextSibling();
    item->moveItem(intDeps_view->currentItem());
  buttonApply->setEnabled(true);
}

 void ProjectConfigurationDlg::addCustomValueClicked()
{
  QListViewItem *item = new QListViewItem(customVariables, i18n("Name"),i18n("Value"));
  customVariables->setSelected(item,true);
  newCustomVariableActive();
  customVariableName->setEnabled(true);
  buttonApply->setEnabled(true);
}
 void ProjectConfigurationDlg::removeCustomValueClicked()
{
  QListViewItem *item = customVariables->currentItem();
  if( item )
  {
    myProjectItem->configuration.m_variables.remove(item->text(0));
    myProjectItem->configuration.m_removed_variables.append(item->text(0));
    delete item;
  }
  buttonApply->setEnabled(true);
}
 void ProjectConfigurationDlg::editCustomValueClicked()
{
  QListViewItem *item = customVariables->currentItem();
  if(item)
  {
     item->setText(0,customVariableName->text());
     item->setText(1,customVariableData->text());

     if(myProjectItem->configuration.m_removed_variables.contains(customVariableName->text()) != 0)
	myProjectItem->configuration.m_removed_variables.remove(customVariableName->text());
  }
  buttonApply->setEnabled(true);
}
 void ProjectConfigurationDlg::upCustomValueClicked()
{
  // custom vars
  QListViewItem *item = customVariables->firstChild();
  if (customVariables->currentItem() == item) {
    KNotifyClient::beep();
    return;
  }
  while (item->nextSibling() != customVariables->currentItem())
    item = item->nextSibling();
  item->moveItem(customVariables->currentItem());
  buttonApply->setEnabled(true);
}

 void ProjectConfigurationDlg::downCustomValueClicked()
{
  if (customVariables->currentItem() == 0 || customVariables->currentItem()->nextSibling() == 0) {
    KNotifyClient::beep();
    return;
  }
  customVariables->currentItem()->moveItem(customVariables->currentItem()->nextSibling());
  buttonApply->setEnabled(true);
}

void ProjectConfigurationDlg::newCustomVariableActive( )
{
  QListViewItem *item = customVariables->currentItem();
  if(item)
  {
	customVariableName->setText(item->text(0));
	customVariableData->setText(item->text(1));
	customVariableName->setFocus();
	customVariableName->setEnabled(false);
  }
}

void ProjectConfigurationDlg::checkPluginChanged( )
{
  if ( checkPlugin->isChecked() && prjWidget->m_part->isQt4Project() )
  {
    checkDesigner->setEnabled( true );
  }else
  {
    checkDesigner->setEnabled( false );
  }
  buttonApply->setEnabled(true);
}

void ProjectConfigurationDlg::qtRequirementChanged()
{
  if ( checkQt->isChecked() && prjWidget->m_part->isQt4Project() )
  {
    qt4Group->setEnabled( true );
  }else
  {
    qt4Group->setEnabled( false );
  }
  buttonApply->setEnabled(true);
}

void ProjectConfigurationDlg::apply()
{
  updateProjectConfiguration();
  prjWidget->updateProjectConfiguration(myProjectItem);
  prjWidget->setupContext();
  buttonApply->setEnabled(false);
}

void ProjectConfigurationDlg::activateApply(int)
{
  buttonApply->setEnabled(true);
}
void ProjectConfigurationDlg::activateApply(const QString&)
{
  buttonApply->setEnabled(true);
}
