/***************************************************************************
            project.cpp - the projectmanagment specific part of CKDevelop
                             -------------------                                         

    begin                : 28 Jul 1998                                        
    copyright            : (C) 1998 by Sandy Meier                         
    email                : smeier@rz.uni-potsdam.de                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/
#include <qprogressdialog.h>
#include <kcursor.h>

#include "ckdevelop.h"
#include "ctoolclass.h"
#include "ckappwizard.h"
#include "debug.h"
#include "cclassview.h"
#include "cprjoptionsdlg.h"
#include "caddexistingfiledlg.h"
#include "cfilepropdlg.h"
#include "cnewfiledlg.h"
#include "cnewclassdlg.h"
#include "cnewprojectdlg.h"
#include "caddnewtranslationdlg.h"
/*********************************************************************
 *                                                                   *
 *                              SLOTS                                *
 *                                                                   *
 ********************************************************************/ 

void CKDevelop::slotProjectNew(){
  QString old_project="";
  if(!CToolClass::searchProgram("perl")){
    return;
  }
  if(!CToolClass::searchProgram("autoconf")){
    return;
  }
  if(!CToolClass::searchProgram("automake")){
    return;
  }
  if(project){
    old_project = prj->getProjectFile();
    if(!slotProjectClose()){
      return;
    }
  }
	// Currently a project open ?
	slotStatusMsg(i18n("Creating new project..."));
			
//	CNewEmptyProject  *dlg = new CNewEmptyProject(this,"NewEmptyProject");
	CNewProjectDlg *new_dlg = new CNewProjectDlg(this,"NewEmptyProject");
/*	if(dlg->exec())
	{
		dlg->CreateNewEmptyProject();
	}
*/
  new_dlg->exec();
  QString file = new_dlg->getNewProjectFile();

  if(new_dlg->generatedNewProject()){
      readProjectFile(file);
  }
  else if (old_project != ""){ // if cancel load the old project again
    readProjectFile(old_project);
  }
	
//	delete(dlg);
	
	slotStatusMsg(IDS_DEFAULT);
}


