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
#include "outputview.h"
#include "ceditwidget.h"
#include "cfinddoctextdlg.h"
#include "cexecuteargdlg.h"
#include "./kwrite/kwdoc.h"
#include "./kwrite/kwdialog.h"
#include "cbugreportdlg.h"
#include "../config.h"
#include "structdef.h"
#include "vc/versioncontrol.h"
#include "print/cconfigenscriptdlg.h"
#include "print/cconfiga2psdlg.h"
#include "plugins/pluginmanagerdlg.h"
#include "customizedlg.h"
#include "editorview.h"
#include "widgets/qextmdi/qextmdichildview.h"
#include "mdiframe.h"
#include "docbrowserview.h"
#include "componentmanager.h"
#include "fileclosealldlg.h"
#include "dlgedit/dialogview.h"
#include "dlgedit/dlgedit.h"



extern KGuiCmdManager cmdMngr;

////////////////////////
// editor commands
///////////////////////
void CKDevelop::doCursorCommand(int cmdNum) {
    if(editor_view != 0){
	editor_view->currentEditor()->doCursorCommand(cmdNum);
    }
}

void CKDevelop::doEditCommand(int cmdNum) {
  if(editor_view != 0){
      editor_view->currentEditor()->doEditCommand(cmdNum);
  }

}

