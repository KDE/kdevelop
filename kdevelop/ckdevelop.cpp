/***************************************************************************
                    ckdevelop.cpp - the main class in CKDevelop
                             -------------------                                         

    begin                : 20 Jul 1998                                        
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



#include <iostream.h>

#include <qclipboard.h>
#include <qevent.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qfont.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qtoolbar.h>

#include <kcursor.h>
#include <kfiledialog.h>
#include <kkeydialog.h>
#include <kmsgbox.h>
#include <ktabctl.h>

// #include <kio_linedit_dlg.h>

#include "ckdevelop.h"
#include "ckdevsetupdlg.h"
#include "cupdatekdedocdlg.h"
#include "ccreatedocdatabasedlg.h"
#include "ctoolclass.h"
#include "cdocbrowser.h"
#include "cfinddoctextdlg.h"
#include "debug.h"
#include "./kwrite/kwdoc.h"
#include "kswallow.h"


////////////
void openFileManagerWindow( const char * )
{
}
///////////


///////////////////////////////////////////////////////////////////////////////////////
// FILE-Menu slots
///////////////////////////////////////////////////////////////////////////////////////

void CKDevelop::slotFileNew(){
  
  slotStatusMsg(i18n("Creating new file..."));
  newFile(false);
  slotStatusMsg(IDS_DEFAULT);
}


void CKDevelop::slotFileOpen(){
  slotStatusMsg(i18n("Opening file..."));

  QString str;
  if(project){
    str = KFileDialog::getOpenFileName(prj->getProjectDir(),"*",this);
  }
  else{
    str = KFileDialog::getOpenFileName(0,"*",this);
  }  
  if (str.isEmpty()) return; //cancel
  switchToFile(str);
  
  slotStatusMsg(IDS_DEFAULT); 
  
}
void CKDevelop::slotFileClose(){
  slotStatusMsg(i18n("Closing file..."));
  TEditInfo* actual_info;
  int message_result;

  if(edit_widget->isModified()){
    message_result = KMsgBox::yesNoCancel(this,i18n("Save?"),
					  i18n("The document was modified,save?"),KMsgBox::QUESTION);
    if (message_result == 1){ // yes
      edit_widget->doSave();
    }
    if (message_result == 3){ // cancel
      slotStatusMsg(IDS_DEFAULT);
      return;
    }
  }

  //search the actual edit_info and remove it
  for(actual_info=edit_infos.first();actual_info != 0;actual_info=edit_infos.next()){
    if (actual_info->filename == edit_widget->getName()){ // found
      KDEBUG(KDEBUG_INFO,CKDEVELOP,"remove edit_info begin\n");
      menu_buffers->removeItem(actual_info->id);
      if(edit_infos.removeRef(actual_info)){
	KDEBUG(KDEBUG_INFO,CKDEVELOP,"remove edit_info end\n");
      }
    }
  }
  // add the next edit to the location
  for(actual_info=edit_infos.first();actual_info != 0;actual_info=edit_infos.next()){
    if ( getTabLocation(actual_info->filename) == getTabLocation(edit_widget->getName())){ // found
      edit_widget->setText(actual_info->text);
      edit_widget->toggleModified(actual_info->modified);
      edit_widget->setName(actual_info->filename);
      setCaption(actual_info->filename);
      KDEBUG1(KDEBUG_INFO,CKDEVELOP,"FOUND A NEXT %s",actual_info->filename.data());
      slotStatusMsg(IDS_DEFAULT);
      return;
    }
  }
  // if not found a successor create an new file
  actual_info = new TEditInfo;
  actual_info->modified=false;
  if (getTabLocation(edit_widget->getName()) == 0) {// header
    actual_info->id = menu_buffers->insertItem("Untitled.h",-2,0);
    actual_info->filename = "Untitled.h";
  }
  else{
    actual_info->id = menu_buffers->insertItem("Untitled.cpp",-2,0);
    actual_info->filename = "Untitled.cpp";
  }
  edit_infos.append(actual_info);

  edit_widget->clear();
  edit_widget->setName(actual_info->filename);
  setCaption(edit_widget->getName());

  slotStatusMsg(IDS_DEFAULT);
}

void CKDevelop::slotFileCloseAll(){
  slotStatusMsg(i18n("Closing all files..."));
  slotStatusMsg(IDS_DEFAULT);
}

void CKDevelop::slotFileSave(){

  QString filename=edit_widget->getName();
  slotStatusMsg(i18n("Saving file "+filename));

  if((filename== "Untitled.cpp") || (filename == "Untitled.h")){
    slotFileSaveAs();
  }
  else{
    edit_widget->doSave();
    // only refresh if header file changed
    if(getTabLocation(filename)==HEADER){
    slotViewRefresh();
    }
  }
  slotStatusMsg(IDS_DEFAULT);
}

void CKDevelop::slotFileSaveAs(){
  slotStatusMsg(i18n("Save file as..."));
  QString name;
  TEditInfo* actual_info;
  if(project){
    name = KFileDialog::getSaveFileName(prj->getProjectDir(),0,this,edit_widget->getName());
  }
  else{
    name = KFileDialog::getSaveFileName(0,0,this,edit_widget->getName());
  }
  if (name.isNull()){
    KDEBUG(KDEBUG_INFO,CKDEVELOP,"Cancel");
    return;
  }
  else {
    edit_widget->toggleModified(true);
    edit_widget->doSave(name); // try the save
    //     if (err == KEdit::KEDIT_OS_ERROR){
    //  cerr << "error";
    //  return; // no action
    // }
    // and now the modifications for the new file
    //search the actual edit_info
    for(actual_info=edit_infos.first();actual_info != 0;actual_info=edit_infos.next()){
      if (actual_info->filename == edit_widget->getName()){ // found
	
 	//update the info-struct in the list
	actual_info->filename = name;
	actual_info->modified = false;
	//update the widget
	edit_widget->setName(name);
	edit_widget->toggleModified(false);
	//update the menu
	QFileInfo fileinfo(name);
	menu_buffers->changeItem(fileinfo.fileName(),actual_info->id);
	//update kdevelop
	setCaption(name);
      }
    } // end_for
  } // end_else
  slotStatusMsg(IDS_DEFAULT);
}

void CKDevelop::slotFileSaveAll(){
  // ok,its a dirty implementation  :-)
  if(!bAutosave || saveTimer->isActive()){
    slotStatusMsg(i18n("Saving all changed files..."));
  }
  else{
    slotStatusMsg(i18n("Autosaving..."));
  }
  TEditInfo* actual_info;
  bool mod = false;
  // save current filename to switch back after saving
  QString visibleFile = edit_widget->getName();
  // ooops...autosave switches tabs...
  int visibleTab=s_tab_view->getCurrentTab();
  // first the 2 current edits
  edit_widget->setUpdatesEnabled(false);

  if(header_widget->isModified()){
    if(header_widget->getName() == "Untitled.h"){
      switchToFile("Untitled.h");
      slotFileSaveAs();
    }
    else{
      header_widget->doSave();
    }
    mod=true;
  }
  if(cpp_widget->isModified()){
    if(cpp_widget->getName() == "Untitled.cpp"){
      switchToFile("Untitled.cpp");
      slotFileSaveAs();
    }
    else{
      cpp_widget->doSave();
    }
    //    mod=true;
  }
  
  for(actual_info=edit_infos.first();actual_info != 0;actual_info=edit_infos.next()){
    KDEBUG1(KDEBUG_INFO,CKDEVELOP,"check file: %s",actual_info->filename.data());
    
    if(actual_info->modified){
      if((actual_info->filename == "Untitled.cpp") || (actual_info->filename == "Untitled.h")){
      	switchToFile(actual_info->filename);
      	slotFileSaveAs();
      	KDEBUG1(KDEBUG_INFO,CKDEVELOP,"file: %s UNTITLED",actual_info->filename.data());
      	mod = true;
      }
      else{
      	switchToFile(actual_info->filename);
      	edit_widget->doSave();
      	KDEBUG1(KDEBUG_INFO,CKDEVELOP,"file: %s ",actual_info->filename.data());
      	if(actual_info->filename.right(2)==".h" || actual_info->filename.right(4)==".hxx")
      	  mod = true;
      }
    }
  }
  if(mod){
    slotViewRefresh();
  }
  // switch back to visible file
  switchToFile(visibleFile);
  edit_widget->setUpdatesEnabled(true);
  edit_widget->repaint();
  // switch back to visible tab
  s_tab_view->setCurrentTab(visibleTab);
  slotStatusMsg(IDS_DEFAULT);
}


void CKDevelop::slotFilePrint(){
  QString file;
  slotFileSave();
  file = edit_widget->getName();
  CPrintDlg* printerdlg = new CPrintDlg(this, file, "suzus");
  printerdlg->resize(600,480);
  printerdlg->exec(); 
  delete (printerdlg);
}

void CKDevelop::slotFileQuit(){
  slotStatusMsg(i18n("Exiting..."));
  close();
}

///////////////////////////////////////////////////////////////////////////////////////
// EDIT-Menu slots
///////////////////////////////////////////////////////////////////////////////////////

void CKDevelop::slotEditUndo(){
  edit_widget->undo();
}
void CKDevelop::slotEditRedo(){
  edit_widget->redo();
}
void CKDevelop::slotEditCut(){
  slotStatusMsg(i18n("Cutting..."));
  edit_widget->cut();
  slotStatusMsg(IDS_DEFAULT); 
}
void CKDevelop::slotEditCopy(){
  slotStatusMsg(i18n("Copying..."));
  if(s_tab_view->getCurrentTab()==2){
    browser_widget->slotCopyText();
  }
	else
    edit_widget->copyText();
  slotStatusMsg(IDS_DEFAULT);
}
void CKDevelop::slotEditPaste(){
  slotStatusMsg(i18n("Pasting selection..."));
  edit_widget->paste();
  slotStatusMsg(IDS_DEFAULT); 
}
void CKDevelop::slotEditInsertFile(){
  slotStatusMsg(i18n("Inserting file contents..."));
  edit_widget->insertFile();
  slotStatusMsg(IDS_DEFAULT); 
}
void CKDevelop::slotEditSearch(){
  slotStatusMsg(i18n("Searching..."));
  edit_widget->search();
  slotStatusMsg(IDS_DEFAULT); 
}
void CKDevelop::slotEditRepeatSearch(){
  slotStatusMsg(i18n("Repeating last search..."));
  if(s_tab_view->getCurrentTab()==BROWSER){
    browser_widget->findTextNext(QRegExp(doc_search_text));
  }
  else{
    edit_widget->searchAgain();
  }
  slotStatusMsg(IDS_DEFAULT); 
}
void CKDevelop::slotEditReplace(){
  slotStatusMsg(i18n("Replacing..."));
  edit_widget->replace();
  slotStatusMsg(IDS_DEFAULT); 
}
void CKDevelop::slotEditSelectAll(){
  slotStatusMsg(i18n("Selecting all..."));
  edit_widget->selectAll();
  slotStatusMsg(IDS_DEFAULT);
}
void CKDevelop::slotEditInvertSelection(){
  edit_widget->invertSelection();
}
void CKDevelop::slotEditDeselectAll(){
  edit_widget->deselectAll();
}

///////////////////////////////////////////////////////////////////////////////////////
// VIEW-Menu slots
///////////////////////////////////////////////////////////////////////////////////////

void CKDevelop::slotViewGotoLine(){
  slotStatusMsg(i18n("Switching to selected line..."));
  edit_widget->gotoLine();
  slotStatusMsg(IDS_DEFAULT); 
}
void CKDevelop::slotViewTTreeView(){
  if(view_menu->isItemChecked(ID_VIEW_TREEVIEW)){
    view_menu->setItemChecked(ID_VIEW_TREEVIEW,false);
    tree_view_pos=top_panner->separatorPos();
    top_panner->setSeparatorPos(0);
  }
  else{
    top_panner->setSeparatorPos(tree_view_pos);
    view_menu->setItemChecked(ID_VIEW_TREEVIEW,true);
  }
  QRect rMainGeom= top_panner->geometry();
  top_panner->resize(rMainGeom.width()-1,rMainGeom.height());
  top_panner->resize(rMainGeom.width()+1,rMainGeom.height());
}
void CKDevelop::showTreeView(bool show){
  if(bAutoswitch)
  {
    if(show){
      if(view_menu->isItemChecked(ID_VIEW_TREEVIEW)){
        return; // it's already visible){
      }
      else{
        top_panner->setSeparatorPos(tree_view_pos);
        view_menu->setItemChecked(ID_VIEW_TREEVIEW,true);
      }
    }
    else{
      if(!view_menu->isItemChecked(ID_VIEW_TREEVIEW)){
        return; // it's already unvisible){
      }
      else{
        view_menu->setItemChecked(ID_VIEW_TREEVIEW,false);
        tree_view_pos=top_panner->separatorPos();
        top_panner->setSeparatorPos(0);
      }
    }
    QRect rMainGeom= top_panner->geometry();
    top_panner->resize(rMainGeom.width()-1,rMainGeom.height());
    top_panner->resize(rMainGeom.width()+1,rMainGeom.height());
   }
}
void CKDevelop::slotViewTOutputView(){
  if(view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW)){
    view_menu->setItemChecked(ID_VIEW_OUTPUTVIEW,false);
    output_view_pos=view->separatorPos();
    view->setSeparatorPos(100);
  }
  else{
    view->setSeparatorPos(output_view_pos);
    view_menu->setItemChecked(ID_VIEW_OUTPUTVIEW,true);
  }
  QRect rMainGeom= view->geometry();
  view->resize(rMainGeom.width()-1,rMainGeom.height());
  view->resize(rMainGeom.width()+1,rMainGeom.height());
}

void CKDevelop::showOutputView(bool show){
  if(bAutoswitch){
    if(show){
      if(view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW)){
        return; // it's already visible
      }
      else{
        view->setSeparatorPos(output_view_pos);
        view_menu->setItemChecked(ID_VIEW_OUTPUTVIEW,true);
      }
    }
    else{
      if(!view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW)){
        return; //it's already unvisible
      }
      else{
        view_menu->setItemChecked(ID_VIEW_OUTPUTVIEW,false);
        output_view_pos=view->separatorPos();
        view->setSeparatorPos(100);
      }
    }
    QRect rMainGeom= view->geometry();
    view->resize(rMainGeom.width()-1,rMainGeom.height());
    view->resize(rMainGeom.width()+1,rMainGeom.height());
  }
}
void CKDevelop::slotViewTStdToolbar(){
 if(view_menu->isItemChecked(ID_VIEW_TOOLBAR)){
   view_menu->setItemChecked(ID_VIEW_TOOLBAR,false);
    enableToolBar(KToolBar::Hide);
  }
  else{
    view_menu->setItemChecked(ID_VIEW_TOOLBAR,true);
    enableToolBar(KToolBar::Show);
  }

}
void CKDevelop::slotViewTBrowserToolbar(){
  if(view_menu->isItemChecked(ID_VIEW_BROWSER_TOOLBAR)){
    view_menu->setItemChecked(ID_VIEW_BROWSER_TOOLBAR,false);
    enableToolBar(KToolBar::Hide,ID_BROWSER_TOOLBAR);
  }
  else{
    view_menu->setItemChecked(ID_VIEW_BROWSER_TOOLBAR,true);
    enableToolBar(KToolBar::Show,ID_BROWSER_TOOLBAR);
  }
}

void CKDevelop::slotViewTStatusbar(){
  
  bViewStatusbar=!bViewStatusbar;
  view_menu->setItemChecked(ID_VIEW_STATUSBAR,bViewStatusbar);
  enableStatusBar();
  
}

void CKDevelop::slotViewRefresh(){
  refreshTrees();
}



///////////////////////////////////////////////////////////////////////////////////////
// BUILD-Menu slots
///////////////////////////////////////////////////////////////////////////////////////

void CKDevelop::slotBuildCompileFile(){
  if(!CToolClass::searchProgram(make_cmd)){
    return;
  }

  showOutputView(true);
  slotFileSave();
  setToolMenuProcess(false);
  slotStatusMsg(i18n("Compiling ")+edit_widget->getName());
  messages_widget->clear();
  process.clearArguments();
  QDir::setCurrent(prj->getProjectDir() + prj->getSubDir()); 
  // get the filename of the implementation file to compile and change extension for make
  QFileInfo fileinfo(cpp_widget->getName());
  KDEBUG1(KDEBUG_INFO,CKDEVELOP,"ObjectFile= %s",QString(fileinfo.baseName()+".o").data());
  //  cerr << "ObjectFile= " << fileinfo.baseName()+".o";
  process << make_cmd <<fileinfo.baseName()+".o";
  process.start(KProcess::NotifyOnExit,KProcess::AllOutput);
}

void CKDevelop::slotBuildRun(){
  slotBuildMake();
  slotStatusMsg(i18n("Running "+prj->getBinPROGRAM()));
  beep=false;
  next_job = "run";
}
void CKDevelop::slotBuildDebug(){

  if(!CToolClass::searchProgram("kdbg")){
    return;
  }
  if(!prj->getBinPROGRAM()){
    slotBuildMake();
  }
  showOutputView(false);
  
  slotStatusMsg(i18n("Running  "+prj->getBinPROGRAM()+"  in KDbg"));

  s_tab_view->setCurrentTab(TOOLS);
  swallow_widget->sWClose(false);
  QDir::setCurrent(prj->getProjectDir() + prj->getSubDir()); 
  swallow_widget->setExeString("kdbg "+ prj->getBinPROGRAM());
  swallow_widget->sWExecute();
  swallow_widget->init();
  
}

void CKDevelop::slotBuildMake(){
  if(!CToolClass::searchProgram(make_cmd)){
    return;
  }
  showOutputView(true);
  setToolMenuProcess(false);
  slotFileSaveAll();
  slotStatusMsg(i18n("Running make..."));
  messages_widget->clear();
  QDir::setCurrent(prj->getProjectDir() + prj->getSubDir()); 
  process.clearArguments();
  if(!prj->getMakeOptions().isEmpty()){
    process << make_cmd << prj->getMakeOptions();
  }
  else{
    process << make_cmd;
  }
  beep = true;
  process.start(KProcess::NotifyOnExit,KProcess::AllOutput);
}

// void CKDevelop::slotBuildMakeWith(){
//   KLineEditDlg *box = new KLineEditDlg(i18n("Make with :"), make_with_cmd.data(), this, true);
//   box->show();

//   if (!box->result())   /* cancelled */
//     return;

