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
#include <qsplitter.h>
#include <kcursor.h>
#include <kstddirs.h>

#include "ckdevelop.h"
#include "ctoolclass.h"
#include "ckappwizard.h"
#include "cclassview.h"
#include "crealfileview.h"
#include "clogfileview.h"
#include "makeview.h"
#include "ceditwidget.h"
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
#include "misc.h"


/*********************************************************************
 *                                                                   *
 *                              SLOTS                                *
 *                                                                   *
 ********************************************************************/ 

void CKDevelop::slotProjectImport()
{}

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
      TEditInfo *next_info=edit_infos.next();
      if(actual_info->modified && handledNames.contains(actual_info->filename)<1){

#warning FIXME: QMessageBox has 3 buttons maximum
#if 0
	KMsgBox *project_close=new KMsgBox(this,i18n("Save changed project files ?"),
					   i18n("The project\n\n")+prj->getProjectName()
					   +i18n("\n\ncontains changed files. Save modified file\n\n")
					   +actual_info->filename+" ?\n\n",KMsgBox::QUESTION,
					   i18n("Yes"), i18n("No"), i18n("Save all"), i18n("Cancel"));
 				// show the messagea and store result in result:
	project_close->show();
    	int result=project_close->result();
#else
        int result = 0; // until problem above is resolved
#endif
	
	// create the save project messagebox
	
	// what to do
	if(result==1){  // Yes- only save the actual file
				// save file as if Untitled and close file
	  if(isUntitled(actual_info->filename))
            {

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
          handledNames.append(actual_info->filename);
	  actual_info->modified=false;
          removeFileFromEditlist(actual_info->filename); // immediate remove
          next_info=edit_infos.first(); // start again... 'cause we deleted an entry
	}
	if(result==3){  // Save all
	  slotFileSaveAll();
	  break;
	}
	if(result==4){ // Cancel
	  cont=false;
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

    QListIterator<Component> it(components);
    for ( ; it.current(); ++it)
        (*it)->projectClosed();
    menu_buffers->clear();

    header_widget->clear();
    cpp_widget->clear();
    stdin_stdout_widget->clear();
    stderr_widget->clear();
    kdlg_dialogs_view->clear();

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
  QFileInfo fi;
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
    if (!QFile::exists((const char*)file)) {
        KMessageBox::sorry(this, i18n("File %1does not exist!").arg(file));
        continue;
    }
    file_info.setFile(file);
    source_name = file_info.fileName();
    dest_name = dest + source_name;
   
    // Fetch the type of the file
    type = CProject::getType( dest_name );


    if(file == dest_name) {
      copy = false;
      KShellProcess process("/bin/sh");
      process << "cat"; // copy is your friend :-) ...cat, too

      if (add_dlg->isTemplateChecked())
      {
       if (CProject::getType(file)==CPP_HEADER)
        {
         temp_template = genfile.genHeaderFile(locate("appdata","temp_template"), prj,fi.fileName());
         process << temp_template;
        }
        else if (CProject::getType(file)==CPP_SOURCE)
              {
               temp_template = genfile.genCPPFile(locate("appdata","temp_template"), prj, fi.fileName());
               process << temp_template;
              }
      }
      process << file;
      process << ">";

      process << "temp.tmp";
      process.start(KProcess::Block,KProcess::AllOutput); // blocked because it is important 
      process.clearArguments();
      process << "mv";
      process << "temp.tmp";
      process << dest_name;
      process.start(KProcess::Block,KProcess::AllOutput);
    }
    else      
    if(QFile::exists(dest_name)){
        int result=KMessageBox::warningYesNoCancel(this,
                                                   i18n("The file\n\n%1\n\n"
                                                        "already exists.\nDo you want overwrite the old one?").arg(source_name));
      if(result==KMessageBox::Yes)
        copy = true;
      if(result==KMessageBox::No)
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
      KShellProcess process("/bin/sh");
      process << "cat"; // copy is your friend :-) ...cat, too

      fi.setFile(file);

      if (add_dlg->isTemplateChecked())
      {
       if (CProject::getType(file)==CPP_HEADER)
        {
         temp_template = genfile.genHeaderFile(locate("appdata","temp_template"), prj,fi.fileName());
         process << temp_template;
        }
        else if (CProject::getType(file)==CPP_SOURCE)
              {
               temp_template = genfile.genCPPFile(locate("appdata","temp_template"), prj, fi.fileName());
               process << temp_template;
              }
      }
      process << file;
      process << ">";

      process << dest+fi.fileName();
      process.start(KProcess::Block,KProcess::AllOutput); // blocked because it is important  
    }
    new_subdir = addFileToProject(dest_name,type,false) || new_subdir; // no refresh
  }
  progress.setProgress( files.count() );
  // if (type != DATA)               // don't load data files (has to be tested if wanted)
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
  
  QString args=prj->getConfigureArgs();

  if(prjdlg.exec()){
    if (prjdlg.needConfigureInUpdate()){
      prj->updateConfigureIn();
      KMessageBox::information(0, i18n("You have modified the projectversion.\n"
                                       "We will regenerate all Makefiles now."));
      setToolMenuProcess(false);
      slotStatusMsg(i18n("Running automake/autoconf and configure..."));
      messages_widget->clear();
      showOutputView(true);
      messages_widget->prepareJob(prj->getProjectDir());
      (*messages_widget) << make_cmd << " -f Makefile.dist  && ";
      (*messages_widget) << ( (prj->getProjectType()=="normal_c") ? "CFLAGS=\"" : "CXXFLAGS=\"" );
      if (!prj->getCXXFLAGS().isEmpty() || !prj->getAdditCXXFLAGS().isEmpty())
      {
       if (!prj->getCXXFLAGS().isEmpty())
          (*messages_widget) << prj->getCXXFLAGS().simplifyWhiteSpace () << " ";
       if (!prj->getAdditCXXFLAGS().isEmpty())
          (*messages_widget) << prj->getAdditCXXFLAGS().simplifyWhiteSpace ();
      }
      (*messages_widget) << "\" " << "LDFLAGS=\" " ;
      if (!prj->getLDFLAGS().isEmpty())
         (*messages_widget) << prj->getLDFLAGS().simplifyWhiteSpace ();
      (*messages_widget) << "\" " << " ./configure" << args;

      messages_widget->startJob();
      return;
    }
    if(prjdlg.needMakefileUpdate()){
      prj->updateMakefilesAm();
      setToolMenuProcess(false);
      slotStatusMsg(i18n("Running configure..."));
      messages_widget->clear();
      showOutputView(true);
      messages_widget->prepareJob(prj->getProjectDir());
      (*messages_widget) << ( (prj->getProjectType()=="normal_c") ? "CFLAGS=\"" : "CXXFLAGS=\"" );
      if (!prj->getCXXFLAGS().isEmpty() || !prj->getAdditCXXFLAGS().isEmpty())
      {
       if (!prj->getCXXFLAGS().isEmpty())
          (*messages_widget) << prj->getCXXFLAGS().simplifyWhiteSpace () << " ";
       if (!prj->getAdditCXXFLAGS().isEmpty())
          (*messages_widget) << prj->getAdditCXXFLAGS().simplifyWhiteSpace ();
      }
      (*messages_widget) << "\" " << "LDFLAGS=\" " ;
      if (!prj->getLDFLAGS().isEmpty())
	(*messages_widget) << prj->getLDFLAGS().simplifyWhiteSpace ();
      (*messages_widget) << "\" " << " ./configure "  << args;
      messages_widget->startJob();
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

    KMessageBox::sorry(0, i18n("This is a Project-File from KDevelop 0.1\nSorry,but it's incompatible with KDevelop >= 0.2.\n"
                               "Please use only new generated projects!"));
    readProjectFile(old_project);
    }
    slotViewRefresh();

    slotStatusMsg(i18n("Ready."));

    QListIterator<Component> it(components);
    for ( ; it.current(); ++it)
        (*it)->projectOpened(prj);
  
  }	
  
}

void CKDevelop::slotProjectOpenRecent(int id_)
{
  slotProjectOpenCmdl(recent_projects.at(id_));
}

void CKDevelop::slotProjectOpenCmdl(QString prjname){

  QString old_project = "";

  if(project){
    old_project = prj->getProjectFile();
    if(!slotProjectClose()){
      return;
    }
  }
  prjname.replace(QRegExp("file:"),"");
  
  QFileInfo info(prjname);

  if (info.isFile()){
    if(!(readProjectFile(prjname))){
      KMessageBox::sorry(0, "This is a Project-File from KDevelop 0.1\nSorry,but it's incompatible with KDevelop >= 0.2.\n"
                         "Please use only new generated projects!");
      readProjectFile(old_project);
    }

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
  if(project){
    old_project = prj->getProjectFile();
    if(!slotProjectClose()){
      return;
    }
  }
  
  slotStatusMsg(i18n("Creating a new frame application..."));
  config->setGroup("General Options");
  CKAppWizard kappw(this,"zutuz",config->readEntry("author_name",""),
		    config->readEntry("author_email",""));
  
  
  kappw.exec();
  QString file = kappw.getProjectFile();
  
  if(kappw.generatedProject()){
    config->setGroup("General Options");
    config->writeEntry("author_name",kappw.getAuthorName());
    config->writeEntry("author_email",kappw.getAuthorEmail());
    config->sync();
    readProjectFile(file);
    if (prj->getProjectType() == "normal_kde" || prj->getProjectType() == "mini_kde") {
      slotProjectMessages();
    }
  }
  else if (old_project != ""){ // if cancel load the old project again
    readProjectFile(old_project);
  }
  slotViewRefresh();
  //cerr << kappw->getProjectFile();
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
  messages_widget->prepareJob(prj->getProjectDir() + prj->getSubDir());
  (*messages_widget) << make_cmd << " messages &&  cd ../po && make merge";
  messages_widget->startJob();
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

  messages_widget->prepareJob(prj->getProjectDir() + prj->getSubDir());
  (*messages_widget) << "kdoc";
  (*messages_widget) << "-p -d" + prj->getProjectDir() + prj->getSubDir() +  "api";
  if (!sources.isEmpty())
      (*messages_widget) << sources;
  if (!link.isEmpty())
      {
          (*messages_widget) << ("-L" + idx_path);
          (*messages_widget) << link;
      }

  messages_widget->startJob();
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

		if((dlg.file).right(8) == ".docbook"){
			QFileInfo info(dlg.file);
			if(!CToolClass::searchProgram("db2html"))
	    	return;
			
			messages_widget->prepareJob(info.dirPath());
			(*messages_widget) << "db2html -d /usr/lib/sgml/stylesheets/kde.dsl ";
			(*messages_widget) << info.fileName();
			messages_widget->startJob();

		}
		else{
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
			messages_widget->prepareJob(info.dirPath());
			if(ksgml){
				(*messages_widget) << "ksgml2html";
				(*messages_widget) << info.fileName();
				(*messages_widget) << "en";
			}
			else{
				(*messages_widget) << "sgml2html";
				(*messages_widget) << info.fileName();
			}
			messages_widget->startJob();
		}
	}
}

void CKDevelop::slotProjectMakeDistSourceTgz(){
  if(!view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW)){
/*    QValueList<int> sizes;
    sizes << output_view_pos;
    view->setSizes(sizes);
*/
    o_tab_view->show();
    view_menu->setItemChecked(ID_VIEW_OUTPUTVIEW,true);
/*    QRect rMainGeom= view->geometry();
    view->resize(rMainGeom.width()-1,rMainGeom.height());
    view->resize(rMainGeom.width()+1,rMainGeom.height());
*/  }

  showOutputView(true);
  error_parser->toogleOff();
  setToolMenuProcess(false);
  slotFileSaveAll();
  slotStatusMsg(i18n("Running make dist..."));
  messages_widget->clear();
  messages_widget->prepareJob(prj->getProjectDir());
  (*messages_widget) << make_cmd << " dist";
  messages_widget->startJob();
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
  QListIterator<Component> it(components);
  for (; it.current(); ++it)
      (*it)->removedFileFromProject(rel_filename);
  refreshTrees();
}