bool CKDevelop::slotProjectClose(){
  // R.Nolden 03.02.99
  slotStatusMsg(i18n("Closing project..."));
  TEditInfo* actual_info;
  bool mod=true;
  bool headerCancel=false;
  bool cppCancel=false;
  
  log_file_tree->storeState(prj);
  
  // check if header widget contains modified file
  if(header_widget->isModified()){
    KDEBUG(KDEBUG_INFO,CKDEVELOP,"header_widget modified file");
    KMsgBox *project_close=new KMsgBox(this,i18n("Save changed project files ?"),
				       i18n("The project\n\n")+prj->getProjectName()
				       +i18n("\n\ncontains changed files. Save modified file\n\n")
				       +header_widget->getName()+" ?\n\n",KMsgBox::QUESTION,
				       i18n("Yes"), i18n("No"), i18n("Save all"), i18n("Cancel"));
    // show the messagea and store result in result:
    project_close->show();
    int result=project_close->result();
    edit_widget=header_widget;
    
    // then show the messagebox
    KDEBUG(KDEBUG_INFO,CKDEVELOP,"header msgbox result");
    // yes- save headerwidget
    if(result== 1){			 	
      if(edit_widget->getName() == "Untitled.h"){
	slotFileSaveAs();    
        slotFileClose();
      }
      else{
	slotFileSave();
        slotFileClose();
      }
      edit_widget->toggleModified(false);      
      mod=true;
    } 
    if(result==2){   // No - no save but close
      KDEBUG(KDEBUG_INFO,CKDEVELOP,"No- close header widget file");
      edit_widget->toggleModified(false);
      slotFileClose();
      mod=true;
    }
    if(result==3){  // Save all
      KDEBUG(KDEBUG_INFO,CKDEVELOP,"Save all");
      slotFileSaveAll();
      mod=true;
    }
    if(result==4){ // Cancel
      KDEBUG(KDEBUG_INFO,CKDEVELOP,"Cancel project close");
      mod=false;
      headerCancel=true;
    }
  } // end header widge close
  
  if(cpp_widget->isModified()){
    KMsgBox *project_close=new KMsgBox(this,i18n("Save changed project files ?"),
				       i18n("The project\n\n")+prj->getProjectName()
				       +i18n("\n\ncontains changed files. Save modified file\n\n")+
				       cpp_widget->getName()+" ?\n\n",KMsgBox::QUESTION,
				       i18n("Yes"), i18n("No"), i18n("Save all"), i18n("Cancel"));
    // show the messagea and store result in result:
    project_close->show();
    int result=project_close->result();
    
    KDEBUG(KDEBUG_INFO,CKDEVELOP,"cpp_widget modified file");
    KDEBUG(KDEBUG_INFO,CKDEVELOP,"cpp msgbox result");
    // yes- save cpp widget
    edit_widget=cpp_widget;
    if(result== 1){			 	
      if(edit_widget->getName() == "Untitled.cpp"){
	slotFileSaveAs();    
        slotFileClose();
      }
      else{
	slotFileSave();
        slotFileClose();
      }
      edit_widget->toggleModified(false);      
      mod=true;
    }  
    if(result==2){   // No - no save but close
      KDEBUG(KDEBUG_INFO,CKDEVELOP,"No- close header widget file");
      edit_widget->toggleModified(false);
      slotFileClose();
      mod=true;
    }
    if(result==3){  // Save all
      KDEBUG(KDEBUG_INFO,CKDEVELOP,"Save all");
      slotFileSaveAll();
      mod=true;
    }
    if(result==4){ // Cancel
      KDEBUG(KDEBUG_INFO,CKDEVELOP,"Cancel project close");
      cppCancel=true;
      mod=false;
    }
    
  }  // end cppwidget close
  
  if(!headerCancel && !cppCancel){
    // for -loop for actual infos
    for(actual_info=edit_infos.first();actual_info != 0;actual_info=edit_infos.next()){
      KDEBUG1(KDEBUG_INFO,CKDEVELOP,"check file: %s",actual_info->filename.data());
      if(actual_info->modified){
	
	KMsgBox *project_close=new KMsgBox(this,i18n("Save changed project files ?"),
					   i18n("The project\n\n")+prj->getProjectName()
					   +i18n("\n\ncontains changed files. Save modified file\n\n")
					   +actual_info->filename+" ?\n\n",KMsgBox::QUESTION,
					   i18n("Yes"), i18n("No"), i18n("Save all"), i18n("Cancel"));
 				// show the messagea and store result in result:
	project_close->show();
    	int result=project_close->result();
	
	KDEBUG(KDEBUG_INFO,CKDEVELOP,"Msgbox");
	// create the save project messagebox
	
	// what to do
	if(result==1){  // Yes- only save the actual file
				// save file as if Untitled and close file
	  if((actual_info->filename == "Untitled.cpp") || (actual_info->filename == "Untitled.h")){
	    KDEBUG(KDEBUG_INFO,CKDEVELOP,"yes- untitled");
	    switchToFile(actual_info->filename);
	    slotFileSaveAs();
	    slotFileClose();
	  }
				// Save file and close it
	  else{
	    KDEBUG(KDEBUG_INFO,CKDEVELOP,"yes- save");
	    switchToFile(actual_info->filename);
	    slotFileSave();
	    slotFileClose();
	    //					if(edit_infos.removeRef(actual_info));
	  }
	  mod = true;
	} 
	
	if(result==2){   // No - no save but close
	  KDEBUG(KDEBUG_INFO,CKDEVELOP,"No- close file");
	  actual_info->modified=false;
	  slotFileClose();
	  mod=true;
	}
	if(result==3){  // Save all
	  KDEBUG(KDEBUG_INFO,CKDEVELOP,"Save all");
	  slotFileSaveAll();
	  mod=true;
	  break;
	}
	if(result==4){ // Cancel
	  mod=false;
	  KDEBUG(KDEBUG_INFO,CKDEVELOP,"Cancel project close");
	  break;
	}
	
      }  // end actual file close
    } // end for-loop
  } // end the if cppCancel && headerCancel
  
  if(mod){
    // not cancel pressed - project closed
    // clear all widgets
    header_widget->clear();
    cpp_widget->clear();
    class_tree->clear();
    log_file_tree->clear();
    real_file_tree->clear();
    menu_buffers->clear();
    messages_widget->clear();
    stdin_stdout_widget->clear();
    stderr_widget->clear();
    
    toolBar(ID_BROWSER_TOOLBAR)->clearCombo(TOOLBAR_CLASS_CHOICE);
    toolBar(ID_BROWSER_TOOLBAR)->clearCombo(TOOLBAR_METHOD_CHOICE);
    
    // re-inititalize the edit widgets
    header_widget->setName("Untitled.h");
    cpp_widget->setName("Untitled.cpp");
    TEditInfo* edit1 = new TEditInfo;
    TEditInfo* edit2 = new TEditInfo;
    edit1->filename = header_widget->getName();
    edit2->filename = cpp_widget->getName();
    
    edit1->id = menu_buffers->insertItem(edit1->filename,-2,0);
    edit1->modified=false;
    edit2->id = menu_buffers->insertItem(edit2->filename,-2,0);
    edit2->modified=false;
    edit_infos.append(edit1);
    edit_infos.append(edit2);
    
    // set project to false and disable all ID_s related to project=true	
    prj->writeProject();
    project=false;
    prj->valid = false;
    delete prj;
    prj = 0;
    
    switchToFile(header_widget->getName());
    
    disableCommand(ID_FILE_NEW);
    disableCommand(ID_KDLG_FILE_NEW);
    disableCommand(ID_FILE_PRINT);
    // doc menu
    disableCommand(ID_HELP_PROJECT_API);
    disableCommand(ID_HELP_USER_MANUAL);
    // build menu
    setToolMenuProcess(false);  
    disableCommand(ID_BUILD_STOP);
    disableCommand(ID_BUILD_AUTOCONF);
    
    // prj menu
    disableCommand(ID_PROJECT_CLOSE);
    disableCommand(ID_PROJECT_ADD_FILE);
    disableCommand(ID_PROJECT_ADD_FILE_NEW);
    disableCommand(ID_PROJECT_ADD_FILE_EXIST);
    disableCommand(ID_PROJECT_ADD_NEW_TRANSLATION_FILE);
    disableCommand(ID_PROJECT_REMOVE_FILE);
    disableCommand(ID_PROJECT_NEW_CLASS);
    disableCommand(ID_PROJECT_FILE_PROPERTIES);
    disableCommand(ID_PROJECT_OPTIONS);
    disableCommand(ID_BUILD_MAKE_DISTRIBUTION);
    
  }
  slotStatusMsg(IDS_DEFAULT);
  if(mod){
    refreshTrees();
  }
  
  return mod; // false if pressed cancel
}

