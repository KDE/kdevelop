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
#include "crealfileview.h"
#include "cprjoptionsdlg.h"
#include "caddexistingfiledlg.h"
#include "cfilepropdlg.h"
#include "cnewfiledlg.h"
#include "cnewclassdlg.h"
#include "caddnewtranslationdlg.h"
#include "cerrormessageparser.h"
#include "./kdlgedit/kdlgeditwidget.h"
#include "./kdlgedit/kdlgpropwidget.h"
#include "./kdlgedit/kdlgdialogs.h"
#include "./kdlgedit/kdlgedit.h"
#include "cmakemanualdlg.h"
#include "cgeneratenewfile.h"
/*********************************************************************
 *                                                                   *
 *                              SLOTS                                *
 *                                                                   *
 ********************************************************************/ 

bool CKDevelop::slotProjectClose(){
  // R.Nolden 03.02.99
  slotStatusMsg(i18n("Closing project..."));
  TEditInfo* actual_info;
  QStrList handledNames;
  bool cont=true;

  log_file_tree->storeState(prj);

  setInfoModified(header_widget->getName(), header_widget->isModified());
  setInfoModified(cpp_widget->getName(), cpp_widget->isModified());

  for(actual_info=edit_infos.first();cont && actual_info != 0;){
//      KDEBUG1(KDEBUG_INFO,CKDEVELOP,"check file: %s",actual_info->filename.data());
      TEditInfo *next_info=edit_infos.next();
      if(actual_info->modified && handledNames.contains(actual_info->filename)<1){
	
	KMsgBox *project_close=new KMsgBox(this,i18n("Save changed project files ?"),
					   i18n("The project\n\n")+prj->getProjectName()
					   +i18n("\n\ncontains changed files. Save modified file\n\n")
					   +actual_info->filename+" ?\n\n",KMsgBox::QUESTION,
					   i18n("Yes"), i18n("No"), i18n("Save all"), i18n("Cancel"));
 				// show the messagea and store result in result:
	project_close->show();
    	int result=project_close->result();
	
//	KDEBUG(KDEBUG_INFO,CKDEVELOP,"Msgbox");
	// create the save project messagebox
	
	// what to do
	if(result==1){  // Yes- only save the actual file
				// save file as if Untitled and close file
	  if(isUntitled(actual_info->filename))
            {
//	    KDEBUG(KDEBUG_INFO,CKDEVELOP,"yes- untitled");
	    switchToFile(actual_info->filename);
            handledNames.append(actual_info->filename);
	    cont=fileSaveAs();
            next_info=edit_infos.first(); // start again... 'cause we deleted an entry
	  }
				// Save file and close it
	  else{
//	    KDEBUG(KDEBUG_INFO,CKDEVELOP,"yes- save");
	    switchToFile(actual_info->filename);
            handledNames.append(actual_info->filename);
	    slotFileSave();
            actual_info->modified=edit_widget->isModified();
            cont=!actual_info->modified; //something went wrong
	  }
	}
	
	if(result==2){   // No - no save but close
//	  KDEBUG(KDEBUG_INFO,CKDEVELOP,"No- close file");
          handledNames.append(actual_info->filename);
	  actual_info->modified=false;
          removeFileFromEditlist(actual_info->filename); // immediate remove
          next_info=edit_infos.first(); // start again... 'cause we deleted an entry
	}
	if(result==3){  // Save all
//	  KDEBUG(KDEBUG_INFO,CKDEVELOP,"Save all");
	  slotFileSaveAll();
	  break;
	}
	if(result==4){ // Cancel
	  cont=false;
//	  KDEBUG(KDEBUG_INFO,CKDEVELOP,"Cancel project close");
	  break;
	}
	
      }  // end actual file close
     actual_info=next_info;
    } // end for-loop

  // check if something went wrong with saving
  if (cont)
  {
    for(actual_info=edit_infos.first(); cont && actual_info != 0;
	actual_info=edit_infos.next())
    {
        if (actual_info->modified)
           cont=false;
    } // end for-loop
  }

  if(cont){
    // cancel wasn't pressed and all sources are saved - project closed
    // clear all widgets

    if(!bKDevelop){
      //      switchToKDevelop();
    }
    //    disableCommand(ID_TOOLS_KDLGEDIT);
    
    class_tree->clear();
    log_file_tree->clear();
    real_file_tree->clear();
    menu_buffers->clear();

    header_widget->clear();
    cpp_widget->clear();
    messages_widget->clear();
    stdin_stdout_widget->clear();
    stderr_widget->clear();
    kdlg_dialogs_view->clear();

    if (dbgController)
			slotDebugStop();

    kdlgedit->slotFileSave();
    
    kdlg_edit_widget->hide();
    kdlg_prop_widget->hide();
    kdlg_tabctl->setTabEnabled("widgets_view",false);
    kdlg_tabctl->setTabEnabled("dialogs_view",false);
    kdlg_tabctl->setTabEnabled("items_view",false);
    kdlg_tabctl->setCurrentTab(1); // dialogs

    //clear all edit_infos before starting a new project
    edit_infos.clear();
    
    toolBar(ID_BROWSER_TOOLBAR)->clearCombo(ID_CV_TOOLBAR_CLASS_CHOICE);
    toolBar(ID_BROWSER_TOOLBAR)->clearCombo(ID_CV_TOOLBAR_METHOD_CHOICE);
    
    // re-inititalize the edit widgets
    header_widget->setName(i18n("Untitled.h"));
    cpp_widget->setName(i18n("Untitled.cpp"));
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
    // doc menu
    disableCommand(ID_HELP_PROJECT_API);
    disableCommand(ID_HELP_USER_MANUAL);
    // build menu
    setToolMenuProcess(false);  
    disableCommand(ID_BUILD_STOP);
    disableCommand(ID_BUILD_AUTOCONF);
    disableCommand(ID_KDLG_BUILD_GENERATE);
    
    // prj menu
    disableCommand(ID_PROJECT_CLOSE);
    disableCommand(ID_PROJECT_ADD_FILE_EXIST);
    disableCommand(ID_PROJECT_ADD_NEW_TRANSLATION_FILE);
    disableCommand(ID_PROJECT_REMOVE_FILE);
    disableCommand(ID_PROJECT_NEW_CLASS);
    disableCommand(ID_PROJECT_FILE_PROPERTIES);
    disableCommand(ID_PROJECT_OPTIONS);
    disableCommand(ID_PROJECT_MAKE_DISTRIBUTION);

    disableCommand(ID_CV_WIZARD);
    disableCommand(ID_CV_GRAPHICAL_VIEW);
	  disableCommand(ID_CV_TOOLBAR_CLASS_CHOICE);
  	disableCommand(ID_CV_TOOLBAR_METHOD_CHOICE);

    file_open_popup->clear();
    file_open_list.clear();
  }

  slotStatusMsg(i18n("Ready."));
  refreshTrees();

  if (!cont)
  {
   setMainCaption();
  }

  return cont; // false if pressed cancel
}