//   make_with_cmd = box->text();
//   delete box;

//   showOutputView(true);
//   setToolMenuProcess(false);
//   slotFileSaveAll();
//   slotStatusMsg(i18n("Running make..."));
//   messages_widget->clear();

//   if ( prj->getProjectType() == "normal_empty" ||
//        prj->getProjectType() == "normal_java")
//     QDir::setCurrent(prj->getProjectDir()); 
//   else
//     QDir::setCurrent(prj->getProjectDir() + prj->getSubDir()); 

//   process.clearArguments();
//   process << make_with_cmd;

//   beep = true;
//   process.start(KProcess::NotifyOnExit,KProcess::AllOutput);
// }

void CKDevelop::slotBuildRebuildAll(){
  if(!CToolClass::searchProgram(make_cmd)){
    return;
  }
  if(!view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW)){
    view->setSeparatorPos(output_view_pos);
    QRect rMainGeom= view->geometry();
    view->resize(rMainGeom.width()-1,rMainGeom.height());
    view->resize(rMainGeom.width()+1,rMainGeom.height());
  }
  showOutputView(true);

  setToolMenuProcess(false);
  slotFileSaveAll();
  slotStatusMsg(i18n("Running make clean-command "));
  messages_widget->clear();
  QDir::setCurrent(prj->getProjectDir() + prj->getSubDir()); 
  process.clearArguments();
  process << make_cmd;
  process << "clean";
  next_job = make_cmd; // checked in slotProcessExited()
  beep = true;
  process.start(KProcess::NotifyOnExit,KProcess::AllOutput);
}
void CKDevelop::slotBuildCleanRebuildAll(){
  if(!CToolClass::searchProgram(make_cmd)){
    return;
  }
  showOutputView(true);
  setToolMenuProcess(false);
  slotFileSaveAll();
  messages_widget->clear();
  slotStatusMsg(i18n("Running make clean and rebuilding all..."));
  QDir::setCurrent(prj->getProjectDir()); 
  shell_process.clearArguments();
  shell_process << make_cmd << "distclean && " << make_cmd 
		<< " -f Makefile.dist && ./configure &&" << make_cmd;
  beep = true;
  shell_process.start(KProcess::NotifyOnExit,KProcess::AllOutput);
}