bool CKDevelop::readProjectFile(QString file){
  QString str;
  QString extension;

  prj = new CProject(file);
  if(!(prj->readProject())){
    return false;
  }
  else {
      project=true;
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
  KMessageBox::information(0, i18n("You have added a new subdir to the project.\nWe will regenerate all Makefiles now."));
  setToolMenuProcess(false);
  slotStatusMsg(i18n("Running automake/autoconf and configure..."));
  messages_widget->clear();
  showOutputView(true);
  messages_widget->prepareJob(prj->getProjectDir());
  //(*messages_widget) << make_cmd << " -f Makefile.dist  && ./configure";
  QString flaglabel=(prj->getProjectType()=="normal_c") ? "CFLAGS=\"" : "CXXFLAGS=\"";
  (*messages_widget) << flaglabel;
  if (!prj->getCXXFLAGS().isEmpty() || !prj->getAdditCXXFLAGS().isEmpty())
  {
            if (!prj->getCXXFLAGS().isEmpty())
                  (*messages_widget) << prj->getCXXFLAGS() << " ";
            if (!prj->getAdditCXXFLAGS().isEmpty())
                  (*messages_widget) << prj->getAdditCXXFLAGS();
  }
  (*messages_widget)  << "\" " << "LDFLAGS=\" " ;
  if (!prj->getLDFLAGS().isEmpty())
                (*messages_widget) << prj->getLDFLAGS();
  (*messages_widget) << "\" ";
  (*messages_widget) << make_cmd << " -f Makefile.dist  && ./configure" << prj->getConfigureArgs();

  messages_widget->startJob();
}
