/***************************************************************************
 *   Copyright (C) 2002 by Jakob Simon-Gaarde                              *
 *   jsgaarde@tdcspace.dk                                                  *
 *   Copyright (C) 2002-2003 by Alexander Dymo                             *
 *   cloudtemple@mksat.net                                                 *
 *   Copyright (C) 2003 by Thomas Hasart                                   *
 *   thasart@gmx.de                                                        *
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

#include <qdialog.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qlistview.h>
#include <iostream>
#include <qregexp.h>
#include <qvalidator.h>
#include <qtabwidget.h>
#include <pathutil.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>
#include <klocale.h>

// in kde 3.4 KStdGuiItem::browse()
KGuiItem browse()
{
	KGuiItem returnItem;
	returnItem.setText(i18n("Browse"));
	returnItem.setIconName("browse");
	returnItem.setToolTip(i18n("Open browser"));
	returnItem.setWhatsThis(i18n("Opens a file browser for seletion of files or directories"));
	return returnItem;
}

// in kde 3.4 KStdGuiItem::up()
KGuiItem up()
{
	KGuiItem returnItem;
	returnItem.setText(i18n("Up"));
	returnItem.setIconName("up");
	returnItem.setToolTip(i18n("Move up"));
	return returnItem;
}
// in kde 3.4 KStdGuiItem::down()
KGuiItem down()
{
KGuiItem returnItem;
	returnItem.setText(i18n("Down"));
	returnItem.setIconName("down");
	returnItem.setToolTip(i18n("Move down"));
	return returnItem;
}
// in kde 3.4 KStdGuiItem::edit()
KGuiItem edit()
{
	KGuiItem returnItem;
	returnItem.setText(i18n("Edit"));
	returnItem.setIconName("edit");
	returnItem.setToolTip(i18n("Edit value"));
	return returnItem;
}

// in kde 3.4 KStdGuiItem::remove()
KGuiItem add()
{
	KGuiItem returnItem;
	returnItem.setText(i18n("Remove"));
	returnItem.setIconName("remove");
	returnItem.setToolTip(i18n("Remove value"));
	return returnItem;
}

// in kde 3.4 KStdGuiItem::add()
KGuiItem remove()
{
	KGuiItem returnItem;
	returnItem.setText(i18n("Add"));
	returnItem.setIconName("add");
	returnItem.setToolTip(i18n("Remove value"));
	return returnItem;
}

ProjectConfigurationDlg::ProjectConfigurationDlg(SubqmakeprojectItem *_item,QListView *_prjList,QWidget* parent, const char* name, bool modal, WFlags fl)
: ProjectConfigurationDlgBase(parent,name,modal,fl)
//=================================================
{
// Remove when we can depend on KDE 3.4
  buttonOk->setGuiItem(KStdGuiItem::ok());
  buttonCancel->setGuiItem(KStdGuiItem::cancel());
  Browse->setGuiItem(browse());
  insideIncMoveUpBtn->setGuiItem(up());
  insideIncMoveDownBtn->setGuiItem(down());
  outsideIncAddBtn->setGuiItem(add());
  outsideIncRemoveBtn->setGuiItem(remove());
  outsideIncEditBtn->setGuiItem(edit());
  outsideIncMoveUpBtn->setGuiItem(up());
  outsideIncMoveDownBtn->setGuiItem(down());
  insideLibMoveUpBtn->setGuiItem(up());
  insideLibMoveDownBtn->setGuiItem(down());
  outsideLibAddBtn->setGuiItem(add());
  outsideLibRemoveBtn->setGuiItem(remove());
  outsideLibEditBtn->setGuiItem(edit());
  outsideLibMoveUpBtn->setGuiItem(up());
  outsideLibMoveDownBtn->setGuiItem(down());
  outsideLibDirAddBtn->setGuiItem(add());
  outsideLibDirRemoveBtn->setGuiItem(remove());
  outsideLibDirEditBtn->setGuiItem(edit());
  outsideLibDirMoveUpBtn->setGuiItem(up());
  outsideLibDirMoveDownBtn->setGuiItem(down());
  intMoveUp_button->setGuiItem(up());
  intMoveDown_button->setGuiItem(down());
  extAdd_button->setGuiItem(add());
  extRemove_button->setGuiItem(remove());
  extEdit_button->setGuiItem(edit());
  extMoveUp_button->setGuiItem(up());
  extMoveDown_button->setGuiItem(down());
  buildmoveup_button->setGuiItem(up());
  buildmovedown_button->setGuiItem(down());
  varAdd_button->setGuiItem(add());
  varRemove_button->setGuiItem(remove());
  varEdit_button->setGuiItem(edit());
  varMoveUp_button->setGuiItem(up());
  varMoveDown_button->setGuiItem(down());
// End remove in kde 3.4

  myProjectItem=_item;
  prjList=_prjList;
//  m_projectConfiguration = conf;
  m_targetLibraryVersion->setValidator(new QRegExpValidator(
    QRegExp("\\d+(\\.\\d+)?(\\.\\d+)"), this));
  UpdateControls();
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
}


void ProjectConfigurationDlg::updateProjectConfiguration()
//=======================================================
{
  // Template
  myProjectItem->configuration.m_requirements = 0;

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
    if (sharedRadio->isOn()){
      myProjectItem->configuration.m_requirements += QD_SHARED;
      myProjectItem->configuration.m_libraryversion = m_targetLibraryVersion->text();
    }
    if (pluginRadio->isOn())
      myProjectItem->configuration.m_requirements += QD_PLUGIN;
    myProjectItem->setPixmap(0,SmallIcon("qmake_lib"));
  }
  else if (radioSubdirs->isChecked())
  {
    myProjectItem->configuration.m_template = QTMP_SUBDIRS;
    myProjectItem->setPixmap(0,SmallIcon("qmake_sub"));
  }

  // Buildmode
  if (radioDebugMode->isChecked())
    myProjectItem->configuration.m_buildMode = QBM_DEBUG;
  if (radioReleaseMode->isChecked())
    myProjectItem->configuration.m_buildMode = QBM_RELEASE;

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
  if (checkDll->isChecked() )
  {
    myProjectItem->configuration.m_requirements += QD_DLL;
  }
  if (checkConsole->isChecked() )
  {
    myProjectItem->configuration.m_requirements += QD_CONSOLE;
  }

  // Warnings
  myProjectItem->configuration.m_warnings = QWARN_OFF;
  if (checkWarning->isChecked())
    myProjectItem->configuration.m_warnings = QWARN_ON;

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
  {
	myProjectItem->configuration.m_variables[item->text(0)] = item->text(1);
  }

  QDialog::accept();


}


void ProjectConfigurationDlg::UpdateControls()
//============================================
{
  QRadioButton *activateRadiobutton=NULL;
  // Project template
  libGroup->setEnabled(false);
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
        pluginRadio->setChecked(true);
      if (myProjectItem->configuration.m_requirements & QD_DLL )
        checkDll->setChecked(true);
      if (myProjectItem->configuration.m_requirements & QD_LIBTOOL )
        checkLibtool->setChecked(true);
      if (myProjectItem->configuration.m_requirements & QD_PKGCONF )
        checkPkgconf->setChecked(true);
      break;
    case QTMP_SUBDIRS:
      activateRadiobutton = radioSubdirs;
      break;
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

  // Warnings
  if (myProjectItem->configuration.m_warnings == QWARN_ON)
  {
    checkWarning->setChecked(true);
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
            if((*it).find(tmpInc)>=0){
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
  	else buildOrderTab->setEnabled(false);
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
}


void ProjectConfigurationDlg::templateLibraryClicked(int)
{
  if (radioLibrary->isChecked())
  {
    libGroup->setEnabled(true);
//    staticRadio->setChecked(true);
    TabBuild->setTabEnabled(buildOrderTab, false);
    TabBuild->setTabEnabled(custVarsTab, true);
    TabBuild->setTabEnabled(depTab,true);
    TabBuild->setTabEnabled(libAddTab,true);
    TabBuild->setTabEnabled(incaddTab,true);
    TabBuild->setTabEnabled(buildOptsTab,true);
    TabBuild->setTabEnabled(configTab,true);
  } else {
    libGroup->setEnabled(false);
  }
}
void ProjectConfigurationDlg::clickSubdirsTemplate()
{
  if (radioSubdirs->isChecked())
  {
    TabBuild->setTabEnabled(buildOrderTab, true);
    TabBuild->setTabEnabled(custVarsTab, true);
    TabBuild->setTabEnabled(depTab,false);
    TabBuild->setTabEnabled(libAddTab,false);
    TabBuild->setTabEnabled(incaddTab,false);
    TabBuild->setTabEnabled(buildOptsTab,false);
    TabBuild->setTabEnabled(configTab,false);
  }
  else
  {
    TabBuild->setTabEnabled(buildOrderTab, false);
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
}


void ProjectConfigurationDlg::buildorderMoveDownClicked()
{
   if (buildorder_listview->currentItem() == 0 || buildorder_listview->currentItem()->nextSibling() == 0) {
        KNotifyClient::beep();
        return;
   }

   buildorder_listview->currentItem()->moveItem(buildorder_listview->currentItem()->nextSibling());
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
}


void ProjectConfigurationDlg::insideIncMoveDownClicked()
{
   if (insideinc_listview->currentItem() == 0 || insideinc_listview->currentItem()->nextSibling() == 0) {
        KNotifyClient::beep();
        return;
   }

   insideinc_listview->currentItem()->moveItem(insideinc_listview->currentItem()->nextSibling());
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
}


void ProjectConfigurationDlg::outsideIncMoveDownClicked()
{
   if (outsideinc_listview->currentItem() == 0 || outsideinc_listview->currentItem()->nextSibling() == 0) {
        KNotifyClient::beep();
        return;
   }

   outsideinc_listview->currentItem()->moveItem(outsideinc_listview->currentItem()->nextSibling());
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
}


void ProjectConfigurationDlg::outsideIncRemoveClicked()
{
    delete outsideinc_listview->currentItem();
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
}


void ProjectConfigurationDlg::insideLibMoveDownClicked()
{
   if (insidelib_listview->currentItem() == 0 || insidelib_listview->currentItem()->nextSibling() == 0) {
        KNotifyClient::beep();
        return;
   }

   insidelib_listview->currentItem()->moveItem(insidelib_listview->currentItem()->nextSibling());
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
}


void ProjectConfigurationDlg::outsideLibMoveDownClicked()
{
   if (outsidelib_listview->currentItem() == 0 || outsidelib_listview->currentItem()->nextSibling() == 0) {
        KNotifyClient::beep();
        return;
   }

   outsidelib_listview->currentItem()->moveItem(outsidelib_listview->currentItem()->nextSibling());
}


void ProjectConfigurationDlg::outsideLibAddClicked()
{
    bool ok;
    QString dir = KInputDialog::getText(i18n("Add Library"), i18n("Add library to link:"), "-l", &ok, 0);
    if (ok && !dir.isEmpty() && dir != "-I")
        new QListViewItem(outsidelib_listview, dir);
}


void ProjectConfigurationDlg::outsideLibRemoveClicked()
{
    delete outsidelib_listview->currentItem();
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
}


void ProjectConfigurationDlg::outsideLibDirMoveDownClicked()
{
   if (outsidelibdir_listview->currentItem() == 0 || outsidelibdir_listview->currentItem()->nextSibling() == 0) {
        KNotifyClient::beep();
        return;
   }

   outsidelibdir_listview->currentItem()->moveItem(outsidelibdir_listview->currentItem()->nextSibling());
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
}


void ProjectConfigurationDlg::outsideLibDirRemoveClicked()
{
    delete outsidelibdir_listview->currentItem();
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
}

void ProjectConfigurationDlg::slotInstallTargetClicked()
{
  if (checkInstallTarget->isChecked() == true)
    m_InstallTargetPath->setEnabled(true);
  else
    m_InstallTargetPath->setEnabled(false);
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
}

void ProjectConfigurationDlg::extMoveDown_button_clicked( )
{
   if (extDeps_view->currentItem() == 0 || extDeps_view->currentItem()->nextSibling() == 0) {
        KNotifyClient::beep();
        return;
   }

   extDeps_view->currentItem()->moveItem(extDeps_view->currentItem()->nextSibling());
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
}

void ProjectConfigurationDlg::extRemove_button_clicked( )
{
    delete extDeps_view->currentItem();
}

void ProjectConfigurationDlg::intMoveDown_button_clicked( )
{
   if (intDeps_view->currentItem() == 0 || intDeps_view->currentItem()->nextSibling() == 0) {
        KNotifyClient::beep();
        return;
   }

   intDeps_view->currentItem()->moveItem(intDeps_view->currentItem()->nextSibling());
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
}

 void ProjectConfigurationDlg::addCustomValueClicked()
{

}
 void ProjectConfigurationDlg::removeCustomValueClicked()
{

}
 void ProjectConfigurationDlg::editCustomValueClicked()
{

}
 void ProjectConfigurationDlg::upCustomValueClicked()
{

}
 void ProjectConfigurationDlg::downCustomValueClicked()
{

}