void CKDevelop::slotBuildDistClean(){
  if(!CToolClass::searchProgram(make_cmd)){
    return;
  }
  showOutputView(true);
  setToolMenuProcess(false);
  slotFileSaveAll();
  slotStatusMsg(i18n("Running make distclean..."));
  messages_widget->clear();
  QDir::setCurrent(prj->getProjectDir());
  process.clearArguments();
  process << make_cmd << "distclean";
  process.start(KProcess::NotifyOnExit,KProcess::AllOutput);
}
void CKDevelop::slotBuildAutoconf(){
  if(!CToolClass::searchProgram("automake")){
    return;
  }
  if(!view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW)){
    view->setSeparatorPos(output_view_pos);
    view_menu->setItemChecked(ID_VIEW_OUTPUTVIEW,true);
    QRect rMainGeom= view->geometry();
    view->resize(rMainGeom.width()-1,rMainGeom.height());
    view->resize(rMainGeom.width()+1,rMainGeom.height());
  }

  showOutputView(true);

  setToolMenuProcess(false);
  slotFileSaveAll();
  slotStatusMsg(i18n("Running autoconf suite..."));
  messages_widget->clear();
  QDir::setCurrent(prj->getProjectDir());
  shell_process.clearArguments();
  shell_process << make_cmd << " -f Makefile.dist";
  shell_process.start(KProcess::NotifyOnExit,KProcess::AllOutput);
  beep = true;
}


void CKDevelop::slotBuildConfigure(){
  slotStatusMsg(i18n("Running ./configure..."));
  showOutputView(true);
  setToolMenuProcess(false);
  slotFileSave();
  messages_widget->clear();
  slotFileSaveAll();
  QDir::setCurrent(prj->getProjectDir()); 
  process.clearArguments();
  process << "./configure";
  process.start(KProcess::NotifyOnExit,KProcess::AllOutput);
   beep = true;
}
void CKDevelop::slotBuildStop(){
  slotStatusMsg(i18n("Killing current process..."));
  setToolMenuProcess(true);
  process.kill();
  shell_process.kill();
  appl_process.kill();
  slotStatusMsg(IDS_DEFAULT);
}
void CKDevelop::slotBuildAPI(){
  if(!CToolClass::searchProgram("kdoc")){
    return;
  }
  showOutputView(true);

  setToolMenuProcess(false);
  slotFileSaveAll();
  slotStatusMsg(i18n("Creating project API-Documentation..."));
  messages_widget->clear();
  config->setGroup("Doc_Location");
  QString doc_kde=config->readEntry("doc_kde");
  QString ref_file=doc_kde+"kdoc-reference/qt.kdoc";

  if(doc_kde.isEmpty() || !QFileInfo(ref_file).exists()){
    KMsgBox::message(this,i18n("Warning"),i18n("The KDE-library documentation is not installed.\n"
                                                "Please update your documentation with the options\n"
                                                "given in theKDevelop Setup dialog, Documentation tab.\n\n"
                                                "Your API-documentation will be created without\n"
                                                "cross-references to the KDE and Qt libraries."),KMsgBox::EXCLAMATION);

    QDir::setCurrent(prj->getProjectDir() + prj->getSubDir());
    shell_process.clearArguments();
    shell_process << "kdoc";
    shell_process << "-p -d" + prj->getProjectDir() + prj->getSubDir() +  "api";
    shell_process << prj->getProjectName();
    shell_process << "*.h";
    shell_process.start(KShellProcess::NotifyOnExit,KShellProcess::AllOutput);
  }
  else{
    QDir::setCurrent(prj->getProjectDir() + prj->getSubDir());
    shell_process.clearArguments();
    shell_process << "kdoc";
    shell_process << "-p -d" + prj->getProjectDir() + prj->getSubDir() +  "api";
    shell_process << "-ufile:" + prj->getProjectDir() + prj->getSubDir() +  "api"+"/";
    shell_process << "-L" + doc_kde + "kdoc-reference";
    shell_process << prj->getProjectName();
    shell_process << "*.h";
    shell_process << "-lqt -lkdecore -lkdeui -lkfile -lkfmlib -lkhtmlw -ljscript -lkab -lkspell";
    shell_process.start(KShellProcess::NotifyOnExit,KShellProcess::AllOutput);
  }
}

void CKDevelop::slotBuildManual(){
  if(!CToolClass::searchProgram("sgml2html")){
    return;
  }
  showOutputView(true);

  setToolMenuProcess(false);
  //  slotFileSaveAll();
  slotStatusMsg(i18n("Creating project Manual..."));
  messages_widget->clear();
  QDir::setCurrent(prj->getProjectDir() + prj->getSubDir() + "/docs/en/");
  process.clearArguments();
  process << "sgml2html";
  process << prj->getSGMLFile();
  process.start(KProcess::NotifyOnExit,KProcess::AllOutput); 

}

///////////////////////////////////////////////////////////////////////////////////////
// TOOLS-Menu slots
///////////////////////////////////////////////////////////////////////////////////////

void CKDevelop::slotToolsKIconEdit(){

  if(!CToolClass::searchProgram("kiconedit")){
    return;
  }

  showOutputView(false);

  s_tab_view->setCurrentTab(TOOLS);
  swallow_widget->sWClose(false);
  swallow_widget->setExeString("kiconedit");
  swallow_widget->sWExecute();
  swallow_widget->init();
}

void CKDevelop::slotToolsKDbg(){

  if(!CToolClass::searchProgram("kdbg")){
    return;
  }

  showOutputView(false);

  s_tab_view->setCurrentTab(TOOLS);
  swallow_widget->sWClose(false);
  swallow_widget->setExeString("kdbg");
  swallow_widget->sWExecute();
  swallow_widget->init();
}


void CKDevelop::slotToolsKTranslator(){
  if(!CToolClass::searchProgram("ktranslator")){
    return;
  }

  showOutputView(false);

  s_tab_view->setCurrentTab(TOOLS);
  swallow_widget->sWClose(false);
  swallow_widget->setExeString("ktranslator");
  swallow_widget->sWExecute();
  swallow_widget->init();

}

///////////////////////////////////////////////////////////////////////////////////////
// OPTIONS-Menu slots
///////////////////////////////////////////////////////////////////////////////////////