void CKDevelop::slotProjectAddNewFile(){
  newFile(true);
}

void CKDevelop::slotAddExistingFiles(){
  QString temp_template;
  CGenerateNewFile genfile;
  bool copy = false;
  ProjectFileType type = DATA;
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
  QString temp_name;
  QString dest_name ;
  QString file;
  QFileInfo file_info;
  int i=files.count();
    
  QProgressDialog progress( i18n("Copying files..."),0, i, this,"",true );
  progress.setCaption(i18n("please wait..."));
  progress.show();
    
  i=0;
  progress.setProgress( i);
  QStrList lNewFiles;
  for(file = files.first(); file !=0;file = files.next()){
    i++;
    copy = false;
    progress.setProgress( i );
    if (!QFile::exists((const char*)file)) {
        KMsgBox::message(this,i18n("Attention"),file +i18n("\n\nFile does not exist!"));
        continue;
    }
    file_info.setFile(file);
    source_name = file_info.fileName();
    dest_name = dest + source_name;
    temp_name = dest + "temp.tmp";

    // Fetch the type of the file
    type = CProject::getType( dest_name );

    if(file == dest_name) {
      copy = false;
      process.clearArguments();
      process << "cat"; // copy is your friend :-) ...cat, too

      if (add_dlg->isTemplateChecked())
      {
       if (CProject::getType(file)==CPP_HEADER)
        {
         temp_template = genfile.genHeaderFile(KApplication::localkdedir()+"/share/apps/kdevelop/temp_template", prj,source_name);
         process << temp_template;
        }
        else if (CProject::getType(file)==CPP_SOURCE)
              {
               temp_template = genfile.genCPPFile(KApplication::localkdedir()+"/share/apps/kdevelop/temp_template", prj, source_name);
               process << temp_template;
              }
      }
      process << file;
      process << ">";

      process << temp_name;
      process.start(KProcess::Block,KProcess::AllOutput); // blocked because it is important 
      process.clearArguments();
      process << "mv";
      process << temp_name;
      process << dest_name;
      process.start(KProcess::Block,KProcess::AllOutput);
    }
    else
    if(QFile::exists(dest_name)){
      int result=KMsgBox::yesNoCancel(this,i18n("File exists!"),
                                      QString(i18n("\nThe file\n\n"))+
				      source_name+
				      i18n("\n\nalready exists.\nDo you want overwrite the old one?\n"));
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
      process << "cat"; // copy is your friend :-) ...cat, too

      if (add_dlg->isTemplateChecked())
      {
       if (CProject::getType(file)==CPP_HEADER)
        {
         temp_template = genfile.genHeaderFile(KApplication::localkdedir()+"/share/apps/kdevelop/temp_template", prj,source_name);
         process << temp_template;
        }
        else if (CProject::getType(file)==CPP_SOURCE)
              {
               temp_template = genfile.genCPPFile(KApplication::localkdedir()+"/share/apps/kdevelop/temp_template", prj, source_name);
               process << temp_template;
              }
      }
      process << file;
      process << ">";

      process << dest+source_name;
      process.start(KProcess::Block,KProcess::AllOutput); // blocked because it is important  
    }

    new_subdir = addFileToProject(dest_name,type,false) || new_subdir; // no refresh
	lNewFiles.append(dest_name);
  }
  progress.setProgress( files.count() );
  // if (type != DATA)               // don't load data files (has to be tested if wanted)
  switchToFile(dest_name);
  refreshTrees(&lNewFiles);
    
  if(new_subdir){
    newSubDir();
  }

}

