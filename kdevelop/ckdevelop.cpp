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
#include <qdir.h>
#include <qclipbrd.h>
#include <qevent.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qfont.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qtoolbar.h>
#include <qmessagebox.h>
#include <qsplitter.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kmenubar.h>
#include <kcursor.h>
#include <kfiledialog.h>
#include <kkeydialog.h>
#include <kmessagebox.h>
#include <ktabctl.h>
#include <stdlib.h>
#include <kstddirs.h>
#include <kmenubar.h>
#include <qtabdialog.h>

#include "./kdlgedit/kdlgedit.h"
#include "./kdlgedit/kdlgpropwidget.h"
#include "./kdlgedit/kdlgproplv.h"
#include "ctoolsconfigdlg.h"
#include "ktipofday.h"

// #include <kio_linedit_dlg.h>

#include "ckdevelop.h"
#include "ckdevsetupdlg.h"
#include "cupdatekdedocdlg.h"
#include "ccreatedocdatabasedlg.h"
#include "ctoolclass.h"
#include "cdocbrowser.h"
#include "doctreeview.h"
#include "makeview.h"
#include "grepview.h"
#include "ceditwidget.h"
#include "cfinddoctextdlg.h"
#include "cexecuteargdlg.h"
#include "./kwrite/kwdoc.h"
#include "./kwrite/kwdialog.h"
#include "kswallow.h"
#include "cerrormessageparser.h"
#include "cbugreportdlg.h"
#include "../config.h"
#include "structdef.h"
#include "vc/versioncontrol.h"
#include "print/cconfigenscriptdlg.h"
#include "print/cconfiga2psdlg.h"
#include "plugins/pluginmanagerdlg.h"


extern KGuiCmdManager cmdMngr;

////////////////////////
// editor commands
///////////////////////
void CKDevelop::doCursorCommand(int cmdNum) {
  if (edit_widget != 0L) edit_widget->doCursorCommand(cmdNum);
}

void CKDevelop::doEditCommand(int cmdNum) {
  if (edit_widget != 0L) edit_widget->doEditCommand(cmdNum);
}

void CKDevelop::doStateCommand(int cmdNum) {
  if (edit_widget != 0L) edit_widget->doStateCommand(cmdNum);
}



///////////////////////////////////////////////////////////////////////////////////////
// FILE-Menu slots
///////////////////////////////////////////////////////////////////////////////////////