void CKDevelop::slotOptionsEditor(){
  slotStatusMsg(i18n("Setting up the Editor..."));
  cpp_widget->optDlg();
  header_widget->copySettings(cpp_widget);
  config->setGroup("KWrite Options");
  edit_widget->writeConfig(config);
  edit_widget->doc()->writeConfig(config);
  slotStatusMsg(IDS_DEFAULT);

}
void CKDevelop::slotOptionsEditorColors(){
  slotStatusMsg(i18n("Setting up the Editor's colors..."));
  cpp_widget->colDlg();
  header_widget->copySettings(cpp_widget);
  config->setGroup("KWrite Options");
  edit_widget->writeConfig(config);
  edit_widget->doc()->writeConfig(config);
  slotStatusMsg(IDS_DEFAULT);

}


void CKDevelop::slotOptionsSyntaxHighlightingDefaults(){
  slotStatusMsg(i18n("Setting up syntax highlighting default colors..."));
  cpp_widget->hlDef();
  header_widget->copySettings(cpp_widget);
  config->setGroup("KWrite Options");
  edit_widget->writeConfig(config);
  edit_widget->doc()->writeConfig(config);
  slotStatusMsg(IDS_DEFAULT);
}
void CKDevelop::slotOptionsSyntaxHighlighting(){
  slotStatusMsg(i18n("Setting up syntax highlighting colors..."));
  cpp_widget->hlDlg();
  header_widget->copySettings(cpp_widget);
  config->setGroup("KWrite Options");
  edit_widget->writeConfig(config);
  edit_widget->doc()->writeConfig(config);
  slotStatusMsg(IDS_DEFAULT);
}
void CKDevelop::slotOptionsDocBrowser(){
   slotStatusMsg(i18n("Configuring Documentation Browser..."));

   CDocBrowserOptionsDlg browserOptions;

   connect( browserOptions.fontOptions, SIGNAL(fontSize(int)),
		 browser_widget, SLOT(slotDocFontSize( int )) );
   connect( browserOptions.fontOptions, SIGNAL(standardFont( const char * )),
		 browser_widget, SLOT(slotDocStandardFont( const char * )) );
   connect( browserOptions.fontOptions, SIGNAL(fixedFont( const char * )),
		 browser_widget, SLOT(slotDocFixedFont( const char * )) );
   connect( browserOptions.colorOptions, SIGNAL(colorsChanged(const QColor&, const QColor&,
			const QColor&, const QColor&, const bool, const bool)),
		 browser_widget, SLOT(slotDocColorsChanged(const QColor&, const QColor&,
            		const QColor&, const QColor&, const bool, const bool)) );

   browserOptions.show();
   slotStatusMsg(IDS_DEFAULT);
}

void CKDevelop::slotOptionsConfigureEnscript(){
  if (!CToolClass::searchProgram("enscript")) {
    return;
  }
  enscriptconf = new CConfigEnscriptDlg(this, "confdialog");
  enscriptconf->resize(610,510);
  enscriptconf->exec();
  delete (enscriptconf);
}

void CKDevelop::slotOptionsConfigureA2ps(){
  if (!CToolClass::searchProgram("a2ps")) {
    return;
  }
  a2psconf = new CConfigA2psDlg(this, "confdialog");
  a2psconf->resize(600,430);
  a2psconf->exec();
  delete (a2psconf);
}

void CKDevelop::slotOptionsKDevelop(){
  slotStatusMsg(i18n("Setting up Documentation paths..."));

  CKDevSetupDlg* setup= new CKDevSetupDlg(this,"Setup",accel);
  setup->show();
  slotStatusMsg(IDS_DEFAULT);
}
// slots needed by the KDevelop Setup
void CKDevelop::slotOptionsMake(){
  config->setGroup("General Options");
  make_cmd=config->readEntry("Make","make");

}

void CKDevelop::slotOptionsAutosave(bool autosave){

  bAutosave=autosave;
  if(bAutosave)
    saveTimer->start(saveTimeout);
  else
    saveTimer->stop();
}

void CKDevelop::slotOptionsAutosaveTime(int time){
  switch(time){
  case 0:
    saveTimeout=3*60*1000;
    break;
  case 1:
    saveTimeout=5*60*1000;
    break;
  case 2:
    saveTimeout=15*60*1000;
    break;
  case 3:
    saveTimeout=30*60*1000;
    break;
  }
}

void CKDevelop::slotOptionsAutoswitch(bool autoswitch){
  bAutoswitch=autoswitch;
}

void CKDevelop::slotOptionsDefaultCV(bool defaultcv){
  bDefaultCV=defaultcv;
}

void CKDevelop::slotOptionsUpdateKDEDocumentation(){
  if(!CToolClass::searchProgram("kdoc")){
    return;
  }
  slotStatusMsg(i18n("Updating KDE-Libs documentation..."));
  config->setGroup("Doc_Location");
  CUpdateKDEDocDlg dlg(this,"test",&shell_process, config);
  if(dlg.exec()){
    slotStatusMsg(i18n("Generating Documentation..."));
    setToolMenuProcess(false);
  }
}
void CKDevelop::slotOptionsCreateSearchDatabase(){
  if(!CToolClass::searchProgram("glimpseindex")){
    return;
  }
  CCreateDocDatabaseDlg dlg(this,"DLG",&shell_process,config);
  if(dlg.exec()){
    slotStatusMsg(i18n("Creating Search Database..."));
  }

  return;

}



///////////////////////////////////////////////////////////////////////////////////////
// HELP-Menu slots
///////////////////////////////////////////////////////////////////////////////////////
void CKDevelop::slotHelpBack(){
  slotStatusMsg(i18n("Switching to last page..."));
  QString str = history_list.prev();
  if (str != 0){
    s_tab_view->setCurrentTab(BROWSER);
    browser_widget->showURL(str);
    enableCommand(ID_HELP_FORWARD);
  }
  if (history_list.prev() == 0){ // no more backs
    disableCommand(ID_HELP_BACK);
    history_list.first(); // set it at first
  }
  else{
    history_list.next();
  }
  KDEBUG1(KDEBUG_INFO,CKDEVELOP,"COUNT HISTORYLIST: %d",history_list.count());
  slotStatusMsg(IDS_DEFAULT);
}

void CKDevelop::slotHelpForward(){
  slotStatusMsg(i18n("Switching to next page..."));
  QString str = history_list.next();
  if (str != 0){
    s_tab_view->setCurrentTab(BROWSER);
    browser_widget->showURL(str);
    enableCommand(ID_HELP_BACK);
  }
  if (history_list.next() == 0){ // no more forwards
   disableCommand(ID_HELP_FORWARD);
    history_list.last(); // set it at last
  }
  else{
    history_list.prev();
  }
  slotStatusMsg(IDS_DEFAULT);
}

void CKDevelop::slotHelpSearchText(QString text){
  if(!CToolClass::searchProgram("glimpse")){
    return;
  }
  slotStatusMsg(i18n("Searching selected text in documentation..."));
  doc_search_text = text.copy(); // save the text

  search_output = ""; // delete all from the last search
  search_process.clearArguments();
  search_process << "glimpse  -H "+ KApplication::localkdedir()+"/share/apps" + "/kdevelop -U -c -y '"+ text +"'";
  search_process.start(KShellProcess::NotifyOnExit,KShellProcess::AllOutput);
}
void CKDevelop::slotHelpSearchText(){
  QString text;
  if(s_tab_view->getCurrentTab()==BROWSER){
    browser_widget->getSelectedText(text);
  }
  else{
    text = edit_widget->markedText();
    if(text == ""){
      text = edit_widget->currentWord();
    }
  }
  slotHelpSearchText(text);
}

void CKDevelop::slotHelpSearch(){
  slotStatusMsg(i18n("Searching for Help on..."));
  CFindDocTextDlg* help_srch_dlg=new CFindDocTextDlg(this,"Search_for_Help_on");
  connect(help_srch_dlg,SIGNAL(signalFind(QString)),this,SLOT(slotDocSText(QString)));
  help_srch_dlg->exec();
}