void CKDevelop::slotProjectAddNewFile(){
  newFile(true);
}

void CKDevelop::slotAddExistingFiles(){
  bool copy = false;
  ProjectFileType type;
  bool new_subdir=false; // if a new subdir was added to the project, we must do a rebuildmakefiles
  QString token;
  QStrList files;
  QString str_files = add_dlg->source_edit->text(); 
  StringTokenizer str_token;
    
  str_token.tokenize(str_files,",");
  while(str_token.hasMoreTokens()){
    token = str_token.nextToken();
    files.append(token);
  }
  QString dest = add_dlg->destination_edit->text();
  if(dest.right(1) != "/"){ // I hope it works now -Sandy
    dest = dest + "/";
  }
  QString source_name;
  QString dest_name ;
  QString file;
  QFileInfo file_info;
  int i=files.count();
    
  QProgressDialog progress( i18n("Copying files..."),0, i, this,"",true );
  progress.setCaption(i18n("please wait..."));
  progress.show();
    
  i=0;
  progress.setProgress( i);

  for(file = files.first(); file !=0;file = files.next()){
    i++;
    copy = false;
    progress.setProgress( i );
    file_info.setFile(file);
    source_name = file_info.fileName();
    dest_name = dest + source_name;
   
    // Fetch the type of the file
    type = CProject::getType( dest_name );
      
    if(QFile::exists(dest_name)){
      int result=KMsgBox::yesNoCancel(this,i18n("Files exists!"),
                                      i18n("\nThe file\n\n"+source_name+"\n\nalready exists.\nDo you want overwrite the old one?\n"));
      if(result==1)
        copy = true;
      if(result==2)
        copy = false;
      if(result==3){
        setCursor( KCursor::arrowCursor() );
        break;;
      }
    }
    else {
      copy = true;
    }
      
    if(copy){
      process.clearArguments();
      process << "cp"; // copy is your friend :-)
      process << file;
      process << dest;
      process.start(KProcess::Block,KProcess::AllOutput); // blocked because it is important  
    }
    new_subdir = addFileToProject(dest_name,type,false) || new_subdir; // no refresh
  }
  progress.setProgress( files.count() );
  switchToFile(dest_name);
  refreshTrees();
    
  if(new_subdir){
    newSubDir();
  }

  delete add_dlg;
}