void CKDevelop::slotFileNew(){
  
  slotStatusMsg(i18n("Creating new file..."));
  newFile(false);
  slotStatusMsg(i18n("Ready."));
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
  if (!str.isEmpty()) // nocancel
  {
    switchToFile(str);
  }

  slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotFileOpen( int id_ ){
  slotStatusMsg(i18n("Opening file..."));

  QString str=file_open_list.at(id_);
	
  switchToFile(str);

  slotStatusMsg(i18n("Ready."));

}

void CKDevelop::slotFileClose(){

    // if edit_widget isn't shown don't proceed
    if (edit_widget==0l)
       return;

    slotStatusMsg(i18n("Closing file..."));
    QString filename = edit_widget->getName();
    int msg_result;

    if(edit_widget->isModified()){
	
	// no autosave if the user intends to save a file
	if (bAutosave)
	    saveTimer->stop();
	
	msg_result = KMessageBox::warningYesNoCancel(this, i18n("The document was modified,save?"));
	// restart autosaving
	if (bAutosave)
	    saveTimer->start(saveTimeout);
	
	if (msg_result == KMessageBox::Yes){ // yes
	
	    if (isUntitled(filename))
		{
		    if (!fileSaveAs())
			msg_result=KMessageBox::Cancel;    // simulate here cancel because fileSaveAs failed....
		}
	    else
		{
		    saveFileFromTheCurrentEditWidget();
		    if (edit_widget->isModified())
			msg_result=KMessageBox::Cancel;		   // simulate cancel because doSave went wrong!
		}
	}
	
	if (msg_result == KMessageBox::Cancel) // cancel
	    {
		setInfoModified(filename, edit_widget->isModified());
		slotStatusMsg(i18n("Ready."));
		return;
	    }
    }

    removeFileFromEditlist(filename);
    setMainCaption();
    slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotFileCloseAll()
{
  slotStatusMsg(i18n("Closing all files..."));
  TEditInfo* actual_info;
  QStrList handledNames;
  bool cont=true;

  setInfoModified(header_widget->getName(), header_widget->isModified());
  setInfoModified(cpp_widget->getName(), cpp_widget->isModified());

  for(actual_info=edit_infos.first();cont && actual_info != 0;)
  {
    TEditInfo *next_info=edit_infos.next();
    if(actual_info->modified && handledNames.contains(actual_info->filename)<1)
    {
#warning FIXME: QMessageBox has 3 buttons maximum
#if 0
        KMsgBox *files_close=new KMsgBox(this,i18n("Save changed files ?"),
                                         i18n("The project\n\n%1\n\ncontains changed files."
                                              "Save modified file\n\n%2?\n\n")
                                         .arg(prj->getProjectName).arg(actual_info->filename),
                                         KMsgBox::QUESTION,
                                         i18n("Yes"), i18n("No"), i18n("Save all"), i18n("Cancel"));

      // show the messagea and store result in result:

      files_close->show();

      int result=files_close->result();
#else
      int result = 0; // until problem above is resolved
#endif

      // create the save project messagebox

      // what to do
      if(result==1) // Yes- only save the actual file
      {
        // save file as if Untitled and close file
        if(isUntitled(actual_info->filename))
        {
          switchToFile(actual_info->filename);
          handledNames.append(actual_info->filename);
	        cont=fileSaveAs();
          next_info=edit_infos.first(); // start again... 'cause we deleted an entry
        }				
        else // Save file and close it
        {
          switchToFile(actual_info->filename);
          handledNames.append(actual_info->filename);
          slotFileSave();
          actual_info->modified=edit_widget->isModified();
          cont=!actual_info->modified; //something went wrong
        }
      }

      if(result==2) // No - no save but close
      {
        handledNames.append(actual_info->filename);
        actual_info->modified=false;
        removeFileFromEditlist(actual_info->filename); // immediate remove
        next_info=edit_infos.first(); // start again... 'cause we deleted an entry
      }

      if(result==3) // Save all
      {
        slotFileSaveAll();
        break;
      }

      if(result==4) // Cancel
      {
        cont=false;
	      break;
      }	
    }  // end actual file close

    actual_info=next_info;
  } // end for-loop

  // check if something went wrong with saving
  if ( cont )
  {
    for( actual_info=edit_infos.first();
         cont && actual_info != 0;
         actual_info=edit_infos.next())
    {
      if ( actual_info->modified )
        cont=false;
    } // end for-loop

    if(cont)
    {
      header_widget->clear();
      cpp_widget->clear();
      menu_buffers->clear();

      //clear all edit_infos before starting a new project
      edit_infos.clear();

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
    }
  }

  slotStatusMsg(i18n("Ready."));
}

bool CKDevelop::saveFileFromTheCurrentEditWidget(){
  // if edit_widget isn't shown don't proceed
  if (edit_widget==0l)
     return false;

  QString filename=edit_widget->getName();
  TEditInfo* actual_info;
  QFileInfo file_info(filename);
  
  for(actual_info=edit_infos.first();actual_info != 0;actual_info=edit_infos.next()){
      if (actual_info->filename == filename ){
	  break;
      }
  }
  if(actual_info == 0) return false; //oops :-(

  if(file_info.lastModified() != actual_info->last_modified){
    if (KMessageBox::questionYesNo(this, 
                                   i18n("The file %1 was modified outside\n this editor. Save anyway?").arg(filename))
        == QMessageBox::No)
        return false;
  }
  edit_widget->doSave();
  QFileInfo file_info2(filename);
  actual_info->last_modified = file_info2.lastModified();
  return true;
}


void CKDevelop::slotFileSave(){
  // if edit_widget isn't shown don't proceed
  if (edit_widget==0l)
     return;

  QString filename=edit_widget->getName();
  QString sShownFilename=QFileInfo(filename).fileName();
  slotStatusMsg(i18n("Saving file ")+sShownFilename);
  
  if(isUntitled(filename)){
    slotFileSaveAs();
  }
  else{
      saveFileFromTheCurrentEditWidget(); // save the current file
      setInfoModified(filename, edit_widget->isModified());
      // only refresh if header file changed
      if(CProject::getType(filename)==CPP_HEADER){
	  slotViewRefresh();
      }
  }
  slotStatusMsg(i18n("Ready."));
  QString sHelpMsg = edit_widget->isModified()? i18n("File %1 not saved.") : i18n("File %1 saved.");
  slotStatusHelpMsg(sHelpMsg.arg(sShownFilename));
}

void CKDevelop::slotFileSaveAs(){
    slotStatusMsg(i18n("Save file as..."));
    
    fileSaveAs();

    setMainCaption();
    slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotFileSaveAll(){
    QStrList handledNames;
    // ok,its a dirty implementation  :-)
    if(!bAutosave || !saveTimer->isActive()){
	slotStatusMsg(i18n("Saving all changed files..."));
    }
    else{
	slotStatusMsg(i18n("Autosaving..."));
    }
    TEditInfo* actual_info;
    bool mod = false;
    // save current filename to switch back after saving
    QString visibleFile = (edit_widget) ? edit_widget->getName() : QString("");
    // ooops...autosave switches tabs...
    int visibleTab=s_tab_view->getCurrentTab();
    // first the 2 current edits
    view->setUpdatesEnabled(false);
    
    setInfoModified(header_widget->getName(), header_widget->isModified());
    setInfoModified(cpp_widget->getName(), cpp_widget->isModified());
    
    statProg->setTotalSteps(edit_infos.count());
    statProg->show();
    statProg->setProgress(0);
    int i=0;
    for(actual_info=edit_infos.first();actual_info != 0;){

	TEditInfo *next_info=edit_infos.next();
	// get now the next info... fileSaveAs can delete the actual_info
	i++;
	statProg->setProgress(i);
	if(actual_info->modified && handledNames.contains(actual_info->filename)<1){
	    if(isUntitled(actual_info->filename)){
		switchToFile(actual_info->filename);
		handledNames.append(actual_info->filename);
		if (fileSaveAs())
		    {
			// maybe saved with another name... so we have to start again
			next_info=edit_infos.first();
			i=0;
		    }
	    
	       
	    }
	    else{
		switchToFile(actual_info->filename,false,false);
		handledNames.append(actual_info->filename);
		saveFileFromTheCurrentEditWidget();
		actual_info->modified=edit_widget->isModified();
		// 	KDEBUG1(KDEBUG_INFO,CKDEVELOP,"file: %s ",actual_info->filename.data());
		if(actual_info->filename.right(2)==".h" || actual_info->filename.right(4)==".hxx")
		    mod = true;
	    }
	}
	actual_info=next_info;
    }
    statProg->hide();
    statProg->reset();
    if(mod){
	slotViewRefresh();
    }
    // switch back to visible file
    if (visibleTab == CPP || visibleTab == HEADER)
	{
	    // Does the visible file still exist??
	    for(actual_info=edit_infos.first();actual_info != 0 && actual_info->filename != visibleFile;
		actual_info=edit_infos.next());
	    
	    if (actual_info)
		switchToFile(visibleFile,false,false); // no force reload and no box if modified outside
	    
	    view->repaint();
	}
    view->setUpdatesEnabled(true);
    // switch back to visible tab
    s_tab_view->setCurrentTab(visibleTab);
    slotStatusMsg(i18n("Ready."));
}


void CKDevelop::slotFilePrint(){
  // if edit_widget isn't shown don't proceed
  if (edit_widget==0l)
       return;

  QString file;
  slotFileSave();
/*  if (s_tab_view->getCurrentTab() == BROWSER) {
  	file = browser_widget->currentURL();
  	CPrintDlg* printerdlg = new CPrintDlg(this, file, "suzus", TRUE);
 	printerdlg->resize(600,480);
	  printerdlg->exec();
  	delete (printerdlg);
  }
  else { */
	  file = edit_widget->getName();
	  CPrintDlg* printerdlg = new CPrintDlg(this, file, "suzus");
	  printerdlg->resize(600,480);
	  printerdlg->exec();
  	delete (printerdlg);
//	}
}

void CKDevelop::slotFileQuit(){
  slotStatusMsg(i18n("Exiting..."));
  saveOptions();
  close();
}

///////////////////////////////////////////////////////////////////////////////////////
// EDIT-Menu slots
///////////////////////////////////////////////////////////////////////////////////////

void CKDevelop::slotEditUndo(){
 if(edit_widget != 0L)
   edit_widget->undo();
}


void CKDevelop::slotEditRedo(){
 if(edit_widget != 0L)
  edit_widget->redo();
}
void CKDevelop::slotEditUndoHistory(){
    if(edit_widget != 0L){
	edit_widget->undoHistory();
    }
}

void CKDevelop::slotEditCut(){
  // if edit_widget isn't shown don't proceed
  if (edit_widget==0l)
     return;

  slotStatusMsg(i18n("Cutting..."));
  edit_widget->cut();
  slotStatusMsg(i18n("Ready."));
}


void CKDevelop::slotEditCopy(){
//slotStatusMsg(i18n("Copying..."));
  if(s_tab_view->getCurrentTab() == 2) {
    slotStatusMsg(i18n("Copying..."));
    browser_widget->slotCopyText();
    slotStatusMsg(i18n("Ready."));
  }
//	else
//    edit_widget->copyText();
//slotStatusMsg(i18n("Ready."));
}


void CKDevelop::slotEditPaste(){
  // if edit_widget isn't shown don't proceed
  if (edit_widget==0l)
     return;
  slotStatusMsg(i18n("Pasting selection..."));
  edit_widget->paste();
  slotStatusMsg(i18n("Ready."));
}


void CKDevelop::slotEditInsertFile(){
  // if edit_widget isn't shown don't proceed
  if (edit_widget==0l)
     return;
  slotStatusMsg(i18n("Inserting file contents..."));
  edit_widget->insertFile();
  slotStatusMsg(i18n("Ready."));
}


void CKDevelop::slotEditSearch(){
  // if edit_widget isn't shown don't proceed
  if (edit_widget==0l)
     return;
  slotStatusMsg(i18n("Searching..."));
  edit_widget->search();
  slotStatusMsg(i18n("Ready."));
}


void CKDevelop::slotEditRepeatSearch(){
  slotStatusMsg(i18n("Repeating last search..."));
  if(s_tab_view->getCurrentTab()==BROWSER){
    browser_widget->findTextNext(QRegExp(doc_search_text));
  }
  else{
  // if edit_widget isn't shown don't proceed
    if (edit_widget!=0l)
     edit_widget->searchAgain();
  }
  slotStatusMsg(i18n("Ready."));
}


void CKDevelop::slotEditSearchInFiles(){
  slotStatusMsg(i18n("Searching in Files..."));
  grepview->showDialog();
  slotStatusMsg(i18n("Ready."));
}


void CKDevelop::slotEditSearchInFiles(QString search){
  slotStatusMsg(i18n("Searching in Files..."));
  grepview->showDialogWithPattern(search);
  slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotEditSearchText(){
  QString text;
  if(s_tab_view->getCurrentTab()==BROWSER){
    browser_widget->getSelectedText(text);
  }
  else{
     // if edit_widget isn't shown don't proceed
    if (edit_widget!=0l)
    {
      text = edit_widget->markedText();
      if(text == ""){
        text = edit_widget->currentWord();
      }
    }
  }

  if (!text.isEmpty())
    slotEditSearchInFiles(text);
  slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotEditReplace(){
  // if edit_widget isn't shown don't proceed
  if (edit_widget==0l)
     return;

  slotStatusMsg(i18n("Replacing..."));
  edit_widget->replace();
  slotStatusMsg(i18n("Ready."));
}


void CKDevelop::slotEditSpellcheck(){
  if (edit_widget!=0l)
	edit_widget->spellcheck();
}


void CKDevelop::slotEditSelectAll(){
  // if edit_widget isn't shown don't proceed
  if (edit_widget==0l)
     return;
  slotStatusMsg(i18n("Selecting all..."));
  edit_widget->selectAll();
  slotStatusMsg(i18n("Ready."));
}





void CKDevelop::slotEditDeselectAll(){
  if (edit_widget!=0l)
    edit_widget->deselectAll();
}

///////////////////////////////////////////////////////////////////////////////////////
// VIEW-Menu slots
///////////////////////////////////////////////////////////////////////////////////////

void CKDevelop::slotViewGotoLine(){
  // if edit_widget isn't shown don't proceed
  if (edit_widget==0l)
     return;
  slotStatusMsg(i18n("Switching to selected line..."));
  edit_widget->gotoLine();
  slotStatusMsg(i18n("Ready."));
}


/** jump to the next error, based on the make output*/
void CKDevelop::slotViewNextError(){
  TErrorMessageInfo info = error_parser->getNext();
  if(info.filename != ""){
#warning FIXME
#if 0
    messages_widget->setCursorPosition(info.makeoutputline-1,0);
    switchToFile(info.filename,info.errorline-1);
    if(!bKDevelop){
      switchToKDevelop();
    }
    slotStatusMsg(messages_widget->textLine(info.makeoutputline-1));
#endif
  }
  else{
    XBell(kapp->getDisplay(),100); // not a next found, beep
  }

  //enable/disable the menus/toolbars
  if(error_parser->hasNext()){
    enableCommand(ID_VIEW_NEXT_ERROR);
  }
  else{
    disableCommand(ID_VIEW_NEXT_ERROR);
  }
  
  if(error_parser->hasPrev()){
    enableCommand(ID_VIEW_PREVIOUS_ERROR);
  }
  else{
    disableCommand(ID_VIEW_PREVIOUS_ERROR);
  }
}


/** jump to the previews error, based on the make output*/
void CKDevelop::slotViewPreviousError(){
  TErrorMessageInfo info = error_parser->getPrev();
  if(info.filename != ""){
#warning FIXME
#if 0
    messages_widget->setCursorPosition(info.makeoutputline-1,0);
    switchToFile(info.filename,info.errorline-1);
    if(!bKDevelop){
      switchToKDevelop();
    }
    slotStatusMsg(messages_widget->textLine(info.makeoutputline-1));
#endif
  }
  else{
    XBell(kapp->getDisplay(),100); // not a previous found, beep
  }
  //enable/disable the menus/toolbars
  if(error_parser->hasNext()){
    enableCommand(ID_VIEW_NEXT_ERROR);
  }
  else{
    disableCommand(ID_VIEW_NEXT_ERROR);
  }
  
  if(error_parser->hasPrev()){
    enableCommand(ID_VIEW_PREVIOUS_ERROR);
  }
  else{
    disableCommand(ID_VIEW_PREVIOUS_ERROR);
  }
}

void CKDevelop::slotViewTTreeView(){
  if(view_menu->isItemChecked(ID_VIEW_TREEVIEW)){
    view_menu->setItemChecked(ID_VIEW_TREEVIEW,false);
    kdlg_view_menu->setItemChecked(ID_VIEW_TREEVIEW,false);
		toolBar()->setButton(ID_VIEW_TREEVIEW,false);
		toolBar(ID_KDLG_TOOLBAR)->setButton(ID_VIEW_TREEVIEW,false);
/*  tree_view_pos=top_panner->sizes()[0];
    QValueList<int> sizes;
    sizes << 0;
    top_panner->setSizes(sizes);
*/
    if (bKDevelop)
     t_tab_view->hide();
    else
     kdlg_tabctl->hide();

  }
  else{
    if (bKDevelop)
     t_tab_view->show();
    else
     kdlg_tabctl->show();
/*    QValueList<int> sizes;
    sizes << tree_view_pos;
    top_panner->setSizes(sizes);
*/    view_menu->setItemChecked(ID_VIEW_TREEVIEW,true);
    kdlg_view_menu->setItemChecked(ID_VIEW_TREEVIEW,true);
		toolBar()->setButton(ID_VIEW_TREEVIEW,true);
		toolBar(ID_KDLG_TOOLBAR)->setButton(ID_VIEW_TREEVIEW,true);
  }
/*  QRect rMainGeom= top_panner->geometry();
  top_panner->resize(rMainGeom.width()+1,rMainGeom.height());
  top_panner->resize(rMainGeom.width(),rMainGeom.height());

*/}

void CKDevelop::slotViewTOutputView(){
  if(view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW)){
    view_menu->setItemChecked(ID_VIEW_OUTPUTVIEW,false);
    kdlg_view_menu->setItemChecked(ID_VIEW_OUTPUTVIEW,false);
		toolBar()->setButton(ID_VIEW_OUTPUTVIEW,false);
		toolBar(ID_KDLG_TOOLBAR)->setButton(ID_VIEW_OUTPUTVIEW,false);
/*    output_view_pos=view->sizes()[0];
    QValueList<int> sizes;
    sizes << 100;
    view->setSizes(sizes);
*/
    o_tab_view->hide();
  }
  else{
/*    QValueList<int> sizes;
    sizes << output_view_pos;
    view->setSizes(sizes);
*/    o_tab_view->show();
    view_menu->setItemChecked(ID_VIEW_OUTPUTVIEW,true);
    kdlg_view_menu->setItemChecked(ID_VIEW_OUTPUTVIEW,true);
		toolBar()->setButton(ID_VIEW_OUTPUTVIEW,true);
		toolBar(ID_KDLG_TOOLBAR)->setButton(ID_VIEW_OUTPUTVIEW,true);
  }
/*  QRect rMainGeom= view->geometry();
  view->resize(rMainGeom.width()+1,rMainGeom.height());
  view->resize(rMainGeom.width(),rMainGeom.height());
*/
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
  if(view_menu->isItemChecked(ID_VIEW_STATUSBAR)){
    view_menu->setItemChecked(ID_VIEW_STATUSBAR,false);
	  kdlg_view_menu->setItemChecked(ID_VIEW_STATUSBAR,false);
  }
  else{
    view_menu->setItemChecked(ID_VIEW_STATUSBAR,true);
  	kdlg_view_menu->setItemChecked(ID_VIEW_STATUSBAR,true);
  }
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
  error_parser->reset();
  error_parser->toogleOn();
  showOutputView(true);
  slotFileSave();
  setToolMenuProcess(false);
  QFileInfo fileinfo(cpp_widget->getName());
  slotStatusMsg(i18n("Compiling ")+cpp_widget->getName());
  messages_widget->clear();
  messages_widget->prepareJob(fileinfo.dirPath());
  // get the filename of the implementation file to compile and change extension for make
  //KDEBUG1(KDEBUG_INFO,CKDEVELOP,"ObjectFile= %s",QString(fileinfo.baseName()+".o").data());
	//  cerr << "ObjectFile= " << fileinfo.baseName()+".o";
  QString flaglabel=(prj->getProjectType()=="normal_c") ? "CFLAGS=\"" : "CXXFLAGS=\"";
  (*messages_widget) << flaglabel;
  if (!prj->getCXXFLAGS().isEmpty() || !prj->getAdditCXXFLAGS().isEmpty())
  {
            if (!prj->getCXXFLAGS().isEmpty())
                  (*messages_widget) << prj->getCXXFLAGS() << " ";
            if (!prj->getAdditCXXFLAGS().isEmpty())
                  (*messages_widget) << prj->getAdditCXXFLAGS();
  }
  (*messages_widget) << "\" " << "LDFLAGS=\" " ;
  if (!prj->getLDFLAGS().isEmpty())
                (*messages_widget) << prj->getLDFLAGS();
  (*messages_widget) << "\" ";
  (*messages_widget) << make_cmd << fileinfo.baseName()+".o";
	messages_widget->startJob();
}

void CKDevelop::slotBuildRun(){
  slotBuildMake();
  slotStatusMsg(i18n("Running ")+prj->getBinPROGRAM());
  beep=false;
  next_job = "run";
}

void CKDevelop::slotBuildRunWithArgs(){
    QString args=prj->getExecuteArgs();
    CExecuteArgDlg argdlg(this,"Arguments",i18n("Execute with Arguments"),args);
    if(argdlg.exec()){
	prj->setExecuteArgs(argdlg.getArguments());		
	prj->writeProject();
	slotBuildMake();
	slotStatusMsg(i18n("Running ")+prj->getBinPROGRAM());
	beep=false;
	next_job = "run_with_args";
    }
}
void CKDevelop::slotBuildDebug(){

  if(!CToolClass::searchProgram("kdbg")){
    return;
  }
  if(!prj->getBinPROGRAM()){
    slotBuildMake();
  }
  if(!bKDevelop)
    switchToKDevelop();

  showOutputView(false);
  showTreeView(false);
  
  slotStatusMsg(i18n("Running %1 in KDbg").arg(prj->getBinPROGRAM()));

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

  //save/generate dialog if needed
  kdlgedit->generateSourcecodeIfNeeded();
  error_parser->reset();
  error_parser->toogleOn();
  showOutputView(true);
  setToolMenuProcess(false);
  slotFileSaveAll();
  slotStatusMsg(i18n("Running make..."));
  messages_widget->clear();
  messages_widget->prepareJob(prj->getProjectDir() + prj->getSubDir());
  QString flaglabel=(prj->getProjectType()=="normal_c") ? "CFLAGS=\"" : "CXXFLAGS=\"";
  (*messages_widget) << flaglabel;
  if (!prj->getCXXFLAGS().isEmpty() || !prj->getAdditCXXFLAGS().isEmpty())
  {
     	if (!prj->getCXXFLAGS().isEmpty())
          (*messages_widget) << prj->getCXXFLAGS() << " ";
		  if (!prj->getAdditCXXFLAGS().isEmpty())
    		  (*messages_widget) << prj->getAdditCXXFLAGS();
  }
	(*messages_widget) << "\" " << "LDFLAGS=\" " ;
	if (!prj->getLDFLAGS().isEmpty())
			(*messages_widget) << prj->getLDFLAGS();
	(*messages_widget) << "\" ";
  (*messages_widget) << make_cmd;
  if(!prj->getMakeOptions().isEmpty())
      (*messages_widget) << prj->getMakeOptions();
  beep = true;
  messages_widget->startJob();
}


void CKDevelop::slotBuildRebuildAll(){
  if(!CToolClass::searchProgram(make_cmd)){
    return;
  }
  error_parser->reset();
  error_parser->toogleOn();
  showOutputView(true);

  setToolMenuProcess(false);
  slotFileSaveAll();
  slotStatusMsg(i18n("Running make clean command "));
  messages_widget->prepareJob(prj->getProjectDir() + prj->getSubDir());
  (*messages_widget) << make_cmd << "clean";
  next_job = make_cmd; // checked in slotProcessExited()
  beep = true;
  messages_widget->startJob();
}


 void CKDevelop::slotBuildCleanRebuildAll(){
  if(!CToolClass::searchProgram(make_cmd)){
    return;
  }
  error_parser->reset();
  error_parser->toogleOn();
  showOutputView(true);
  setToolMenuProcess(false);
  slotFileSaveAll();
  messages_widget->clear();
  slotStatusMsg(i18n("Running make clean and rebuilding all..."));
  messages_widget->prepareJob(prj->getProjectDir());
  (*messages_widget) << make_cmd << "distclean && " << make_cmd 
		<< " -f Makefile.dist && ";
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
  (*messages_widget) << "\" "<< "./configure " << prj->getConfigureArgs() << " && " << make_cmd;

  beep = true;
  messages_widget->startJob();
}

void CKDevelop::slotBuildDistClean(){
  if(!CToolClass::searchProgram(make_cmd)){
    return;
  }
  error_parser->reset();
  error_parser->toogleOn();
  showOutputView(true);
  setToolMenuProcess(false);
  slotFileSaveAll();
  slotStatusMsg(i18n("Running make distclean..."));
  messages_widget->clear();
  messages_widget->prepareJob(prj->getProjectDir());
  (*messages_widget) << make_cmd << "distclean";
  messages_widget->startJob();
}


 void CKDevelop::slotBuildAutoconf(){
  if(!CToolClass::searchProgram("automake")){
    return;
  }
  if(!CToolClass::searchProgram("autoconf")){
    return;
  }
  showOutputView(true);
  error_parser->toogleOff();
  setToolMenuProcess(false);
  slotFileSaveAll();
  slotStatusMsg(i18n("Running autoconf/automake suite..."));
  messages_widget->clear();
  messages_widget->prepareJob(prj->getProjectDir());
  (*messages_widget) << make_cmd;
  if(QFileInfo(QDir::current(),"Makefile.dist").exists())
      (*messages_widget) << " -f Makefile.dist";
  else
      (*messages_widget) << " -f Makefile.cvs";
  messages_widget->startJob();
  beep = true;
}


void CKDevelop::slotBuildConfigure(){
    QString args=prj->getConfigureArgs();
    CExecuteArgDlg argdlg(this,"Arguments",i18n("Configure with Arguments"),args);
    if(!argdlg.exec())
        return;
    
    prj->setConfigureArgs(argdlg.getArguments());		
    prj->writeProject();

  slotStatusMsg(i18n("Running ./configure..."));

  showOutputView(true);
  setToolMenuProcess(false);
  error_parser->toogleOff();
  slotFileSave();
  slotFileSaveAll();
  messages_widget->clear();
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
  (*messages_widget)  << "\" "<< "./configure " << argdlg.getArguments();
  messages_widget->startJob();
  beep = true;
}


void CKDevelop::slotBuildStop(){
  slotStatusMsg(i18n("Killing current process..."));
  setToolMenuProcess(true);
  messages_widget->killJob();
  appl_process.kill();
  slotStatusMsg(i18n("Ready."));
}


///////////////////////////////////////////////////////////////////////////////////////
// TOOLS-Menu slots
///////////////////////////////////////////////////////////////////////////////////////


void CKDevelop::slotToolsTool(int tool){

    if(!CToolClass::searchProgram(tools_exe.at(tool)) ){
	return;
    }
    if(!bKDevelop)
	switchToKDevelop();
    
    showOutputView(false);

    QString argument=tools_argument.at(tool);
 		
    // This allows us to replace the macro %H with the header file name, %S with the source file name
    // and %D with the project directory name.  Any others we should have?
    argument.replace( QRegExp("%H"), header_widget->getName() );
    argument.replace( QRegExp("%S"), cpp_widget->getName() );
    if(project){
      argument.replace( QRegExp("%D"), prj->getProjectDir() );
    }
    s_tab_view->setCurrentTab(TOOLS);
    swallow_widget->sWClose(false);
    if(argument.isEmpty()){
      swallow_widget->setExeString(tools_exe.at(tool));
    }
    else{
      swallow_widget->setExeString(tools_exe.at(tool)+argument);
    }
    swallow_widget->sWExecute();
    swallow_widget->init();
}



///////////////////////////////////////////////////////////////////////////////////////
// OPTIONS-Menu slots
///////////////////////////////////////////////////////////////////////////////////////

void CKDevelop::slotOptionsEditor(){
    slotStatusMsg(i18n("Setting up the Editor..."));
    
    QTabDialog *qtd = new QTabDialog(this, "tabdialog", TRUE);
    
    qtd->setCaption(i18n("Options Editor"));
    
    // indent options
    IndentConfigTab *indentConfig = new IndentConfigTab(qtd, cpp_widget);
    qtd->addTab(indentConfig, i18n("Indent"));
    
    // select options
    SelectConfigTab *selectConfig = new SelectConfigTab(qtd, cpp_widget);
    qtd->addTab(selectConfig, i18n("Select"));
    
    // edit options
    EditConfigTab *editConfig = new EditConfigTab(qtd, cpp_widget);
    qtd->addTab(editConfig, i18n("Edit"));
    
    
    qtd->setOkButton(i18n("OK"));
    qtd->setCancelButton(i18n("Cancel"));
    

    if (qtd->exec()) {
	// indent options
	indentConfig->getData(cpp_widget);
	indentConfig->getData(header_widget);
	// select options
	selectConfig->getData(cpp_widget);
	selectConfig->getData(header_widget);
	// edit options
	editConfig->getData(cpp_widget);
	editConfig->getData(header_widget);
	
	config->setGroup("KWrite Options");
	cpp_widget->writeConfig(config);
	cpp_widget->doc()->writeConfig(config);
	
	slotStatusMsg(i18n("Ready."));
    }
    delete qtd;

}
void CKDevelop::slotOptionsEditorColors(){
  slotStatusMsg(i18n("Setting up the Editor's colors..."));
  cpp_widget->colDlg();
  config->setGroup("KWrite Options");
  cpp_widget->writeConfig(config);
  cpp_widget->doc()->writeConfig(config);
  header_widget->copySettings(cpp_widget);
  config->setGroup("KWrite Options");
  header_widget->readConfig(config);
  header_widget->doc()->readConfig(config);
  slotStatusMsg(i18n("Ready."));

}


void CKDevelop::slotOptionsSyntaxHighlightingDefaults(){
  slotStatusMsg(i18n("Setting up syntax highlighting default colors..."));
  cpp_widget->hlDef();
  config->setGroup("KWrite Options");
  cpp_widget->writeConfig(config);
  cpp_widget->doc()->writeConfig(config);
  header_widget->copySettings(cpp_widget);
  config->setGroup("KWrite Options");
  header_widget->readConfig(config);
  header_widget->doc()->readConfig(config);
  slotStatusMsg(i18n("Ready."));
}


 void CKDevelop::slotOptionsSyntaxHighlighting(){
  slotStatusMsg(i18n("Setting up syntax highlighting colors..."));
  cpp_widget->hlDlg();
  config->setGroup("KWrite Options");
  cpp_widget->writeConfig(config);
  cpp_widget->doc()->writeConfig(config);
  header_widget->copySettings(cpp_widget);
  config->setGroup("KWrite Options");
  header_widget->readConfig(config);
  header_widget->doc()->readConfig(config);
  slotStatusMsg(i18n("Ready."));
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
   slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotOptionsToolsConfigDlg(){
  slotStatusMsg(i18n("Configuring Tools-Menu entries..."));
  CToolsConfigDlg* configdlg= new CToolsConfigDlg(this,"configdlg");
  configdlg->show();

  tools_menu->clear();
  kdlg_tools_menu->clear();
	setToolmenuEntries();
  slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotOptionsSpellchecker(){

}

void CKDevelop::slotOptionsConfigureEnscript(){
  if (!CToolClass::searchProgram("enscript")) {
    return;
  }
  CConfigEnscriptDlg *enscriptconf = new CConfigEnscriptDlg(this, "confdialog");
  enscriptconf->resize(610,510);
  enscriptconf->exec();
  delete (enscriptconf);
}

void CKDevelop::slotOptionsConfigureA2ps(){
  if (!CToolClass::searchProgram("a2ps")) {
    return;
  }
  CConfigA2psDlg *a2psconf = new CConfigA2psDlg(this, "confdialog");
  a2psconf->resize(600,430);
  a2psconf->exec();
  delete (a2psconf);
}

void CKDevelop::slotOptionsKDevelop(){
  slotStatusMsg(i18n("Setting up KDevelop..."));

  CKDevSetupDlg* setup = new CKDevSetupDlg(this, accel, cmdMngr, "Setup");
  //setup->show();
  if (setup->exec()) {
    if (setup->hasChangedPath())
        {
            QListIterator<Component> it(components);
            for ( ; it.current(); ++it)
                (*it)->docPathChanged();
        }

    // kwrite keys
    cmdMngr.changeAccels();
    cmdMngr.writeConfig(kapp->getConfig());
  } else {
    // cancel keys
    cmdMngr.restoreAccels();
  }

  delete setup;
  	
  accel->readSettings();
  setKeyAccel();
  slotStatusMsg(i18n("Ready."));
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
  saveTimer->changeInterval(saveTimeout);
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
  CUpdateKDEDocDlg dlg(messages_widget, config, this, "update_kdedoc");
  if(dlg.exec()){
    slotStatusMsg(i18n("Generating Documentation..."));
    setToolMenuProcess(false);
    if (dlg.isUpdated())
    {
        config->writeEntry("doc_kde",dlg.getDocPath());
        config->sync();
        doc_tree->refresh(prj);
    }
  }
}
void CKDevelop::slotOptionsCreateSearchDatabase(){
  if(!CToolClass::searchProgram("glimpseindex")){
    return;
  }
  CCreateDocDatabaseDlg dlg(messages_widget, config, this,"DLG");
  if(dlg.exec()){
    slotStatusMsg(i18n("Creating Search Database..."));
  }
}
	
void CKDevelop::slotPluginPluginManager(){
    TImportantPtrInfo info;

    //fill all struct fields
    info.kdev = this;
    info.plugin_menu = plugin_menu;

    PluginManagerDlg dlg(this,"plg",&info);
    dlg.show();
}

void CKDevelop::slotBookmarksSet(){
	if(s_tab_view->getCurrentTab()==BROWSER)
		slotBookmarksAdd();
	else{
		if(edit_widget==header_widget)
			header_widget->setBookmark();
		if(edit_widget==cpp_widget)
			cpp_widget->setBookmark();
	}
}
void CKDevelop::slotBookmarksAdd(){
	if(s_tab_view->getCurrentTab()==BROWSER){
		doc_bookmarks->clear();
		doc_bookmarks_list.append(browser_widget->currentURL());
		doc_bookmarks_title_list.append(browser_widget->currentTitle());
		
		uint i;
    for ( i =0 ; i < doc_bookmarks_list.count(); i++){
      doc_bookmarks->insertItem(BarIcon("html"),doc_bookmarks_title_list.at(i));
    }
	}
	if(edit_widget==header_widget)
		header_widget->addBookmark();
	if(edit_widget==cpp_widget)
		cpp_widget->addBookmark();

}
void CKDevelop::slotBookmarksClear(){
	if(s_tab_view->getCurrentTab()==BROWSER){
		doc_bookmarks_list.clear();
		doc_bookmarks_title_list.clear();
		doc_bookmarks->clear();
	}		
	else{
		if(edit_widget==header_widget)
			header_widget->clearBookmarks();
		if(edit_widget==cpp_widget)
			cpp_widget->clearBookmarks();
	}	
}

void CKDevelop::slotBoomarksBrowserSelected(int id_){
	slotStatusMsg(i18n("Opening bookmark..."));
	QString file= doc_bookmarks_list.at(id_);
	slotURLSelected( browser_widget, file,1,"test");	
  slotStatusMsg(i18n("Ready."));
}	


///////////////////////////////////////////////////////////////////////////////////////
// HELP-Menu slots
///////////////////////////////////////////////////////////////////////////////////////
void CKDevelop::slotHelpBack(){
  slotStatusMsg(i18n("Switching to last page..."));
  QString str = history_list.prev();
  if (str != 0){
    if(!bKDevelop)
      switchToKDevelop();
    s_tab_view->setCurrentTab(BROWSER);
    browser_widget->showURL(str);
  }

  slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotHelpForward(){
  slotStatusMsg(i18n("Switching to next page..."));
  QString str = history_list.next();
  if (str != 0){
    if(!bKDevelop)
      switchToKDevelop();
    s_tab_view->setCurrentTab(BROWSER);
    browser_widget->showURL(str);
  }

  slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotHelpHistoryBack( int id_){
	slotStatusMsg(i18n("Opening history page..."));
	
  QString str = history_list.at(id_);
  if (str != 0){
    if(!bKDevelop)
      switchToKDevelop();
    s_tab_view->setCurrentTab(BROWSER);
    browser_widget->showURL(str);
  }

  slotStatusMsg(i18n("Ready."));

}

void CKDevelop::slotHelpHistoryForward( int id_){
	slotStatusMsg(i18n("Opening history page..."));

  int cur=history_list.at()+1;	
  QString str = history_list.at(cur+id_);
  if (str != 0){
    if(!bKDevelop)
      switchToKDevelop();
    s_tab_view->setCurrentTab(BROWSER);
    browser_widget->showURL(str);
  }

  slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotHelpBrowserReload(){
	slotStatusMsg(i18n("Reloading page..."));
  if(!bKDevelop)
    switchToKDevelop();
  s_tab_view->setCurrentTab(BROWSER);
  browser_widget->setFocus();
	browser_widget->showURL(browser_widget->currentURL(), true);
	slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotHelpSearchText(QString text){
  int pos;

  if(!CToolClass::searchProgram("glimpse")){
    return;
  }

  /// stripping error causing \n's
  if(!text.isEmpty())
  {
   text.replace(QRegExp("^\n"), "");
   pos=text.find("\n");
   if (pos>-1)
    text=text.left(pos);

   text.replace(QRegExp("'"), "'\\''"); // handle ' in a right way
  }

  if(text.isEmpty()){
    KMessageBox::sorry(this, i18n("You must select a text for searching the documentation!"));
    return;
  }
  //  cerr << ":" << text << ":" << endl;

  doc_search_display_text = text.copy(); // save the text
  text=CToolClass::escapetext(text, false);  // change the text for using with regexp
  doc_search_text = text.copy();

  slotStatusMsg(i18n("Searching selected text in documentation..."));
  if(!QFile::exists(locateLocal("appdata",".glimpse_index"))){
    if(KMessageBox::questionYesNo(this, i18n("KDevelop couldn't find the search database.\n"
                                             "Do you want to generate it now?")) == KMessageBox::Yes){
      slotOptionsCreateSearchDatabase();
    }
    return;
  }
	enableCommand(ID_HELP_BROWSER_STOP);
  search_output = ""; // delete all from the last search
  search_process.clearArguments();
  search_process << "glimpse  -H "+ KGlobal::dirs()->getSaveLocation("appdata")+" -U -c -y '"+ text +"'";
  search_process.start(KShellProcess::NotifyOnExit,KShellProcess::AllOutput);
}
void CKDevelop::slotHelpSearchText(){
  QString text;
  if(s_tab_view->getCurrentTab()==BROWSER){
    browser_widget->getSelectedText(text);
  }
  else{
    // if edit_widget isn't shown don't proceed
    if (edit_widget!=0l)
    {
      text = edit_widget->markedText();
      if(text == ""){
        text = edit_widget->currentWord();
      }
    }
  }

  if (!text.isEmpty())
  slotHelpSearchText(text);
}

void CKDevelop::slotHelpSearch(){
  slotStatusMsg(i18n("Searching for Help on..."));
  CFindDocTextDlg* help_srch_dlg=new CFindDocTextDlg(this,"Search_for_Help_on");
  connect(help_srch_dlg,SIGNAL(signalFind(QString)),this,SLOT(slotHelpSearchText(QString)));
  help_srch_dlg->exec();
	delete help_srch_dlg;
}


void CKDevelop::slotHelpReference(){
  QString file = CToolClass::locatehtml("kdevelop/reference/C/cref.html");
  if (file.isNull())
      file = CToolClass::locatehtml("kdevelop/cref.html");
  slotURLSelected(browser_widget,"file:" + file,1,"test");
}


void CKDevelop::slotHelpQtLib(){
  config->setGroup("Doc_Location");
  QString doc_qt = config->readEntry("doc_qt", QT_DOCDIR);
  slotURLSelected(browser_widget,"file:" + doc_qt + "/index.html",1,"test");
}


void CKDevelop::showLibsDoc(const char *libname)
{
  config->setGroup("Doc_Location");
  QString doc_kde = config->readEntry("doc_kde", KDELIBS_DOCDIR);
  QString url = "file:" + doc_kde + "/" + libname + "/index.html";
  slotURLSelected(browser_widget, url,1,"test");
}
    
void CKDevelop::slotHelpKDECoreLib(){
  showLibsDoc("kdecore");
}


void CKDevelop::slotHelpKDEGUILib(){
  showLibsDoc("kdeui");
}


void CKDevelop::slotHelpKDEKFileLib(){
  showLibsDoc("kfile");
}


void CKDevelop::slotHelpKDEHTMLLib(){
  config->setGroup("Doc_Location");
  QString doc_kde = config->readEntry("doc_kde", KDELIBS_DOCDIR);
  QString file = doc_kde + "/khtml/index.html";
  slotURLSelected(browser_widget,"file:" +file  ,1,"test");
}


void CKDevelop::slotHelpAPI(){
  if(project){
    QString api_file=prj->getProjectDir() + prj->getSubDir() +  "/api/index.html";
    if(!QFileInfo(api_file).exists()){
        if (KMessageBox::questionYesNo(this, i18n("The Project API documentation is not present.\n" 
                                                  "Would you like to generate it now ?"))
            == KMessageBox::No)
            return;
        
        slotProjectAPI();
    }
    else{
        slotStatusMsg(i18n("Switching to project API Documentation..."));
        slotURLSelected(browser_widget,api_file,1,"test");     
        slotStatusMsg(i18n("Ready.")); 
    }
  }
}


void CKDevelop::slotHelpManual(){
  if(project){

    QString name = prj->getSGMLFile().copy();
    QFileInfo finfo(name);
 
    QString doc_file = finfo.dirPath() +"/"+finfo.baseName()+ ".html";
    if(!QFileInfo(doc_file).exists()){
    	if (KMessageBox::questionYesNo(this, i18n("The Project manual documentation is not present.\n" 
                                                         "Would you like to generate the handbook now?"))
            == KMessageBox::No)
            return;
        
        slotProjectManual();
    }
    else{
        slotStatusMsg(i18n("Switching to project Manual..."));
        slotURLSelected(browser_widget,doc_file,1,"test");
    	slotStatusMsg(i18n("Ready."));
    }
  }
}

void CKDevelop::slotHelpContents(){
  QString file = CToolClass::locatehtml("kdevelop/index.html");
  slotURLSelected(browser_widget,"file:" + file,1,"test");

}

void CKDevelop::slotHelpTutorial(){
  QString file = CToolClass::locatehtml("kdevelop/programming/index.html");
  slotURLSelected(browser_widget,"file:" + file,1,"test");
	
}


void CKDevelop::slotHelpTipOfDay(){
	KTipofDay* tipdlg=new KTipofDay(this, "tip of the day");
	tipdlg->show();

	delete tipdlg;	
}




void CKDevelop::slotHelpHomepage(){
  if(vfork() > 0) {
    // drop setuid, setgid
    setgid(getgid());
    setuid(getuid());
    
    execlp("kfmclient", "kfmclient", "exec", QString("http://www.kdevelop.org").data(), 0);
    _exit(0);
  }
}


void CKDevelop::slotHelpBugReport(){
    
    config->setGroup("General Options");
    TBugReportInfo info;
    info.author = config->readEntry("author_name","");
    info.email = config->readEntry("author_email","");
    info.os = config->readEntry("os","");
    info.kde_version = config->readEntry("kde_version","");
    info.qt_version = config->readEntry("qt_version","");
    info.compiler = config->readEntry("compiler","");

    CBugReportDlg dlg(this,"bug",info,config->readEntry("kdevelop_bug_report_email","submit@bugs.kde.org"));
    if( dlg.exec()){
	config->writeEntry("author_name",dlg.name);
	config->writeEntry("author_email",dlg.email_address);
	config->writeEntry("os",dlg.os);
	config->writeEntry("qt_version",dlg.qt_version);
	config->writeEntry("kde_version",dlg.kde_version);
	config->writeEntry("compiler",dlg.compiler);
    }
    
    
}


void CKDevelop::slotHelpAbout(){
  QMessageBox aboutmsg(this, "About KDevelop");
  aboutmsg.setCaption(i18n("About KDevelop..."));
  aboutmsg.setButtonText(1, i18n("OK"));
  aboutmsg.setText(i18n("\t   KDevelop Version "+version+" \n\n"
  											"\t(c) 1998,1999 The KDevelop Team \n\n"
                        "Sandy Meier <smeier@rz.uni-potsdam.de>\n"
                        "Stefan Heidrich <sheidric@rz.uni-potsdam.de>\n"
                        "Ralf Nolden <Ralf.Nolden@post.rwth-aachen.de>\n"
                        "Jonas Nordin <jonas.nordin@cenacle.se>\n"
                        "Pascal Krahmer <pascal@beast.de>\n"
			"Bernd Gehrmann <bernd@physik.hu-berlin.de>\n"
                        "Stefan Bartel <bartel@rz.uni-potsdam.de>\n"
			"Jörgen Olsson<jorgen@cenacle.net>\n"
                        "Martin Piskernig <martin.piskernig@stuwo.at>\n"
			"Walter Tasin <tasin@e-technik.fh-muenchen.de>\n\n"
                        "See The KDevelop User Manual, Chapter Authors\n"
                        "for further information.\n\n"
                        "This program is free software; you can\n"
                        "redistribute it and/or modify it under\n"
                        "the terms of the GNU General Public License\n"
                        "as published by the Free Software Foundation;\n"
                        "either version 2 of the License, or (at your\n"
                        "option) any later version.\n\n"
												"This program is distributed in the hope that\n"
												"it will be useful, but WITHOUT ANY WARRANTY; \n"
												"without even the implied warranty of\n"
												"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
												"See the GNU General Public License for more details."));

  QPixmap pm(locate("kdev_pic","about_logo.bmp"));
  aboutmsg.setIconPixmap(pm);
  aboutmsg.show();
}

void CKDevelop::slotKDlgViewPropView(){
  if(kdlg_view_menu->isItemChecked(ID_KDLG_VIEW_PROPVIEW)){
    kdlg_view_menu->setItemChecked(ID_KDLG_VIEW_PROPVIEW,false);

/*  properties_view_pos=kdlg_top_panner->sizes()[0];
    QValueList<int> sizes;
    sizes << 100;
    kdlg_top_panner->setSizes(sizes);
*/  kdlg_prop_widget->getListView()->hide();
  }
  else{
/*    QValueList<int> sizes;
    sizes << properties_view_pos;
    kdlg_top_panner->setSizes(sizes);
*/  kdlg_prop_widget->getListView()->show();
    kdlg_view_menu->setItemChecked(ID_KDLG_VIEW_PROPVIEW,true);
  }
/*  QRect rMainGeom= kdlg_top_panner->geometry();
  kdlg_top_panner->resize(rMainGeom.width()+1,rMainGeom.height());
  kdlg_top_panner->resize(rMainGeom.width(),rMainGeom.height());
*/
}
void CKDevelop::slotKDlgViewToolbar(){
  if(kdlg_view_menu->isItemChecked(ID_KDLG_VIEW_TOOLBAR)){
    kdlg_view_menu->setItemChecked(ID_KDLG_VIEW_TOOLBAR,false);
    enableToolBar(KToolBar::Hide,ID_KDLG_TOOLBAR);
  }
  else{
    kdlg_view_menu->setItemChecked(ID_KDLG_VIEW_TOOLBAR,true);
    enableToolBar(KToolBar::Show,ID_KDLG_TOOLBAR);
  }
}

void CKDevelop::slotKDlgViewStatusbar(){

}

/*********************************************************************
 *                                                                   *
 *             SLOTS FOR THE CLASSBROWSER WIZARD BUTTON              *
 *                                                                   *
 ********************************************************************/

/*---------------------------- CKDevelop::slotClassbrowserViewClass()
 * slotClassbrowserViewClass()
 *   Event when the user wants to view the classdeclaration from the
 *   browser toolbar.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::slotClassbrowserViewClass()
{
  QComboBox* classCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_CLASS_CHOICE);
  QString classname = classCombo->currentText();

  CVGotoDeclaration( classname, "", THCLASS );
}

/*---------------------------- CKDevelop::slotClassbrowserViewDeclaration()
 * slotClassbrowserViewDeclaration()
 *   Event when the user wants to view a declaration from the
 *   browser toolbar/menu.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::slotClassbrowserViewDeclaration()
{
  QComboBox* classCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_CLASS_CHOICE);
  QComboBox* methodCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_METHOD_CHOICE);
  QString classname = classCombo->currentText();
  QString methodname = methodCombo->currentText();

  CVGotoDeclaration( classname, methodname, 
                     ( methodname.isEmpty() ? THCLASS : THPUBLIC_METHOD ) );
}

/*----------------------- CKDevelop::slotClassbrowserViewDefinition()
 * slotClassbrowserViewDefinition()
 *   Event when the user wants to view a definition from the  browser
 *   toolbar/menu.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::slotClassbrowserViewDefinition()
{
  QComboBox* classCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_CLASS_CHOICE);
  QComboBox* methodCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_METHOD_CHOICE);
  QString classname = classCombo->currentText();
  QString methodname = methodCombo->currentText();

  if( methodname.isEmpty() )
    CVGotoDeclaration( classname, "", THCLASS );
  else
    CVGotoDefinition( classname, methodname, THPUBLIC_METHOD );
}

/*------------------------------ CKDevelop::slotClassbrowserNewMethod()
 * slotClassbrowserNewMethod()
 *   Event when the user wants to create a new method from the browser
 *   toolbar/menu.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::slotClassbrowserNewMethod()
{
  QComboBox* classCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_CLASS_CHOICE);
  QString classname = classCombo->currentText();

  if( !classname.isEmpty() )
    slotCVAddMethod( classname );
}

/*------------------------- CKDevelop::slotClassbrowserNewAttribute()
 * slotClassbrowserNewAttribute()
 *   Event when the user wants to create a new attribute from the 
 *   browser toolbar/menu.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::slotClassbrowserNewAttribute()
{
  QComboBox* classCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_CLASS_CHOICE);
  QString classname = classCombo->currentText();

  if( !classname.isEmpty() )
    slotCVAddAttribute( classname );
}

/////////////////////////////////////////////////////////////////////
// Other slots and functions needed
/////////////////////////////////////////////////////////////////////

void CKDevelop::slotStatusMsg(const char *text)
{
  ///////////////////////////////////////////////////////////////////
  // change status message permanently
//  statusBar()->clear();
    statProg->hide();
    statusBar()->changeItem(text, ID_STATUS_MSG );
}


void CKDevelop::slotStatusHelpMsg(const char *text)
{
  ///////////////////////////////////////////////////////////////////
    // change status message of whole statusbar temporary (text, msec)
    statusBar()->message(text, 2000);
    QTimer::singleShot ( 2000, statProg, SLOT(hide()) );

}

void CKDevelop::enableCommand(int id_)
{
  kdev_menubar->setItemEnabled(id_,true);
  kdlg_menubar->setItemEnabled(id_,true);
  accel->setItemEnabled(id_,true);

//  menuBar()->setItemEnabled(id_,true);
  toolBar()->setItemEnabled(id_,true);
  toolBar(ID_BROWSER_TOOLBAR)->setItemEnabled(id_,true);
  toolBar(ID_KDLG_TOOLBAR)->setItemEnabled(id_,true);
}

void CKDevelop::disableCommand(int id_)
{
  kdev_menubar->setItemEnabled(id_,false);
  kdlg_menubar->setItemEnabled(id_,false);
  accel->setItemEnabled(id_,false);

//  menuBar()->setItemEnabled(id_,false);
  toolBar()->setItemEnabled(id_,false);
  toolBar(ID_BROWSER_TOOLBAR)->setItemEnabled(id_,false);
  toolBar(ID_KDLG_TOOLBAR)->setItemEnabled(id_,false);
}

void CKDevelop::slotNewStatus()
{
  int config;
  config = edit_widget->config();
  statusBar()->changeItem(config & cfOvr ? "OVR" : "INS",ID_STATUS_INS_OVR);
  // set new caption... maybe the file content is changed
  setMainCaption();
}


void CKDevelop::slotMarkStatus() {

  if (edit_widget != 0L) {
    if (edit_widget->hasMarkedText()) {
      enableCommand(ID_EDIT_CUT);
      enableCommand(ID_EDIT_COPY);
    } else{
      disableCommand(ID_EDIT_CUT);
      disableCommand(ID_EDIT_COPY);
    }
  }
}


/*
void CKDevelop::slotCPPMarkStatus(KWriteView *,bool bMarked)

  int item=s_tab_view->getCurrentTab();
  if (item==CPP)
  {
    if(bMarked){
      enableCommand(ID_EDIT_CUT);
      enableCommand(ID_EDIT_COPY);
    }
    else{
      disableCommand(ID_EDIT_CUT);
      disableCommand(ID_EDIT_COPY);
    }
  }		
}

void CKDevelop::slotHEADERMarkStatus(KWriteView *, bool bMarked)
{
  int item=s_tab_view->getCurrentTab();
  if (item==HEADER)
  {
      if(bMarked){
        enableCommand(ID_EDIT_CUT);
        enableCommand(ID_EDIT_COPY);
      }
      else{
        disableCommand(ID_EDIT_CUT);
        disableCommand(ID_EDIT_COPY);
      }		
  }
}
*/
void CKDevelop::slotBROWSERMarkStatus(KHTMLView *, bool bMarked)
{
  int item=s_tab_view->getCurrentTab();
  if (item==BROWSER)
  {
      if(bMarked){
        enableCommand(ID_EDIT_COPY);
      }
      else{
        disableCommand(ID_EDIT_COPY);
      }		
  }
}

void CKDevelop::slotClipboardChanged()
{
  int item = s_tab_view->getCurrentTab();
  QString text = QApplication::clipboard()->text();
//  if(!bContents || item==BROWSER || item==TOOLS)
  if(text.isEmpty() || item==BROWSER || item==TOOLS)
    disableCommand(ID_EDIT_PASTE);
  else
    enableCommand(ID_EDIT_PASTE);
}

void CKDevelop::slotNewLineColumn()
{
  // if edit_widget isn't shown don't proceed
  if (edit_widget==0l)
      return;
  QString str = i18n("Line: %1 Col: %2")
      .arg(edit_widget->currentLine()+1)
      .arg(edit_widget->currentColumn()+1);
  statusBar()->changeItem(str, ID_STATUS_LN_CLM);
} 
void CKDevelop::slotNewUndo(){
  int state;
  state = (edit_widget) ? edit_widget->undoState() : 0;
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


void CKDevelop::slotURLSelected(KHTMLView* ,QString url,int,QString){
//	enableCommand(ID_HELP_BROWSER_STOP);
  if(!bKDevelop)
    switchToKDevelop();
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



  QString str = history_list.current();
  //if it's a url-request from the search result jump to the correct point
  if (str.contains("kdevelop/search_result.html")){
    prev_was_search_result=true; // after this time, jump to the searchkey
  }

}

void CKDevelop::slotURLonURL(KHTMLView*, QString url)
{
	if ( url )
	{
		statusBar()->changeItem(url,ID_STATUS_MSG);
	}
	else
	{
		statusBar()->changeItem(i18n("Ready."), ID_STATUS_MSG);
	}
}

void CKDevelop::slotDocumentDone( KHTMLView * ){
  QString actualURL=browser_widget->currentURL();
  QString actualTitle=browser_widget->currentTitle();
  int cur =  history_list.at()+1; // get the current index
  int found =  history_list.find(actualURL); // get the current index
  int pos = actualURL.findRev('#');
  QString url_wo_ref=actualURL; // without ref

  if(prev_was_search_result){
    browser_widget->findTextBegin();
    browser_widget->findTextNext(QRegExp(doc_search_text));
  }

  if (s_tab_view->getCurrentTab()==BROWSER)
     setMainCaption(BROWSER);

  if (pos!=-1)
   url_wo_ref = actualURL.left(pos);

  // insert into the history-list
  // the following if-statement isn't necessary, because
  //   slotDocumentDone isn't called in the other cases [use of KFMclient for non file://....htm(l)]
  if(actualURL.left(7) != "http://" && url_wo_ref.right(4).find("htm", FALSE)>-1){
   // http aren't added to the history list ...

   if (found == -1)
   {
    if(cur == 0 ){
      history_list.append(actualURL);
      history_title_list.append(actualTitle);
    }
    else{
      history_list.insert(cur,actualURL);
      history_title_list.insert(cur, actualTitle);
    }
   }
   else
   {
     // the desired URL was already found in the list

     if (actualURL.contains("kdevelop/search_result.html") &&
	history_title_list.at(found)!=actualTitle)
     {
         // this means... a new search_result.html is selected and an old one
         // was found in list
         //   so append it at the end
         history_list.remove(found);
         history_title_list.remove(found);
         // append now the new one
         cur=history_list.count();
         history_list.insert(cur,actualURL);
         history_title_list.insert(cur, actualTitle);
     }
     else
     if (prev_was_search_result)
      {
         // this means... sort the found entry after the search_result.html-entry
         //   so we can always use the back button to get the last search results
         history_list.remove(found);
         history_title_list.remove(found);
         // correct cur after removing a list element
         if (found<cur)
           cur--;
         history_list.insert(cur,actualURL);
         history_title_list.insert(cur, actualTitle);
      }
      else
      {
         cur=found;
      }
   }

   // set now the pointer of the history list
   history_list.at(cur);

   // reorganize the prev- and the next-historylist
   history_next->clear();
   history_prev->clear();
		
   int i;
   for ( i =0 ; i < cur; i++){
       history_prev->insertItem(history_title_list.at(i));
   }

   for (i = cur+1 ; i < (int) history_list.count(); i++){
       history_next->insertItem(history_title_list.at(i));
   }

   // disable the back button if necessary
   if (cur == 0){ // no more backwards
 	disableCommand(ID_HELP_BACK);
   }
   else {
 	enableCommand(ID_HELP_BACK);
   }

   // disable the forward button if necessary
   if (cur >= ((int) history_list.count())-1){ // no more forwards
 	disableCommand(ID_HELP_FORWARD);
   }
   else {
 	enableCommand(ID_HELP_FORWARD);
   }

  }

  prev_was_search_result=false;
  disableCommand(ID_HELP_BROWSER_STOP);
}


void CKDevelop::slotApplReceivedStdout(KProcess*,char* buffer,int buflen){
  stdin_stdout_widget->insert(QString::fromLatin1(buffer, buflen));
  showOutputView(true);
}


void CKDevelop::slotApplReceivedStderr(KProcess*,char* buffer,int buflen){
  stderr_widget->insert(QString::fromLatin1(buffer, buflen));
  showOutputView(true);
}


void CKDevelop::slotSearchReceivedStdout(KProcess* /*proc*/,char* buffer,int buflen){
  search_output += QString::fromLatin1(buffer, buflen);
}


void CKDevelop::slotSearchProcessExited(KProcess*){
	disableCommand(ID_HELP_BROWSER_STOP);
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

     KMessageBox::sorry(0, i18n("\"%1\" not found in documentation!").arg(doc_search_display_text));
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

   QString filename =locate("appdata","search_result.html");
   QFile file(filename);
   QTextStream stream(&file);
   file.open(IO_WriteOnly);

   stream << "<HTML>";
   stream << "<HEAD><TITLE> - " << i18n("Search for: ") << doc_search_display_text;
   stream << "</TITLE></HEAD><BODY BGCOLOR=\"#ffffff\"><BR> <TABLE><TR><TH>";
   stream << i18n("Title") << "<TH>" << i18n("Hits") << "\n";
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


void CKDevelop::slotClickedOnMessagesWidget(int row){
  TErrorMessageInfo info;
  info = error_parser->getInfo(row+1);
  if(info.filename != ""){
    if(!bKDevelop)
      switchToKDevelop();
#warning FIXME
#if 0
    messages_widget->setCursorPosition(info.makeoutputline,0);
    switchToFile(info.filename,info.errorline-1);
#endif
  }
  else{
     XBell(kapp->getDisplay(),100); // not a next found, beep
  }
}


void CKDevelop::slotProcessExited(KProcess *proc){
  setToolMenuProcess(true);
  slotStatusMsg(i18n("Ready."));
  bool ready = true;
  QString result="";
  if (proc->normalExit()) {
    
    result= ((proc->exitStatus()) ? i18n("*** failed ***\n") : 
          i18n("*** success ***\n"));
    if ( proc== &appl_process)
      result.sprintf(i18n("*** exit-code: %i ***\n"), 
		     proc->exitStatus());
    
    if (next_job == make_cmd){ // rest from the rebuild all
      messages_widget->prepareJob(prj->getProjectDir() + prj->getSubDir());
      (*messages_widget) << make_cmd;
      if(!prj->getMakeOptions().isEmpty())
      	(*messages_widget) << prj->getMakeOptions();
      setToolMenuProcess(false);
      messages_widget->startJob();
      next_job = "";
      ready=false;
    }
    if ((next_job == "run"  || next_job == "run_with_args") && proc->exitStatus() == 0){ 
      // rest from the buildRun
      appl_process.clearArguments();
      QDir::setCurrent(prj->getProjectDir() + prj->getSubDir());
      stdin_stdout_widget->clear();
      stderr_widget->clear();
      QString args = prj->getExecuteArgs();
      QString program = prj->getBinPROGRAM().lower();
      
      
      if(next_job == "run_with_args"){
	if(!args.isEmpty()){
	  program = prj->getBinPROGRAM().lower() + " "+args;
	}
      }

      // Warning: not every user has the current directory in his path !
      QString exec_str;
      if(prj->getProjectType() == "normal_cpp" || prj->getProjectType() == "normal_c"){
	o_tab_view->setCurrentTab(STDINSTDOUT);
	exec_str = CToolClass::searchInstProgram("konsole")? "konsole" : "xterm";
        exec_str += " -e /bin/sh -c './";
        exec_str += program;
        exec_str += ";echo \"\n";
        exec_str += i18n("Press Enter to continue!");
        exec_str += "\";read'";
      } else {
	o_tab_view->setCurrentTab(STDERR);
	exec_str = "./" + program;
      }
      appl_process << exec_str;
      cerr << endl << "EXEC:" << exec_str;
      setToolMenuProcess(false);
      appl_process.start(KProcess::NotifyOnExit,KProcess::All);
      next_job = "";
      ready = false;
    }
      
    next_job = "";
  }
  else {
    result= i18n("*** process exited with error(s) ***\n");
    next_job = "";
    
  }
  if (!result.isEmpty())
  {
     messages_widget->insertStdoutLine(result);
  }
  if (ready){ // start the error-message parser
#warning FIXME
      QString str1; // = messages_widget->text();
      
      if(error_parser->getMode() == CErrorMessageParser::MAKE){
	  error_parser->parseInMakeMode(&str1,prj->getProjectDir() + prj->getSubDir());
      }
      if(error_parser->getMode() == CErrorMessageParser::SGML2HTML){
	  error_parser->parseInSgml2HtmlMode(&str1,prj->getProjectDir() + prj->getSubDir() + "/docs/en/" + prj->getSGMLFile());
      }
      //enable/disable the menus/toolbars
      if(error_parser->hasNext()){
	  enableCommand(ID_VIEW_NEXT_ERROR);
      }
      else{
	  disableCommand(ID_VIEW_NEXT_ERROR);
      }
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

  // if no edit widget is currently visible, then edit_widget stays 0L
  edit_widget = 0L;

  if (item == HEADER || item == CPP)
  {
     // enableCommand(ID_FILE_SAVE);  is handled by setMainCaption()
    enableCommand(ID_FILE_SAVE_AS);
    enableCommand(ID_FILE_CLOSE);

    enableCommand(ID_FILE_PRINT);

    QString text=QApplication::clipboard()->text();
    if(text.isEmpty())
      disableCommand(ID_EDIT_PASTE);
    else
      enableCommand(ID_EDIT_PASTE);

    enableCommand(ID_EDIT_INSERT_FILE);
    enableCommand(ID_EDIT_SEARCH);
    enableCommand(ID_EDIT_REPEAT_SEARCH);
    enableCommand(ID_EDIT_REPLACE);
    enableCommand(ID_EDIT_SPELLCHECK);
    enableCommand(ID_EDIT_INDENT);
    enableCommand(ID_EDIT_UNINDENT);
    enableCommand(ID_EDIT_SELECT_ALL);
    enableCommand(ID_EDIT_DESELECT_ALL);
    enableCommand(ID_EDIT_INVERT_SELECTION);

  }

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
//    setMainCaption();  is called by slotNewStatus()
    slotNewLineColumn();
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
//    setMainCaption();  is called by slotNewStatus()
    slotNewLineColumn();
  }

  if (item == HEADER || item == CPP)
  {
    int state;
    state = edit_widget->undoState();
    //undo
    if(state & 1)
      enableCommand(ID_EDIT_UNDO);
    else
      disableCommand(ID_EDIT_UNDO);
    //redo
    if(state & 2)
      enableCommand(ID_EDIT_REDO);
    else
      disableCommand(ID_EDIT_REDO);

    slotMarkStatus();
/*
//    QString str = edit_widget->markedText();
    if (edit_widget->hasMarkedText()){
      enableCommand(ID_EDIT_CUT);
      enableCommand(ID_EDIT_COPY);
    }
    else{
      disableCommand(ID_EDIT_CUT);
      disableCommand(ID_EDIT_COPY);
    }*/		
  }

  if(item == BROWSER || item == TOOLS)
  {
    disableCommand(ID_BUILD_COMPILE_FILE);

    disableCommand(ID_FILE_SAVE);
    disableCommand(ID_FILE_SAVE_AS);
    disableCommand(ID_FILE_CLOSE);

    disableCommand(ID_FILE_PRINT);

    disableCommand(ID_EDIT_UNDO);
    disableCommand(ID_EDIT_REDO);
    disableCommand(ID_EDIT_CUT);
    disableCommand(ID_EDIT_PASTE);
    disableCommand(ID_EDIT_INSERT_FILE);
    disableCommand(ID_EDIT_SEARCH);
    disableCommand(ID_EDIT_REPEAT_SEARCH);
    disableCommand(ID_EDIT_REPLACE);
    disableCommand(ID_EDIT_SPELLCHECK);
    disableCommand(ID_EDIT_INDENT);
    disableCommand(ID_EDIT_UNINDENT);
    disableCommand(ID_EDIT_SELECT_ALL);
    disableCommand(ID_EDIT_DESELECT_ALL);
    disableCommand(ID_EDIT_INVERT_SELECTION);
  }

  if(item == BROWSER){
    if(bAutoswitch)
      t_tab_view->setCurrentTab(DOC);
    browser_widget->setFocus();

    if (browser_widget->isTextSelected())
      enableCommand(ID_EDIT_COPY);
    else
      disableCommand(ID_EDIT_COPY);

    setMainCaption(BROWSER);
  }

  if(item == TOOLS){
    disableCommand(ID_EDIT_COPY);
    setMainCaption(TOOLS);
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


void CKDevelop::slotLogFileTreeSelected(QString file){
  switchToFile(file);
}

void CKDevelop::slotRealFileTreeSelected(QString file){
  switchToFile(file);
}

void CKDevelop::slotUpdateFileFromVCS(QString file){
    switchToFile(file,false,false); // force not reload and no messagebox if modified on disc, because slotFileSave() will do it
    slotFileSave();
    prj->getVersionControl()->update(file);
    switchToFile(file,true,false);
}
void CKDevelop::slotCommitFileToVCS(QString file){
    switchToFile(file,false,false);
    slotFileSave();
    prj->getVersionControl()->commit(file);
    switchToFile(file,true,false);
}

void CKDevelop::slotUpdateDirFromVCS(QString dir){
    slotFileSaveAll();

    prj->getVersionControl()->update(dir);
    TEditInfo* actual_info;
    
    QListIterator<TEditInfo> it(edit_infos); // iterator for edit_infos list

    for ( ; it.current(); ++it ) {
	actual_info = it.current();
	QFileInfo file_info(actual_info->filename);
	if(actual_info->last_modified != file_info.lastModified()){ // reload only changed files
	    switchToFile(actual_info->filename,true,false); //force reload, no modified on disc messagebox
	}
    }
}

void CKDevelop::slotCommitDirToVCS(QString dir){
    slotFileSaveAll();
    prj->getVersionControl()->commit(dir);

    TEditInfo* actual_info;
    
    QListIterator<TEditInfo> it(edit_infos); // iterator for edit_infos list

    for ( ; it.current(); ++it ) {
	actual_info = it.current();
	QFileInfo file_info(actual_info->filename);
	if(actual_info->last_modified != file_info.lastModified()){ // reload only changed files
	    switchToFile(actual_info->filename,true,false); //force reload, no modified on disc messagebox
	}
    }
    
}
void CKDevelop::slotDocTreeSelected(QString url_file){
  if(url_file == "API-Documentation"){
    slotHelpAPI();
    return;
  }
  if(url_file == "User-Manual"){
    slotHelpManual();
    return;
  }

  slotURLSelected(browser_widget,"file:"+ url_file,1,"test");
  
}

void CKDevelop::slotTCurrentTab(int item){
    t_tab_view->setCurrentTab(item);
}

void CKDevelop::slotSCurrentTab(int item){
    s_tab_view->setCurrentTab(item);
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

void CKDevelop::slotGrepDialogItemSelected(const QString &filename,int linenumber){
  switchToFile(filename,linenumber);
}

void CKDevelop::slotToolbarClicked(int item){
  switch (item) {
  case ID_FILE_NEW:
    slotFileNew();
    break;
  case ID_PROJECT_OPEN:
    slotProjectOpen();
    break;
  case ID_FILE_OPEN:
    slotFileOpen();
    break;
  case ID_FILE_SAVE:
    slotFileSave();
    break;
  case ID_KDLG_FILE_SAVE:
    kdlgedit->slotFileSave();
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
  case ID_KDLG_EDIT_UNDO:
    kdlgedit->slotEditUndo();
    break;
  case ID_EDIT_REDO:
    slotEditRedo();
    break;
  case ID_KDLG_EDIT_REDO:
    kdlgedit->slotEditRedo();
    break;
  case ID_EDIT_COPY:
    slotEditCopy();
    break;
  case ID_KDLG_EDIT_COPY:
    kdlgedit->slotEditCopy();
    break;
  case ID_EDIT_PASTE:
    slotEditPaste();
    break;
  case ID_KDLG_EDIT_PASTE:
    kdlgedit->slotEditPaste();
    break;
  case ID_EDIT_CUT:
    slotEditCut();
    break;
  case ID_KDLG_EDIT_CUT:
    kdlgedit->slotEditCut();
    break;
  case ID_VIEW_REFRESH:
    slotViewRefresh();
    break;
	case ID_VIEW_TREEVIEW:
		slotViewTTreeView();
		break;
	case ID_VIEW_OUTPUTVIEW:
		slotViewTOutputView();
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
  case ID_TOOLS_KDLGEDIT:
    switchToKDlgEdit();
    break;
  case ID_KDLG_TOOLS_KDEVELOP:
    switchToKDevelop();
    break;
  case ID_HELP_BACK:
    slotHelpBack();
    break;
  case ID_HELP_FORWARD:
    slotHelpForward();
    break;
	case ID_HELP_BROWSER_RELOAD:
		slotHelpBrowserReload();
		break;
	case ID_HELP_BROWSER_STOP:
		browser_widget->cancelAllRequests();
	        messages_widget->killJob();
		disableCommand(ID_HELP_BROWSER_STOP);
		break;
	case ID_HELP_CONTENTS:
		slotHelpContents();
		break;
  case ID_HELP_SEARCH_TEXT:
    slotHelpSearchText();
    break;
  case ID_HELP_SEARCH:
    slotHelpSearch();
    break;
  case ID_KDLG_BUILD_GENERATE:
    kdlgedit->slotBuildGenerate();
    break;
  case ID_CV_WIZARD:
    // Make the button toggle between declaration and definition.
    if(cv_decl_or_impl){
      slotClassbrowserViewDeclaration();
      cv_decl_or_impl=false;
    }
    else{
      slotClassbrowserViewDefinition();
      cv_decl_or_impl=true;
    }		
    break;
  }
}

void CKDevelop::statusCallback(int id_){
  switch(id_){
    ON_STATUS_MSG(ID_FILE_NEW,                              i18n("Creates a new file"))
    ON_STATUS_MSG(ID_FILE_OPEN,   							            i18n("Opens an existing file"))
  ON_STATUS_MSG(ID_FILE_CLOSE,       						          i18n("Closes the actual file"))

  ON_STATUS_MSG(ID_FILE_SAVE,        						          i18n("Save the actual document"))
  ON_STATUS_MSG(ID_FILE_SAVE_AS,     						          i18n("Save the document as..."))
  ON_STATUS_MSG(ID_FILE_SAVE_ALL,    						          i18n("Save all changed files"))

  ON_STATUS_MSG(ID_FILE_PRINT,       						          i18n("Prints the current document"))

//  ON_STATUS_MSG(ID_FILE_CLOSE_WINDOW,i18n("Closes the current window"))

  ON_STATUS_MSG(ID_FILE_QUIT,        						          i18n("Exits the program"))

	ON_STATUS_MSG(ID_EDIT_UNDO,															i18n("Reverts the last editing step"))
	ON_STATUS_MSG(ID_EDIT_REDO,															i18n("Re-execute the last undone step"))
	
  ON_STATUS_MSG(ID_EDIT_CUT,                     			    i18n("Cuts the selected section and puts it to the clipboard"))
  ON_STATUS_MSG(ID_EDIT_COPY,                    			    i18n("Copys the selected section to the clipboard"))
  ON_STATUS_MSG(ID_EDIT_PASTE,                   			    i18n("Pastes the clipboard contents to actual position"))

  ON_STATUS_MSG(ID_EDIT_INSERT_FILE,             			    i18n("Inserts a file at the current position"))

  ON_STATUS_MSG(ID_EDIT_SEARCH,                  			    i18n("Searchs the file for an expression"))
  ON_STATUS_MSG(ID_EDIT_REPEAT_SEARCH,           			    i18n("Repeats the last search"))
  ON_STATUS_MSG(ID_EDIT_REPLACE,                 			    i18n("Searchs and replace expression"))
	ON_STATUS_MSG(ID_EDIT_SEARCH_IN_FILES,									i18n("Opens the search in files dialog to search for expressions over several files"))
	
	ON_STATUS_MSG(ID_EDIT_INDENT,														i18n("Moves the selection to the right"))
	ON_STATUS_MSG(ID_EDIT_UNINDENT,													i18n("Moves the selection to the left"))
	
  ON_STATUS_MSG(ID_EDIT_SELECT_ALL,              			    i18n("Selects the whole document contents"))
	ON_STATUS_MSG(ID_EDIT_DESELECT_ALL,											i18n("Deselects the whole document contents"))
	ON_STATUS_MSG(ID_EDIT_INVERT_SELECTION,									i18n("Inverts the current selection"))

	
  ON_STATUS_MSG(ID_VIEW_GOTO_LINE,               			    i18n("Goes to Line Number..."))
	ON_STATUS_MSG(ID_VIEW_NEXT_ERROR,												i18n("Switches to the file and line the next error was reported"))
	ON_STATUS_MSG(ID_VIEW_PREVIOUS_ERROR,										i18n("Switches to the file and line the previous error was reported"))
	
  ON_STATUS_MSG(ID_VIEW_TREEVIEW,              				    i18n("Enables / disables the treeview"))
  ON_STATUS_MSG(ID_VIEW_OUTPUTVIEW,            				    i18n("Enables / disables the outputview"))

  ON_STATUS_MSG(ID_VIEW_TOOLBAR,                  			  i18n("Enables / disables the standard toolbar"))
  ON_STATUS_MSG(ID_VIEW_BROWSER_TOOLBAR,       				    i18n("Enables / disables the browser toolbar"))
  ON_STATUS_MSG(ID_VIEW_STATUSBAR,             				    i18n("Enables / disables the statusbar"))

  ON_STATUS_MSG(ID_VIEW_REFRESH,                			    i18n("Refreshes current view"))
  ON_STATUS_MSG(ID_VIEW_IN_KFM,					   								i18n("Opens the current document in the KFM browser"))
  ON_STATUS_MSG(ID_PROJECT_KAPPWIZARD,            			  i18n("Generates a new project with Application Wizard"))
  ON_STATUS_MSG(ID_PROJECT_OPEN,			            	      i18n("Opens an existing project"))
  ON_STATUS_MSG(ID_PROJECT_CLOSE,                 			  i18n("Closes the current project"))
  ON_STATUS_MSG(ID_PROJECT_ADD_FILE_EXIST,        			  i18n("Adds an existing file to the project"))
	ON_STATUS_MSG(ID_PROJECT_ADD_NEW_TRANSLATION_FILE,			i18n("Adds a new language for internationalization to the project"))
  ON_STATUS_MSG(ID_PROJECT_REMOVE_FILE,           			  i18n("Removes file from the project"))

  ON_STATUS_MSG(ID_PROJECT_NEW_CLASS,             			  i18n("Creates a new Class frame structure and files"))

  ON_STATUS_MSG(ID_PROJECT_FILE_PROPERTIES,       			  i18n("Shows the current file properties"))
  ON_STATUS_MSG(ID_PROJECT_OPTIONS,               			  i18n("Sets project and compiler options"))
	ON_STATUS_MSG(ID_PROJECT_MESSAGES,											i18n("Invokes make to create the message file by extracting all i18n() macros"))
  ON_STATUS_MSG(ID_PROJECT_MAKE_PROJECT_API,        			i18n("Creates the Project's API with KDoc"))
  ON_STATUS_MSG(ID_PROJECT_MAKE_USER_MANUAL,        			i18n("Creates the Project's User Manual with the sgml-file"))
	ON_STATUS_MSG(ID_PROJECT_MAKE_DISTRIBUTION,							i18n("Creates distribution packages from the current project"))
	ON_STATUS_MSG(ID_PROJECT_MAKE_DISTRIBUTION_SOURCE_TGZ,  i18n("Creates a tar.gz file from the current project sources"))

  ON_STATUS_MSG(ID_BUILD_COMPILE_FILE,                    i18n("Compiles the current sourcefile"))
  ON_STATUS_MSG(ID_KDLG_BUILD_GENERATE,                   i18n("Generates the sourcefiles for the dialog"))
  ON_STATUS_MSG(ID_BUILD_MAKE,                    			  i18n("Invokes make-command"))
  ON_STATUS_MSG(ID_BUILD_REBUILD_ALL,             			  i18n("Rebuilds the program"))
  ON_STATUS_MSG(ID_BUILD_CLEAN_REBUILD_ALL,       			  i18n("Invokes make clean and rebuild all"))
  ON_STATUS_MSG(ID_BUILD_STOP,                    			  i18n("Stops make immediately"))
  ON_STATUS_MSG(ID_BUILD_RUN,                     			  i18n("Invokes make-command and runs the program"))
  ON_STATUS_MSG(ID_BUILD_RUN_WITH_ARGS,										i18n("Lets you set run-arguments to the binary and invokes the make-command"))

  ON_STATUS_MSG(ID_BUILD_DEBUG,                   			  i18n("Invokes make and KDbg debugging the binary"))
  ON_STATUS_MSG(ID_BUILD_DISTCLEAN,               			  i18n("Invokes make distclean and deletes all compiled files"))
  ON_STATUS_MSG(ID_BUILD_AUTOCONF,                			  i18n("Invokes automake and co."))
  ON_STATUS_MSG(ID_BUILD_CONFIGURE,               			  i18n("Invokes ./configure"))
 

  ON_STATUS_MSG(ID_TOOLS_KDLGEDIT,												i18n("Changes to the KDevelop dialogeditor"))
  ON_STATUS_MSG(ID_KDLG_TOOLS_KDEVELOP,										i18n("Changes to KDevelop project editor"))

  ON_STATUS_MSG(ID_OPTIONS_EDITOR,              			    i18n("Sets the Editor's behavoir"))
  ON_STATUS_MSG(ID_OPTIONS_EDITOR_COLORS,       			    i18n("Sets the Editor's colors"))
  ON_STATUS_MSG(ID_OPTIONS_SYNTAX_HIGHLIGHTING_DEFAULTS,  i18n("Sets the highlighting default colors"))
  ON_STATUS_MSG(ID_OPTIONS_SYNTAX_HIGHLIGHTING, 			    i18n("Sets the highlighting colors"))
  ON_STATUS_MSG(ID_OPTIONS_DOCBROWSER,     	  				    i18n("Configures the Browser options"))
	ON_STATUS_MSG(ID_OPTIONS_TOOLS_CONFIG_DLG,							i18n("Configures the Tools-Menu entries"))
  ON_STATUS_MSG(ID_OPTIONS_PRINT,       			            i18n("Configures printing options"))
  ON_STATUS_MSG(ID_OPTIONS_PRINT_ENSCRIPT,       	        i18n("Configures the printer to use enscript"))
  ON_STATUS_MSG(ID_OPTIONS_PRINT_A2PS,       			        i18n("Configures the printer to use a2ps"))
  ON_STATUS_MSG(ID_OPTIONS_KDEVELOP,              		    i18n("Configures KDevelop"))

  ON_STATUS_MSG(ID_BOOKMARKS_SET,													i18n("Sets a bookmark to the current window file"))
  ON_STATUS_MSG(ID_BOOKMARKS_ADD,													i18n("Adds a bookmark to the current window file"))
  ON_STATUS_MSG(ID_BOOKMARKS_CLEAR,												i18n("Clears the bookmark list for the current window"))

  ON_STATUS_MSG(ID_HELP_BACK,                      			  i18n("Switchs to last browser page"))
  ON_STATUS_MSG(ID_HELP_FORWARD,                   			  i18n("Switchs to next browser page"))

  ON_STATUS_MSG(ID_HELP_BROWSER_RELOAD,                   i18n("Reloads the current browser page"))
  ON_STATUS_MSG(ID_HELP_BROWSER_STOP,                   	i18n("Cancels the document request"))


  ON_STATUS_MSG(ID_HELP_SEARCH_TEXT,              				i18n("Searchs the selected text in the documentation"))
  ON_STATUS_MSG(ID_HELP_SEARCH,                           i18n("Lets you search individually for an expression"))

  ON_STATUS_MSG(ID_HELP_CONTENTS,                  			  i18n("Switchs to KDevelop's User Manual"))
  ON_STATUS_MSG(ID_HELP_TUTORIAL,													i18n("Switchs to the KDevelop Programming Handbook"))
  ON_STATUS_MSG(ID_HELP_TIP_OF_DAY,												i18n("Opens the Tip of the Day dialog with hints for using KDevelop"))

  ON_STATUS_MSG(ID_HELP_REFERENCE,                			  i18n("Switchs to the C/C++-Reference"))
  ON_STATUS_MSG(ID_HELP_QT_LIBRARY,                			  i18n("Switchs to the QT-Documentation"))
  ON_STATUS_MSG(ID_HELP_KDE_CORE_LIBRARY,          			  i18n("Switchs to the KDE-Core-Documentation"))
  ON_STATUS_MSG(ID_HELP_KDE_GUI_LIBRARY,           			  i18n("Switchs to the KDE-GUI-Documentation"))
  ON_STATUS_MSG(ID_HELP_KDE_KFILE_LIBRARY,          			i18n("Switchs to the KDE-File-Documentation"))
  ON_STATUS_MSG(ID_HELP_KDE_HTML_LIBRARY,          			  i18n("Switchs to the KDE-Html-Documentation"))
  ON_STATUS_MSG(ID_HELP_PROJECT_API,		           			  i18n("Switchs to the project's API-Documentation"))
  ON_STATUS_MSG(ID_HELP_USER_MANUAL,               			  i18n("Switchs to the project's User-Manual"))

  ON_STATUS_MSG(ID_HELP_HOMEPAGE,                 			  i18n("Enter the KDevelop Homepage"))
  ON_STATUS_MSG(ID_HELP_BUG_REPORT,			                 	i18n("Sends a bug-report email to the KDevelop Team"))

  ON_STATUS_MSG(ID_HELP_DLGNOTES,                 			  i18n("Some information about the dialog editor..."))
  ON_STATUS_MSG(ID_HELP_ABOUT,                    			  i18n("Programmer's Hall of Fame..."))

  ON_STATUS_MSG(ID_CV_WIZARD,															i18n("Switchs to declaration/implementation"))
  ON_STATUS_MSG(ID_CV_VIEW_DECLARATION,										i18n("Switchs to the method's declaration"))
  ON_STATUS_MSG(ID_CV_VIEW_DEFINITION,										i18n("Switchs to the method's definition"))
  ON_STATUS_MSG(ID_CV_GRAPHICAL_VIEW,											i18n("Opens the graphical inheritance tree"))
  ON_STATUS_MSG(ID_CV_CLASS_TOOL,													i18n("Opens the classtool dialog"))
  ON_STATUS_MSG(ID_CV_CLASS_BASE_CLASSES,									i18n("Displays the inherited classes of the current class"))
  ON_STATUS_MSG(ID_CV_CLASS_DERIVED_CLASSES,							i18n("Displays the classes who inherit the current class"))
  ON_STATUS_MSG(ID_CV_FOLDER_NEW,													i18n("Creates a new folder"))
  ON_STATUS_MSG(ID_CV_FOLDER_DELETE,											i18n("Deletes the current folder"))
  ON_STATUS_MSG(ID_CV_CLASS_DELETE,												i18n("Deletes the current class"))
  ON_STATUS_MSG(ID_CV_VIEW_CLASS_DECLARATION,							i18n("Goes to the class declaration"))
  ON_STATUS_MSG(ID_CV_METHOD_NEW,													i18n("Opens the New Method dialog"))
  ON_STATUS_MSG(ID_CV_METHOD_DELETE,											i18n("Deletes the current class method"))
  ON_STATUS_MSG(ID_CV_ATTRIBUTE_NEW,											i18n("Creates a new attribute for the current class"))
  ON_STATUS_MSG(ID_CV_ATTRIBUTE_DELETE,										i18n("Deletes the current class attribute"))
  ON_STATUS_MSG(ID_CV_IMPLEMENT_VIRTUAL,									i18n("Creates a virtual method"))
  ON_STATUS_MSG(ID_CV_ADD_SLOT_SIGNAL,										i18n("Adds a signal/slot mechanism"))
  
	ON_STATUS_MSG(ID_KDLG_FILE_CLOSE,												i18n("Closes the current dialog"))
	ON_STATUS_MSG(ID_KDLG_FILE_SAVE,												i18n("Saves the current dialog"))
	ON_STATUS_MSG(ID_KDLG_FILE_SAVE_AS,											i18n("Saves the current dialog under a new filename"))
	
	ON_STATUS_MSG(ID_KDLG_VIEW_PROPVIEW,										i18n("Enables/Disables the properties window"))
	ON_STATUS_MSG(ID_KDLG_VIEW_TOOLBAR,											i18n("Enables / disables the standard toolbar"))
	ON_STATUS_MSG(ID_KDLG_VIEW_STATUSBAR,										i18n("Enables / disables the statusbar"))

	ON_STATUS_MSG(ID_KDLG_VIEW_REFRESH,											i18n("Refreshes current view"))
	ON_STATUS_MSG(ID_KDLG_VIEW_GRID,												i18n("Sets the grid size of the editing widget grid snap"))

	// LFV popups
	ON_STATUS_MSG(ID_LFV_NEW_GROUP,													i18n("Lets you create a new logical file group"))
	ON_STATUS_MSG(ID_LFV_REMOVE_GROUP,											i18n("Removes the selected logical file group"))
 	ON_STATUS_MSG(ID_LFV_GROUP_PROP,												i18n("Shows the group's properties"))
	ON_STATUS_MSG(ID_LFV_SHOW_PATH_ITEM,										i18n("Displays the absolute / relative path"))
	ON_STATUS_MSG(ID_FILE_DELETE,														i18n("Deletes the selected file"))
	
	default: slotStatusMsg(i18n("Ready"));
	}
}

