void CKDevelop::slotHelpReference(){
  KLocale *kloc = KApplication::getKApplication()->getLocale();

  QString strpath = KApplication::kde_htmldir().copy() + "/";
  QString file;
  // first try the locale setting
  file = strpath + kloc->language() + '/' + "kdevelop/reference/C/cref.html";
  if( !QFileInfo( file ).exists() ){
  // not found: use the default
  	file = strpath + "default/" + "kdevelop/reference/C/cref.html";
  }
  if( !QFileInfo( file ).exists() ){
  file = strpath + kloc->language() + '/' + "kdevelop/cref.html";
  }
  if( !QFileInfo( file ).exists() ){
    // not found: use the default
    file = strpath + "default/" + "kdevelop/cref.html";
  }
  slotURLSelected(browser_widget,"file:" + file,1,"test");
}
void CKDevelop::slotHelpQtLib(){
  config->setGroup("Doc_Location");
  slotURLSelected(browser_widget,"file:" + config->readEntry("doc_qt") + "index.html",1,"test");
}
void CKDevelop::slotHelpKDECoreLib(){
  config->setGroup("Doc_Location");
  slotURLSelected(browser_widget,"file:" + config->readEntry("doc_kde") + "kdecore/index.html",1,"test");
}
void CKDevelop::slotHelpKDEGUILib(){
  config->setGroup("Doc_Location");
  slotURLSelected(browser_widget,"file:" +  config->readEntry("doc_kde") + "kdeui/index.html",1,"test");
}
void CKDevelop::slotHelpKDEKFileLib(){
  config->setGroup("Doc_Location");
  slotURLSelected(browser_widget,"file:" +  config->readEntry("doc_kde") + "kfile/index.html",1,"test");
}
void CKDevelop::slotHelpKDEHTMLLib(){
  config->setGroup("Doc_Location");
	QString file=config->readEntry("doc_kde") + "khtmlw/index.html";
  if( !QFileInfo( file ).exists() ){
    // not found: use khtml
    file =config->readEntry("doc_kde") + "khtml/index.html";
  }
  slotURLSelected(browser_widget,"file:" +file  ,1,"test");
}


void CKDevelop::slotHelpAPI(){
  if(project){
    slotStatusMsg(i18n("Switching to project API Documentation..."));
    slotURLSelected(browser_widget,prj->getProjectDir() + prj->getSubDir() +  "api/index.html",1,"test");
    slotStatusMsg(IDS_DEFAULT);
  }
}
void CKDevelop::slotHelpManual(){
  if(project){
    slotStatusMsg(i18n("Switching to project Manual..."));
    unsigned int index = prj->getSGMLFile().length()-4;
    QString name = prj->getSGMLFile().copy();
    name.remove(index,4);
    slotURLSelected(browser_widget,prj->getProjectDir() + prj->getSubDir() + "docs/en/" + name + "html",1,"test");
    slotStatusMsg(IDS_DEFAULT);
  }
}