void CKDevelop::doStateCommand(int cmdNum) {
  if(editor_view != 0){
      editor_view->currentEditor()->doStateCommand(cmdNum);
  }
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
    KURL url;
    slotStatusMsg(i18n("Opening file..."));
    if(project){
	url = KFileDialog::getOpenURL(prj->getProjectDir(),"*",this);
    }
    else{
    url = KFileDialog::getOpenURL(0,"*",this);
    }
    if( !url.isLocalFile() )
    {
      KMessageBox::sorry( 0L, i18n( "Only local files supported yet" ) );
      return;
    }
    if (!url.isEmpty()) // nocancel
	{
	    switchToFile(url.path());
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
   //  // if editor_view->editor isn't shown don't proceed
    if (editor_view==0)
	return;
    slotStatusMsg(i18n("Closing file..."));
    editor_view->close();
    setMainCaption();
    slotStatusMsg(i18n("Ready."));
}

bool CKDevelop::slotFileCloseAll(){
    QStringList files;
    QStringList selected_files;
    EditorView *tmp_editor_view =0;
    slotStatusMsg(i18n("Closing all files..."));


    // get all modified files

    QList<QextMdiChildView> editorviews = mdi_main_frame->childrenOfType("EditorView");
    QListIterator<QextMdiChildView> it(editorviews);
    for (; it.current(); ++it) {
	tmp_editor_view = static_cast<EditorView*>(it.current());
	if (tmp_editor_view->currentEditor()->isModified() ) {
	    files.append(tmp_editor_view->currentEditor()->getName());
	}
    }
    if(files.count() > 0){  // only if files are modified
	FileCloseAllDlg dlg(this,"DLG",&files);
	if(dlg.exec()){
	    // save all selected files
	    dlg.getSelectedFiles(&selected_files);
	    QList<QextMdiChildView> editorviews = mdi_main_frame->childrenOfType("EditorView");
	    QListIterator<QextMdiChildView> it(editorviews);
		
	    for (; it.current(); ++it) {
		tmp_editor_view = static_cast<EditorView*>(it.current());
		if(selected_files.contains(tmp_editor_view->currentEditor()->getName())){
		    saveFile(tmp_editor_view->currentEditor()->getName());
		}
		tmp_editor_view->ask_by_closing = false; // questions,no dlg boxes just closing :-)
		tmp_editor_view->close();
	    }
	}
	else{
	    return false;
	}
	
    }
    else {
	QListIterator<QextMdiChildView> it2(editorviews);
	for (; it2.current(); ++it2) {
	    EditorView *tmp_editor_view = static_cast<EditorView*>(it2.current());
	    tmp_editor_view->close();
	}
    }

    slotStatusMsg(i18n("Ready."));

    return true;
}

void CKDevelop::slotFileWasSaved(EditorView* editor){
     ComponentManager::self()->notifySavedFile(editor->currentEditor()->getName());
}


bool CKDevelop::saveFile(QString abs_filename){
    // Editors...
    cerr << "\nsaving " << abs_filename;
    QList<QextMdiChildView> editorviews = mdi_main_frame->childrenOfType("EditorView");
    QListIterator<QextMdiChildView> it(editorviews);

    for (; it.current(); ++it) {
	EditorView *tmp_editor_view = static_cast<EditorView*>(it.current());
	if(tmp_editor_view->currentEditor()->isEditing(abs_filename)){
	    if (tmp_editor_view->currentEditor()->modifiedOnDisk()) {
		if (KMessageBox::questionYesNo(this,
					       i18n("The file %1 was modified outside\n this editor. Save anyway?").arg(abs_filename))
		    == KMessageBox::No){
		    return false;
		}
	    }
	    QString sShownFilename=QFileInfo(abs_filename).fileName();
	    slotStatusMsg(i18n("Saving file ")+sShownFilename);
	    tmp_editor_view->currentEditor()->doSave();
	    ComponentManager::self()->notifySavedFile(abs_filename);
	    slotStatusMsg(i18n("Ready."));
	    return true;
	}
    }
}


void CKDevelop::slotFileSave(){
    // if editor_view->editor isn't shown don't proceed
    if (editor_view==0)
	return;

   QString filename=editor_view->currentEditor()->getName();
   saveFile(filename);
   //   saveFileFromTheCurrentEditWidget(); // save the current file
}

void CKDevelop::slotFileSaveAs(){
    slotStatusMsg(i18n("Save file as..."));

    fileSaveAs();

    setMainCaption();
    slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotFileSaveAll(){
    if(!bAutosave || !saveTimer->isActive()){
 	slotStatusMsg(i18n("Saving all changed files..."));
    }
    else{
 	slotStatusMsg(i18n("Autosaving..."));
    }

//??? F.B.   view->setUpdatesEnabled(false);

    // Editors...
    QList<QextMdiChildView> editorviews = mdi_main_frame->childrenOfType("EditorView");
    statProg->setTotalSteps(editorviews.count());
    statProg->show();
    statProg->setProgress(0);
    int i = 0;
    QListIterator<QextMdiChildView> it(editorviews);

    for (; it.current(); ++it) {
	EditorView *tmp_editor_view = static_cast<EditorView*>(it.current());
	if(tmp_editor_view->currentEditor()->isModified()){
	    if (tmp_editor_view->currentEditor()->modifiedOnDisk()) {
		if (KMessageBox::questionYesNo(this, i18n("The file %1 was modified outside\n this editor. Save anyway?").arg(tmp_editor_view->currentEditor()->getName())) == KMessageBox::No){
		}
		else{
		    tmp_editor_view->currentEditor()->doSave();
		}
	    }
	    else{
		tmp_editor_view->currentEditor()->doSave();
	    }
	}
	
	
	i++;
	statProg->setProgress(i);
    }

    statProg->hide();
    statProg->reset();

//??? F.B.    view->setUpdatesEnabled(true);
    slotStatusMsg(i18n("Ready."));
}


void CKDevelop::slotFilePrint(){
    // if editor_view isn't shown don't proceed
    if (editor_view==0)
	return;

    QString file;
    slotFileSave();

    file = editor_view->currentEditor()->getName();
    CPrintDlg* printerdlg = new CPrintDlg(this, file, "suzus");
    printerdlg->resize(600,480);
    printerdlg->exec();
    delete (printerdlg);
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
    if(editor_view != 0)
	editor_view->currentEditor()->undo();
}


void CKDevelop::slotEditRedo(){
    if(editor_view != 0)
	editor_view->currentEditor()->redo();

}
void CKDevelop::slotEditUndoHistory(){
    if(editor_view != 0)
	editor_view->currentEditor()->undoHistory();

}

void CKDevelop::slotEditCut(){
    QextMdiChildView* item = mdi_main_frame->activeWindow();
    if(item != 0){
	if ( item->inherits("DialogView")){
	    dlgedit->slotEditCut();
	}
	if(item->inherits("EditorView")){
	    EditorView* editorview = static_cast<EditorView*>(item);
	    editorview->currentEditor()->cut();
	}
	slotStatusMsg(i18n("Ready."));
    }

}


void CKDevelop::slotEditCopy(){
    QextMdiChildView* item = mdi_main_frame->activeWindow();
    if(item != 0){
	if ( item->inherits("DialogView")){
	    dlgedit->slotEditCopy();
	}
	if(item->inherits("EditorView")){
	    EditorView* editorview = static_cast<EditorView*>(item);
	    editorview->currentEditor()->copy();
	}
	
	if(item->inherits("DocBrowserView")){
	    browser_widget->slotCopyText();
	    slotStatusMsg(i18n("Ready."));
	}
    }
}


void CKDevelop::slotEditPaste(){
    QextMdiChildView* item = mdi_main_frame->activeWindow();
    if(item != 0){
	if ( item->inherits("DialogView")){
	    dlgedit->slotEditPaste();
	}
	if(item->inherits("EditorView")){
	    EditorView* editorview = static_cast<EditorView*>(item);
	    editorview->currentEditor()->paste();
	}
	slotStatusMsg(i18n("Ready."));
    }

}


void CKDevelop::slotEditInsertFile(){
  // if editor_view isn't shown don't proceed
    if(editor_view == 0)
	return;
    slotStatusMsg(i18n("Inserting file contents..."));
    editor_view->currentEditor()->insertFile();
    slotStatusMsg(i18n("Ready."));
}


void CKDevelop::slotEditSearch(){
  // if editor_view->editor isn't shown don't proceed
    if(editor_view == 0)
	return;
    slotStatusMsg(i18n("Searching..."));
    editor_view->currentEditor()->search();
    slotStatusMsg(i18n("Ready."));
}


void CKDevelop::slotEditRepeatSearch(){
    slotStatusMsg(i18n("Repeating last search..."));
    if(mdi_main_frame->activeWindow() == browser_view){
	browser_widget->findTextNext(QRegExp(doc_search_text));
    }
    else{
	// if editor_view->editor isn't shown don't proceed
	if (editor_view !=0 )
	    editor_view->currentEditor()->searchAgain();
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
    if(mdi_main_frame->activeWindow() == browser_view){
	browser_widget->selectedText();
    }
    else{
	// if editor_view->editor isn't shown don't proceed
	if (editor_view !=0)
	    {
		text = editor_view->currentEditor()->markedText();
		if(text == ""){
		    text = editor_view->currentEditor()->currentWord();
		}
	    }
    }

    if (!text.isEmpty())
	slotEditSearchInFiles(text);

    slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotEditReplace(){
  // if editor_view->editor isn't shown don't proceed
    if(editor_view == 0)
	return;

    slotStatusMsg(i18n("Replacing..."));
    editor_view->currentEditor()->replace();
    slotStatusMsg(i18n("Ready."));
}


void CKDevelop::slotEditSpellcheck(){
   if(editor_view != 0)
       editor_view->currentEditor()->spellcheck();
}


void CKDevelop::slotEditSelectAll(){

    QextMdiChildView* item = mdi_main_frame->activeWindow();
    if(item != 0){
	if ( item->inherits("DialogView")){
	    dlgedit->slotEditSelectAll();
	}
	if(item->inherits("EditorView")){
	    EditorView* editorview = static_cast<EditorView*>(item);
	    editorview->currentEditor()->selectAll();
	}
	slotStatusMsg(i18n("Ready."));
    }


}

void CKDevelop::slotEditDeselectAll(){
    QextMdiChildView* item = mdi_main_frame->activeWindow();
    if(item != 0){
	if ( item->inherits("DialogView")){
	    dlgedit->slotEditDeselectAll();
	}
	if(item->inherits("EditorView")){
	    EditorView* editorview = static_cast<EditorView*>(item);
	    editorview->currentEditor()->deselectAll();
	}
	slotStatusMsg(i18n("Ready."));
    }

}

///////////////////////////////////////////////////////////////////////////////////////
// VIEW-Menu slots
///////////////////////////////////////////////////////////////////////////////////////

void CKDevelop::slotViewGotoLine(){
  // if editor_view->editor isn't shown don't proceed
    if (editor_view==0)
	return;
    slotStatusMsg(i18n("Switching to selected line..."));
    editor_view->currentEditor()->gotoLine();
    slotStatusMsg(i18n("Ready."));
}


/** jump to the next error, based on the make output*/
void CKDevelop::slotViewNextError(){
  messages_widget->nextError();
#if 0
  TErrorMessageInfo info = error_parser->getNext();
  if(info.filename != ""){
    messages_widget->setCursorPosition(info.makeoutputline-1,0);
    switchToFile(info.filename,info.errorline-1);
    if(!bKDevelop){
      switchToKDevelop();
    }
    slotStatusMsg(messages_widget->textLine(info.makeoutputline-1));
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
#endif
}


/** jump to the previews error, based on the make output*/
void CKDevelop::slotViewPreviousError(){
  messages_widget->prevError();
#if 0
  TErrorMessageInfo info = error_parser->getPrev();
  if(info.filename != ""){
    messages_widget->setCursorPosition(info.makeoutputline-1,0);
    switchToFile(info.filename,info.errorline-1);
    if(!bKDevelop){
      switchToKDevelop();
    }
    slotStatusMsg(messages_widget->textLine(info.makeoutputline-1));
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
#endif
}


void CKDevelop::slotViewTTreeView()
{
  bool willshow = !view_menu->isItemChecked(ID_VIEW_TREEVIEW);
  view_menu->setItemChecked(ID_VIEW_TREEVIEW, willshow);
  toolBar()->setButton(ID_VIEW_TREEVIEW, willshow);
    if (willshow)
      t_tab_view->show();
    else
      t_tab_view->hide();
}

void CKDevelop::slotViewTOutputView()
{
  bool willshow = !view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW);
  view_menu->setItemChecked(ID_VIEW_OUTPUTVIEW, willshow);
  toolBar()->setButton(ID_VIEW_OUTPUTVIEW, willshow);
  if (willshow)
    o_tab_view->show();
  else
    o_tab_view->hide();
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
  }
  else{
    view_menu->setItemChecked(ID_VIEW_STATUSBAR,true);
  }
  enableStatusBar();
}

void CKDevelop::slotViewTMDIViewTaskbar(){
  if(view_menu->isItemChecked(ID_VIEW_MDIVIEWTASKBAR)){
    view_menu->setItemChecked(ID_VIEW_MDIVIEWTASKBAR,false);
    mdi_main_frame->hideViewTaskBar();
  }
  else{
    view_menu->setItemChecked(ID_VIEW_MDIVIEWTASKBAR,true);
    mdi_main_frame->showViewTaskBar();
  }
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
  QFileInfo fileinfo(editor_view->currentEditor()->getName());
  slotStatusMsg(i18n("Compiling ")+editor_view->currentEditor()->getName());
  messages_widget->prepareJob(fileinfo.dirPath());
  // get the filename of the implementation file to compile and change extension for make
  //KDEBUG1(KDEBUG_INFO,CKDEVELOP,"ObjectFile= %s",QString(fileinfo.baseName()+".o").data());
	//  cerr << "ObjectFile= " << fileinfo.baseName()+".o";
  // This should not be necessary thanks autoconf:
  //(*messages_widget) << prj->getCompilationEnvironment();
    (*messages_widget) << make_cmd << fileinfo.baseName()+".o";
    messages_widget->startJob();
}

/**
 * a) make
 * b) run
 */
void CKDevelop::slotBuildRun(){
    //    editor_view->setFocus();
//    switchToFile("/home/smeier/.bashrc");
//    return;
  slotBuildMake();
  slotStatusMsg(i18n("Running ")+prj->getBinPROGRAM());
  beep=false;
  next_job = "run";
}

/**
 * a) make
 * b) run_with_args
 */
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

/**
 * a) make
 * b) run a debugger
 */
void CKDevelop::slotBuildDebug(){
    // integrated debugger and external tools
}

/**
 * a) make
 */
void CKDevelop::slotBuildMake(){
  if(!CToolClass::searchProgram(make_cmd)){
    return;
  }

  //save/generate dialog if needed
  //  dlgedit->generateSourcecodeIfNeeded();
  showOutputView(true);
  setToolMenuProcess(false);
  slotFileSaveAll();
  slotStatusMsg(i18n("Running make..."));
  messages_widget->prepareJob(prj->getProjectDir() + prj->getSubDir());
  // This should not be necessary thanks autoconf:
  //(*messages_widget) << prj->getCompilationEnvironment();
  (*messages_widget) << make_cmd;
  if(!prj->getMakeOptions().isEmpty())
      (*messages_widget) << prj->getMakeOptions();
  beep = true;
  messages_widget->startJob();
}

/**
 * a) make clean
 * b) make
 */
void CKDevelop::slotBuildRebuildAll(){
  if(!CToolClass::searchProgram(make_cmd)){
    return;
  }
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


/**
 * a) make
 * b) make distclean
 * c) make -f Makefile.dist
 * d) configure
 */
 void CKDevelop::slotBuildCleanRebuildAll(){
  if(!CToolClass::searchProgram(make_cmd)){
    return;
  }
  showOutputView(true);
  setToolMenuProcess(false);
  slotFileSaveAll();
  slotStatusMsg(i18n("Running make clean and rebuilding all..."));
  messages_widget->prepareJob(prj->getProjectDir());
  (*messages_widget) << make_cmd << "distclean && " << make_cmd
		<< " -f Makefile.dist && ";
  (*messages_widget) << prj->getCompilationEnvironment();
  (*messages_widget) << "./configure " << prj->getConfigureArgs() << " && " << make_cmd;

  beep = true;
  messages_widget->startJob();
}

/**
 * a) make distclean
 */
void CKDevelop::slotBuildDistClean(){
  if(!CToolClass::searchProgram(make_cmd)){
    return;
  }
  showOutputView(true);
  setToolMenuProcess(false);
  slotFileSaveAll();
  slotStatusMsg(i18n("Running make distclean..."));
  messages_widget->clear();
  messages_widget->prepareJob(prj->getProjectDir());
  (*messages_widget) << make_cmd << "distclean";
  messages_widget->startJob();
}


/**
 * a) make -f Makefile.dist
 */
void CKDevelop::slotBuildAutoconf(){
  if(!CToolClass::searchProgram("automake")){
    return;
  }
  if(!CToolClass::searchProgram("autoconf")){
    return;
  }
  showOutputView(true);
  setToolMenuProcess(false);
  slotFileSaveAll();
  slotStatusMsg(i18n("Running autoconf/automake suite..."));
  messages_widget->prepareJob(prj->getProjectDir());
  (*messages_widget) << make_cmd;
  if(QFileInfo(QDir::current(),"Makefile.dist").exists())
      (*messages_widget) << " -f Makefile.dist";
  else
      (*messages_widget) << " -f Makefile.cvs";
  messages_widget->startJob();
  beep = true;
}

/**
 * a) configure
 */

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
  slotFileSave();
  slotFileSaveAll();
  messages_widget->prepareJob(prj->getProjectDir());
  (*messages_widget) << prj->getCompilationEnvironment();
  (*messages_widget)  << "./configure " << argdlg.getArguments();
  messages_widget->startJob();
  beep = true;
}


void CKDevelop::slotBuildStop(){
  slotStatusMsg(i18n("Killing current process..."));
  setToolMenuProcess(true);
  ComponentManager::self()->notifyCompilationAborted();
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

}



///////////////////////////////////////////////////////////////////////////////////////
// OPTIONS-Menu slots
///////////////////////////////////////////////////////////////////////////////////////

void CKDevelop::slotOptionsEditor(){
    // a unvisible editor for configuration
    CEditWidget* editor_widget = new CEditWidget(this,"temp");
    QTabDialog *qtd = new QTabDialog(this, "tabdialog", TRUE);
    EditorView* edit_view;


    slotStatusMsg(i18n("Setting up the Editor..."));
    qtd->setCaption(i18n("Options Editor"));
    editor_widget->hide();
    config->setGroup("KWrite Options");
    editor_widget->readConfig(config);
    editor_widget->doc()->readConfig(config);

    // indent options
    IndentConfigTab *indentConfig = new IndentConfigTab(qtd, editor_widget);
    qtd->addTab(indentConfig, i18n("Indent"));

    // select options
    SelectConfigTab *selectConfig = new SelectConfigTab(qtd, editor_widget);
    qtd->addTab(selectConfig, i18n("Select"));

    // edit options
    EditConfigTab *editConfig = new EditConfigTab(qtd, editor_widget);
    qtd->addTab(editConfig, i18n("Edit"));


    qtd->setOkButton(i18n("OK"));
    qtd->setCancelButton(i18n("Cancel"));


    if (qtd->exec()) {
	
	indentConfig->getData(editor_widget);
	selectConfig->getData(editor_widget);
	editConfig->getData(editor_widget);
	config->setGroup("KWrite Options");
	editor_widget->writeConfig(config);
	editor_widget->doc()->writeConfig(config);
	
	// every current editor view get this new configuration
	QList<QextMdiChildView> editorviews = mdi_main_frame->childrenOfType("EditorView");
	QListIterator<QextMdiChildView> it(editorviews);
	for (; it.current(); ++it) {
	    edit_view = static_cast<EditorView*>(it.current());
	    indentConfig->getData(edit_view->editorfirstview);
	    selectConfig->getData(edit_view->editorfirstview);
	    editConfig->getData(edit_view->editorfirstview);
	    edit_view->syncronizeSettings(); // update the settings in the second view
	}
	
    }
    delete qtd;
    slotStatusMsg(i18n("Ready."));
}
void CKDevelop::slotOptionsEditorColors(){
    // a unvisible editor for configuration
    CEditWidget* editor_widget = new CEditWidget(this,"temp");
    EditorView* edit_view;

    slotStatusMsg(i18n("Setting up the Editor's colors..."));
    editor_widget->hide();
    config->setGroup("KWrite Options");
    editor_widget->readConfig(config);
    editor_widget->doc()->readConfig(config);
    editor_widget->colDlg();
    editor_widget->writeConfig(config);
    editor_widget->doc()->writeConfig(config);


    // every current editor view get this new configuration
    QList<QextMdiChildView> editorviews = mdi_main_frame->childrenOfType("EditorView");
    QListIterator<QextMdiChildView> it(editorviews);
    for (; it.current(); ++it) {
	edit_view = static_cast<EditorView*>(it.current());
	edit_view->editorfirstview->readConfig(config);
	edit_view->editorfirstview->doc()->readConfig(config);
	edit_view->syncronizeSettings(); // update the settings in the second view
    }

    slotStatusMsg(i18n("Ready."));

}


void CKDevelop::slotOptionsSyntaxHighlightingDefaults(){
    EditorView* edit_view;
    // a unvisible editor for configuration
    CEditWidget* editor_widget = new CEditWidget(this,"temp");

    slotStatusMsg(i18n("Setting up syntax highlighting defaults..."));
    editor_widget->hide();
    config->setGroup("KWrite Options");
    editor_widget->readConfig(config);
    editor_widget->doc()->readConfig(config);
    editor_widget->hlDef();
    editor_widget->writeConfig(config);
    editor_widget->doc()->writeConfig(config);


    // every current editor view get this new configuration
    QList<QextMdiChildView> editorviews = mdi_main_frame->childrenOfType("EditorView");
    QListIterator<QextMdiChildView> it(editorviews);
    for (; it.current(); ++it) {
	edit_view = static_cast<EditorView*>(it.current());
	edit_view->editorfirstview->readConfig(config);
	edit_view->editorfirstview->doc()->readConfig(config);
	edit_view->syncronizeSettings(); // update the settings in the second view
    }
    slotStatusMsg(i18n("Ready."));
}


void CKDevelop::slotOptionsSyntaxHighlighting(){
    EditorView* edit_view;
    // a unvisible editor for configuration
    CEditWidget* editor_widget = new CEditWidget(this,"temp");

    slotStatusMsg(i18n("Setting up syntax highlighting colors..."));
    editor_widget->hide();
    config->setGroup("KWrite Options");
    editor_widget->readConfig(config);
    editor_widget->doc()->readConfig(config);
    editor_widget->hlDlg();
    editor_widget->writeConfig(config);
    editor_widget->doc()->writeConfig(config);

    // every current editor view get this new configuration
    QList<QextMdiChildView> editorviews = mdi_main_frame->childrenOfType("EditorView");
    QListIterator<QextMdiChildView> it(editorviews);
    for (; it.current(); ++it) {
	edit_view = static_cast<EditorView*>(it.current());
	edit_view->editorfirstview->readConfig(config);
	edit_view->editorfirstview->doc()->readConfig(config);
	
     }

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

void CKDevelop::slotOptionsCustomize(){
  slotStatusMsg(i18n("Setting up KDevelop..."));
  CustomizeDialog *dlg = new CustomizeDialog(this, "customize kdevelop dialog");
  ComponentManager::self()->createConfigWidgets(dlg);
  dlg->exec();
  delete dlg;
}

void CKDevelop::slotOptionsKDevelop(){
  slotStatusMsg(i18n("Setting up KDevelop..."));

  CKDevSetupDlg* setup = new CKDevSetupDlg(this, accel, cmdMngr, "Setup");
  //setup->show();
  if (setup->exec()) {
    if (setup->hasChangedPath())
        {
            ComponentManager::self()->notifyDocPathChanged();
        }

    // kwrite keys
    cmdMngr.changeAccels();
    cmdMngr.writeConfig(kapp->config());
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
        ComponentManager::self()->notifyDocPathChanged();
    }
  }
}
void CKDevelop::slotOptionsCreateSearchDatabase(){
  bool foundGlimpse = CToolClass::searchInstProgram("glimpseindex");
  bool foundHtDig = CToolClass::searchInstProgram("htdig");
  if(!foundGlimpse && !foundHtDig){
    KMessageBox::error(0, i18n("KDevelop needs either \"glimpseindex\" or \"htdig\" to work properly.\n\tPlease install one!"));
    return;
  }
  CCreateDocDatabaseDlg dlg(messages_widget, config, this,"DLG", foundGlimpse, foundHtDig);
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
#warning FIXME Bookmarks: need new implemation
// 	if(s_tab_view->getCurrentTab()==BROWSER)
// 		slotBookmarksAdd();
// 	else{
// 		if(editor_view->editor==header_widget)
// 			header_widget->setBookmark();
// 		if(editor_view->editor==cpp_widget)
// 			cpp_widget->setBookmark();
// 	}
}
void CKDevelop::slotBookmarksAdd(){
#warning FIXME Booksmarks need new implemation
// 	if(s_tab_view->getCurrentTab()==BROWSER){
// 		doc_bookmarks->clear();
// 		doc_bookmarks_list.append(browser_widget->currentURL());
// 		doc_bookmarks_title_list.append(browser_widget->currentTitle());
		
// 		uint i;
//     for ( i =0 ; i < doc_bookmarks_list.count(); i++){
//       doc_bookmarks->insertItem(BarIcon("html"),doc_bookmarks_title_list.at(i));
//     }
// 	}
// 	if(editor_view->editor==header_widget)
// 		header_widget->addBookmark();
// 	if(editor_view->editor==cpp_widget)
// 		cpp_widget->addBookmark();

}
void CKDevelop::slotBookmarksClear(){
#warning FIXME Booksmarks need new implemation
	// if(s_tab_view->getCurrentTab()==BROWSER){
// 		doc_bookmarks_list.clear();
// 		doc_bookmarks_title_list.clear();
// 		doc_bookmarks->clear();
// 	}		
// 	else{
// 		if(editor_view->editor==header_widget)
// 			header_widget->clearBookmarks();
// 		if(editor_view->editor==cpp_widget)
// 			cpp_widget->clearBookmarks();
// 	}	
}

void CKDevelop::slotBoomarksBrowserSelected(int id_){
  slotStatusMsg(i18n("Opening bookmark..."));
  slotURLSelected(doc_bookmarks_list.at(id_), QString(), 1);
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
    browser_view->setFocus();
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
    browser_view->setFocus();
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
    browser_view->setFocus();
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
    browser_view->setFocus();
    browser_widget->showURL(str);
  }

  slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotHelpBrowserReload(){
	slotStatusMsg(i18n("Reloading page..."));
  if(!bKDevelop)
      switchToKDevelop();
  browser_view->setFocus();
//  browser_widget->widget()->setFocus();
//  browser_widget->setFocus();
  browser_widget->showURL(browser_widget->currentURL(), true);
  slotStatusMsg(i18n("Ready."));
}

//*****************************************************************************
// void encodeURL(String &str)
//   Convert a normal string to a URL 'safe' string.  This means that
//   all characters not explicitly mentioned in the URL BNF will be
//   escaped.  The escape character is '%' and is followed by 2 hex
//   digits representing the octet.
//
QString encodeURL(const QString &str)
{
    QString	temp;
    static char	*digits = "0123456789ABCDEF";
    const char	*p;

    for (p = str; p && *p; p++)
    {
	if (isascii(*p) && (isdigit(*p) || isalpha(*p)))
	    temp += *p;
	else
	{
	    temp += '%';
	    temp += digits[(*p >> 4) & 0x0f];
	    temp += digits[*p & 0x0f];
	}
    }
    return temp;
}

void CKDevelop::slotHelpSearchText(QString text){
  int pos;

  useGlimpse = CToolClass::searchInstProgram("glimpse");
  useHtDig = CToolClass::searchInstProgram("htsearch");

  if (!useGlimpse && !useHtDig)
  {
    KMessageBox::error(0,i18n("KDevelop needs either \"glimpse\" or \"htsearch\" to work properly.\n\tPlease install one!"));
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
  if(useGlimpse && !QFile::exists(locateLocal("appdata",".glimpse_index"))){
    if(!useHtDig) {
      if(KMessageBox::questionYesNo(this, i18n("KDevelop couldn't find the search database.\n"
                                               "Do you want to generate it now?")) == KMessageBox::Yes){
        slotOptionsCreateSearchDatabase();
      }
      return;
    }
    useGlimpse = false;
  }
	enableCommand(ID_HELP_BROWSER_STOP);
  search_output = ""; // delete all from the last search
  search_process.clearArguments();
  if (useGlimpse)
  {
    search_process << "glimpse  -H "+ KGlobal::dirs()->saveLocation("appdata")+" -U -c -y '"+ text +"'";
    search_process.start(KShellProcess::NotifyOnExit,KShellProcess::AllOutput);
  }
  if (useHtDig)
  {
    search_process << "htsearch -c "+ locate("appdata", "tools/htdig.conf") + " \"format=&matchesperpage=30&word=" +
      encodeURL(text) + "\" | sed -e '/file:\\/\\/localhost/s//file:\\/\\//g' > " +
      locateLocal("appdata","search_result.html");
    search_process.start(KShellProcess::NotifyOnExit,KShellProcess::AllOutput);
  }
}
void CKDevelop::slotHelpSearchText(){
    QString text;
    if(mdi_main_frame->activeWindow() == browser_view){
	browser_widget->selectedText();
    }
    else{
	// if editor_view->editor isn't shown don't proceed
	if (editor_view !=0)
	    {
		text = editor_view->currentEditor()->markedText();
		if(text == ""){
		    text = editor_view->currentEditor()->currentWord();
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
  slotURLSelected("file:" + file, QString(), 1);
}


void CKDevelop::slotHelpQtLib(){
  config->setGroup("Doc_Location");
  QString doc_qt = config->readEntry("doc_qt", QT_DOCDIR);
  slotURLSelected("file:" + doc_qt + "/index.html", QString(), 1);
}


void CKDevelop::showLibsDoc(const char *libname)
{
  config->setGroup("Doc_Location");
  QString doc_kde = config->readEntry("doc_kde", KDELIBS_DOCDIR);
  QString url = "file:" + doc_kde + "/" + libname + "/index.html";
  slotURLSelected(url, QString(), 1);
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
  slotURLSelected("file:" + file, QString(), 1);
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
        slotURLSelected(api_file, QString(), 1);
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
        slotURLSelected(doc_file, QString(), 1);
    	slotStatusMsg(i18n("Ready."));
    }
  }
}

void CKDevelop::slotHelpContents(){
  QString file = CToolClass::locatehtml("kdevelop/index.html");
  slotURLSelected("file:" + file, QString(), 1);

}

void CKDevelop::slotHelpTutorial(){
  QString file = CToolClass::locatehtml("kdevelop/programming/index.html");
  slotURLSelected("file:" + file, QString(), 1);
	
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
  accel->setItemEnabled(id_,true);

//  menuBar()->setItemEnabled(id_,true);
  toolBar()->setItemEnabled(id_,true);
  toolBar(ID_BROWSER_TOOLBAR)->setItemEnabled(id_,true);
}

void CKDevelop::disableCommand(int id_)
{
  kdev_menubar->setItemEnabled(id_,false);
  accel->setItemEnabled(id_,false);

//  menuBar()->setItemEnabled(id_,false);
  toolBar()->setItemEnabled(id_,false);
  toolBar(ID_BROWSER_TOOLBAR)->setItemEnabled(id_,false);
}

void CKDevelop::slotNewStatus()
{

  int config;
  //  EditorView* editor_view = getCurrentEditorView();
  if(editor_view !=0){
      config = editor_view->currentEditor()->config();
      statusBar()->changeItem(config & cfOvr ? "OVR" : "INS",ID_STATUS_INS_OVR);
      // set new caption... maybe the file content is changed
      setMainCaption();
  }
}


void CKDevelop::slotMarkStatus() {
    EditorView* editor_view = getCurrentEditorView();
    if (editor_view != 0) {
	if (editor_view->currentEditor()->hasMarkedText()) {
	    enableCommand(ID_EDIT_CUT);
	    enableCommand(ID_EDIT_COPY);
	} else{
	    disableCommand(ID_EDIT_CUT);
	    disableCommand(ID_EDIT_COPY);
	}
    }
}



void CKDevelop::slotBROWSERMarkStatus(bool bMarked){
    if (browser_view->hasFocus()){
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
    QString text = QApplication::clipboard()->text();
    if(text.isEmpty() || browser_view->hasFocus())
	disableCommand(ID_EDIT_PASTE);
    else
	enableCommand(ID_EDIT_PASTE);
}

void CKDevelop::slotNewLineColumn()
{
    if (editor_view==0)
	return;
    QString str = i18n("Line: %1 Col: %2")
	.arg(editor_view->currentEditor()->currentLine()+1)
	.arg(editor_view->currentEditor()->currentColumn()+1);
    statusBar()->changeItem(str, ID_STATUS_LN_CLM);
}
void CKDevelop::slotNewUndo(){
    if(editor_view !=0){
	int state;
	state = (editor_view->currentEditor()) ? editor_view->currentEditor()->undoState() : 0;
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
}


void CKDevelop::slotURLSelected(const QString &url, const QString &, int){
//    browser_widget->setFocus();

    if(url.contains("kdevelop/search_result.html") != 0){
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

void CKDevelop::slotURLonURL(const QString &url){
    statusBar()->changeItem(url.isNull()? i18n("Ready.") : url, ID_STATUS_MSG);
}

void CKDevelop::slotDocumentDone(){
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
		    else{
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

    QString filename =locateLocal("appdata","search_result.html");
    if (useHtDig)
	{
	    slotURLSelected("file:" + filename, QString(), 1);
	    return;
	}

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
   slotURLSelected("file:" + filename, QString(), 1);

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



void CKDevelop::slotProcessExited(KProcess *proc){
  setToolMenuProcess(true);
  slotStatusMsg(i18n("Ready."));
  bool ready = true;
  if (proc->normalExit()) {
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
      QString program = prj->getBinPROGRAM().lower();

      if(next_job == "run_with_args"){
        QString args = prj->getExecuteArgs();
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
      cerr << endl << "EXEC:" << exec_str;
      setToolMenuProcess(false);
      outputview->prepareJob(prj->getProjectDir() + prj->getSubDir());
      (*outputview) << exec_str;
      outputview->startJob();
      next_job = "";
      ready = false;
    }

    next_job = "";
  }
#if 0
  if (ready){ // start the error-message parser
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
#endif

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

    QList<QextMdiChildView> editorviews = mdi_main_frame->childrenOfType("EditorView");
    QListIterator<QextMdiChildView> it(editorviews);
    for (; it.current(); ++it) {
	EditorView *tmp_editor_view = static_cast<EditorView*>(it.current());
	if(tmp_editor_view->currentEditor()->modifiedOnDisk()){
	    //force reload, no modified on disc messagebox
	    switchToFile(tmp_editor_view->currentEditor()->getName(),true,false);
	}
    }
}

void CKDevelop::slotCommitDirToVCS(QString dir){
    slotFileSaveAll();
    prj->getVersionControl()->commit(dir);
    //?? can files change on disc, so we must reload them?
}
void CKDevelop::slotDocTreeSelected(const QString &filename){
  slotURLSelected("file:" + filename, QString(), 1);
}

void CKDevelop::slotTCurrentTab(int item){
    t_tab_view->setCurrentTab(item);
}


void CKDevelop::slotSwitchFileRequest(const QString &filename,int linenumber){
  switchToFile(filename,linenumber);
}

void CKDevelop::slotMDIGetFocus(QextMdiChildView* item){
    editor_view = 0;
    int type = CPP_HEADER;

    if ( item->inherits("EditorView")){
      editor_view =  static_cast<EditorView*>(item);
      type = CProject::getType(editor_view->currentEditor()->getName());
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


      slotNewUndo();
      slotNewStatus();
      slotNewLineColumn();

      int state;
      state = editor_view->currentEditor()->undoState();
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


      if (type == CPP_HEADER){
	if(bAutoswitch && t_tab_view->getCurrentTab()==DOC){	
	  if ( bDefaultCV)
	    t_tab_view->setCurrentTab(CV);
	  else
	    t_tab_view->setCurrentTab(LFV);
	}
	disableCommand(ID_BUILD_COMPILE_FILE);
      }
      if (type == CPP_SOURCE){
	if(bAutoswitch && t_tab_view->getCurrentTab()==DOC){	
	  if ( bDefaultCV)
	    t_tab_view->setCurrentTab(CV);
	  else
	    t_tab_view->setCurrentTab(LFV);
	}
	cerr << "CPP_SOURCE\n";
	if(project && build_menu->isItemEnabled(ID_BUILD_MAKE)){
	  enableCommand(ID_BUILD_COMPILE_FILE);
	}
      }
    }



    // Dialog Views --------------------------------------------

    if ( item->inherits("DialogView")){
      DialogView* dialog_view = static_cast<DialogView*>(item);
      // TODO
      dlgedit->setCurrentDialogWidget(dialog_view->dialogWidget());
      disableCommand(ID_FILE_SAVE_AS);
      enableCommand(ID_FILE_CLOSE);

      disableCommand(ID_FILE_PRINT);
      enableCommand(ID_EDIT_PASTE);

      disableCommand(ID_EDIT_INSERT_FILE);
      disableCommand(ID_EDIT_SEARCH);
      disableCommand(ID_EDIT_REPEAT_SEARCH);
      disableCommand(ID_EDIT_REPLACE);
      disableCommand(ID_EDIT_SPELLCHECK);
      disableCommand(ID_EDIT_INDENT);
      disableCommand(ID_EDIT_UNINDENT);
      enableCommand(ID_EDIT_SELECT_ALL);
      enableCommand(ID_EDIT_DESELECT_ALL);
      disableCommand(ID_EDIT_INVERT_SELECTION);
      disableCommand(ID_BUILD_COMPILE_FILE);

      if(bAutoswitch){	
	  t_tab_view->setCurrentTab(DLG);
	
      }

    }


    if(browser_view == item){ // browser selected TODO tools
	cerr << "BROWSER";
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
	
	if(bAutoswitch){
	    t_tab_view->setCurrentTab(DOC);
//	    browser_widget->widget()->setFocus();
//	    browser_widget->setFocus();
	
	    if (browser_widget->isTextSelected())
		enableCommand(ID_EDIT_COPY);
	    else
		disableCommand(ID_EDIT_COPY);

	    setMainCaption(BROWSER);
	}
	editor_view = 0;
    }



    //   if(item == TOOLS){
    //         disableCommand(ID_EDIT_COPY);
    //         setMainCaption(TOOLS);
    //   }
    //   //  s_tab_current = item;
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
	        browser_widget->closeURL();
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
      dlgedit->slotBuildGenerate();
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

void CKDevelop::slotSwitchToChildframeMode() {
  showMaximized();

  dockbase_messages_widget->manualDock(dockbase_mdi_main_frame, DockBottom);
  dockbase_grepview->manualDock(dockbase_messages_widget, DockCenter);
  dockbase_outputview->manualDock(dockbase_messages_widget, DockCenter);

  dockbase_class_tree->manualDock(dockbase_mdi_main_frame, DockLeft);
  dockbase_log_file_tree->manualDock(dockbase_class_tree, DockCenter);
  dockbase_real_file_tree->manualDock(dockbase_class_tree, DockCenter);
  dockbase_doc_tree->manualDock(dockbase_class_tree, DockCenter);
  dockbase_widprop_split_view->manualDock(dockbase_class_tree, DockCenter);
}

void CKDevelop::slotSwitchToToplevelMode() {
  int dy1 = 30;
  int dy2 = 30+5;
  int dx2 = 5;

  int tbHeight = mdi_main_frame->taskBarHeight();

  QPoint bPos;
  QPoint bGlobPos;
  QRect bGlobRect;
  int bHeight = 0;
  int bWidth = 0;
  if( DockB.dock != 0L) {
    // calculate the bottom widget geometry (mapped to desktop)
    bHeight = DockB.dock->height();
    bWidth  = DockB.dock->width();
    bPos = QPoint( DockB.dock->pos());
    bGlobPos = QPoint( DockB.dock->mapToGlobal( bPos) - bPos);
    bGlobRect = QRect( bGlobPos.x(), bGlobPos.y()+dy1+tbHeight/2, bWidth-dx2, bHeight-dy2-tbHeight/2);
  }

  QPoint lPos;
  QPoint lGlobPos;
  QRect lGlobRect;
  int lHeight = 0;
  int lWidth = 0;
  if( DockL.dock != 0L) {
    // calculate the left view geometry (mapped to desktop)
    lHeight = DockL.dock->height();
    lWidth  = DockL.dock->width();
    lPos = QPoint( DockL.dock->pos());
    lGlobPos = QPoint( DockL.dock->mapToGlobal( lPos) - lPos);
    lGlobRect = QRect( lGlobPos.x(), lGlobPos.y()+dy1+tbHeight, lWidth-dx2, lHeight-dy2-tbHeight/2 );
  }

  QPoint rPos;
  QPoint rGlobPos;
  QRect rGlobRect;
  int rHeight = 0;
  int rWidth = 0;
  if( DockR.dock != 0L) {
    // calculate the right tab view geometry (mapped to desktop)
    rHeight = DockR.dock->height();
    rWidth  = DockR.dock->width();
    rPos = QPoint( DockR.dock->pos());
    rGlobPos = QPoint( DockR.dock->mapToGlobal( rPos) - rPos);
    rGlobRect = QRect( rGlobPos.x(), rGlobPos.y()+dy1+tbHeight, rWidth-dx2, rHeight-dy2-tbHeight/2 );
  }

  QPoint tPos;
  QPoint tGlobPos;
  QRect tGlobRect;
  int tHeight = 0;
  int tWidth = 0;
  if( DockT.dock != 0L) {
    // calculate the top tab view geometry (mapped to desktop)
    tHeight = DockT.dock->height();
    tWidth  = DockT.dock->width();
    tPos = QPoint( DockT.dock->pos());
    tGlobPos = QPoint( DockT.dock->mapToGlobal( tPos) - tPos);
    tGlobRect = QRect( tGlobPos.x(), tGlobPos.y()+dy1+tbHeight, tWidth-dx2, tHeight-dy2-tbHeight/2 );
  }

  // undock the trees window and the output window (make them toplevel)
  DockWidget* dockB = DockB.dock;
  DockWidget* dockL = DockL.dock;
  DockWidget* dockR = DockR.dock;
  DockWidget* dockT = DockT.dock;
  if( DockB.dock != 0L) {
    DockB.dock->unDock();
    resize( width(), height() - bHeight);
  }
  if( DockL.dock != 0L) {
    DockL.dock->unDock();
    resize( width(), height() - lHeight + tbHeight);
  }
  if( DockR.dock != 0L) {
    DockR.dock->unDock();
    if( dockL == 0L)
      resize( width(), height() - rHeight + tbHeight);
  }
  if( DockT.dock != 0L) {
    DockT.dock->unDock();
    resize( width(), height() - tHeight);
  }

  // resize and position all on the desktop
  if( dockB != 0L) {
    dockB->setGeometry( bGlobRect);
    dockB->show();
  }
  if( dockL != 0L) {
    dockL->setGeometry( lGlobRect);
    dockL->show();
  }
  if( dockR != 0L) {
    dockR->setGeometry( rGlobRect);
    dockR->show();
  }
  if( dockT != 0L) {
    dockT->setGeometry( tGlobRect);
    dockT->show();
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
  ON_STATUS_MSG(ID_VIEW_MDIVIEWTASKBAR,          			    i18n("Enables / disables the MDI view taskbar"))

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



	ON_STATUS_MSG(ID_VIEW_GRID_DLG,												i18n("Sets the grid size of the editing widget grid snap"))

	// LFV popups
	ON_STATUS_MSG(ID_LFV_NEW_GROUP,													i18n("Lets you create a new logical file group"))
	ON_STATUS_MSG(ID_LFV_REMOVE_GROUP,											i18n("Removes the selected logical file group"))
 	ON_STATUS_MSG(ID_LFV_GROUP_PROP,												i18n("Shows the group's properties"))
	ON_STATUS_MSG(ID_LFV_SHOW_PATH_ITEM,										i18n("Displays the absolute / relative path"))
	ON_STATUS_MSG(ID_FILE_DELETE,														i18n("Deletes the selected file"))
	
	default: slotStatusMsg(i18n("Ready"));
	}
}

















