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
#include "ckdevelop.h"


void CKDevelop::slotProjectNew(){
	// Currently a project open ?
  if(project)
			slotProjectClose();      // close current project

/* TODO:
  add a dialog to create empty project file, just set
  the project name, directory and project Type (application (KDE; QT; X11; Terminal; Library)).
  Then create directory and project file
  - then call slotProjectAddExistingFiles to add all files needed by the new app.
  Makefile.am - don't know yet how to create it for the project Type. Maybe Prototypes ?
  Ralf */
			

}


bool CKDevelop::slotProjectClose(){
  // R.Nolden 03.02.99
  slotStatusMsg(i18n("Closing project..."));
  TEditInfo* actual_info;
  bool mod=true;
  bool headerCancel=false;
  bool cppCancel=false;
  
  
  // check if header widget contains modified file
  if(header_widget->isModified()){
    cerr << "header_widget modified file" << endl;
    KMsgBox *project_close=new KMsgBox(this,i18n("Save changed project files ?"),i18n("The project\n\n")+prj->getProjectName()
				       +i18n("\n\ncontains changed files. Save modified file\n\n")+header_widget->getName()+" ?\n\n",KMsgBox::QUESTION,
				       i18n("Yes"), i18n("No"), i18n("Save all"), i18n("Cancel"));
    // show the messagea and store result in result:
    project_close->show();
    int result=project_close->result();
    edit_widget=header_widget;
    
    // then show the messagebox
    cerr << "header msgbox result"<<endl;
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
      cerr << "No- close header widget file" << endl;
      edit_widget->toggleModified(false);
      slotFileClose();
      mod=true;
    }
    if(result==3){  // Save all
      cerr << "Save all" <<endl;
      slotFileSaveAll();
      mod=true;
    }
    if(result==4){ // Cancel
      cerr << "Cancel project close" <<endl;
      mod=false;
      headerCancel=true;
    }
  } // end header widge close
  
  if(cpp_widget->isModified()){
    KMsgBox *project_close=new KMsgBox(this,i18n("Save changed project files ?"),i18n("The project\n\n")+prj->getProjectName()
				       +i18n("\n\ncontains changed files. Save modified file\n\n")+cpp_widget->getName()+" ?\n\n",KMsgBox::QUESTION,
				       i18n("Yes"), i18n("No"), i18n("Save all"), i18n("Cancel"));
    // show the messagea and store result in result:
    project_close->show();
    int result=project_close->result();
    
    cerr <<"cpp_widget modified file"<<endl;
    cerr << "cpp msgbox result"<<endl;
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
      cerr << "No- close header widget file" << endl;
      edit_widget->toggleModified(false);
      slotFileClose();
      mod=true;
    }
    if(result==3){  // Save all
      cerr << "Save all" <<endl;
      slotFileSaveAll();
      mod=true;
    }
    if(result==4){ // Cancel
			cerr << "Cancel project close" <<endl;
			cppCancel=true;
			mod=false;
		}
    
  }  // end cppwidget close
  
  if(!headerCancel && !cppCancel){
    // for -loop for actual infos
    for(actual_info=edit_infos.first();actual_info != 0;actual_info=edit_infos.next()){
      cerr << "check file:" << actual_info->filename << endl;
      if(actual_info->modified){
	
	KMsgBox *project_close=new KMsgBox(this,i18n("Save changed project files ?"),i18n("The project\n\n")+prj->getProjectName()
					   +i18n("\n\ncontains changed files. Save modified file\n\n")+actual_info->filename+" ?\n\n",KMsgBox::QUESTION,
					   i18n("Yes"), i18n("No"), i18n("Save all"), i18n("Cancel"));
 				// show the messagea and store result in result:
	project_close->show();
    	int result=project_close->result();
	
	cerr << "Msgbox" << endl;
	// create the save project messagebox
	
	// what to do
	if(result==1){  // Yes- only save the actual file
				// save file as if Untitled and close file
	  if((actual_info->filename == "Untitled.cpp") || (actual_info->filename == "Untitled.h")){
	    cerr << "yes- untitled" << endl;
	    switchToFile(actual_info->filename);
	    slotFileSaveAs();
	    slotFileClose();
	  }
				// Save file and close it
	  else{
	    cerr << "yes- save" << endl;
	    switchToFile(actual_info->filename);
	    slotFileSave();
	    slotFileClose();
	    //					if(edit_infos.removeRef(actual_info));
	  }
	  mod = true;
	} 
	
	if(result==2){   // No - no save but close
	  cerr << "No- close file" << endl;
	  actual_info->modified=false;
	  slotFileClose();
	  mod=true;
	}
	if(result==3){  // Save all
	  cerr << "Save all" <<endl;
	  slotFileSaveAll();
	  mod=true;
	  break;
	}
	if(result==4){ // Cancel
	  mod=false;
	  cerr << "Cancel project close" <<endl;
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
    project=false;
    prj->valid = false;
    delete prj;

    
    switchToFile(header_widget->getName());
    
    disableCommand(ID_FILE_NEW);
    // doc menu
    disableCommand(ID_DOC_PROJECT_API_DOC);
    disableCommand(ID_DOC_USER_MANUAL);
    // build menu
    setToolMenuProcess(false);  
    disableCommand(ID_BUILD_STOP);
    disableCommand(ID_BUILD_AUTOCONF);
    
    // prj menu
    disableCommand(ID_PROJECT_CLOSE);
    disableCommand(ID_PROJECT_ADD_FILE);
    disableCommand(ID_PROJECT_ADD_FILE_NEW);
    disableCommand(ID_PROJECT_ADD_FILE_EXIST);
    disableCommand(ID_PROJECT_REMOVE_FILE);
    disableCommand(ID_PROJECT_NEW_CLASS);
    disableCommand(ID_PROJECT_FILE_PROPERTIES);
    disableCommand(ID_PROJECT_OPTIONS);
    
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

void CKDevelop::slotProjectAddExistingFiles(){
  QString type;
  CAddExistingFileDlg dlg(this,"test",prj);
  
  dlg.destination_edit->setText(prj->getProjectDir()+ prj->getSubDir());
  if(dlg.exec()){
    QString token;
    QStrList files;
    QString str_files = dlg.source_edit->text(); 
    StringTokenizer str_token;
    
    str_token.tokenize(str_files,",");
    while(str_token.hasMoreTokens()){
      token = str_token.nextToken();
      files.append(token);
    }
    QString dest = dlg.destination_edit->text();
    if(dest.right(1) != '/'){
      dest = dest + '/';
    }
    QString source_name;
    QString dest_name ;
    QString file;
    QFileInfo file_info;
    for(file = files.first(); file !=0;file = files.next()){
      file_info.setFile(file);
      source_name = file_info.fileName();
      dest_name = dest + source_name;
      
      type = "DATA";
      if (dest_name.right(2) == ".h"){
      type = "HEADER";
      }
      if (getTabLocation(dest_name) == CPP){
	type = "SOURCE";
      }
         
      // if not copy the file to the correct location 
      process.clearArguments();
      process << "cp"; // copy is your friend :-)
      process << file;
      process << dest;
      process.start(KProcess::Block,KProcess::AllOutput); // blocked because it is important  
      
      addFileToProject(dest_name,type,false); // no refresh
    }
    switchToFile(dest_name);
    refreshTrees();
  }
}

void CKDevelop::slotProjectRemoveFile(){
  QString name = log_file_tree->getCurrentItem()->getText();
  delFileFromProject(name);
}

void CKDevelop::slotProjectOptions(){
  CPrjOptionsDlg prjdlg(this,"optdialog",prj);
  prjdlg.show();
}

void CKDevelop::newFile(bool add_to_project){
  CNewFileDlg dlg(this,"test",true,0,prj);
  dlg.setUseTemplate();
  if (add_to_project){
    dlg.setAddToProject();
  }
  if(!dlg.exec()) return; // cancel
  
  QString filename = dlg.fileName();
  QString complete_filename;
  complete_filename = dlg.location() + filename;
  
  // load into the widget
  switchToFile(complete_filename);
  
  QString type = "DATA";
  QString filetype = dlg.fileType();
  if(filetype == "HEADER"){
    type = "HEADER";
  }
  if(filetype == "CPP"){
    type = "SOURCE";
  }
  
  // add the file to the project if necessary
  if (dlg.addToProject() == true){
    addFileToProject(complete_filename,type);
  }
  
}
void CKDevelop::addFileToProject(QString complete_filename,QString type,bool refresh){
  QString rel_name = complete_filename;
  rel_name.replace(QRegExp(prj->getProjectDir()),"");
  prj->addFileToProject(rel_name);
  TFileInfo info;
  info.rel_name = rel_name;
  info.type = type;
  info.dist = true;
  info.install=false;
  info.install_location = "";
  prj->writeFileInfo(info);
  prj->writeProject();
  prj->updateMakefilesAm();
  if(refresh){
    refreshTrees();
  }
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

  // set the menus enable
  // file menu
  
  enableCommand(ID_FILE_NEW);
  // doc menu
  enableCommand(ID_DOC_PROJECT_API_DOC);
  enableCommand(ID_DOC_USER_MANUAL);
  // build menu
  setToolMenuProcess(true);

  // prj menu
  enableCommand(ID_PROJECT_CLOSE);
  enableCommand(ID_PROJECT_ADD_FILE);
  enableCommand(ID_PROJECT_ADD_FILE_NEW);
  enableCommand(ID_PROJECT_ADD_FILE_EXIST);
  enableCommand(ID_PROJECT_REMOVE_FILE);
  enableCommand(ID_PROJECT_NEW_CLASS);
  enableCommand(ID_PROJECT_FILE_PROPERTIES);
  enableCommand(ID_PROJECT_OPTIONS);

  enableCommand(ID_BUILD_AUTOCONF);
  project=true;
  slotOptionsRefresh();
  return true;
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
    prj->addFileToProject(prj->getSubDir() + source_info.fileName());
    TFileInfo file_info;
    file_info.rel_name = prj->getSubDir() + source_info.fileName();
    file_info.type = "SOURCE";
    file_info.dist = true;
    file_info.install = false;
    prj->writeFileInfo(file_info);
    
    prj->addFileToProject(prj->getSubDir() + header_info.fileName());
    file_info.rel_name = prj->getSubDir() + header_info.fileName();
    file_info.type = "HEADER";
    file_info.dist = true;
    file_info.install = false;
    prj->writeFileInfo(file_info);
    
    prj->updateMakefilesAm();
    slotOptionsRefresh();
  }
}

void CKDevelop::slotProjectFileProperties(){
  CFilePropDlg dlg(this,"DLG",prj);
  dlg.show();
}




















































