void CKDevelop::slotHelpContents(){
  
  KLocale *kloc = KApplication::getKApplication()->getLocale();
  
  QString strpath = KApplication::kde_htmldir().copy() + "/";
  QString file;
  // first try the locale setting
  file = strpath + kloc->language() + '/' + "kdevelop/index.html";
  
  if( !QFileInfo( file ).exists() ){
    // not found: use the default
    file = strpath + "default/" + "kdevelop/index.html";
  }
  slotURLSelected(browser_widget,"file:" + file,1,"test");
}
void CKDevelop::slotHelpHomepage(){
  //  slotURLSelected(browser_widget,"http://anakonda.alpha.org/~smeier/kdevelop/index.html",1,"test");
}
void CKDevelop::slotHelpAbout(){
  KMsgBox::message(this,i18n("About KDevelop..."),i18n("\t   KDevelop Version "+version+" \n\n\t(c) 1998,1999 KDevelop Team \n
Sandy Meier <smeier@rz.uni-potsdam.de>
Stefan Heidrich <sheidric@rz.uni-potsdam.de>
Stefan Bartel <bartel@rz.uni-potsdam.de>
Ralf Nolden <Ralf.Nolden@post.rwth-aachen.de>
                             
KDevelop contains sourcecode from KWrite 0.98 
(c) by Jochen Wilhelmy <digisnap@cs.tu-berlin.de>
"));

}


/////////////////////////////////////////////////////////////////////
// Other slots and functions needed
/////////////////////////////////////////////////////////////////////

void CKDevelop::slotStatusMsg(const char *text)
{
  ///////////////////////////////////////////////////////////////////
  // change status message permanently
  statusBar()->clear();
  statusBar()->changeItem(text, ID_STATUS_MSG );
}


void CKDevelop::slotStatusHelpMsg(const char *text)
{
  ///////////////////////////////////////////////////////////////////
  // change status message of whole statusbar temporary (text, msec)
  statusBar()->message(text, 2000);
}

void CKDevelop::enableCommand(int id_)
{
  menuBar()->setItemEnabled(id_,true);
  toolBar()->setItemEnabled(id_,true);
  toolBar(ID_BROWSER_TOOLBAR)->setItemEnabled(id_,true);
}

void CKDevelop::disableCommand(int id_)
{
  menuBar()->setItemEnabled(id_,false);
  toolBar()->setItemEnabled(id_,false);
  toolBar(ID_BROWSER_TOOLBAR)->setItemEnabled(id_,false);
}

void CKDevelop::slotNewStatus()
{
  int config;
  config = edit_widget->config();
  statusBar()->changeItem(config & cfOvr ? "OVR" : "INS",ID_STATUS_INS_OVR);
}

void CKDevelop::slotNewLineColumn()
{
  QString linenumber;
  linenumber.sprintf(i18n("Line: %d Col: %d"), 
     			edit_widget->currentLine() +1,
			edit_widget->currentColumn() +1);
  statusBar()->changeItem(linenumber.data(), ID_STATUS_LN_CLM);
} 
void CKDevelop::slotNewUndo(){
  int state;
  state = edit_widget->undoState();
  //undo
  if(state & 1){
    enableCommand(ID_EDIT_UNDO);
  }
  else{
    disableCommand(ID_EDIT_UNDO);
  }
  //redo
  if(state & 2){
    enableCommand(ID_EDIT_REDO);
  }
  else{
    disableCommand(ID_EDIT_REDO);
  }
  
}


void CKDevelop::slotURLSelected(KHTMLView* ,const char* url,int,const char*){
  showOutputView(false);
  s_tab_view->setCurrentTab(BROWSER);
  browser_widget->setFocus();
  QString url_str = url;
  if(url_str.contains("kdevelop/search_result.html") != 0){
    browser_widget->showURL(url,true); // with reload if equal
  }
  else{
    browser_widget->showURL(url); // without reload if equal
  }
  if (!history_list.isEmpty()){
    enableCommand(ID_HELP_BACK);
  }

  setCaption("KDevelop " + version + ":  "+url);

  QString str = history_list.current();
  //if it's a url-request from the search result jump to the correct point
  if (str.contains("kdevelop/search_result.html")){
    prev_was_search_result=true; // after this time, jump to the searchkey
  }
  // insert into the history-list
  int cur =  history_list.at(); // get the current index
  if(cur == -1){
     history_list.append(url);
  }
  else{
    history_list.insert(cur+1,url);
  }
}

void CKDevelop::slotURLonURL(KHTMLView*, const char *url )
{
	if ( url )
	{
		statusBar()->changeItem(url,ID_STATUS_MSG);
	}
	else
	{
		statusBar()->changeItem(IDS_DEFAULT, ID_STATUS_MSG);
	}
}

void CKDevelop::slotDocumentDone( KHTMLView *_view ){
  if(prev_was_search_result){
    browser_widget->findTextBegin();
    browser_widget->findTextNext(QRegExp(doc_search_text));
  }
  prev_was_search_result=false;

}

void CKDevelop::slotReceivedStdout(KProcess*,char* buffer,int buflen){
  int x,y;
  messages_widget->cursorPosition(&x,&y);
  QString str(buffer,buflen+1);
  messages_widget->insertAt(str,x,y);
  o_tab_view->setCurrentTab(MESSAGES);
}
void CKDevelop::slotReceivedStderr(KProcess*,char* buffer,int buflen){
  int x,y;
  messages_widget->cursorPosition(&x,&y);
  QString str(buffer,buflen+1);
  messages_widget->insertAt(str,x,y);
  o_tab_view->setCurrentTab(MESSAGES);
}
void CKDevelop::slotApplReceivedStdout(KProcess*,char* buffer,int buflen){
  int x,y;
  showOutputView(true);
  stdin_stdout_widget->cursorPosition(&x,&y);
  QString str(buffer,buflen+1);
  stdin_stdout_widget->insertAt(str,x,y);
}
void CKDevelop::slotApplReceivedStderr(KProcess*,char* buffer,int buflen){
  int x,y;
  showOutputView(true);
  stderr_widget->cursorPosition(&x,&y);
  QString str(buffer,buflen+1);
  stderr_widget->insertAt(str,x,y);
}


void CKDevelop::slotSearchReceivedStdout(KProcess* proc,char* buffer,int buflen){
  QString str(buffer,buflen+1);
  search_output = search_output + str;
}
void CKDevelop::slotSearchProcessExited(KProcess*){
  //  cerr << search_output;
  int pos=0;
  int nextpos=0;
  QStrList list;
  QStrList sort_list;
  QString str;
  QString found_str;
  int i=0;
  int max=0;

  while((nextpos = search_output.find('\n',pos)) != -1){
    str = search_output.mid(pos,nextpos-pos);
    list.append(str);
    pos = nextpos+1;
  }
  if (list.isEmpty()){

     KMsgBox::message(0,i18n("Not found!"),"\"" + doc_search_text + i18n("\" not found in documentation!"),KMsgBox::INFORMATION);
    return;
  }

  // //lets sort it a little bit
   for(;i<30;i++){
    max =0;
    found_str = "";
    for(str = list.first();str != 0;str = list.next()){
      if (searchToolGetNumber(str) >= max){
	found_str = str.copy();
	max = searchToolGetNumber(str);
      }
    }
    if(found_str != ""){
      sort_list.append(found_str);
      list.remove(found_str);
    }

  }

   QString filename = KApplication::localkdedir()+"/share/apps" + "/kdevelop/search_result.html";
   QFile file(filename);
   QTextStream stream(&file);
   file.open(IO_WriteOnly);

   stream << "<HTML><HEAD></HEAD><BODY BGCOLOR=\"#ffffff\"><BR> <TABLE><TR><TH>Title<TH>Hits\n";
   QString numstr;
   for(str = sort_list.first();str != 0;str = sort_list.next() ){
     stream << "<TR><TD><A HREF=\""+searchToolGetURL(str)+"\">"+
			   searchToolGetTitle(str)+"</A><TD>"+
			   numstr.setNum(searchToolGetNumber(str)) + "\n";
   }

   stream << "\n</TABLE></BODY></HTML>";

   file.close();
   slotURLSelected(browser_widget,"file:" + filename,1,"test");

}
QString CKDevelop::searchToolGetTitle(QString str){
  int pos = str.find(' ');
  pos = str.find(' ',pos);
  int end_pos = str.find(':',pos);
  return str.mid(pos,end_pos-pos);
}
QString CKDevelop::searchToolGetURL(QString str){
  int pos = str.find(' ');
  return str.left(pos);
}
int CKDevelop::searchToolGetNumber(QString str){
  int pos =str.findRev(':');
  QString sub = str.right((str.length()-pos-2));
  return sub.toInt();
}
void CKDevelop::slotKeyPressedOnStdinStdoutWidget(int key){
  char a = key;
  appl_process.writeStdin(&a,1);
}
void CKDevelop::slotClickedOnMessagesWidget(){
  int x,y;
  int error_line;
  QString text;
  QString error_line_str;
  QString error_filename;
  int pos1,pos2; // positions in the string
  QRegExp reg(":[0-9]*:"); // is it an error line?, I hope it works

  messages_widget->cursorPosition(&x,&y);
  text = messages_widget->textLine(x);
  if((pos1=reg.match(text)) == -1) return; // not an error line

  // extract the error-line
  pos2 = text.find(':',pos1+1);
  error_line_str = text.mid(pos1+1,pos2-pos1-1);
  error_line = error_line_str.toInt();

  // extract the filename
  pos2 = text.findRev(' ',pos1);
  if (pos2 == -1) {
    pos2 = 0; // the filename is at the begining of the string
  }
  else { pos2++; }

  error_filename = text.mid(pos2,pos1-pos2);

  // switch to the file
  if (error_filename.find('/') == -1){ // it is a file outer the projectdir ?
    error_filename = prj->getProjectDir() + prj->getSubDir() + error_filename;
  }
  if (QFile::exists(error_filename)){
    switchToFile(error_filename);
    edit_widget->setCursorPosition(error_line-1,0);
    edit_widget->setFocus();
  }

}
void CKDevelop::slotProcessExited(KProcess* proc){
  setToolMenuProcess(true);
  slotStatusMsg(IDS_DEFAULT);
  bool ready = true;
  if (process.normalExit()) {
    if (next_job == make_cmd){ // rest from the rebuild all
      QDir::setCurrent(prj->getProjectDir() + prj->getSubDir());
      process.clearArguments();
      if(!prj->getMakeOptions().isEmpty()){
      	process << make_cmd << prj->getMakeOptions();
      }
      else{
      	process << make_cmd;
      }
      setToolMenuProcess(false);
      process.start(KProcess::NotifyOnExit,KProcess::AllOutput);
      next_job = "";
      ready=false;
    }
    if (next_job == "run" && process.exitStatus() == 0){ // rest from the buildRun
      QDir::setCurrent(prj->getProjectDir() + prj->getSubDir());
      stdin_stdout_widget->clear();
      stderr_widget->clear();
      if(prj->getProjectType() == "normal_cpp"){
      	o_tab_view->setCurrentTab(STDINSTDOUT);
      }
      else{
      	o_tab_view->setCurrentTab(STDERR);
      }

      appl_process.clearArguments();
      // Warning: not every user has the current directory in his path !
      appl_process << "./" + prj->getBinPROGRAM().lower();
      setToolMenuProcess(false);
      appl_process.start(KProcess::NotifyOnExit,KProcess::All);
      next_job = "";
      ready = false;
    }
    if (next_job == "refresh"){ // rest from the add projectfile
      refreshTrees();
    }
  }
  else {
    KDEBUG(KDEBUG_WARN,CKDEVELOP,"process exited with error(s)...");
  }
  if(beep && ready){
    XBell(kapp->getDisplay(),100); //beep :-)
    beep = false;
  }
}


void CKDevelop::slotTTabSelected(int item){
  if(item == DOC ){
    // disable the outputview
    showOutputView(false);
  }
}
void CKDevelop::slotSTabSelected(int item){
  lasttab = s_tab_view->getCurrentTab();

  if (item == HEADER){
    if(bAutoswitch && t_tab_view->getCurrentTab()==DOC){	
      if ( bDefaultCV)
	t_tab_view->setCurrentTab(CV);
      else
	t_tab_view->setCurrentTab(LFV);
    }
    disableCommand(ID_BUILD_COMPILE_FILE);
    edit_widget = header_widget;
    edit_widget->setFocus();
    slotNewUndo();
    slotNewStatus();
    slotNewLineColumn();
		if(project)
	    setCaption("KDevelop " + version + ": " +prj->getProjectName()+":  "+ edit_widget->getName());
		else
	    setCaption("KDevelop " + version + ": " + edit_widget->getName());

  }
  if (item == CPP){
    if(bAutoswitch && t_tab_view->getCurrentTab()==DOC){	
      if ( bDefaultCV)
	t_tab_view->setCurrentTab(CV);
      else
	t_tab_view->setCurrentTab(LFV);
    }
    if(project && build_menu->isItemEnabled(ID_BUILD_MAKE)){
      enableCommand(ID_BUILD_COMPILE_FILE);
    }
    edit_widget = cpp_widget;
    edit_widget->setFocus();
    slotNewUndo();
    slotNewStatus();
    slotNewLineColumn();
		if(project)
	    setCaption("KDevelop " + version + ": " +prj->getProjectName()+":  "+ edit_widget->getName());
		else
	    setCaption("KDevelop " + version + ": " + edit_widget->getName());

  }
  if(item == BROWSER){
    if(bAutoswitch)
      t_tab_view->setCurrentTab(DOC);
    disableCommand(ID_BUILD_COMPILE_FILE);
    browser_widget->setFocus();
    setCaption("KDevelop " + version + ":  "+ browser_widget->currentURL());
  }
  if(item == TOOLS){
		disableCommand(ID_BUILD_COMPILE_FILE);
	}
  //  s_tab_current = item;

}

void CKDevelop::slotMenuBuffersSelected(int id){
  TEditInfo* info;

  for(info=edit_infos.first();info != 0;info=edit_infos.next()){
    if (info->id == id){
      switchToFile(info->filename);
      return; // if found than return
    }
  }
}


void CKDevelop::slotLogFileTreeSelected(int index){
  // no action on Project or Group click with left button
  //  cerr << "SELECTED\n";
  if (log_file_tree->itemAt(index)->hasChild() == true) return; // no action on child
  if(!(log_file_tree->isFile(index))) return; // it is not file

  if(!log_file_tree->leftButton()) return; // right button: return
  KPath* path;
  QString* str;
  path = log_file_tree->itemPath(index);
  str = path->pop();



  switchToFile(prj->getProjectDir() + *str);
  //  cerr << "SELECTED2\n";
}

void CKDevelop::slotRealFileTreeSelected(int index){
  if (real_file_tree->itemAt(index)->hasChild() == true) return; // no action
  KPath* path = real_file_tree->itemPath(index);
  QString str_path;
  QString* str;
  //cerr << index;
  str = path->pop();
  str_path =  *str + str_path;
  while ((str = path->pop()) != 0){
    if (str->right(1) == "/"){
      str_path =  *str + str_path;
    }
    else{
      str_path =  *str + "/"+ str_path;
    }
  }
  //  cerr << str_path;
  switchToFile(str_path);

}

void CKDevelop::slotDocTreeSelected(int index){
  QString string = doc_tree->itemAt(index)->getText() ;
  if( string == "Documentation"){
    slotHelpContents();
    return;
  }
  if (doc_tree->itemAt(index)->hasChild() == true) return; // no action
  KPath* path;
  QString* str;
  path = doc_tree->itemPath(index);
  str = path->pop();

  KLocale *kloc = KApplication::getKApplication()->getLocale();

  QString strpath = KApplication::kde_htmldir().copy() + "/";
  QString file;

  config->setGroup("Doc_Location");
  if(*str == i18n("Tutorial") ){
    // first try the locale setting
    file = strpath + kloc->language() + '/' + "kdevelop/tutorial.html";
    if( !QFileInfo( file ).exists() ){
      // not found: use the default
      file = strpath + "default/" + "kdevelop/tutorial.html";
    }

    slotURLSelected(browser_widget,"file:" + file,1,"test");
    return;
  }
  if(*str == i18n("Manual") ){
    // first try the locale setting
    file = strpath + kloc->language() + '/' + "kdevelop/index.html";

    if( !QFileInfo( file ).exists() ){
      // not found: use the default
      file = strpath + "default/" + "kdevelop/index.html";
    }
    slotURLSelected(browser_widget,"file:" + file,1,"test");
    return;
  }
  if(*str == i18n("C/C++ Reference") ){
    // first try the locale setting
    file = strpath + kloc->language() + '/' + "kdevelop/reference/C/cref.html";

    if( !QFileInfo( file ).exists() ){
      // not found: use the default
      file = strpath + "default/" + "kdevelop/reference/C/cref.html";
    }
	  if( !QFileInfo( file ).exists() ){
  		// show the translated error page
  		file = strpath + kloc->language() + '/' + "kdevelop/cref.html";
  	}
    if( !QFileInfo( file ).exists() ){
      // not found: use the default error page
      file = strpath + "default/" + "kdevelop/cref.html";
    }
    slotURLSelected(browser_widget,"file:" + file,1,"test");
    return;
  }
  if(*str == i18n("Qt-Library") ){
    slotURLSelected(browser_widget,"file:" +config->readEntry("doc_qt") + "index.html",1,"test");
    return;
  }
  if(*str == i18n("KDE-Core-Library") ){
    slotURLSelected(browser_widget,"file:" + config->readEntry("doc_kde") + "kdecore/index.html",1,"test");
    return;
  }
  if(*str == i18n("KDE-UI-Library") ){
    slotURLSelected(browser_widget,"file:" + config->readEntry("doc_kde") + "kdeui/index.html",1,"test");
    return;
  }
  if(*str == i18n("KDE-KFile-Library") ){
    slotURLSelected(browser_widget,"file:" + config->readEntry("doc_kde") + "kfile/index.html",1,"test");
    return;
  }
  if(*str == i18n("KDE-KHTMLW-Library") ){
    slotURLSelected(browser_widget,"file:" + config->readEntry("doc_kde") + "khtmlw/index.html",1,"test");
     return;
  }
  if(*str == i18n("KDE-KHTML-Library") ){
    slotURLSelected(browser_widget,"file:" + config->readEntry("doc_kde") + "khtml/index.html",1,"test");
     return;
  }
  if(*str == i18n("KDE-KFM-Library") ){
    slotURLSelected(browser_widget,"file:" + config->readEntry("doc_kde") + "kfmlib/index.html",1,"test");
     return;
  }
  if(*str == i18n("KDE-KDEutils-Library") ){
    slotURLSelected(browser_widget,"file:" + config->readEntry("doc_kde") + "kdeutils/index.html",1,"test");
     return;
  }
  if(*str == i18n("KDE-KAB-Library") ){
    slotURLSelected(browser_widget,"file:" + config->readEntry("doc_kde") + "kab/index.html",1,"test");
     return;
  }
  if(*str == i18n("KDE-KSpell-Library") ){
    slotURLSelected(browser_widget,"file:" + config->readEntry("doc_kde") + "kspell/index.html",1,"test");
     return;
  }
  if(*str == i18n("API-Documentation") ){
    slotHelpAPI();
     return;
  }
  if(*str == i18n("User-Manual") ){
    slotHelpManual();
     return;
  }
  //
  config->setGroup("Other_Doc_Location");
  QFileInfo file_info(config->readEntry(*str));
  if(file_info.isFile()){
    slotURLSelected(browser_widget,"file:" + config->readEntry(*str),1,"test");
  }

}

void CKDevelop::slotTCurrentTab(int item){
  t_tab_view->setCurrentTab(item);
}

void CKDevelop::slotSCurrentTab(int item){
  s_tab_view->setCurrentTab(item);
}

void CKDevelop::slotShowC() {
  s_tab_view->setCurrentTab( CPP );
}

void CKDevelop::slotShowHeader() {
  s_tab_view->setCurrentTab( HEADER );
}

void CKDevelop::slotShowHelp() {
  s_tab_view->setCurrentTab( BROWSER );
}

void CKDevelop::slotShowTools() {
  s_tab_view->setCurrentTab( TOOLS );
}

void CKDevelop::slotToggleLast() {
  if ( lasttab != s_tab_view->getCurrentTab() )
    s_tab_view->setCurrentTab( lasttab );
  else
    switchToFile( lastfile );
}

void CKDevelop::slotBufferMenu( const QPoint& point ) {
  menu_buffers->popup( point );
}

void CKDevelop::closeEvent(QCloseEvent* e){
  config->setGroup("General Options");
  config->writeEntry("width",width());
  config->writeEntry("height",height());

  config->writeEntry("view_panner_pos",view->separatorPos());
  config->writeEntry("top_panner_pos",top_panner->separatorPos());

  config->writeEntry("tree_view_pos",tree_view_pos);
  config->writeEntry("output_view_pos",output_view_pos);

  config->writeEntry("show_tree_view",view_menu->isItemChecked(ID_VIEW_TREEVIEW));
  config->writeEntry("show_output_view",view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW));

  config->writeEntry("show_std_toolbar",view_menu->isItemChecked(ID_VIEW_TOOLBAR));
  config->writeEntry("show_browser_toolbar",view_menu->isItemChecked(ID_VIEW_BROWSER_TOOLBAR));
  config->writeEntry("show_statusbar",view_menu->isItemChecked(ID_VIEW_STATUSBAR));
  config->writeEntry("LastActiveTab", s_tab_view->getCurrentTab());
  config->writeEntry("LastActiveTree", t_tab_view->getCurrentTab());

  config->writeEntry("Autosave",bAutosave);
  config->writeEntry("Autosave Timeout",saveTimeout);

  config->writeEntry("Make",make_cmd);

  config->setGroup("Files");
  config->writeEntry("cpp_file",cpp_widget->getName());
  config->writeEntry("header_file",header_widget->getName());
  config->writeEntry("browser_file",history_list.current());

  config->setGroup("Files");
  config->writeEntry("project_file","");
  if(project){
    config->writeEntry("project_file",prj->getProjectFile());
    prj->setCurrentWorkspaceNumber(workspace);
    saveCurrentWorkspaceIntoProject();
    prj->writeProject();
    if(!slotProjectClose()){ // if not ok,pressed cancel
      e->ignore();
      return; //not close!
    }
  }
  e->accept();
  swallow_widget->sWClose(false);

  config->sync();
  KDEBUG(KDEBUG_INFO,CKDEVELOP,"KTMainWindow::closeEvent()");
  KTMainWindow::closeEvent(e);
}

void CKDevelop::slotToolbarClicked(int item){
  switch (item) {
/*  case ID_FILE_NEW:
    slotFileNewFile();
    break;
*/
  case ID_PROJECT_OPEN:
    slotProjectOpen();
    break;
  case ID_FILE_OPEN:
    slotFileOpen();
    break;
  case ID_FILE_SAVE:
    slotFileSave();
    break;
  case ID_FILE_SAVE_ALL:
    slotFileSaveAll();
    break;
  case ID_FILE_PRINT:
    slotFilePrint();
    break;
  case ID_EDIT_UNDO:
    slotEditUndo();
    break;
  case ID_EDIT_REDO:
    slotEditRedo();
    break;
  case ID_EDIT_COPY:
    slotEditCopy();
    break;
  case ID_EDIT_PASTE:
    slotEditPaste();
    break;
  case ID_EDIT_CUT:
    slotEditCut();
    break;
  case ID_VIEW_REFRESH:
    slotViewRefresh();
    break;
  case ID_BUILD_COMPILE_FILE:
  	slotBuildCompileFile();
  	break;
  case ID_BUILD_MAKE:
    slotBuildMake();
    break;
  case ID_BUILD_REBUILD_ALL:
  	slotBuildRebuildAll();
  	break;

  case ID_BUILD_DEBUG:
    slotBuildDebug();
    break;
  case ID_BUILD_RUN:
    slotBuildRun();
    break;
  case ID_BUILD_STOP:
    slotBuildStop();
    break;
  case ID_HELP_BACK:
    slotHelpBack();
    break;
  case ID_HELP_FORWARD:
    slotHelpForward();
    break;
  case ID_HELP_SEARCH_TEXT:
    slotHelpSearchText();
    break;
  case ID_HELP_SEARCH:
    slotHelpSearch();
    break;
  }
}


BEGIN_STATUS_MSG(CKDevelop)
  ON_STATUS_MSG(ID_FILE_NEW,                              i18n("Creates a new file"))
  ON_STATUS_MSG(ID_FILE_OPEN,   							            i18n("Opens an existing file"))
  ON_STATUS_MSG(ID_FILE_CLOSE,       						          i18n("Closes the actual file"))

  ON_STATUS_MSG(ID_FILE_SAVE,        						          i18n("Save the actual document"))
  ON_STATUS_MSG(ID_FILE_SAVE_AS,     						          i18n("Save the document as..."))
  ON_STATUS_MSG(ID_FILE_SAVE_ALL,    						          i18n("Save all changed files"))

  ON_STATUS_MSG(ID_FILE_PRINT,       						          i18n("Prints the current document"))

//  ON_STATUS_MSG(ID_FILE_CLOSE_WINDOW,i18n("Closes the current window"))

  ON_STATUS_MSG(ID_FILE_QUIT,        						          i18n("Exits the program"))


  ON_STATUS_MSG(ID_EDIT_CUT,                     			    i18n("Cuts the selected section and puts it to the clipboard"))
  ON_STATUS_MSG(ID_EDIT_COPY,                    			    i18n("Copys the selected section to the clipboard"))
  ON_STATUS_MSG(ID_EDIT_PASTE,                   			    i18n("Pastes the clipboard contents to actual position"))
  ON_STATUS_MSG(ID_EDIT_SELECT_ALL,              			    i18n("Selects the whole document contents"))
  ON_STATUS_MSG(ID_EDIT_INSERT_FILE,             			    i18n("Inserts a file at the current position"))
  ON_STATUS_MSG(ID_EDIT_SEARCH,                  			    i18n("Searchs the file for an expression"))
  ON_STATUS_MSG(ID_EDIT_REPEAT_SEARCH,           			    i18n("Repeats the last search"))
  ON_STATUS_MSG(ID_EDIT_REPLACE,                 			    i18n("Searchs and replace expression"))


  ON_STATUS_MSG(ID_VIEW_GOTO_LINE,               			    i18n("Goes to Line Number..."))

  ON_STATUS_MSG(ID_VIEW_TREEVIEW,              				    i18n("Enables / disables the treeview"))
  ON_STATUS_MSG(ID_VIEW_OUTPUTVIEW,            				    i18n("Enables / disables the outputview"))

  ON_STATUS_MSG(ID_VIEW_TOOLBAR,                  			  i18n("Enables / disables the standard toolbar"))
  ON_STATUS_MSG(ID_VIEW_BROWSER_TOOLBAR,       				    i18n("Enables / disables the browser toolbar"))
  ON_STATUS_MSG(ID_VIEW_STATUSBAR,             				    i18n("Enables / disables the statusbar"))

  ON_STATUS_MSG(ID_VIEW_REFRESH,                			    i18n("Refreshes current view"))


  ON_STATUS_MSG(ID_PROJECT_KAPPWIZARD,            			  i18n("Generates a new project with KAppWizard"))
  ON_STATUS_MSG(ID_PROJECT_NEW,                   			  i18n("Creates a new project"))
  ON_STATUS_MSG(ID_PROJECT_OPEN,			            	      i18n("Opens an existing project"))
  ON_STATUS_MSG(ID_PROJECT_CLOSE,                 			  i18n("Closes the current project"))
  ON_STATUS_MSG(ID_PROJECT_ADD_FILE,              			  i18n("Adds a file to the current project"))
  ON_STATUS_MSG(ID_PROJECT_ADD_FILE_NEW,          			  i18n("Adds a new file to the project"))
  ON_STATUS_MSG(ID_PROJECT_ADD_FILE_EXIST,        			  i18n("Adds an existing file to the project"))
  ON_STATUS_MSG(ID_PROJECT_REMOVE_FILE,           			  i18n("Removes file from the project"))

  ON_STATUS_MSG(ID_PROJECT_NEW_CLASS,             			  i18n("Creates a new Class frame structure and files"))

  ON_STATUS_MSG(ID_PROJECT_FILE_PROPERTIES,       			  i18n("Shows the current file properties"))
  ON_STATUS_MSG(ID_PROJECT_OPTIONS,               			  i18n("Sets project and compiler options"))

  
  ON_STATUS_MSG(ID_BUILD_COMPILE_FILE,                    i18n("Compiles the current sourcefile"))
  ON_STATUS_MSG(ID_BUILD_MAKE,                    			  i18n("Invokes make-command"))
  ON_STATUS_MSG(ID_BUILD_REBUILD_ALL,             			  i18n("Rebuilds the program"))
  ON_STATUS_MSG(ID_BUILD_CLEAN_REBUILD_ALL,       			  i18n("Invokes make clean and rebuild all"))
  ON_STATUS_MSG(ID_BUILD_STOP,                    			  i18n("Stops make immediately"))
  ON_STATUS_MSG(ID_BUILD_RUN,                     			  i18n("Invokes make-command and runs the program"))
  ON_STATUS_MSG(ID_BUILD_DEBUG,                   			  i18n("Invokes make and KDbg debugging the binary"))
  ON_STATUS_MSG(ID_BUILD_DISTCLEAN,               			  i18n("Invokes make distclean and deletes all compiled files"))
  ON_STATUS_MSG(ID_BUILD_AUTOCONF,                			  i18n("Invokes automake and co."))
  ON_STATUS_MSG(ID_BUILD_CONFIGURE,               			  i18n("Invokes ./configure"))
  ON_STATUS_MSG(ID_BUILD_MAKE_PROJECT_API,        			  i18n("Creates the Project's API with KDoc"))
  ON_STATUS_MSG(ID_BUILD_MAKE_USER_MANUAL,        			  i18n("Creates the Project's User Manual with the sgml-file"))

  ON_STATUS_MSG(ID_TOOLS_KDBG,                    			  i18n("Starts KDbg in the tools window"))
  ON_STATUS_MSG(ID_TOOLS_KTRANSLATOR,             			  i18n("Starts KTranslator in the tools window"))
  ON_STATUS_MSG(ID_TOOLS_KICONEDIT,               			  i18n("Starts KIconedit in the tools window"))

  ON_STATUS_MSG(ID_OPTIONS_EDITOR,              			    i18n("Sets the Editor's behavoir"))
  ON_STATUS_MSG(ID_OPTIONS_EDITOR_COLORS,       			    i18n("Sets the Editor's colors"))
  ON_STATUS_MSG(ID_OPTIONS_SYNTAX_HIGHLIGHTING_DEFAULTS,  i18n("Sets the highlighting default colors"))
  ON_STATUS_MSG(ID_OPTIONS_SYNTAX_HIGHLIGHTING, 			    i18n("Sets the highlighting colors"))
  ON_STATUS_MSG(ID_OPTIONS_DOCBROWSER,     	  				    i18n("Configures the Browser options"))
  ON_STATUS_MSG(ID_OPTIONS_PRINT,       			            i18n("Configures printing options"))
  ON_STATUS_MSG(ID_OPTIONS_PRINT_ENSCRIPT,       	        i18n("Configures the printer to use enscript"))
  ON_STATUS_MSG(ID_OPTIONS_PRINT_A2PS,       			        i18n("Configures the printer to use a2ps"))
  ON_STATUS_MSG(ID_OPTIONS_KDEVELOP,              		    i18n("Configures KDevelop"))

  ON_STATUS_MSG(ID_HELP_BACK,                      			  i18n("Switchs to last browser page"))
  ON_STATUS_MSG(ID_HELP_FORWARD,                   			  i18n("Switchs to next browser page"))

  ON_STATUS_MSG(ID_HELP_SEARCH_TEXT,              				  i18n("Searchs the selected text in the documentation"))
  ON_STATUS_MSG(ID_HELP_SEARCH,                           i18n("Lets you search individually for an expression"))

  ON_STATUS_MSG(ID_HELP_CONTENTS,                  			  i18n("Switch to KDevelop's User Manual"))

  ON_STATUS_MSG(ID_HELP_REFERENCE,                			  i18n("Switchs to the C/C++-Reference"))
  ON_STATUS_MSG(ID_HELP_QT_LIBRARY,                			  i18n("Switchs to the QT-Documentation"))
  ON_STATUS_MSG(ID_HELP_KDE_CORE_LIBRARY,          			  i18n("Switchs to the KDE-Core-Documentation"))
  ON_STATUS_MSG(ID_HELP_KDE_GUI_LIBRARY,           			  i18n("Switchs to the KDE-GUI-Documentation"))
  ON_STATUS_MSG(ID_HELP_KDE_KFILE_LIBRARY,          			  i18n("Switchs to the KDE-File-Documentation"))
  ON_STATUS_MSG(ID_HELP_KDE_HTML_LIBRARY,          			  i18n("Switchs to the KDE-Html-Documentation"))
  ON_STATUS_MSG(ID_HELP_PROJECT_API,		           			  i18n("Switchs to the project's API-Documentation"))
  ON_STATUS_MSG(ID_HELP_USER_MANUAL,               			  i18n("Switchs to the project's User-Manual"))

  ON_STATUS_MSG(ID_HELP_HOMEPAGE,                 			  i18n("Enter the KDevelop Homepage"))
  ON_STATUS_MSG(ID_HELP_ABOUT,                    			  i18n("Programmer's Hall of Fame..."))

END_STATUS_MSG()

























































