void CKDevelop::slotProjectAddExistingFiles(){
  add_dlg = new CAddExistingFileDlg(this,"test",prj);

  add_dlg->destination_edit->setText(prj->getProjectDir()+ prj->getSubDir());

  if(add_dlg->exec()){
    QTimer::singleShot(100,this,SLOT(slotAddExistingFiles()));
  }
}

void CKDevelop::slotProjectRemoveFile(){
    QString name = log_file_tree->currentItem()->text(0);
    delFileFromProject(name);
}

void CKDevelop::slotProjectOptions(){
  CPrjOptionsDlg prjdlg(this,"optdialog",prj);
  prjdlg.show();
  if (prjdlg.needConfigureInUpdate()){
    prj->updateConfigureIn();
    KMsgBox::message(0,i18n("Information"),i18n("You have modified the projectversion.\nWe will regenerate all Makefiles now."),KMsgBox::INFORMATION);
    setToolMenuProcess(false);
    slotStatusMsg(i18n("Running automake/autoconf and configure..."));
    messages_widget->clear();
    showOutputView(true);
    QDir::setCurrent(prj->getProjectDir());
    shell_process.clearArguments();
    shell_process << make_cmd << " -f Makefile.dist  && ./configure";
    shell_process.start(KProcess::NotifyOnExit,KProcess::AllOutput);
  }
}
void CKDevelop::slotProjectNewClass(){
  CNewClassDlg* dlg = new CNewClassDlg(this,"newclass",prj);
  if(dlg->exec()){
    QString source_file=dlg->getImplFile() ;
    QString header_file=dlg->getHeaderFile();
    switchToFile(source_file);
    switchToFile(header_file);

    QFileInfo header_info(header_file);
    QFileInfo source_info(source_file);
    TFileInfo file_info;
    file_info.rel_name = prj->getSubDir() + source_info.fileName();
    file_info.type = CPP_SOURCE;
    file_info.dist = true;
    file_info.install = false;
    prj->addFileToProject(prj->getSubDir() + source_info.fileName(),file_info);
    
    file_info.rel_name = prj->getSubDir() + header_info.fileName();
    file_info.type = CPP_HEADER;
    file_info.dist = true;
    file_info.install = false;
    prj->addFileToProject(prj->getSubDir() + header_info.fileName(),file_info);
   
    prj->updateMakefilesAm();
    
    slotViewRefresh();
  }
}

void CKDevelop::slotProjectFileProperties(){
  CFilePropDlg dlg(this,"DLG",prj);
  dlg.show();
}

void CKDevelop::slotShowFileProperties(QString rel_name){
  CFilePropDlg dlg(this,"DLG",prj,rel_name);
  dlg.show();
}

void CKDevelop::slotProjectOpen(){
  QString old_project = "";

  if(project){
    old_project = prj->getProjectFile();
    if(!slotProjectClose()){
			slotViewRefresh();
      return;
    }
  }
  slotStatusMsg(i18n("Opening project..."));
  QString str;
  str = KFileDialog::getOpenFileName(0,"*.kdevprj",this);
  if (str.isEmpty() && old_project != ""){
    readProjectFile(old_project);
		slotViewRefresh();
    return; //cancel
  }
 
 
  QFileInfo info(str);
  
  if (info.isFile()){
    if(!(readProjectFile(str))){

    KMsgBox::message(0,str,"This is a Project-File from KDevelop 0.1\nSorry,but it's incompatible with KDevelop >= 0.2.\nPlease use only new generated projects!");
    readProjectFile(old_project);
		slotViewRefresh();
    }
  	else
			slotViewRefresh();

    slotStatusMsg(IDS_DEFAULT);
  }	
  
  
}

void CKDevelop::slotProjectOpenCmdl(const char* prjname){

  QString old_project = "";

  if(project){
    old_project = prj->getProjectFile();
    if(!slotProjectClose()){
      return;
    }
  }

  QFileInfo info(prjname);

  if (info.isFile()){
    if(!(readProjectFile(prjname))){

    KMsgBox::message(0,prjname,"This is a Project-File from KDevelop 0.1\nSorry,but it's incompatible with KDevelop >= 0.2.\nPlease use only new generated projects!");
    readProjectFile(old_project);
    slotViewRefresh();
    }

    slotStatusMsg(IDS_DEFAULT);
  }	
}

