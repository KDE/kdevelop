/***************************************************************************
            project.cpp - the projectmanagment specific part of CKDevelop
                             -------------------                                         

    version              :                                   
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
#include <iostream.h>
#include "cprjoptionsdlg.h"
#include "cnewclassdlg.h"
#include "cfilepropdlg.h"
#include "caddexistingfiledlg.h"

void CKDevelop::slotProjectAddNewFile(){
  newFile(true);
  
}

void CKDevelop::slotProjectAddExistingFile(){
  QString type;
  CAddExistingFileDlg dlg(this,"test",&prj);
  
  dlg.destination_edit->setText(prj.getProjectDir()+ prj.getSubDir());
  if(dlg.exec()){
    QFileInfo file_info(dlg.source_edit->text());
    QString source_name = file_info.fileName();
    QString dest_name = dlg.destination_edit->text() + source_name;
    
    type = "DATA";
    if (dest_name.right(2) == ".h"){
      type = "HEADER";
    }
    if (getTabLocation(dest_name) == CPP){
      type = "SOURCE";
    }
    
    
    
    // if not copy the file to the correct location 
    process.clearArguments();
    process << "cp"; // copy iq your friend :-)
    process << dlg.source_edit->text();
    process << dlg.destination_edit->text();
    process.start(KProcess::Block,KProcess::AllOutput); // blocked because it is important  
    
    addFileToProject(dest_name,type);
    switchToFile(dest_name);
  }
  
  
}

void CKDevelop::slotProjectRemoveFile(){
  QString name = log_file_tree->getCurrentItem()->getText();
  delFileFromProject(name);
}

void CKDevelop::slotProjectOptions(){
  CPrjOptionsDlg prj(this,"optdialog",&prj);
  prj.show();
}

void CKDevelop::newFile(bool add_to_project){
  CNewFileDlg dlg(this,"test",true,0,&prj);
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
void CKDevelop::addFileToProject(QString complete_filename,QString type){
  QString rel_name = complete_filename;
  rel_name.replace(QRegExp(prj.getProjectDir()),"");
  prj.addFileToProject(rel_name);
  TFileInfo info;
  info.rel_name = rel_name;
  info.type = type;
  info.dist = true;
  info.install=false;
  info.install_location = "";
  prj.writeFileInfo(info);
  prj.writeProject();
  refreshTrees();
}
void CKDevelop::delFileFromProject(QString rel_filename){

  prj.removeFileFromProject(rel_filename);
  prj.writeProject();
  refreshTrees();
}
bool CKDevelop::readProjectFile(QString file){
  QString str;
  if(!(prj.readProject(file))){
    return false;
  }
  
  str = prj.getProjectDir() + prj.getSubDir() + prj.getProjectName().lower() + ".cpp";
  if(QFile::exists(str)){
    switchToFile(str);
  }
  str = prj.getProjectDir() + prj.getSubDir() + prj.getProjectName().lower() + ".h";
  if(QFile::exists(str)){
    switchToFile(str);
  }
  // set the menus enable
  // file menu

  enableCommand(ID_FILE_NEW_FILE);
  // doc menu
  enableCommand(ID_DOC_PROJECT_API_DOC);
  enableCommand(ID_DOC_USER_MANUAL);
  // build menu
  setToolMenuProcess(true);  
  // prj menu
  enableCommand(ID_PROJECT_ADD_FILE);
  enableCommand(ID_PROJECT_ADD_FILE_NEW);
  enableCommand(ID_PROJECT_ADD_FILE_EXIST);
  enableCommand(ID_PROJECT_REMOVE_FILE);
  enableCommand(ID_PROJECT_NEW_CLASS);
  enableCommand(ID_PROJECT_FILE_PROPERTIES);
  enableCommand(ID_PROJECT_OPTIONS);
  
  project=true;
  slotOptionsRefresh();
  return true;
}


void CKDevelop::slotProjectNewClass(){
  CNewClassDlg* dlg = new CNewClassDlg(this,"newclass",&prj);
  if(dlg->exec()){
    QString source_file=dlg->getImplFile() ;
    QString header_file=dlg->getHeaderFile();
    switchToFile(source_file);
    switchToFile(header_file);

    QFileInfo header_info(header_file);
    QFileInfo source_info(source_file);
    prj.addFileToProject(prj.getSubDir() + source_info.fileName());
    TFileInfo file_info;
    file_info.rel_name = prj.getSubDir() + source_info.fileName();
    file_info.type = "SOURCE";
    file_info.dist = true;
    file_info.install = false;
    prj.writeFileInfo(file_info);
    
    prj.addFileToProject(prj.getSubDir() + header_info.fileName());
    file_info.rel_name = prj.getSubDir() + header_info.fileName();
    file_info.type = "HEADER";
    file_info.dist = true;
    file_info.install = false;
    prj.writeFileInfo(file_info);
    
    prj.createMakefilesAm();
    slotOptionsRefresh();
  }
}

void CKDevelop::slotProjectFileProperties(){
  CFilePropDlg dlg(this,"DLG",&prj);
  dlg.show();
}