void CKDevelop::slotProjectAddExistingFiles(){
  add_dlg = new CAddExistingFileDlg(this,"test",prj);

  add_dlg->destination_edit->setText(prj->getProjectDir()+ prj->getSubDir());

  if(add_dlg->exec()){
//    QTimer::singleShot(100,this,SLOT(slotAddExistingFiles()));
      slotAddExistingFiles();
  }
  delete add_dlg;
  add_dlg=0;
}

void CKDevelop::slotProjectRemoveFile(){
    QString name = log_file_tree->currentItem()->text(0);
    delFileFromProject(name);
}

void CKDevelop::slotProjectOptions(){
  CPrjOptionsDlg prjdlg(this,"optdialog",prj);
  QString shell = getenv("SHELL");
  QString flaglabel;
  if(shell == "/bin/bash"){
      flaglabel=(prj->getProjectType()=="normal_c") ? "CFLAGS=\"" : "CXXFLAGS=\"";
  }
  else{
      flaglabel=(prj->getProjectType()=="normal_c") ? "env CFLAGS=\"" : "env CXXFLAGS=\"";
  }
  
  QString args=prj->getConfigureArgs();

  if(prjdlg.exec()){
    if (prjdlg.needConfigureInUpdate()){
      prj->updateConfigureIn();
      KMsgBox::message(0,i18n("Information"),i18n("You have modified the projectversion.\nWe will regenerate all Makefiles now."),KMsgBox::INFORMATION);
      setToolMenuProcess(false);
      slotStatusMsg(i18n("Running automake/autoconf and configure..."));
      messages_widget->clear();
      showOutputView(true);
      QDir::setCurrent(prj->getProjectDir());
      shell_process.clearArguments();
      shell_process << make_cmd << " -f Makefile.dist  && ";
      shell_process << flaglabel;
      if (!prj->getCXXFLAGS().isEmpty() || !prj->getAdditCXXFLAGS().isEmpty())
      {
       if (!prj->getCXXFLAGS().isEmpty())
          shell_process << prj->getCXXFLAGS().simplifyWhiteSpace () << " ";
       if (!prj->getAdditCXXFLAGS().isEmpty())
          shell_process << prj->getAdditCXXFLAGS().simplifyWhiteSpace ();
      }
      shell_process  << "\" " << "LDFLAGS=\" " ;
      if (!prj->getLDFLAGS().isEmpty())
         shell_process << prj->getLDFLAGS().simplifyWhiteSpace ();
      shell_process  << "\" " << " ./configure" << args;

      shell_process.start(KProcess::NotifyOnExit,KProcess::AllOutput);
      return;
    }
    if(prjdlg.needMakefileUpdate()){
      prj->updateMakefilesAm();
      setToolMenuProcess(false);
      slotStatusMsg(i18n("Running configure..."));
      messages_widget->clear();
      showOutputView(true);
      QDir::setCurrent(prj->getProjectDir());
      shell_process.clearArguments();
      shell_process << flaglabel;
      if (!prj->getCXXFLAGS().isEmpty() || !prj->getAdditCXXFLAGS().isEmpty())
      {
       if (!prj->getCXXFLAGS().isEmpty())
          shell_process << prj->getCXXFLAGS().simplifyWhiteSpace () << " ";
       if (!prj->getAdditCXXFLAGS().isEmpty())
          shell_process << prj->getAdditCXXFLAGS().simplifyWhiteSpace ();
      }
      shell_process  << "\" " << "LDFLAGS=\" " ;
      if (!prj->getLDFLAGS().isEmpty())
	shell_process << prj->getLDFLAGS().simplifyWhiteSpace ();
      shell_process  << "\" " << " ./configure "  << args;

      shell_process.start(KProcess::NotifyOnExit,KProcess::AllOutput);
    }
  }
  
}