void CKDevelop::slotProjectNewAppl(){
  QString old_project="";
  if(!CToolClass::searchProgram("perl")){
    return;
  }
  if(!CToolClass::searchProgram("autoconf")){
    return;
  }
  if(!CToolClass::searchProgram("automake")){
    return;
  }
  if(project){
    old_project = prj->getProjectFile();
    if(!slotProjectClose()){
      return;
    }
  }
  
  slotStatusMsg(i18n("Creating a new frame application..."));
  CKAppWizard* kappw  = new CKAppWizard (this,"zutuz");
  kappw->setCaption("ApplicationWizard");
  kappw->exec();
  QString file = kappw->getProjectFile();
  
  if(kappw->generatedProject()){
    readProjectFile(file);
    if (prj->getProjectType() == "normal_kde" || prj->getProjectType() == "mini_kde") {
      slotBuildMessages();
    }
  }
  else if (old_project != ""){ // if cancel load the old project again
    readProjectFile(old_project);
  }
  slotViewRefresh();
  //cerr << kappw->getProjectFile();
  slotStatusMsg(IDS_DEFAULT); 
}

void  CKDevelop::slotProjectWorkspaces(int id){
  if(project_menu->isItemChecked(id)){
    return; // we are already in this workspace
  }
  saveCurrentWorkspaceIntoProject();
  
  // and now the new workspace
  switch(id){
  case ID_PROJECT_WORKSPACES_1:
    switchToWorkspace(1);
    break;
  case ID_PROJECT_WORKSPACES_2:
    switchToWorkspace(2);
    break;
  case ID_PROJECT_WORKSPACES_3:
    switchToWorkspace(3);
    break;
  }
   
}