void CKDevelop::slotProjectNewClass(){
  CNewClassDlg* dlg = new CNewClassDlg(this,"newclass",prj);
  if(dlg->exec()){
    QString source_file=dlg->getImplFile() ;
    QString header_file=dlg->getHeaderFile();
    switchToFile(source_file);
    switchToFile(header_file);
// added by Alex Kern, Alexander.Kern@saarsoft.de
//
/*  QFileInfo header_info(header_file);
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
    prj->addFileToProject(prj->getSubDir() + header_info.fileName(),file_info);*/
    bool new_subdir1, new_subdir2;
    new_subdir1 = addFileToProject(source_file, CPP_SOURCE, false);
    new_subdir2 = addFileToProject(header_file, CPP_HEADER, false);

    if(new_subdir1 || new_subdir2)
      newSubDir();

    prj->updateMakefilesAm();
	QStrList lToRefresh;
	lToRefresh.autoDelete();
	lToRefresh.append(source_file);
	lToRefresh.append(header_file);
	refreshTrees(&lToRefresh);
    //slotViewRefresh();
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

  
	slotStatusMsg(i18n("Opening project..."));
	QString str;
	str = KFileDialog::getOpenFileName(0,"*.kdevprj",this);
	slotProjectOpenCmdl(str);
}

void CKDevelop::slotProjectOpenRecent(int id_)
{
  slotProjectOpenCmdl(recent_projects.at(id_));
}

void CKDevelop::slotProjectOpenCmdl(QString prjname)
{
	QString old_project = "";

	prjname.replace(QRegExp("file:"),"");
	QFileInfo info(prjname);

	if (info.isFile())		//if the new project file is not valid, do nothing
	{
		if(project)
		{
			old_project = prj->getProjectFile();
			if(!slotProjectClose())		//the user may have pressed cancel in which case the state is undetermined
			{
				readProjectFile(old_project);
				slotViewRefresh();
				return;
			}
  		}
  
    	if(!(readProjectFile(prjname)))		//the readProjectFile is now garanteed not to modify the state if it fails
		{

		    KMsgBox::message(0,prjname,"This is a Project-File from KDevelop 0.1\nSorry,but it's incompatible with KDevelop >= 0.2.\nPlease use only new generated projects!");
//		    readProjectFile(old_project);		//not needed anymore
    	}
		else
			slotViewRefresh();
		slotStatusMsg(i18n("Ready."));
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
  
  slotStatusMsg(i18n("Creating a new frame application..."));
  config->setGroup("General Options");
  CKAppWizard kappw(this,"zutuz",config->readEntry("author_name",""),
		    config->readEntry("author_email",""));
  
  
  kappw.setCaption(i18n("ApplicationWizard"));
  kappw.exec();
  QString file = kappw.getProjectFile();
  
  if(kappw.generatedProject())
  {
    config->setGroup("General Options");
    config->writeEntry("author_name",kappw.getAuthorName());
    config->writeEntry("author_email",kappw.getAuthorEmail());
    config->sync();
	
	if(project)		//now that we know that a new project will be built we can close the previous one
	{
    	old_project = prj->getProjectFile();
    	if(!slotProjectClose())				//the user may have pressed cancel in which case the state is undetermined
		{
			readProjectFile(old_project);
			slotViewRefresh();
			return;
		}
  	}

    readProjectFile(file);
    if (prj->getProjectType() == "normal_kde" || prj->getProjectType() == "mini_kde") 
	{
      slotProjectMessages();
    }
  	slotViewRefresh();	// a new project started, this is legitimate
  }
  slotStatusMsg(i18n("Ready."));
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
    slotProjectMessages();
  }
}
void CKDevelop::slotAddFileToProject(QString abs_filename){
  ProjectFileType type = CProject::getType( abs_filename );

  addFileToProject(abs_filename, type, false);

  // Parse the file if it's a sourcefile.
  if( type == CPP_SOURCE || type == CPP_HEADER )
    class_tree->addFile( abs_filename );

  log_file_tree->refresh( prj );
}

void CKDevelop::slotProjectMessages(){
  if(!CToolClass::searchProgram("xgettext")){
    return;
  }
  error_parser->toogleOff();
  showOutputView(true);
  setToolMenuProcess(false);
  slotFileSaveAll();
  slotStatusMsg(i18n("Creating pot-file in /po..."));
  messages_widget->clear();
  error_parser->toogleOff();
  QDir::setCurrent(prj->getProjectDir() + prj->getSubDir());
  shell_process.clearArguments();
  //shellprocess << make_cmd;
  shell_process << make_cmd + " messages &&  cd ../po && make merge";
  shell_process.start(KProcess::NotifyOnExit,KProcess::AllOutput);
  beep = true;
}

void CKDevelop::slotProjectAPI(){
  if(!CToolClass::searchProgram("kdoc")){
    return;
  }
  showOutputView(true);

  setToolMenuProcess(false);
  error_parser->toogleOff();
  slotFileSaveAll();
  slotStatusMsg(i18n("Creating project API-Documentation..."));
  messages_widget->clear();

#ifdef WITH_KDOC2
  config->setGroup("Doc_Location");
  QString idx_path = config->readEntry("kdoc_index", KDOC_INDEXDIR);
  if (idx_path.isEmpty())
      idx_path = config->readEntry("doc_kde", KDELIBS_DOCDIR)
          + "/kdoc-reference";
  QString link;
  if ( QFileInfo(idx_path + "/qt.kdoc").exists() ||
       QFileInfo(idx_path + "/qt.kdoc.gz").exists() )
      link = "-lqt ";
  // This could me made a lot smarter...
  if ( QFileInfo(idx_path + "/kdecore.kdoc").exists() ||
       QFileInfo(idx_path + "/kdecore.kdoc.gz").exists() )
      link += "-lkdecore -lkdeui -lkfile -lkfmlib -lkhtml -ljscript -lkab -lkspell";

  QDir::setCurrent(prj->getProjectDir() + prj->getSubDir());
  int dirlength = QDir::currentDirPath().length()+1;

  QString sources;
  QStrList headerlist(prj->getHeaders());
  QStrListIterator it(headerlist);
  for (; it.current(); ++it)
      {
          QString file = it.current();
          file.remove(0, dirlength);
          sources += file;
          sources += " ";
      }

  shell_process.clearArguments();
  shell_process << "kdoc";
  shell_process << "-p -d" + prj->getProjectDir() + prj->getSubDir() +  "api";
  if (!sources.isEmpty())
      shell_process << sources;
  if (!link.isEmpty())
      {
          shell_process << ("-L" + idx_path);
          shell_process << link;
      }

#else
 
  config->setGroup("Doc_Location");
  QString doc_kde=config->readEntry("doc_kde");
  QString qt_ref_file=doc_kde+"kdoc-reference/qt.kdoc";
  QString kde_ref_file=doc_kde+"kdoc-reference/kdecore.kdoc";
  QString khtmlw_ref_file=doc_kde+"kdoc-reference/khtmlw.kdoc";

	QStrList headerlist(prj->getHeaders());
	uint i;

  QDir::setCurrent(prj->getProjectDir() + prj->getSubDir());
	QString dir=QDir::currentDirPath();
  uint dirlength=dir.length()+1;

  shell_process.clearArguments();

  if( !QFileInfo(kde_ref_file).exists()){
    shell_process << "kdoc";
    shell_process << "-p -d" + prj->getProjectDir() + prj->getSubDir() +  "api";
//    shell_process << "*.h";
    shell_process << prj->getProjectName();
		for (i=0; i < headerlist.count(); i++){
			QString file=headerlist.at(i);
			QString header=file.remove(0,dirlength);
			shell_process << header;
			shell_process << " ";
		}
  }
  else if(!QFileInfo(qt_ref_file).exists()){
    shell_process << "kdoc";
    shell_process << "-p -d" + prj->getProjectDir() + prj->getSubDir() +  "api";
    shell_process << "-ufile:" + prj->getProjectDir() + prj->getSubDir() +  "api"+"/";
    shell_process << "-L" + doc_kde + "kdoc-reference";
    shell_process << prj->getProjectName();
		for (i=0; i < headerlist.count(); i++){
			QString file=headerlist.at(i);
			QString header=file.remove(0,dirlength);
			shell_process << header;
			shell_process << " ";
		}
		if(!QFileInfo(khtmlw_ref_file).exists()){
    	shell_process << "-lkdecore -lkdeui -lkfile -lkfmlib -lkhtml -ljscript -lkab -lkspell";
		}
		else{
    	shell_process << "-lkdecore -lkdeui -lkfile -lkfmlib -lkhtmlw -ljscript -lkab -lkspell";
		}
  }
  else{
    shell_process << "kdoc";
    shell_process << "-p -d" + prj->getProjectDir() + prj->getSubDir() +  "api";
    shell_process << "-ufile:" + prj->getProjectDir() + prj->getSubDir() +  "api"+"/";
    shell_process << "-L" + doc_kde + "kdoc-reference";
    shell_process << prj->getProjectName();
		for (i=0; i < headerlist.count(); i++){
			QString file=headerlist.at(i);
			QString header=file.remove(0,dirlength);
			shell_process << header;
			shell_process << " ";
		}
		if(!QFileInfo(khtmlw_ref_file).exists()){
    	shell_process << "-lqt -lkdecore -lkdeui -lkfile -lkfmlib -lkhtml -ljscript -lkab -lkspell";
		}
		else{
    	shell_process << "-lqt -lkdecore -lkdeui -lkfile -lkfmlib -lkhtmlw -ljscript -lkab -lkspell";
		}
  }
  
#endif
  
  shell_process.start(KShellProcess::NotifyOnExit,KShellProcess::AllOutput);
}

void CKDevelop::slotProjectManual(){

    CMakeManualDlg dlg(this,"tesr",prj->getSGMLFile());
    if(dlg.exec()){
	
	showOutputView(true);
	error_parser->toogleOn(CErrorMessageParser::SGML2HTML);
	setToolMenuProcess(false);
	//  slotFileSaveAll();
	slotStatusMsg(i18n("Creating project Manual..."));
	messages_widget->clear();
	
	bool ksgml = true;
	if(dlg.program == "sgml2html") ksgml = false;
	prj->setSGMLFile(dlg.file);
	CGenerateNewFile generator;
	QFileInfo info(dlg.file);
	
	if(ksgml){
	    
	    QString nif_file = info.dirPath() + "/" + info.baseName()+ ".nif";
	    if(!QFile::exists(nif_file)){
		generator.genNifFile(nif_file);
	    }
	}
	QDir::setCurrent(info.dirPath());
        shell_process.clearArguments();
	if(ksgml){
	    shell_process << "ksgml2html";
	    shell_process << info.fileName();
	    shell_process << "en";
	}
	else{
	    shell_process << "sgml2html";
	    shell_process << info.fileName();
	}
        shell_process.start(KProcess::NotifyOnExit,KProcess::AllOutput);
    }
}

void CKDevelop::slotProjectMakeDistSourceTgz(){
  if(!view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW)){
    view->setSeparatorPos(output_view_pos);
    view_menu->setItemChecked(ID_VIEW_OUTPUTVIEW,true);
    QRect rMainGeom= view->geometry();
    view->resize(rMainGeom.width()-1,rMainGeom.height());
    view->resize(rMainGeom.width()+1,rMainGeom.height());
  }

  showOutputView(true);
  error_parser->toogleOff();
  setToolMenuProcess(false);
  slotFileSaveAll();
  slotStatusMsg(i18n("Running make dist..."));
  messages_widget->clear();
  QDir::setCurrent(prj->getProjectDir());
  shell_process.clearArguments();
  shell_process << make_cmd << " dist";
  shell_process.start(KProcess::NotifyOnExit,KProcess::AllOutput);
  beep = true;
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

  // Get the filetype.
  type = CProject::getType( complete_filename );
  if(type == KDEV_DIALOG){
      kdlgedit->slotFileCloseForceSave();
      kdlg_edit_widget->newDialog();
  }
  // load into the widget
  switchToFile(complete_filename);
  
  
  
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
		   
  rel_name.remove(0,prj->getProjectDir().length());
  //  rel_name.replace(QRegExp(prj->getProjectDir()),"");
  //  cerr << "getProDir():" << prj->getProjectDir() << endl;
  //  cerr << "*rel_name2*:" << rel_name << endl;

  TFileInfo info;
  if( type == KDEV_DIALOG){
    TDialogFileInfo dinfo;
    dinfo.rel_name = rel_name;
    dinfo.type = type;
    dinfo.dist = true;
    dinfo.install = false;
    //...
    dinfo.is_toplevel_dialog = true;
    
    new_subdir = prj->addDialogFileToProject(dinfo.rel_name,dinfo);
    //    ((CKDevelop*)parent())->kdlg_get_edit_widget()->newDialog();			
    //    ((CKDevelop*)parent())->kdlg_get_edit_widget()->saveToFile(l_dialog_file);
    
    info.rel_name = rel_name;
  }
  else{ // normal File
    info.rel_name = rel_name;
    info.type = type;
    info.dist = ( type != PO );
    
    info.install=false;
    info.install_location = "";
    new_subdir = prj->addFileToProject(rel_name,info);
    
    
  }
  prj->writeProject();
  prj->updateMakefilesAm();
  
  if(refresh)
    refreshTrees(&info);
  
  return new_subdir;
}

void CKDevelop::delFileFromProject(QString rel_filename){

  prj->removeFileFromProject(rel_filename);
  prj->writeProject();
  QStrList lDeletedFile;
  lDeletedFile.autoDelete();
  QString lAbsoluteFileName = prj->getProjectDir() + rel_filename;
  lDeletedFile.append(lAbsoluteFileName);
  refreshTrees(&lDeletedFile);
}

bool CKDevelop::readProjectFile(QString file){
  QString str;
  QString extension;

/*
  prj = new CProject(file);
  if(!(prj->readProject())){
    return false;
  }
  else {
      project=true;
  }
*/
CProject * lNewProject = new CProject(file);
  if(!(lNewProject->readProject())){
    return false;
  }
  else {
      project=true;
	  prj = lNewProject;
  }

  // str = prj.getProjectDir() + prj.getSubDir() + prj.getProjectName().lower() + ".cpp";
  //   if(QFile::exists(str)){
  //     switchToFile(str);
  //   }

  // if this is a c project then change Untitled.cpp to Untitled.c
  if (prj->getProjectType()=="normal_c")
  {
    TEditInfo *actual_info=0l;
    for(actual_info=edit_infos.first();actual_info != 0 && actual_info->filename!=i18n("Untitled.cpp");
         actual_info=edit_infos.next());
    if (actual_info)
    {
      actual_info->filename = i18n("Untitled.c");
      menu_buffers->changeItem(actual_info->filename, actual_info->id);
      if (cpp_widget->getName()==i18n("Untitled.cpp"))
        cpp_widget->setName(actual_info->filename);
    }
   }

  extension=(prj->getProjectType()=="normal_c") ? "c" : "cpp";
  str = prj->getProjectDir() + prj->getSubDir() + prj->getProjectName().lower() + ".h";
  if(QFile::exists(str)){
    switchToFile(str);
  }
  str = prj->getProjectDir() + prj->getSubDir() + "main."+extension;
  if(QFile::exists(str)){
    switchToFile(str);
  }

// TODO: Add function to read last opened files from project to restore project workspace

  switchToWorkspace(prj->getCurrentWorkspaceNumber());
  // set the menus enable
  // file menu
  
  enableCommand(ID_FILE_NEW);
  // doc menu
  enableCommand(ID_HELP_PROJECT_API);
  enableCommand(ID_HELP_USER_MANUAL);
  // build menu
  setToolMenuProcess(true);

  // prj menu
  enableCommand(ID_PROJECT_CLOSE);
  enableCommand(ID_PROJECT_ADD_FILE_EXIST);

  if(prj->isKDEProject() || prj->isQtProject()){
    enableCommand(ID_TOOLS_KDLGEDIT);
    kdlg_tabctl->setTabEnabled("dialogs_view",true);
  }  

  if (prj->isKDEProject()){
   enableCommand(ID_PROJECT_ADD_NEW_TRANSLATION_FILE);
  }
  else{
    disableCommand(ID_PROJECT_ADD_NEW_TRANSLATION_FILE);
  }
  if(prj->isCustomProject()){
    disableCommand(ID_PROJECT_FILE_PROPERTIES);
    enableCommand(ID_PROJECT_OPTIONS);
  }
  else{
    enableCommand(ID_PROJECT_FILE_PROPERTIES);
    enableCommand(ID_PROJECT_OPTIONS);
  }
  
  if(prj->isKDEProject() || prj->isQtProject()){
    kdlg_tabctl->setTabEnabled("dialogs_view",true);
    kdlg_tabctl->setCurrentTab(1); // dialogs
  
  }

  enableCommand(ID_PROJECT_REMOVE_FILE);
  enableCommand(ID_PROJECT_WORKSPACES);
  enableCommand(ID_BUILD_AUTOCONF);
  enableCommand(ID_PROJECT_MAKE_DISTRIBUTION);

  if (prj->getProjectType()!="normal_c")  // activate class wizard unless it is a normal C project
  {
    enableCommand(ID_PROJECT_NEW_CLASS);
    enableCommand(ID_CV_WIZARD);
    enableCommand(ID_CV_GRAPHICAL_VIEW);
    enableCommand(ID_CV_TOOLBAR_CLASS_CHOICE);
    enableCommand(ID_CV_TOOLBAR_METHOD_CHOICE);
  }

  addRecentProject(file);
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
  current.openfiles.removeRef(i18n("Untitled.h"));
  current.openfiles.removeRef(i18n("Untitled.cpp"));
  current.openfiles.removeRef(i18n("Untitled.c"));
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
//  shell_process << make_cmd << " -f Makefile.dist  && ./configure";
  QString flaglabel=(prj->getProjectType()=="normal_c") ? "CFLAGS=\"" : "CXXFLAGS=\"";
  shell_process << flaglabel;
  if (!prj->getCXXFLAGS().isEmpty() || !prj->getAdditCXXFLAGS().isEmpty())
  {
            if (!prj->getCXXFLAGS().isEmpty())
                  shell_process << prj->getCXXFLAGS() << " ";
            if (!prj->getAdditCXXFLAGS().isEmpty())
                  shell_process << prj->getAdditCXXFLAGS();
  }
  shell_process  << "\" " << "LDFLAGS=\" " ;
  if (!prj->getLDFLAGS().isEmpty())
                shell_process << prj->getLDFLAGS();
  shell_process  << "\" ";
  shell_process << make_cmd << " -f Makefile.dist  && ./configure" << prj->getConfigureArgs();
  shell_process.start(KProcess::NotifyOnExit,KProcess::AllOutput);
}