void CKDevelop::slotProjectAddNewTranslationFile(){
  CAddNewTranslationDlg dlg(this,0,prj);
  QString file;
  if (dlg.exec()){
    file = dlg.getLangFile();
    file = prj->getProjectDir() + "po/" + file;
    QFile nfile(file); // create a empty file
    nfile.open(IO_WriteOnly);
    nfile.close();
    addFileToProject(file, PO); 
    slotBuildMessages();
  }
}
void CKDevelop::slotAddFileToProject(QString abs_filename){
  ProjectFileType type = CProject::getType( abs_filename );

  addFileToProject(abs_filename, type, true);
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC METHODS                           *
 *                                                                   *
 ********************************************************************/ 

/*---------------------------------------------- CKDevelop::newFile()
 * newFile()
 *   Create a new file and add it to the project.
 *
 * Parameters:
 *   add_to_project      Should the file be added to the project?
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::newFile(bool add_to_project){
  ProjectFileType type;
  bool new_subdir=false;
  QString complete_filename;
  CNewFileDlg dlg(this,"test",true,0,prj);

  dlg.setUseTemplate();
  if (add_to_project){
    dlg.setAddToProject();
  }
  if(!dlg.exec()) return; // cancel
  
  complete_filename = dlg.location() + dlg.fileName();
  
  // load into the widget
  switchToFile(complete_filename);
  
  // Get the filetype.
  type = CProject::getType( complete_filename );
  
  // add the file to the project if necessary
  if (dlg.addToProject() == true){
    new_subdir = addFileToProject(complete_filename,type);
  }
  if(new_subdir){
    newSubDir();
  }
  
}

/*-------------------------------------- CKDevelop::addFileToProject()
 * addFileToProject()
 *   Add a file with a specified type to the project.
 *
 * Parameters:
 *   complete_filename   The absolute filename.
 *   type                Type of file.
 *   refresh             If to refresh the trees.
 *
 * Returns:
 *   true                If a new subdir was added.
 *-----------------------------------------------------------------*/
bool CKDevelop::addFileToProject(QString complete_filename,
                                 ProjectFileType type,
                                 bool refresh)
{
  bool new_subdir = false;
  QString rel_name = complete_filename;
  
  // normalize it a little bit
  rel_name.replace(QRegExp("///"),"/"); // remove ///
  rel_name.replace(QRegExp("//"),"/"); // remove //
		   
  rel_name.replace(QRegExp(prj->getProjectDir()),"");
  
  TFileInfo info;
  info.rel_name = rel_name;
  info.type = type;
  info.dist = ( type != PO );

  info.install=false;
  info.install_location = "";
  new_subdir = prj->addFileToProject(rel_name,info);
  
  prj->writeProject();
  prj->updateMakefilesAm();

  if(refresh)
    refreshTrees(&info);

  return new_subdir;
}

void CKDevelop::delFileFromProject(QString rel_filename){

  prj->removeFileFromProject(rel_filename);
  prj->writeProject();
  refreshTrees();
}

bool CKDevelop::readProjectFile(QString file){
  QString str;
  prj = new CProject(file);
  if(!(prj->readProject())){
    return false;
  }

  // str = prj.getProjectDir() + prj.getSubDir() + prj.getProjectName().lower() + ".cpp";
  //   if(QFile::exists(str)){
  //     switchToFile(str);
  //   }
  str = prj->getProjectDir() + prj->getSubDir() + prj->getProjectName().lower() + ".h";
  if(QFile::exists(str)){
    switchToFile(str);
  }
  str = prj->getProjectDir() + prj->getSubDir() + "main.cpp";
  if(QFile::exists(str)){
    switchToFile(str);
  }

// TODO: Add function to read last opened files from project to restore project workspace

  switchToWorkspace(prj->getCurrentWorkspaceNumber());
  // set the menus enable
  // file menu
  
  enableCommand(ID_FILE_NEW);
  enableCommand(ID_KDLG_FILE_NEW);
  enableCommand(ID_FILE_PRINT);
  // doc menu
  enableCommand(ID_HELP_PROJECT_API);
  enableCommand(ID_HELP_USER_MANUAL);
  // build menu
  setToolMenuProcess(true);

  // prj menu
  enableCommand(ID_PROJECT_CLOSE);
  enableCommand(ID_PROJECT_ADD_FILE);
  enableCommand(ID_PROJECT_ADD_FILE_NEW);
  enableCommand(ID_PROJECT_ADD_FILE_EXIST);

  if (prj->getProjectType() != "normal_kde" && prj->getProjectType() != "mini_kde"){
    disableCommand(ID_PROJECT_ADD_NEW_TRANSLATION_FILE);
  }
  else{
    enableCommand(ID_PROJECT_ADD_NEW_TRANSLATION_FILE);
  }
  if(prj->getProjectType() != "normal_empty"){
    enableCommand(ID_PROJECT_FILE_PROPERTIES);
    enableCommand(ID_PROJECT_OPTIONS);
  }
  else{
    disableCommand(ID_PROJECT_FILE_PROPERTIES);
    disableCommand(ID_PROJECT_OPTIONS);
  }
  

  enableCommand(ID_PROJECT_REMOVE_FILE);
  enableCommand(ID_PROJECT_NEW_CLASS);
  
  enableCommand(ID_PROJECT_WORKSPACES);

  enableCommand(ID_BUILD_AUTOCONF);
  enableCommand(ID_BUILD_MAKE_DISTRIBUTION);
	
  project=true;
  return true;
}




void  CKDevelop::saveCurrentWorkspaceIntoProject(){
  TWorkspace current;
  TEditInfo* actual_info;

  // save the current workspace
  current.id = workspace;
  for(actual_info=edit_infos.first();actual_info != 0;actual_info=edit_infos.next()){
    current.openfiles.append(actual_info->filename);
    debug(actual_info->filename);
  }
  current.openfiles.removeRef("Untitled.h");
  current.openfiles.removeRef("Untitled.cpp");
  current.header_file = header_widget->getName();
  current.cpp_file = cpp_widget->getName();
  current.browser_file =history_list.current();
  current.show_treeview =view_menu->isItemChecked(ID_VIEW_TREEVIEW);
  current.show_output_view = view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW);

  prj->writeWorkspace(current);
}

void CKDevelop::newSubDir(){
  if(prj->getProjectType() == "normal_empty"){
    return; // no makefile handling
  }
  KMsgBox::message(0,i18n("Information"),i18n("You have added a new subdir to the project.\nWe will regenerate all Makefiles now."),KMsgBox::INFORMATION);
  setToolMenuProcess(false);
  slotStatusMsg(i18n("Running automake/autoconf and configure..."));
  messages_widget->clear();
  showOutputView(true);
  QDir::setCurrent(prj->getProjectDir());
  shell_process.clearArguments();
  shell_process << make_cmd << " -f Makefile.dist  && ./configure";
  shell_process.start(KProcess::NotifyOnExit,KProcess::AllOutput);
}































































































