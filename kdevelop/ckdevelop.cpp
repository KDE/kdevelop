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

#include "ckdevelop.h"
#include "api.h"
#include "core.h"

#include "cclassview.h"
#include "setup/ccreatedocdatabasedlg.h"
#include "cdocbrowser.h"
//#include "ceditwidget.h"
//#include "cerrormessageparser.h"
#include "cexecuteargdlg.h"
#include "cfinddoctextdlg.h"
#include "ckdevaccel.h"
#include "ckdevsetupdlg.h"
#include "clogfileview.h"
#include "coutputwidget.h"
#include "crealfileview.h"
#include "ctoolclass.h"
#include "ctoolsconfigdlg.h"
#include "setup/cupdatekdedocdlg.h"
#include "ckonsolewidget.h"

#include "dbgtoolbar.h"
#include "dbgpsdlg.h"
#include "debug.h"
#include "doctreeview.h"
#include "grepdialog.h"
#include "structdef.h"

//#include "print/cprintdlg.h"
#include "vc/versioncontrol.h"
#include "./dbg/vartree.h"
#include "./dbg/gdbcontroller.h"
#include "./dbg/brkptmanager.h"
#include "./dbg/breakpoint.h"
#include "./dbg/framestack.h"
#include "./dbg/memview.h"
#include "./dbg/disassemble.h"

//#include "./kwrite/kwdoc.h"
#include <kate/document.h>
#include <kate/view.h>

#include "wzconnectdlgimpl.h"
#include "docviewman.h"
#include "kdevsession.h"

#include <ktip.h>
#include <kaboutdialog.h>
#include <kcombobox.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <khtmlview.h>
#include <kkeydialog.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kprinter.h>
#include <krun.h>
#include <kstatusbar.h>
#include <kstddirs.h>
#include <ktabctl.h>
#include <knotifyclient.h>
#include <kbuttonbox.h>

#include <qclipbrd.h>
#include <qdir.h>
#include <qevent.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qfont.h>
#include <qmessagebox.h>
#include <qobjectlist.h>
#include <qpaintdevicemetrics.h>
#include <qprogressbar.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qtoolbar.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qpainter.h>

#include <stdlib.h>
#include <ctype.h>

#include <X11/Xlib.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
using namespace std;

CKDevelop* CKDevelop::m_instance = 0;


#define WITH_CPP_REPARSE

///////////////////////////////////////////////////////////////////////////////////////
// FILE-Menu slots
///////////////////////////////////////////////////////////////////////////////////////

void CKDevelop::slotFileNew(){

  slotStatusMsg(i18n("Creating new file..."));
  newFile(false);
  slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotFileNew(const char* dir){

  slotStatusMsg(i18n("Creating new file..."));
  newFile(false, dir);
  slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotFileOpen(){
  slotStatusMsg(i18n("Opening file..."));

  QStringList strList;

  if(!lastOpenDir.isEmpty()) {
    strList = KFileDialog::getOpenFileNames(lastOpenDir, "*", this);
  } else if(project) {
    strList = KFileDialog::getOpenFileNames(prj->getProjectDir(), "*", this);
  } else {
    strList = KFileDialog::getOpenFileNames(QString::null, "*", this);
  }
  if (!strList.isEmpty()) {
    int lSlashPos = strList[0].findRev('/');
    if (lSlashPos != -1) {
      lastOpenDir = strList[0];
      lastOpenDir.truncate(lSlashPos);
    }
  }

  QStringList::Iterator it;
  for( it = strList.begin(); it != strList.end(); ++it )
    switchToFile(*it);

  slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotFileOpen( int id_ )
{
  slotStatusMsg(i18n("Opening file..."));

  int index;
  if ((index = file_open_popup->indexOf(id_)) >=0)
  {
    QString str=file_open_list.at(index);
    switchToFile(str);
  }

  slotStatusMsg(i18n("Ready."));
}

// closes all KWrite documents and their views but not the document browser views
void CKDevelop::slotFileCloseAll()
{
  debug("CKDevelop::slotFileCloseAll !\n");
  if (bAutosave)
    saveTimer->stop();
  slotStatusMsg(i18n("Closing all files..."));
  m_docViewManager->doFileCloseAll();
  if (bAutosave)
    saveTimer->start(saveTimeout);
  slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotFileClose()
{
  // no autosave if the user intends to save a file
  if (bAutosave)
    saveTimer->stop();
  m_docViewManager->doFileClose();
  // restart autosaving
  if (bAutosave)
    saveTimer->start(saveTimeout);
  slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotFileSave()
{
  if (!m_docViewManager->currentEditView()) return;
  QString filename = m_docViewManager->currentEditView()->getDoc()->docName();
  QString sShownFilename = QFileInfo(filename).fileName();
  slotStatusMsg(i18n("Saving file %1").arg(sShownFilename));

  if (isUntitled(filename)) {
    slotFileSaveAs();
  } else {
    m_docViewManager->checkAndSaveFileOfCurrentEditView(false);
  }

  slotStatusMsg(i18n("Ready."));
  if (m_docViewManager->currentEditDoc()->isModified())
    slotStatusHelpMsg(i18n("File %1 not saved.").arg(sShownFilename));
  else
    slotStatusHelpMsg(i18n("File %1 saved.").arg(sShownFilename));
}

void CKDevelop::slotFileSaveAs(){
    slotStatusMsg(i18n("Save file as..."));

    fileSaveAs();

    setMainCaption();
    slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotFileSaveAll()
{
  // ok,its a dirty implementation  :-)
  if(!bAutosave || !saveTimer->isActive())
    slotStatusMsg(i18n("Saving all changed files..."));
  else
    slotStatusMsg(i18n("Autosaving..."));

  //    mainSplitter->setUpdatesEnabled(false);

  m_docViewManager->saveModifiedFiles();

  if (!m_docViewManager->curDocIsBrowser()) {
      QWidget* pWdg = m_docViewManager->currentEditView();
      if (pWdg) {
        pWdg->setFocus();  // sets the main caption anyway
      }
      else {
        setMainCaption();
      }
  }
  else {
      QWidget* pWdg = m_docViewManager->currentBrowserView();
      if (pWdg) {
        pWdg->setFocus();  // sets the main caption anyway
      }
      else {
        setMainCaption(BROWSER);
      }
  }

  //  mainSplitter->setUpdatesEnabled(true);
  slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotFilePrint()
{
  if (!m_docViewManager->currentEditView())
    return;

  slotStatusMsg(i18n("Printing..."));
  slotFileSave();

  // Here's where you need to replace this and add the file selection dialog
  // also need to grab the printer setup stuff and put that on the dialog
  // activated by a button
  // then call printImpl(filesToPrint, printer);
  // if you're using qt or printer->

  QString file = m_docViewManager->currentEditView()->getDoc()->docName();
  if (file.isEmpty())
    return;

  KPrinter printer;

  // QPrinter::setup() invokes a print dialog, configures the printer object,
  // and returns TRUE if the user wants to print or FALSE if not.
  if ( printer.setup(this) )
  {
    QStringList filesToPrint;
    filesToPrint.append(file);
    printImpl(filesToPrint, &printer);
  }

  slotStatusMsg(i18n("Ready"));
}


// Prints all the files in the given list to the print device supplied
// At this point the printer device must be completely setup
//
// I based this on qt's example code but corrupted it beyound recognition.
// QUOTE
// In Qt, output to printer use the exact same code as output to screen,
// pixmaps and picture metafiles. Therefore, we don't call a QPrinter function
// to draw text, we call a QPainter function. QPainter works on all the output
// devices and has a device independent API. Most of its code is device-independent,
// too, which means that it's less likely that your application will have odd bugs.
// (If the same code is used to print as to draw on the screen, it's less likely that
// you'll have printing-only or screen-only bugs.)

void CKDevelop::printImpl(QStringList& list, KPrinter* printer)
{
  int pageNo = 1;
  QSize margins = printer->margins();
  int marginHeight = margins.height();

  slotStatusMsg(i18n("Printing..."));             // in case printing takes any time.
  QPainter p;
  if( !p.begin( printer ) )
    return;                                       // paint on printer

// hmm Should we set the font to the kwrite font?
//  p.setFont( e->font() );

  int yPos        = 0;                            // y position for each line
  QFontMetrics fm = p.fontMetrics();
  QPaintDeviceMetrics metrics( printer );         // need width/height
                                                  // of printer surface

  for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
  {
    // Need to toss a page between files. We do it here because when the
    // print ends then a new page should be tossed by the printer code _only_
    // if the user has set the system to do that.
    // ie prevents two pages being tossed at end of job
    if (it != list.begin())
    {
      // Toss to a newpage between files
      printer->newPage();
      yPos = 0;
    }

    QString fileName = (*it);
    QFile f( fileName );
    if ( !f.open( IO_ReadOnly ) )
      continue;

    QTextStream t(&f);
    while ( !t.eof() )
    {
      // read next line
      QString s = t.readLine();
      if ( marginHeight + yPos > metrics.height() - marginHeight )
      {
        // Before we print each line: Is there space for it on the current page, given the margins
        // we want to use? IF not, we want to start a new page.
        QString msg( "Printing (page " );
        msg += QString::number( ++pageNo );
        msg += ")...";
        slotStatusMsg(msg);
        printer->newPage();                         // no more room on this page
        yPos = 0;                                   // back to top of page
      }

      // prints the text
      p.drawText( marginHeight, marginHeight + yPos,
                  metrics.width(), fm.lineSpacing(),
                  ExpandTabs,
                  s );

      // Keep count of how much of the paper we've used.
      yPos = yPos + fm.lineSpacing();
    }

    // Finished this file
    f.close();
  }

  // At this point we've printed all of the text in all of the files
  // so we send job to printer - note that if they selected an output file
  // then this will do nothing - the ps file has been generated already.
  p.end();
}


void CKDevelop::slotFileQuit(){
  slotStatusMsg(i18n("Exiting..."));
  saveOptions();
  close();
}

///////////////////////////////////////////////////////////////////////////////////////
// EDIT-Menu slots
///////////////////////////////////////////////////////////////////////////////////////

void CKDevelop::slotEditSearchInFiles()
{
  slotStatusMsg(i18n("Searching in Files..."));
  if(project){
    grep_dlg->setDirName(prj->getProjectDir());
  }
  grep_dlg->show();
  grep_dlg->raise();
  slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotEditSearchInFiles(QString search)
{
  int pos;
  slotStatusMsg(i18n("Searching in Files..."));

  if(project){
    grep_dlg->setDirName(prj->getProjectDir());
  }
  grep_dlg->show();

  search.replace(QRegExp("^\n"), "");
  pos=search.find("\n");
  if (pos>-1)
   search=search.left(pos);

  search=realSearchText2regExp(search, true);
  grep_dlg->slotSearchFor(search);
  grep_dlg->raise();
  slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotEditSearchText()
{
  QString text;
  m_docViewManager->doSearchText(text);
  if (!text.isEmpty())
    slotEditSearchInFiles(text);
  slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotEditReplace()
{
  if (!m_docViewManager->currentEditView()) return;
  slotStatusMsg(i18n("Replacing..."));
  m_docViewManager->currentEditView()->replace();
  slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotEditIndent()
{
  if (m_docViewManager->currentEditView())
    m_docViewManager->currentEditView()->indent();
}
void CKDevelop::slotEditUnindent()
{
  if (m_docViewManager->currentEditView())
    m_docViewManager->currentEditView()->unIndent();
}

void CKDevelop::slotEditComment()
{
  if (m_docViewManager->currentEditView())
    m_docViewManager->currentEditView()->comment();
}

void CKDevelop::slotEditUncomment()
{
  if (m_docViewManager->currentEditView())
    m_docViewManager->currentEditView()->uncomment();
}

/*
void CKDevelop::slotEditSpellcheck()
{
  if (m_docViewManager->currentEditView())
    m_docViewManager->currentEditView()->spellcheck();
}
*/

///////////////////////////////////////////////////////////////////////////////////////
// VIEW-Menu slots
///////////////////////////////////////////////////////////////////////////////////////

void CKDevelop::slotViewGotoLine()
{
  if (!m_docViewManager->currentEditView()) return;
  slotStatusMsg(i18n("Switching to selected line..."));
  m_docViewManager->currentEditView()->gotoLine();
  slotStatusMsg(i18n("Ready."));
}
/** jump to the next error, based on the make output*/
void CKDevelop::slotViewNextError(){
  messages_widget->viewNextError();

//  TErrorMessageInfo info = error_parser->getNext();
//  if(info.filename != ""){
//    messages_widget->setCursorPosition(info.makeoutputline-1,0);
//    switchToFile(info.filename,info.errorline-1);
////    if(!bKDevelop){
////      switchToKDevelop();
////    }
//    slotStatusMsg(messages_widget->textLine(info.makeoutputline-1));
//  }
//  else{
//    XBell(kapp->getDisplay(),100); // not a next found, beep
//  }
//
//  //enable/disable the menus/toolbars
//  if(error_parser->hasNext()){
//    enableCommand(ID_VIEW_NEXT_ERROR);
//  }
//  else{
//    disableCommand(ID_VIEW_NEXT_ERROR);
//  }
//
//  if(error_parser->hasPrev()){
//    enableCommand(ID_VIEW_PREVIOUS_ERROR);
//  }
//  else{
//    disableCommand(ID_VIEW_PREVIOUS_ERROR);
//  }
}
/** jump to the previews error, based on the make output*/
void CKDevelop::slotViewPreviousError(){
  messages_widget->viewPreviousError();

//  TErrorMessageInfo info = error_parser->getPrev();
//  if(info.filename != ""){
//    messages_widget->setCursorPosition(info.makeoutputline-1,0);
//    switchToFile(info.filename,info.errorline-1);
////    if(!bKDevelop){
////      switchToKDevelop();
////    }
//    slotStatusMsg(messages_widget->textLine(info.makeoutputline-1));
//  }
//  else{
//    XBell(kapp->getDisplay(),100); // not a previous found, beep
//  }
//  //enable/disable the menus/toolbars
//  if(error_parser->hasNext()){
//    enableCommand(ID_VIEW_NEXT_ERROR);
//  }
//  else{
//    disableCommand(ID_VIEW_NEXT_ERROR);
//  }
//
//  if(error_parser->hasPrev()){
//    enableCommand(ID_VIEW_PREVIOUS_ERROR);
//  }
//  else{
//    disableCommand(ID_VIEW_PREVIOUS_ERROR);
//  }
}

/**
* This method prepares the actual toggling which is done after by toggleGroupOfToolViewCovers(..).
* It just collects the interesting tree views (actually their dockwidget covers) in a list.
*/
void CKDevelop::slotViewTTreeView()
{
  // build a list of dockwidgets that covers some tree views
  // and which are of interest at the moment
  QList<KDockWidget> dockWdgList;
  if (class_tree)
    dockWdgList.append( (KDockWidget*)class_tree->parentWidget()->parentWidget());
  if (log_file_tree)
    dockWdgList.append( (KDockWidget*)log_file_tree->parentWidget()->parentWidget());
  if (real_file_tree)
    dockWdgList.append( (KDockWidget*)real_file_tree->parentWidget()->parentWidget());
  if (doc_tree)
    dockWdgList.append( (KDockWidget*)doc_tree->parentWidget()->parentWidget());
  if (dbgController != 0L) {  // when debugging, only
    if (var_viewer)
      dockWdgList.append( (KDockWidget*)var_viewer->parentWidget()->parentWidget());
  }

  toggleGroupOfToolViewCovers(ID_VIEW_TREEVIEW, &dockWdgList);
}

/**
* This method prepares the actual toggling which is done after by toggleGroupOfToolViewCovers(..).
* It just collects the interesting output views (actually their dockwidget covers) in a list.
*/
void CKDevelop::slotViewTOutputView()
{
  // build a list of dockwidgets that covers some tree views
  // and which are of interest at the moment
  QList<KDockWidget> dockWdgList;
  if (messages_widget)
    dockWdgList.append( (KDockWidget*)messages_widget->parentWidget()->parentWidget());
  if (stdin_stdout_widget)
    dockWdgList.append( (KDockWidget*)stdin_stdout_widget->parentWidget()->parentWidget());
  if (stderr_widget)
    dockWdgList.append( (KDockWidget*)stderr_widget->parentWidget()->parentWidget());
  if (konsole_widget)
    dockWdgList.append( (KDockWidget*)konsole_widget->parentWidget()->parentWidget());
  if (brkptManager)
    dockWdgList.append( (KDockWidget*)brkptManager->parentWidget()->parentWidget());
  if (dbgController != 0L) {  // when debugging, only
    if (frameStack)
      dockWdgList.append( (KDockWidget*)frameStack->parentWidget()->parentWidget());
    if (disassemble)
      dockWdgList.append( (KDockWidget*)disassemble->parentWidget()->parentWidget());
#if defined(GDB_MONITOR) || defined(DBG_MONITOR)
    if (dbg_widget)
      dockWdgList.append( (KDockWidget*)dbg_widget->parentWidget()->parentWidget());
#endif
  }

  toggleGroupOfToolViewCovers(ID_VIEW_OUTPUTVIEW, &dockWdgList);
}

/**
* In step 1: this method checks the state of the tool-views and their positions.
* In step 2: it toggles the tool-view group from the users point of view.
*/
void CKDevelop::toggleGroupOfToolViewCovers(int type, QList<KDockWidget>* pToolViewCoverList)
{
  // find out if we have to toggle on or off (checking the toolbar button doesn't work! :( bug in kdeui?)
  bool bToggleOn = true;
  if (type == ID_VIEW_TREEVIEW) {
    if (isToolViewVisible(class_tree))
      bToggleOn = false;
  }
  else if (type == ID_VIEW_OUTPUTVIEW) {
    if (isToolViewVisible(messages_widget))
      bToggleOn = false;
  }

    //----------------------
    // Step 1: checks the state of the tool-views and their positions
    //----------------------
  QListIterator<KDockWidget> it( *pToolViewCoverList);
  QList<KDockWidget> rootDockWidgetList;
  QListIterator<KDockWidget> it2( rootDockWidgetList);
  KDockWidget* pDockW = 0L;

  // search their ancenstors (they can also be mutual ones) and store them in rootDockWidgetList
  while ((pDockW = it.current()) != 0L) { // for all of the dockwidget covers of tree views
    ++it;
    KDockWidget* pRootDockW = 0L;
    QWidget* pW = pDockW;
    // find the oldest ancestor of the current dockwidget
    // depending on if we toggle on or off
    if (bToggleOn) {
      while (pW) {
        if (pW->inherits("KDockWidget")) {
          pRootDockW = (KDockWidget*) pW;
        }
        pW = pW->parentWidget();
      }
    }
    else {
      KDockWidget* pUndockCandidate = 0L;
      bool bFinished = false;
      while (!bFinished) {
        if (pW->inherits("KDockWidget")) {
          pUndockCandidate = (KDockWidget*) pW;
          if (pUndockCandidate->enableDocking() != KDockWidget::DockNone)
            pRootDockW = pUndockCandidate;
        }
        if (pW->isTopLevel()) {
          bFinished = true;
        }
        else {
          pW = pW->parentWidget();
        }
      }
    }

    if (pRootDockW) {
      // if that oldest ancestor is not already in the list, append it
      bool found = false;
      if (!rootDockWidgetList.isEmpty()) {
        for ( it2.toFirst(); it2.current() && !found; ++it2 ) {
          KDockWidget* pDockW = it2.current();
          if (pDockW == pRootDockW)
            found = true;
        }
        if (!found) {
          rootDockWidgetList.append( pRootDockW);
        }
      }
      else {
        rootDockWidgetList.append( pRootDockW);
      }
    }
  }

    //----------------------
    // Step 2: toggle the tool-view group from the users point of view
    //----------------------

  // now really show/hide the chosen dockwidgets as well as toggling the view menu button
  if (bToggleOn) {
    // dock back whole groups of dockwidgets
    for ( it2.toFirst(); it2.current(); ++it2 ) { // for all found root dockwidgets
      KDockWidget* pCur = it2.current();
      if (pCur->isVisible()) {
        if (pCur->isTopLevel()) {
          pCur->raise();
        }
      }
      else {
        bool bDockBackIsPossible = pCur->isDockBackPossible();
        if (bDockBackIsPossible) {
                    QWidget* oldParentWdg = pCur->parentWidget();
          pCur->dockBack();
                    if (pCur->parentWidget() == oldParentWdg) {
                      bDockBackIsPossible = false;
                    }
        }
        if (!bDockBackIsPossible) {
          // oops...somehow dockBack() failed :-(
          //
          // now try a weak attempt to emulate a dockBack() if it aint possible
          // (one should actually fix KDockWidget in a way that dockBack() is always possible)
          KDockWidget::DockPosition dockPos;
          int percent;
          if (type == ID_VIEW_TREEVIEW) {
            dockPos = KDockWidget::DockLeft;
            percent = 25;
          }
          else {
            dockPos = KDockWidget::DockBottom;
            percent = 70;
          }
          KDockWidget* pTargetDock = pCur->manualDock(m_pDockbaseAreaOfDocumentViews, dockPos, percent);
          if ((pTargetDock == pCur) || (pTargetDock == 0L)) {
            if (m_pDockbaseOfTabPage) {
              if (m_pDockbaseOfTabPage->parentDockTabGroup()) {
                KDockWidget* pDockW = dynamic_cast<KDockWidget*>(m_pDockbaseOfTabPage->parentDockTabGroup()->parentWidget());
                pCur->manualDock(pDockW, dockPos, percent);
              }
              else {
                pCur->manualDock(m_pDockbaseOfTabPage, dockPos, percent);
              }
            }
          }
          if (pCur->isTopLevel()) {
            pCur->show();
          }
        }
      }
    }
//    toolBar()->setButton(type, true);
  }
  else{
    m_bToggleToolViewsIsPending = true;
    // undock (hide) whole groups of dockwidgets
    for ( it2.toFirst(); it2.current(); ++it2 ) { // for all found root dockwidgets
      KDockWidget* pCur = it2.current();
      if (pCur->isVisible()) {
        pCur->undock();
      }
    }
    m_bToggleToolViewsIsPending = false;
//    toolBar()->setButton(type, false);
  }

  QWidget* pCurView = activeWindow();
  if (pCurView) {
    pCurView->setFocus();
  }
}

void CKDevelop::slotViewTStdToolbar(){
        KToggleAction* pViewToolbarAction = dynamic_cast<KToggleAction*>
                                            (actionCollection()->action("view_toolbar"));
        if (!pViewToolbarAction) return;
//        kdDebug() << "in CKDevelop::slotViewTStdToolbar():\n";
        if (pViewToolbarAction->isChecked()) {
//                kdDebug() << "pViewToolbarAction is checked.\n";
                toolBar("mainToolBar")->show();
        }
        else {
//                kdDebug() << "pViewToolbarAction is not checked.\n";
                toolBar("mainToolBar")->hide();
        }
}
void CKDevelop::slotViewTBrowserToolbar(){
        KToggleAction* pViewToolbarAction = dynamic_cast<KToggleAction*>
                                            (actionCollection()->action("view_browser"));
        if (!pViewToolbarAction) return;
        if (pViewToolbarAction->isChecked()) {
                toolBar(ID_BROWSER_TOOLBAR)->show();
        }
        else {
                toolBar(ID_BROWSER_TOOLBAR)->hide();
        }
}

void CKDevelop::slotViewTStatusbar(){
        KToggleAction* pViewToolbarAction = dynamic_cast<KToggleAction*>
                                            (actionCollection()->action("view_status"));
        if (!pViewToolbarAction) return;
        if (pViewToolbarAction->isChecked()) {
                statusBar()->show();
        }
        else {
                statusBar()->hide();
        }
}

void CKDevelop::slotViewMdiViewTaskbar(){
        KToggleAction* pViewToolbarAction = dynamic_cast<KToggleAction*>
                                            (actionCollection()->action("view_mdi"));
        if (!pViewToolbarAction) return;
        if (pViewToolbarAction->isChecked()) {
                showViewTaskBar();
        }
        else {
                hideViewTaskBar();
        }
}

void CKDevelop::showViewTaskBar()
{
  QextMdiMainFrm::showViewTaskBar();
}

void CKDevelop::hideViewTaskBar()
{
  QextMdiMainFrm::hideViewTaskBar();
}

void CKDevelop::slotViewRefresh(){
  refreshTrees();
}

void CKDevelop::slotViewTabIcons(){
//    KToggleAction* pToggleAction = dynamic_cast<KToggleAction*>
//                                  (actionCollection()->action("view_tab_text"));
//    pToggleAction->setChecked(false);
//    pToggleAction = dynamic_cast<KToggleAction*>
//                    (actionCollection()->action("view_tab_icons"));
//    pToggleAction->setChecked(true);
//    pToggleAction = dynamic_cast<KToggleAction*>
//                    (actionCollection()->action("view_tab_texticons"));
//    pToggleAction->setChecked(false);
    QWidget* w[5];
    w[0] = class_tree;
    w[1] = log_file_tree;
    w[2] = real_file_tree;
    w[3] = doc_tree;
    w[4] = var_viewer;
    QPixmap pm[5];
    pm[0] = SmallIcon("CVclass");
    pm[1] = SmallIcon("attach");
    pm[2] = SmallIcon("folder");
    pm[3] = SmallIcon("contents");
    pm[4] = SmallIcon("brace");
    int count = var_viewer ? 5 : 4;
    int i;
    for (i = 0; i < count; i++) {
      KDockWidget* pDockWdg = dockManager->findWidgetParentDock(w[i]->parentWidget());
      pDockWdg->setIcon(pm[i]);
      pDockWdg->undock();
      pDockWdg->setTabPageLabel("");
      pDockWdg->dockBack();
    }
}
void CKDevelop::slotViewTabText(){
//    KToggleAction* pToggleAction = dynamic_cast<KToggleAction*>
//                                  (actionCollection()->action("view_tab_text"));
//    pToggleAction->setChecked(true);
//    pToggleAction = dynamic_cast<KToggleAction*>
//                    (actionCollection()->action("view_tab_icons"));
//    pToggleAction->setChecked(false);
//    pToggleAction = dynamic_cast<KToggleAction*>
//                    (actionCollection()->action("view_tab_texticons"));
//    pToggleAction->setChecked(false);
    QWidget* w[5];
    w[0] = class_tree;
    w[1] = log_file_tree;
    w[2] = real_file_tree;
    w[3] = doc_tree;
    w[4] = var_viewer;
    QString str[5];
    str[0] = i18n("Classes");
    str[1] = i18n("Groups");
    str[2] = i18n("Files");
    str[3] = i18n("Books");
    str[4] = i18n("Watch");
    int count = var_viewer ? 5 : 4;
    int i;
    for (i = 0; i < count; i++) {
      KDockWidget* pDockWdg = dockManager->findWidgetParentDock(w[i]->parentWidget());
      pDockWdg->undock();
      pDockWdg->setIcon(QPixmap());
      pDockWdg->setTabPageLabel(str[i]);
      pDockWdg->dockBack();
    }
}
void CKDevelop::slotViewTabTextIcons(){
//    KToggleAction* pToggleAction = dynamic_cast<KToggleAction*>
//                                  (actionCollection()->action("view_tab_text"));
//    pToggleAction->setChecked(false);
//    pToggleAction = dynamic_cast<KToggleAction*>
//                    (actionCollection()->action("view_tab_icons"));
//    pToggleAction->setChecked(false);
//    pToggleAction = dynamic_cast<KToggleAction*>
//                    (actionCollection()->action("view_tab_texticons"));
//    pToggleAction->setChecked(true);
    QWidget* w[5];
    w[0] = class_tree;
    w[1] = log_file_tree;
    w[2] = real_file_tree;
    w[3] = doc_tree;
    w[4] = var_viewer;
    QString str[5];
    str[0] = i18n("Classes");
    str[1] = i18n("Groups");
    str[2] = i18n("Files");
    str[3] = i18n("Books");
    str[4] = i18n("Watch");
    QPixmap pm[5];
    pm[0] = SmallIcon("CVclass");
    pm[1] = SmallIcon("attach");
    pm[2] = SmallIcon("folder");
    pm[3] = SmallIcon("contents");
    pm[4] = SmallIcon("brace");
    int count = var_viewer ? 5 : 4;
    int i;
    for (i = 0; i < count; i++) {
      KDockWidget* pDockWdg = dockManager->findWidgetParentDock(w[i]->parentWidget());
      pDockWdg->setIcon(pm[i]);
      pDockWdg->undock();
      pDockWdg->setTabPageLabel(str[i]);
      pDockWdg->dockBack();
    }
}



///////////////////////////////////////////////////////////////////////////////////////
// BUILD-Menu slots
///////////////////////////////////////////////////////////////////////////////////////

void CKDevelop::slotBuildCompileFile(){
  Kate::Document* pCurrentDoc = m_docViewManager->currentEditDoc();
  if (!pCurrentDoc)
    return;

  if(!CToolClass::searchProgram(make_cmd)){
    return;
  }

//  error_parser->reset();
//  error_parser->toogleOn();
  showOutputView(true);
  slotFileSave();
  setToolMenuProcess(false);
  slotStatusMsg(i18n("Compiling %1").arg(pCurrentDoc->docName()));
  messages_widget->start();
  process.clearArguments();
  // get the filename of the implementation file to compile and change extension for make
  //KDEBUG1(KDEBUG_INFO,CKDEVELOP,"ObjectFile= %s",QString(fileinfo.baseName()+".o").data());
//  kdDebug() << "ObjectFile= " << fileinfo.baseName()+".o";
  QFileInfo fileinfo(pCurrentDoc->docName());
  QString actualDir=fileinfo.dirPath();
//  error_parser->setStartDir(actualDir);

         KComboBox* compile_combo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_COMPILE_CHOICE);
         QString conf=compile_combo->currentText();
  QString flags, makefile;

  if (prj->getProjectType()!="normal_empty")
  {
           QString cppflags, cflags, cxxflags, addcxxflags, ldflags, group;
    if(conf==i18n("(Default)")){
                  QDir::setCurrent(actualDir);
                   cxxflags=prj->getCXXFLAGS().simplifyWhiteSpace();
                   addcxxflags=prj->getAdditCXXFLAGS().simplifyWhiteSpace();
                   ldflags=prj->getLDFLAGS().simplifyWhiteSpace();
                   config->setGroup("Compiler");
                   QString arch=config->readEntry("Architecture","i386");
                   QString platf=config->readEntry("Platform","linux");
                   group="Compilearch "+arch+"-"+platf;
                }
    else{
            QString vpath=m_pKDevSession->getVPATHSubdir(conf);
                  QDir dir(vpath);
                  // change to VPATH subdir, create it if not existant
                  if(!dir.exists())
                          dir.mkdir(vpath);
                  actualDir=actualDir.right(actualDir.length()-prj->getProjectDir().length());
                  QDir::setCurrent(vpath+"/"+actualDir);
                   group="Compilearch "+
                                                                                           m_pKDevSession->getArchitecture(conf)+"-"+
                                                                                           m_pKDevSession->getPlatform(conf);
                   cppflags=m_pKDevSession->getCPPFLAGS(conf).simplifyWhiteSpace();
                   cflags=m_pKDevSession->getCFLAGS(conf).simplifyWhiteSpace();
                   cxxflags=m_pKDevSession->getCXXFLAGS(conf).simplifyWhiteSpace();
                   addcxxflags=m_pKDevSession->getAdditCXXFLAGS(conf).simplifyWhiteSpace();
                   ldflags=m_pKDevSession->getLDFLAGS(conf).simplifyWhiteSpace();
           }
           config->setGroup(group);
           flags += "CPP=\""+ config->readEntry("CPP","cpp")+ "\" ";
           flags += "CC=\"" + config->readEntry("CC","gcc")+ "\" ";
           flags += "CXX=\"" + config->readEntry("CXX","g++")+ "\" ";
          flags += "CPPFLAGS=\"" + cppflags + "\" ";
           if(prj->getProjectType()=="normal_c"){
                   flags += "CFLAGS=\"" + cflags + " " + cxxflags + " " + addcxxflags + "\" " ;
           }
           else{
       flags += "CFLAGS=\"" + cflags + "\" ";
       flags += "CXXFLAGS=\"" + cxxflags + " " + addcxxflags + "\" ";
           }
    flags += "LDFLAGS=\"" + ldflags+ "\" " ;
    cerr << QDir::currentDirPath() << endl;
          debug("run: %s %s %s.o\n", flags.data(), make_cmd.data(), fileinfo.baseName().data());
          process << flags << make_cmd << fileinfo.baseName()+".o";
  }
  else
  {
    QString makefile=actualDir+"/Makefile";
    process << make_cmd;
          debug("run: %s ", make_cmd.data());
    if (!QFileInfo(makefile).exists())
    {
      makefile=prj->getProjectDir()+prj->getSubDir()+"Makefile";
      if (!QFileInfo(makefile).exists())
        makefile=prj->getProjectDir()+"Makefile";
      if (QFileInfo(makefile).exists())
      {
                          process << "-f" << makefile;
              debug("-f %s ", makefile.data());
                        }
    }
          debug("%s.o\n",fileinfo.baseName().data());
    process << fileinfo.baseName()+".o";
  }


  process.start(KProcess::NotifyOnExit,KProcess::AllOutput);
}

void CKDevelop::slotBuildRun()
{
  KConfig *config=KGlobal::config();
  bool isDirty=isProjectDirty();
  int qYesNoCancel=0;
  int rebuildType;

  config->setGroup("MakeOptionsSettings");
  rebuildType=config->readNumEntry("RebuildType", 2);
  if (rebuildType==0 && isDirty)
    qYesNoCancel=QMessageBox::warning(this,i18n("Project Sources Have Been Modified"),
                    i18n("Should the project be rebuild before starting the application?"),
                    i18n("Yes"), i18n("No"), i18n("Cancel"),0,2);

  if (qYesNoCancel!=2)
  {
    beep=false;
    prj->writeProject();
    if (rebuildType==2 || (isDirty && qYesNoCancel==0))
    {
      next_job = "run";
      slotBuildMake();
    }
    else
      slotStartRun();
  }
}

void CKDevelop::slotBuildRunWithArgs()
{
  KConfig *config = KGlobal::config();
  bool isDirty = isProjectDirty();
  int qYesNoCancel = 0;
  int rebuildType;

  config->setGroup("MakeOptionsSettings");
  rebuildType=config->readNumEntry("RebuildType", 2);
  if (rebuildType==0 && isDirty)
    qYesNoCancel=QMessageBox::warning(this,i18n("Project Sources Have Been Modified"),
                    i18n("Should the project be rebuild before starting the application?"),
                    i18n("Yes"), i18n("No"), i18n("Cancel"),0,2);

  if (qYesNoCancel!=2)
  {
    beep=false;
    prj->writeProject();
    if (rebuildType==2 || (isDirty && qYesNoCancel==0))
    {
      next_job = "run_with_args";
      slotBuildMake();
    }
    else
      slotStartRun(true);
  }
}

void CKDevelop::slotStartRun(bool bWithArgs)
{
  bool bContinue=true;
   // rest from the buildRun
  appl_process.clearArguments();

         KComboBox* compile_combo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_COMPILE_CHOICE);
        QString conf=compile_combo->currentText();
  QString runFromDir;
  if(conf==i18n("(Default)"))
                runFromDir = prj->getRunFromDir();
        else{
                runFromDir = m_pKDevSession->getVPATHSubdir(conf);
                if(runFromDir.right(1)!="/")
                        runFromDir+="/";
                runFromDir+=prj->getSubDir();
        }
  QDir::setCurrent(runFromDir);

  QString libtool     = prj->getLibtool();
  QString binProgram  = prj->getExecutable();

  stdin_stdout_widget->clear();
  stderr_widget->clear();

  QString args = prj->getExecuteArgs();

  if(bWithArgs)
  {
    CExecuteArgDlg argdlg(this,i18n("Arguments"),i18n("Execute with Arguments"),args);
    if(argdlg.exec())
    {
        args=argdlg.getArguments();
        prj->setExecuteArgs(args);
        if(!args.isEmpty())
        {
          binProgram = binProgram+" "+args;
        }
    }
    else
      bContinue=false;
  }

  if (bContinue)
  {
    slotStatusMsg(i18n("Running %1 (from %2)").arg(binProgram).arg(runFromDir));
    cout << runFromDir << endl;
    // Warning: not every user has the current directory in his path !
    if(prj->getProjectType() == "normal_cpp" || prj->getProjectType() == "normal_c" || prj->getProjectType() == "normal_objc")
    {
       dockManager->findWidgetParentDock(stdin_stdout_widget->parentWidget())->makeDockVisible();
       QString term = "xterm";
       QString exec_str = term + " -e sh -c '" + binProgram + "'";

       if(CToolClass::searchInstProgram("konsole"))
       {
         term = "konsole";
       }
       if(CToolClass::searchInstProgram("ksh"))
       {
         exec_str = term + " -e ksh -c '" + binProgram +
            ";echo \"\n" + QString(i18n("Press Enter to continue!")) + "\";read'";
       }
       if(CToolClass::searchInstProgram("csh"))
       {
         exec_str = term +" -e csh -c '" + binProgram +
            ";echo \"\n" + QString(i18n("Press Enter to continue!")) + "\";$<'";
       }
       if(CToolClass::searchInstProgram("tcsh"))
       {
          exec_str =  term +" -e tcsh -c '" + binProgram +
            ";echo \"\n" + QString(i18n("Press Enter to continue!")) + "\";$<'";
       }
       if(CToolClass::searchInstProgram("bash"))
       {
          exec_str =  term +" -e bash -c '" + binProgram +
          ";echo \"\n" + QString(i18n("Press Enter to continue!")) + "\";read'";
       }
       appl_process << exec_str;
       kdDebug() << endl << "EXEC:" << exec_str;
    }
    else if (prj->isKDE2Project()) {
       const QString oldGroup = config->group();
       config->setGroup("QT2");
       QString kde2dir =  QString("KDEDIRS=") + config->readEntry("kde2dir") + " ";
       config->setGroup(oldGroup);

       appl_process << kde2dir << binProgram;
       kdDebug() << endl << "EXEC:" << kde2dir << binProgram;
       dockManager->findWidgetParentDock(stderr_widget->parentWidget())->makeDockVisible();
    }
    else if(prj->isKDEProject() || prj->isQtProject() || prj->isQt2Project())
    {
      appl_process << binProgram;
      kdDebug() << endl << "EXEC:" << binProgram;
       dockManager->findWidgetParentDock(stderr_widget->parentWidget())->makeDockVisible();
    }
    else
    {
      appl_process << binProgram;
      kdDebug() << endl << "EXEC:" << binProgram;
       dockManager->findWidgetParentDock(stderr_widget->parentWidget())->makeDockVisible();
    }

    setToolMenuProcess(false);
    appl_process.start(KProcess::NotifyOnExit,KProcess::All);
  }
}

void CKDevelop::slotDebugActivator(int id)
{
  switch (id)
  {
    case ID_DEBUG_START:
      slotBuildDebug();
      break;
    case ID_DEBUG_RUN:
      ASSERT(dbgInternal);
      slotDebugRun();
      break;
    case ID_DEBUG_RUN_CURSOR:
      ASSERT(dbgInternal);
      if (m_docViewManager->currentEditView())
        kdDebug() << "m_docViewManager->currentEditView()->slotRunToCursor() not available\n";
        //m_docViewManager->currentEditView()->slotRunToCursor();
      break;
    case ID_DEBUG_STEP:
      ASSERT(dbgInternal && dbgController);
      dbgController->slotStepInto();
      break;
    case ID_DEBUG_STEP_INST:
      ASSERT(dbgInternal && dbgController);
      dbgController->slotStepIntoIns();
      break;
    case ID_DEBUG_NEXT:
      ASSERT(dbgInternal && dbgController);
      dbgController->slotStepOver();
      break;
    case ID_DEBUG_NEXT_INST:
      ASSERT(dbgInternal && dbgController);
      dbgController->slotStepOverIns();
      break;
    case ID_DEBUG_STOP:
      ASSERT(dbgInternal);
      slotDebugStop();
      break;
    case ID_DEBUG_BREAK_INTO:
      ASSERT(dbgInternal && dbgController);
      dbgController->slotBreakInto();
      break;
    case ID_DEBUG_MEMVIEW:
      ASSERT(dbgInternal);
      slotDebugMemoryView();
      break;
    case ID_DEBUG_FINISH:
      ASSERT(dbgInternal);
      dbgController->slotStepOutOff();
      break;
  }
}

void CKDevelop::slotDebugRunToCursor()
{
  if (!dbgController)
    return;

  if (!m_docViewManager->currentEditView())
    return;
  kdDebug() << "m_docViewManager->currentEditView()->slotRunToCursor() not available\n";
  //m_docViewManager->currentEditView()->slotRunToCursor();
}

void CKDevelop::slotDebugStepInto()
{
  if (!dbgController)
    return;

  dbgController->slotStepInto();
}

void CKDevelop::slotDebugStepIntoIns()
{
  if (!dbgController)
    return;
  dbgController->slotStepIntoIns();
}

void CKDevelop::slotDebugStepOver()
{
  if (!dbgController)
    return;
  dbgController->slotStepOver();
}

void CKDevelop::slotDebugStepOverIns()
{
  if (!dbgController)
    return;

  dbgController->slotStepOverIns();
}

void CKDevelop::slotDebugBreakInto()
{
  if (!dbgController)
    return;
  dbgController->slotBreakInto();
}

void CKDevelop::slotDebugStepOutOff()
{
  if (!dbgController)
    return;

  dbgController->slotStepOutOff();
}

void CKDevelop::slotDebugInterrupt()
{
  if (!dbgController)
    return;

  dbgController->slotBreakInto();
}

void CKDevelop::slotDebugToggleBreakpoint()
{
  if (!brkptManager)
    return;

  if (!m_docViewManager->currentEditView()) return;
//  m_docViewManager->currentEditView()->slotToggleBreakpoint();
  kdDebug() << "m_docViewManager->currentEditView()->slotToggleBreakpoint() not available\n";
}


void CKDevelop::slotDebugRun()
{
  if (!dbgController)
    return;

  // and start the debugger going
  dbgController->slotRun();
}

void CKDevelop::slotDebugStop()
{
  setDebugMenuProcess(false);

  if (dbgShuttingDown || !dbgInternal)
    return;

  dbgShuttingDown = true;
  delete dbgController;
  dbgController = 0;

  brkptManager->reset();
  frameStack->clear();
  var_viewer->clear();
  disassemble->clear();
  disassemble->slotActivate(false);

#if defined(GDB_MONITOR) || defined(DBG_MONITOR)
//  dbg_widget->clear();
#endif
  if (m_docViewManager->currentEditView())
  {
//    m_docViewManager->currentEditView()->clearStepLine();
    kdDebug() << "m_docViewManager->currentEditView()->clearStepLine() not available\n";
    brkptManager->refreshBP(m_docViewManager->currentEditView()->getDoc()->docName());
  }

  if (dbgInternal && dbgController) {
    dockManager->findWidgetParentDock(var_viewer->parentWidget())->makeDockVisible();
    dockManager->findWidgetParentDock(disassemble->parentWidget())->makeDockVisible();
    dockManager->findWidgetParentDock(frameStack->parentWidget())->makeDockVisible();
  }
  else {
    dockManager->findWidgetParentDock(frameStack->parentWidget())->undock();
    dockManager->findWidgetParentDock(disassemble->parentWidget())->undock();
    dockManager->findWidgetParentDock(var_viewer->parentWidget())->undock();
  }

  frameStack->setEnabled(dbgInternal && dbgController);
  disassemble->setEnabled(dbgInternal && dbgController);
  var_viewer->setEnabled(dbgInternal && dbgController);

  // We disabled autosaving when debugging, so if they wanted
  // it we have to restart it
  if (bAutosave)
    saveTimer->start(saveTimeout);
}

void CKDevelop::slotDebugShowStepInSource(const QString& filename,int linenumber,
                                          const QString& /*address*/)
{
  if (filename.isEmpty())
  {
//  i have no other way than disabling this for now, the debugger will not be
//  functional yet, we have to see how to solve this with kate's interface
//  it should be possible though. (rokrau 6/28/01)
//    if (m_docViewManager->currentEditView())
//      m_docViewManager->currentEditView()->clearStepLine();
    kdDebug() << "m_docViewManager->currentEditView()->clearStepLine() not available\n";
  }
  else
  {
    // The editor starts at line 0 but GDB starts at line 1. Fix that now!
    switchToFile(filename);
//  i have no other way than disabling this for now, the debugger will not be
//  functional yet, we have to see how to solve this with kate's interface
//  it should be possible though. (rokrau 6/28/01)
//    m_docViewManager->currentEditView()->clearStepLine();
//    m_docViewManager->currentEditView()->setStepLine(linenumber-1);
    kdDebug() << "m_docViewManager->currentEditView()->clearStepLine() not available\n";
    kdDebug() << "m_docViewManager->currentEditView()->setStepLine() not available\n";
//    machine_widget(address);
  }
}

void CKDevelop::slotDebugGoToSourcePosition(const QString& filename,int linenumber)
{
  switchToFile(filename,linenumber);
}

void CKDevelop::slotDebugRefreshBPState(const Breakpoint* BP)
{
  if (!m_docViewManager->currentEditView()) return;
  if (BP->hasSourcePosition() &&
      (m_docViewManager->currentEditView()->getDoc()->docName() == BP->filename()))
  {
    if (BP->isActionDie())
    {
//  i have no other way than disabling this for now, the debugger will not be
//  functional yet, we have to see how to solve this with kate's interface
//  it should be possible though. (rokrau 6/28/01)
//      m_docViewManager->currentEditView()->delBreakpoint(BP->lineNo()-1);
      kdDebug() << "m_docViewManager->currentEditView()->delBreakpoint() not available\n";
      return;
    }

    // The editor starts at line 0 but GDB starts at line 1. Fix that now!
//  i have no other way than disabling this for now, the debugger will not be
//  functional yet, we have to see how to solve this with kate's interface
//  it should be possible though. (rokrau 6/28/01)
//    m_docViewManager->currentEditView()->setBreakpoint(BP->lineNo()-1, -1/*BP->id()*/, BP->isEnabled(), BP->isPending() );
    kdDebug() << "m_docViewManager->currentEditView()->setBreakpoint() not available\n";
  }
}

// All we need to do is make sure the display is uptodate.
void CKDevelop::slotDebugBPState(Breakpoint* BP)
{
  slotDebugRefreshBPState(BP);
}


void CKDevelop::slotDebugMemoryView()
{
  if (!dbgController)
    return;

  MemoryView* memoryView = new MemoryView(this, "Memory view");
  connect(  memoryView,     SIGNAL(disassemble(const QString&, const QString&)),
            dbgController,  SLOT(slotDisassemble(const QString&, const QString&)));
  connect(  memoryView,     SIGNAL(memoryDump(const QString&, const QString&)),
            dbgController,  SLOT(slotMemoryDump(const QString&, const QString&)));
  connect(  memoryView,     SIGNAL(registers()),
            dbgController,  SLOT(slotRegisters()));
  connect(  memoryView,     SIGNAL(libraries()),
            dbgController,  SLOT(slotLibraries()));

  connect(  dbgController,  SIGNAL(rawGDBMemoryDump(char*)),
            memoryView,     SLOT(slotRawGDBMemoryView(char*)));
  connect(  dbgController,  SIGNAL(rawGDBDisassemble(char*)),
            memoryView,     SLOT(slotRawGDBMemoryView(char*)));
  connect(  dbgController,  SIGNAL(rawGDBRegisters(char*)),
            memoryView,     SLOT(slotRawGDBMemoryView(char*)));
  connect(  dbgController,  SIGNAL(rawGDBLibraries(char*)),
            memoryView,     SLOT(slotRawGDBMemoryView(char*)));

  memoryView->exec();
  delete memoryView;
}

void CKDevelop::slotDebugStatus(const QString& msg, int state)
{
  QString stateIndicator("P");    // default to "paused"

  if (state & s_appBusy)
  {
    stateIndicator = "A";
//  i have no other way than disabling this for now, the debugger will not be
//  functional yet, we have to see how to solve this with kate's interface
//  it should be possible though. (rokrau 6/28/01)
//    if (m_docViewManager->currentEditView())
//      m_docViewManager->currentEditView()->clearStepLine();
    kdDebug() << "m_docViewManager->currentEditView()->clearStepLine() not available\n";
  }

  if (state & (s_dbgNotStarted|s_appNotStarted))
    stateIndicator = " ";

  if (state & s_programExited)
  {
    stateIndicator = "E";
//  i have no other way than disabling this for now, the debugger will not be
//  functional yet, we have to see how to solve this with kate's interface
//  it should be possible though. (rokrau 6/28/01)
//    if (m_docViewManager->currentEditView())
//      m_docViewManager->currentEditView()->clearStepLine();
    kdDebug() << "m_docViewManager->currentEditView()->clearStepLine() not available\n";
  }

  statusBar()->changeItem(stateIndicator, ID_STATUS_DBG);

  if (!msg.isEmpty())
    slotStatusMsg(msg);
}

void CKDevelop::slotDebugAttach()
{
         KComboBox* compile_combo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_COMPILE_CHOICE);
        QString conf=compile_combo->currentText();
  QString runFromDir;
  if(conf==i18n("(Default)"))
                runFromDir = prj->getRunFromDir();
        else{
                runFromDir = m_pKDevSession->getVPATHSubdir(conf);
                if(runFromDir.right(1)!="/")
                        runFromDir+="/";
                runFromDir+=prj->getSubDir();
        }
  QDir::setCurrent(runFromDir);
  if (dbgInternal)
  {
    QString libtool     = prj->getLibtool();;
    QString binProgram  = prj->getExecutable();

    if (dbgController)
      slotDebugStop();

    slotStatusMsg(i18n("Debug running process..."));
    // Display a dialog with a list of available processes that
    // th debugger can attach to.
    Dbg_PS_Dialog psDlg(this, "process");
    if (psDlg.exec())
    {
      if (int pid = psDlg.pidSelected())
      {
        slotStatusMsg(i18n("Attach to process %1 in %2").arg(pid).arg(dbgExternalCmd));
        setupInternalDebugger();
        dbgController->slotStart(binProgram, QString(), libtool);
        dbgController->slotAttachTo(pid);
        QDir::setCurrent(prj->getProjectDir());
      }
    }
  }
  else
    slotBuildDebug();   // Starts a debugger (external in this case)
}


void CKDevelop::slotDebugExamineCore()
{
         KComboBox* compile_combo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_COMPILE_CHOICE);
        QString conf=compile_combo->currentText();
  QString runFromDir;
  if(conf==i18n("(Default)"))
                runFromDir = prj->getRunFromDir();
        else{
                runFromDir = m_pKDevSession->getVPATHSubdir(conf);
                if(runFromDir.right(1)!="/")
                        runFromDir+="/";
                runFromDir+=prj->getSubDir();
        }
  QDir::setCurrent(runFromDir);
  if (dbgInternal)
  {
    QString libtool     = prj->getLibtool();;
    QString binProgram  = prj->getExecutable();

    if (dbgController)
      slotDebugStop();

    slotStatusMsg(i18n("Enter core file to examine..."));

    if (project)
    {
      if (QString coreFile = KFileDialog::getOpenFileName(prj->getProjectDir(),"core"))
      {
        slotStatusMsg(i18n("Examine core file %1 in %2").arg(coreFile).arg(dbgExternalCmd));
        setupInternalDebugger();
        dbgController->slotStart(binProgram, QString(), libtool);
        dbgController->slotCoreFile(coreFile);
//        QDir::setCurrent(prj->getProjectDir());
     }
    }
  }
  else
    slotBuildDebug();   // Starts a debugger (external in this case)
}

void CKDevelop::slotDebugNamedFile()
{
  if (dbgInternal)
  {
    if (dbgController)
      slotDebugStop();

    slotStatusMsg(i18n("Enter executable to debug..."));
    if (project)
    {
      if (QString debugFile = KFileDialog::getOpenFileName(prj->getProjectDir(),"*"))
      {
        slotStatusMsg(i18n("Debugging %1 in %2").arg(debugFile).arg(dbgExternalCmd));
        setupInternalDebugger();
        QDir::setCurrent(debugFile);
        dbgController->slotStart(debugFile, prj->getDebugArgs());
        brkptManager->slotSetPendingBPs();
        slotDebugRun();
      }
    }
  }
  else
    slotBuildDebug();   // Starts a debugger (external in this case)
}

// I need this one for the accel keys
void CKDevelop::slotBuildDebugStart()
{
  slotBuildDebug();
}

void CKDevelop::slotBuildDebug(bool bWithArgs)
{
  KConfig *config=KGlobal::config();
  bool isDirty=isProjectDirty();
  int qYesNoCancel=0;
  int rebuildType;

  config->setGroup("MakeOptionsSettings");
  rebuildType=config->readNumEntry("RebuildType", 2);
  if (rebuildType==0 && isDirty)
    qYesNoCancel=QMessageBox::warning(this,i18n("Project Sources Have Been Modified"),
                    i18n("Should the project be rebuild before starting the debug session?"),
                    i18n("Yes"), i18n("No"), i18n("Cancel"),0,2);

  if (qYesNoCancel!=2)
  {
//    if(!bKDevelop)
//      switchToKDevelop();

    beep=false;
    prj->writeProject();
    if (rebuildType==2 || (isDirty && qYesNoCancel==0))
    {
      if (bWithArgs)
        next_job="debug_with_args";
      else
        next_job="debug";
      slotBuildMake();
    }
    else
    {
      if (bWithArgs)
        slotStartDebugRunWithArgs();
      else
        slotStartDebug();
    }
  }
}

void CKDevelop::slotDebugRunWithArgs()
{
  slotBuildDebug(true);
}

void CKDevelop::slotStartDebugRunWithArgs()
{
         KComboBox* compile_combo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_COMPILE_CHOICE);
        QString conf=compile_combo->currentText();
  QString runFromDir;
  if(conf==i18n("(Default)"))
                runFromDir = prj->getRunFromDir();
        else{
                runFromDir = m_pKDevSession->getVPATHSubdir(conf);
                if(runFromDir.right(1)!="/")
                        runFromDir+="/";
                runFromDir+=prj->getSubDir();
        }
  QDir::setCurrent(runFromDir);
  QString libtool     = prj->getLibtool();;
  QString binProgram  = prj->getExecutable();

  QString args=prj->getDebugArgs();
  if (args.isEmpty())
    args=prj->getExecuteArgs();

  CExecuteArgDlg argdlg(this,i18n("Arguments"),i18n("Debug with arguments"), args);
  if (argdlg.exec())
  {
    args = argdlg.getArguments();
    prj->setDebugArgs(args);
    prj->writeProject();

    stdin_stdout_widget->clear();
    stderr_widget->clear();

    slotStatusMsg(i18n("Debugging %1 (with arg %2 %3)")
                        .arg(binProgram)
                        .arg(args)
                        .arg(libtool.isEmpty()? QString("") : i18n("Is the 3th arg in - Debugging %1 (with arg %2 %3)", "with libtool")));

    setupInternalDebugger();
    dbgController->slotStart(binProgram, args, libtool);
    brkptManager->slotSetPendingBPs();
    slotDebugRun();
  }
}

void CKDevelop::slotStartDebug()
{
         KComboBox* compile_combo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_COMPILE_CHOICE);
        QString conf=compile_combo->currentText();
  QString runFromDir;
  if(conf==i18n("(Default)"))
                runFromDir = prj->getRunFromDir();
        else{
                runFromDir = m_pKDevSession->getVPATHSubdir(conf);
                if(runFromDir.right(1)!="/")
                        runFromDir+="/";
                runFromDir+=prj->getSubDir();
        }
  QDir::setCurrent(runFromDir);
  QString libtool     = prj->getLibtool();;
  QString binProgram  = prj->getExecutable();

  // if we can run the application, so we can clear the Makefile.am-changed-flag
  prj->clearMakefileAmChanged();

  if (dbgInternal)
  {
    if (dbgController)
      slotDebugStop();

    stdin_stdout_widget->clear();
    stderr_widget->clear();

    slotStatusMsg(i18n("Debugging %1 (from %2 %3) in internal debugger")
                          .arg(binProgram)
                          .arg(runFromDir)
                          .arg(libtool.isEmpty()? QString("") : i18n("Is the 3rg arg in - Debugging %1 (from %2 %3) in internal debugger", "with libtool")));
    setupInternalDebugger();
    dbgController->slotStart(binProgram, QString(), libtool);
    brkptManager->slotSetPendingBPs();
    slotDebugRun();
    return;
  }

  if(!CToolClass::searchProgram(dbgExternalCmd)){
    return;
  }

  slotStatusMsg(i18n("Debugging %1 (from %2) in %3")
                    .arg(binProgram)
                    .arg(runFromDir)
                    .arg(dbgExternalCmd));

  KShellProcess process("/bin/sh");
  process << dbgExternalCmd+ " " + binProgram;
  process.start(KProcess::DontCare);
}

void CKDevelop::setDebugMenuProcess(bool enable)
{
  setToolMenuProcess(!enable);
  bool onOff = dbgInternal && enable;

//  toolBar()->setItemEnabled(ID_DEBUG_RUN,                   onOff);
//  toolBar()->setItemEnabled(ID_DEBUG_STEP,                  onOff);
//  toolBar()->setItemEnabled(ID_DEBUG_NEXT,                  onOff);
//  toolBar()->setItemEnabled(ID_DEBUG_FINISH,                onOff);

//  debug_menu->setItemEnabled(ID_DEBUG_RUN,                  onOff);
//  debug_menu->setItemEnabled(ID_DEBUG_RUN_CURSOR,           onOff);
//  debug_menu->setItemEnabled(ID_DEBUG_NEXT,                 onOff);
//  debug_menu->setItemEnabled(ID_DEBUG_NEXT_INST,            onOff);
//  debug_menu->setItemEnabled(ID_DEBUG_STEP,                 onOff);
//  debug_menu->setItemEnabled(ID_DEBUG_STEP_INST,            onOff);
//  debug_menu->setItemEnabled(ID_DEBUG_FINISH,               onOff);
//  debug_menu->setItemEnabled(ID_DEBUG_MEMVIEW,              onOff);
//  debug_menu->setItemEnabled(ID_DEBUG_BREAK_INTO,           onOff);
//  debug_menu->setItemEnabled(ID_DEBUG_STOP,                 enable);

  // now create/destroy the floating toolbar
  if (onOff && dbgController && dbgEnableFloatingToolbar)
  {
    dbgToolbar = new DbgToolbar(dbgController, this);
    dbgToolbar->show();
    connect(  dbgController,  SIGNAL(dbgStatus(const QString&,int)),
              dbgToolbar,     SLOT(slotDbgStatus(const QString&,int)));
  }
  else
  {
    // Always try and delete this when the toolbar is disabled
    delete dbgToolbar;
    dbgToolbar = 0;
  }
}

void CKDevelop::slotDebugRunUntil(const QString& filename, int line)
{
  if (dbgController)
    dbgController->slotRunUntil(filename, line);
}

void CKDevelop::setupInternalDebugger()
{
  ASSERT(!dbgController);
  if (dbgController)
    return;

  saveTimer->stop();  // stop the autosaving
//  slotStatusMsg(i18n("Running %1 (from %2) in internal debugger").arg(prj->getBinPROGRAM()).arg(prj->getRunFromDir()));

  dbgController = new GDBController(var_viewer->varTree(), frameStack);
  dbgShuttingDown = false;
  setDebugMenuProcess(true);  // MUST be after dbgController

  if (dbgInternal && dbgController) {
    dockManager->findWidgetParentDock(var_viewer->parentWidget())->makeDockVisible();
    dockManager->findWidgetParentDock(disassemble->parentWidget())->makeDockVisible();
    dockManager->findWidgetParentDock(frameStack->parentWidget())->makeDockVisible();
  }
  else {
    dockManager->findWidgetParentDock(frameStack->parentWidget())->undock();
    dockManager->findWidgetParentDock(disassemble->parentWidget())->undock();
    dockManager->findWidgetParentDock(var_viewer->parentWidget())->undock();
  }
  frameStack->setEnabled(dbgInternal && dbgController);
  disassemble->setEnabled(dbgInternal && dbgController);
  var_viewer->setEnabled(dbgInternal && dbgController);

  connect(  dbgController,    SIGNAL(rawGDBBreakpointList (char*)),
            brkptManager,     SLOT(slotParseGDBBrkptList(char*)));
  connect(  dbgController,    SIGNAL(rawGDBBreakpointSet(char*, int)),
            brkptManager,     SLOT(slotParseGDBBreakpointSet(char*, int)));
  connect(  dbgController,    SIGNAL(acceptPendingBPs()),
            brkptManager,     SLOT(slotSetPendingBPs()));
  connect(  dbgController,    SIGNAL(unableToSetBPNow(int)),
            brkptManager,     SLOT(slotUnableToSetBPNow(int)));

  connect(  dbgController,    SIGNAL(dbgStatus(const QString&,int)),
            this,             SLOT(slotDebugStatus(const QString&,int)));
  connect(  dbgController,    SIGNAL(showStepInSource(const QString&,int, const QString&)),
            this,             SLOT(slotDebugShowStepInSource(const QString&,int, const QString&)));
  connect(  dbgController,    SIGNAL(ttyStdout(const char*)),
            this,             SLOT(slotApplReceivedStdout(const char*)));
  connect(  dbgController,    SIGNAL(ttyStderr(const char*)),
            this,             SLOT(slotApplReceivedStderr(const char*)));

#if defined(GDB_MONITOR) || defined(DBG_MONITOR)
  connect(  dbgController,    SIGNAL(rawData(const QString&)),
            this,             SLOT(slotDebugReceivedStdout(const QString&)));
#endif

  connect(  brkptManager,     SIGNAL(publishBPState(Breakpoint*)),
            dbgController,    SLOT(slotBPState(Breakpoint*)));
  connect(  brkptManager,     SIGNAL(clearAllBreakpoints()),
            dbgController,    SLOT(slotClearAllBreakpoints()));

  connect(  frameStack,       SIGNAL(selectFrame(int, int, bool)),
            dbgController,    SLOT(slotSelectFrame(int, int, bool)));

  connect(  var_viewer->varTree(),  SIGNAL(expandItem(VarItem*)),
            dbgController,          SLOT(slotExpandItem(VarItem*)));
  connect(  var_viewer->varTree(),  SIGNAL(expandUserItem(VarItem*, const QCString&)),
            dbgController,          SLOT(slotExpandUserItem(VarItem*, const QCString&)));
  connect(  var_viewer->varTree(),  SIGNAL(setLocalViewState(bool)),
            dbgController,          SLOT(slotSetLocalViewState(bool)));

  connect(  disassemble,    SIGNAL(disassemble(const QString&, const QString&)),
            dbgController,  SLOT(slotDisassemble(const QString&, const QString&)));
  connect(  dbgController,  SIGNAL(showStepInSource(const QString&,int, const QString&)),
            disassemble,    SLOT(slotShowStepInSource(const QString&,int, const QString&)));
  connect(  dbgController,  SIGNAL(rawGDBDisassemble(char*)),
            disassemble,    SLOT(slotDisassemble(char*)));

  slotTCurrentTab(VAR);
}


/*
 * bool CKDevelop::RunMake(const CProject::Makefiletype type, const QString& target)
 *
 * Purpose: Run the make command according to type for the project
 *          with the given target.
 *          This function find the correct make command, make options,
 *          compiler and linker options then calls the correct makefile with
 *          the correct target.
 *          This is an attempt to clean up some of the source to build the current
 *          project and to get rid of code duplication.
 * Author:  rokrau, 3/2001
 * In:
 *          const CProject::Makefiletype type,
 *          the type of Makefile, i.e. whether we are calling the toplevel makefile
 *          or a cvs makefile.
 *          const QString& target,
 *          the maefile target. Can be QString::null
 * Out:     N/A
 * Return:  void
 */
bool CKDevelop::RunMake(const CMakefile::Type type, const QString& target)
{
  debug("RunMake with target : %s !\n", target.data());

  stateChanged( "build_project", StateReverse );
  stateChanged( "debug", StateReverse );

  // first, lets set the make command
  if(!CToolClass::searchProgram(make_cmd)){
    QMessageBox::warning(this,i18n("Running Make"),
                         i18n("Make command not found"));
    return false;
  }

  // second, lets take care of compiler and linker flags  !! formerly excluding normal_empty - why ? R.N.
  KComboBox* compile_combo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_COMPILE_CHOICE);
  QString conf=compile_combo->currentText();
  QString flags, makefile;

  if (type!=CMakefile::cvs) {

    QString cppflags, cflags, cxxflags, addcxxflags, ldflags, group;
    if( conf==i18n("(Default)") ) {

      QDir::setCurrent(prj->getProjectDir());
      cxxflags=prj->getCXXFLAGS().simplifyWhiteSpace();
      addcxxflags=prj->getAdditCXXFLAGS().simplifyWhiteSpace();
      ldflags=prj->getLDFLAGS().simplifyWhiteSpace();
      config->setGroup("Compiler");
      QString arch=config->readEntry("Architecture","i386");
      QString platf=config->readEntry("Platform","linux");
      group="Compilearch "+arch+"-"+platf;

    } else {

      QString vpath=m_pKDevSession->getVPATHSubdir(conf);
      QDir dir(vpath);

      // change to VPATH subdir, create it if not existant
      if(!dir.exists())
        dir.mkdir(vpath);

      QDir::setCurrent(vpath);
      group = "Compilearch " +
        m_pKDevSession->getArchitecture(conf)+"-"+
        m_pKDevSession->getPlatform(conf);
      cppflags=m_pKDevSession->getCPPFLAGS(conf).simplifyWhiteSpace();
      cflags=m_pKDevSession->getCFLAGS(conf).simplifyWhiteSpace();
      cxxflags=m_pKDevSession->getCXXFLAGS(conf).simplifyWhiteSpace();
      addcxxflags=m_pKDevSession->getAdditCXXFLAGS(conf).simplifyWhiteSpace();
      ldflags=m_pKDevSession->getLDFLAGS(conf).simplifyWhiteSpace();
    }

    config->setGroup(group);
    flags += "CPP=\""+ config->readEntry("CPP","cpp")+ "\" ";
    flags += "CC=\"" + config->readEntry("CC","gcc")+ "\" ";
    flags += "CXX=\"" + config->readEntry("CXX","g++")+ "\" ";
    flags += "CPPFLAGS=\"" + cppflags + "\" ";
    if(prj->getProjectType()=="normal_c"){
      flags += "CFLAGS=\"" + cflags + " " + cxxflags + " " + addcxxflags + "\" " ;
    } else {
      flags += "CFLAGS=\"" + cflags + "\" ";
      flags += "CXXFLAGS=\"" + cxxflags + " " + addcxxflags + "\" ";
    }
    flags += "LDFLAGS=\"" + ldflags+ "\" " ;
  }

  if (type==CMakefile::cvs) {
    makefile=prj->getCvsMakefile();
    if (makefile.isNull()) {
      QMessageBox::warning( this,
        i18n("Makefile Not Found"),
        i18n("The build-Makefile to create the configure-script "
            "wasn't found. Please make sure your toplevel "
            "source directory contains a build-Makefile with the "
            "respective calls to automake and autoconf to build "
            "the configure script. The filename may either be 'Makefile.cvs' "
            "or 'Makefile.dist'."));
      return false;
    }
    QDir::setCurrent(prj->getProjectDir());
  } else {
    if(conf==i18n("(Default)")){
      makefile=prj->getTopMakefile();
      /* it is possible that we dont have a makefile
         right away, then we make an extra pass at
         finding and setting one */
      if (makefile.isNull()) {
        prj->setTopMakefile();
        makefile=prj->getTopMakefile();
      }
    } else {
      if(QFileInfo("Makefile").exists())
        makefile="Makefile";
    }
  }

  // if we still dont have a makefile something is really wrong
  if (makefile.isNull()) {
    QMessageBox::warning( this,
      i18n("Makefile Not Found"),
      i18n("You want to build your project by running 'make', "
           "but there is no Makefile in this directory.\n\n"
           "Hints:\n"
           "1. You may have forgotten to create the Makefiles.\n"
           "   In that case, create them by running Build->Configure\n\n"
           "2. Alternatively, this directory does not belong to your project.\n"
           "   Check the settings in Project->Options->MakeOptions.") );
    return false;
  }

  debug("makefile : %s !\n", makefile.data());

  // set the path where make will run
  if(conf==i18n("(Default)")){ // only needed for default, VPATH dir already set above
    QString makefileDir=QFileInfo(makefile).dirPath(TRUE);
    QDir::setCurrent(makefileDir);
  }

  // Kill the debugger if it's running
  if (dbgController)
    slotDebugStop();
  showOutputView(true);
  setToolMenuProcess(false);
  slotFileSaveAll();
  messages_widget->start();

  debug("set make args !\n");

  // set the make arguments
  process.clearArguments();
  if (!flags.isEmpty())
    process << flags;
  process << make_cmd << prj->getMakeOptions() << " -f" << makefile;
  if (!target.isEmpty())
   process << target;

  debug("run: %s %s %s -f %s %s", (!flags.isEmpty() ? flags.data() : ""), make_cmd.data(), prj->getMakeOptions().data(), makefile.data(), target.data());

  if (next_job.isEmpty())
    next_job="make_end";

  // rock'n roll
  return process.start( KProcess::NotifyOnExit, KProcess::AllOutput );
}

void CKDevelop::slotBuildMake()
{
  slotStatusMsg(i18n("Running make..."));
  RunMake(CMakefile::toplevel,"");
}

void CKDevelop::slotBuildMakeClean()
{
  slotStatusMsg(i18n("Running make clean..."));
  RunMake(CMakefile::toplevel,"clean");
}

void CKDevelop::slotBuildCompileDir(const QString& target)
{
  slotStatusMsg(i18n("Running make..."));
  RunMake(CMakefile::regular,target);
}

void CKDevelop::slotBuildRebuildAll()
{
  slotStatusMsg(i18n("Running make clean and make all..."));
  RunMake(CMakefile::toplevel,"clean");
    next_job = "all";
}

void CKDevelop::slotBuildDistClean()
{
  slotStatusMsg(i18n("Running make distclean..."));
  RunMake(CMakefile::toplevel,"distclean");
}
void CKDevelop::slotBuildAutoconf()
{
  slotStatusMsg(i18n("Rebuilding autoconf..."));
  if(!CToolClass::searchProgram("automake")){
    return;
  }
  if(!CToolClass::searchProgram("autoconf")){
    return;
  }
  RunMake(CMakefile::cvs,"");
}
void CKDevelop::slotBuildCleanRebuildAll()
{
  prj->updateMakefilesAm();
  slotBuildDistClean();
  next_job = "autoconf+configure+make";
}

void CKDevelop::RunConfigure(const QString &conf, bool ask){
        QString args, shellcommand;
  QString vpath;

        if(conf==i18n("(Default)")){ // blddir=srcdir
                args=prj->getConfigureArgs();
        }
        else
  {
                args=m_pKDevSession->getConfigureArgs(conf);
        }

  if(ask){  //         only open dialog when asked
          CExecuteArgDlg argdlg(this,i18n("Arguments"),i18n("Configure with Arguments"),args);
          if(argdlg.exec()){
                        args=argdlg.getArguments();
                  if(conf==i18n("(Default)")){
                    prj->setConfigureArgs(argdlg.getArguments());
                    prj->writeProject();
                  }
                  else{
                          m_pKDevSession->setConfigureArgs(conf,argdlg.getArguments());
                  }
          }
          else{
            return;
          }
        }
  slotDebugStop();

  slotStatusMsg(i18n("Running ./configure..."));
  showOutputView(true);
  setToolMenuProcess(false);
//  error_parser->toogleOff();
  messages_widget->start();
  slotFileSaveAll();
  if(conf==i18n("(Default)"))
  {
    vpath=prj->getProjectDir();
  }
  else
  {
                vpath=m_pKDevSession->getVPATHSubdir(conf);
                QDir dir(vpath);
                // change to VPATH subdir, create it if not existant
                if(!dir.exists())
                        dir.mkdir(vpath);
         }

  QDir::setCurrent(vpath);

        QString cppflags, cflags, cxxflags, addcxxflags, ldflags, group;
        // get all other strings
  if(conf==i18n("(Default)"))
  {
                cxxflags=prj->getCXXFLAGS().simplifyWhiteSpace();
                addcxxflags=prj->getAdditCXXFLAGS().simplifyWhiteSpace();
                ldflags=prj->getLDFLAGS().simplifyWhiteSpace();
                // export CC, CXX, CPP
                config->setGroup("Compiler");
                QString arch=config->readEntry("Architecture","i386");
                QString platf=config->readEntry("Platform","linux");
                group="Compilearch "+arch+"-"+platf;
  }
        else
  {
                group="Compilearch "+
                                                                                        m_pKDevSession->getArchitecture(conf)+"-"+
                                                                                        m_pKDevSession->getPlatform(conf);
                cppflags=m_pKDevSession->getCPPFLAGS(conf).simplifyWhiteSpace();
                cflags=m_pKDevSession->getCFLAGS(conf).simplifyWhiteSpace();
                cxxflags=m_pKDevSession->getCXXFLAGS(conf).simplifyWhiteSpace();
                addcxxflags=m_pKDevSession->getAdditCXXFLAGS(conf).simplifyWhiteSpace();
                ldflags=m_pKDevSession->getLDFLAGS(conf).simplifyWhiteSpace();
        }

  config->setGroup(group);
         shellcommand += "CPP=\"" + config->readEntry("CPP","cpp") + "\" ";
         shellcommand += "CC=\"" + config->readEntry("CC","gcc") + "\" ";
         shellcommand += "CXX=\"" + config->readEntry("CXX","g++") + "\" ";
        shellcommand += "CPPFLAGS=\"" + cppflags + "\" ";
         if(prj->getProjectType()=="normal_c")
  {
                  shellcommand += "CFLAGS=\"" + cflags + " " + cxxflags + " " + addcxxflags + "\" " ;
         }
         else
  {
     shellcommand += "CFLAGS=\"" + cflags + "\" ";
     shellcommand += "CXXFLAGS=\"" + cxxflags + " " + addcxxflags + "\" ";
         }
  shellcommand += "LDFLAGS=\"" + ldflags+ "\" " ;
  // this check is only to handle a strange bug
  //   if vpath==project dir the rule for .ui files won't be accepted (Walter)
  if (vpath!=prj->getProjectDir())
     shellcommand += prj->getProjectDir() +"/configure "+ args;
  else
     shellcommand += "./configure "+ args;

        shell_process.clearArguments();
        shell_process << shellcommand;
        debug("run: %s\n", shellcommand.data());
        shell_process.start(KProcess::NotifyOnExit,KProcess::AllOutput);
  beep = true;

}
void CKDevelop::slotBuildConfigure(){
        // get current config from combo and retrieve configure args
        KComboBox* compile_combo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_COMPILE_CHOICE);
        QString curr=compile_combo->currentText();
        RunConfigure(curr, true);
}



void CKDevelop::slotBuildStop(){
  slotStatusMsg(i18n("Killing current process..."));
  slotDebugStop();
  setToolMenuProcess(true);
  process.kill();
  shell_process.kill();
  appl_process.kill();
  slotStatusMsg(i18n("Ready."));
}


///////////////////////////////////////////////////////////////////////////////////////
// TOOLS-Menu slots
///////////////////////////////////////////////////////////////////////////////////////


void CKDevelop::slotToolsTool(int tool)
{
        kdDebug() << "in CKDevelop::slotToolsTool(), sender is: " << sender()->name() << "\n";
// took out the old code and fixed this, with a "crude hack(TM)"
// i believe this must be replaced with the gideon part (rokrau 02/16/02)
        ToolAppList::Iterator it;
        for( it = toolList.begin(); it != toolList.end(); ++it ) {
                if ((*it).getExeName() == sender()->name())
                        break;
        }
        if (it == toolList.end()) return; // this can not be, right? :-)
        CToolApp toolApp = (*it);
        if ( !CToolClass::searchProgram( toolApp.getExeName() ) )
                return;

//  if(!bKDevelop)
//    switchToKDevelop();

//  showOutputView(false);

  QString argument = toolApp.getArgs();

  // This allows us to replace the macro %H with the header file name, %S with the source file name,
  // %T with the currently selected text, %W with the currently selected word
  // and %D with the project directory name.  Any others we should have?
  Kate::Document* ced = m_docViewManager->currentEditDoc() ;
  if (ced) {
    QString fName = ced->docName();
    argument.replace( QRegExp("%H"), fName );
    argument.replace( QRegExp("%S"), fName );
  }
  Kate::View* cev = m_docViewManager->currentEditView();
  if (cev) {
    QString mText = ced->selection();
    QString cWord = cev->currentWord();
    KRun::shellQuote(mText); // encode the strings for the shell
    KRun::shellQuote(cWord);
    argument.replace( QRegExp("%T"), mText );
    argument.replace( QRegExp("%W"), cWord );
  }
  if(project){
    argument.replace( QRegExp("%D"), prj->getProjectDir() );
  }

  QString process_call;
  process_call = toolApp.getExeName();

  if ( !argument.isEmpty() ) {
    process_call += " " + argument;
  }

  kdDebug() << "Tool wanted <" << process_call << ">" << endl;

//  This was the old way we did it:
//  (void) KRun::runCommand (process_call);

  // We need to create a KShellProcess otherwise the STDOUT / STDERR couldn't be catched
  // The pointer will be deleted when it emitsprocessExited (KProcess)
  KShellProcess* proc = new KShellProcess();

  *proc << process_call;
  if ( toolApp.isOutputCaptured() ) {
    connect ( proc, SIGNAL(receivedStdout(KProcess*, char*, int)), this, SLOT(slotReceivedStdout(KProcess*, char*, int)));
    connect ( proc, SIGNAL(receivedStderr(KProcess*, char*, int)), this, SLOT(slotReceivedStderr(KProcess*, char*, int)));
    connect ( proc, SIGNAL(processExited(KProcess*)), this, SLOT(slotToolProcessExited(KProcess*)));
    proc->start( KProcess::NotifyOnExit, KProcess::AllOutput );
  } else {
    proc->start( KProcess::DontCare, KProcess::NoCommunication );
  }

}

void CKDevelop::slotToolProcessExited (KProcess* proc)
{
  // Create a singleShot driver to safely clean up the finished Process
  if ( proc ) {
    m_FinishedToolProcesses.append(proc);
    QTimer::singleShot( 0, this, SLOT(cleanUpToolProcesses()) );
  }
  kdDebug() << "Process for Tool finished" << endl;
}

void CKDevelop::cleanUpToolProcesses()
{
  // Clean up all finished Processes that were started from the Tools menu.
  KProcess* proc;
  for ( proc=m_FinishedToolProcesses.first(); proc != 0; proc=m_FinishedToolProcesses.next() ) {
    delete proc;
  }
  m_FinishedToolProcesses.clear();
  kdDebug() << "Clean up Processes" << endl;
}


///////////////////////////////////////////////////////////////////////////////////////
// OPTIONS-Menu slots
///////////////////////////////////////////////////////////////////////////////////////

void CKDevelop::slotOptionsEditor(){
  slotStatusMsg(i18n("Setting up the Editor..."));
  m_docViewManager->doOptionsEditor();
  slotStatusMsg(i18n("Ready."));
}

// we use kate's new integrated setup dialog for this
// (rokrau 6/28/01)
//void CKDevelop::slotOptionsEditorColors(){
//  slotStatusMsg(i18n("Setting up the Editor's colors..."));
//  m_docViewManager->doOptionsEditorColors();
//  slotStatusMsg(i18n("Ready."));
//}
//
//void CKDevelop::slotOptionsSyntaxHighlightingDefaults(){
//  slotStatusMsg(i18n("Setting up syntax highlighting default colors..."));
//  m_docViewManager->doOptionsSyntaxHighlightingDefaults();
//  slotStatusMsg(i18n("Ready."));
//}
//
//void CKDevelop::slotOptionsSyntaxHighlighting(){
//  slotStatusMsg(i18n("Setting up syntax highlighting colors..."));
//  m_docViewManager->doOptionsSyntaxHighlighting();
//  slotStatusMsg(i18n("Ready."));
//}

void CKDevelop::slotOptionsDocBrowser(){
   slotStatusMsg(i18n("Configuring Documentation Browser..."));

   CDocBrowserOptionsDlg browserOptions;
   CDocBrowser* pDocBr = m_docViewManager->currentBrowserDoc();

   if ( pDocBr ) {
     connect( browserOptions.fontOptions, SIGNAL(configChanged()),
     pDocBr, SLOT(slotDocBrowserOptions()) );
     connect( browserOptions.colorOptions, SIGNAL(configChanged()),
     pDocBr, SLOT(slotDocBrowserOptions()) );
   }
   browserOptions.exec();
   slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotOptionsToolsConfigDlg(){
    slotStatusMsg(i18n("Configuring Tools-Menu entries..."));
//  CToolsConfigDlg* configdlg= new CToolsConfigDlg(this,"configdlg");
        CToolsConfigDlg configdlg(this,"configdlg");
        configdlg.exec();

//    tools_menu->clear();
    setToolmenuEntries();
    slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotOptionsSpellchecker(){

}

//void CKDevelop::slotOptionsConfigureEnscript(){
//  if (!CToolClass::searchProgram("enscript")) {
//    return;
//  }
//  enscriptconf = new CConfigEnscriptDlg(this, "confdialog");
//  enscriptconf->resize(610,510);
//  enscriptconf->exec();
//  delete (enscriptconf);
//}
//
//void CKDevelop::slotOptionsConfigureA2ps(){
//  if (!CToolClass::searchProgram("a2ps")) {
//    return;
//  }
//  a2psconf = new CConfigA2psDlg(this, "confdialog");
//  a2psconf->resize(600,430);
//  a2psconf->exec();
//  delete (a2psconf);
//}

void CKDevelop::slotOptionsKDevelop(){
  slotStatusMsg(i18n("Setting up KDevelop..."));

    int curMdiMode = mdiMode();    // ask QextMDI what the current mode is
  CKDevSetupDlg* setup= new CKDevSetupDlg( accel, this, "Setup", curMdiMode);
  Core::getInstance()->doEmitConfigWidget( setup );
  // setup->show();
  if (setup->exec())
  {
    if (setup->hasChangedPath())
    {
     doc_tree->changePathes();
     doc_tree->refresh(prj);
    }
        curMdiMode = setup->mdiMode();    // use new set mode
  }

  delete setup;

  // This might have changed
  initDebugger();

  // the MDI mode might have changed, too
  switch (curMdiMode) {
  case QextMdi::ToplevelMode:
    switchToToplevelMode();
    break;
  case QextMdi::ChildframeMode:
    switchToChildframeMode();
    break;
  case QextMdi::TabPageMode:
    switchToTabPageMode();
    break;
  default:
    break;
  }

  config->setGroup("General Options");
  // read setting whether to use the ctags search database
  bCTags = config->readBoolEntry("use_ctags", false);

  // other stuff that could have changed as well
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

void CKDevelop::slotOptionsStartupEditing(bool startupEditing){
  bStartupEditing=startupEditing;
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
  QString qtDocu = config->readEntry("doc_qt",QT_DOCDIR);
  QString kdeDocu = config->readEntry("doc_kde",KDELIBS_DOCDIR);

    QDialog parentDlg(this, "update_kde_doc_parentdlg", true);
  parentDlg.setCaption(i18n("KDE Library Documentation Update..."));
  CUpdateKDEDocDlg embeddedDlg(&shell_process, kdeDocu, qtDocu, &parentDlg, true, "update_kde_doc_dlg");
    QObject::connect(    embeddedDlg.cancel_button, SIGNAL(clicked()), &parentDlg, SLOT(reject()) );

    QVBoxLayout* vbl = new QVBoxLayout(&parentDlg);
    vbl->addWidget(&embeddedDlg);

    if (parentDlg.exec()){
    slotStatusMsg(i18n("Generating Documentation..."));
    setToolMenuProcess(false);
    if (embeddedDlg.isUpdated())
    {
        config->writeEntry("doc_kde",embeddedDlg.getDocPath());
        config->sync();
        // doc_tree->refresh(prj);
        // doing this by next_job ... if the documentation generation has finished
              next_job="doc_refresh";
    }
  }
}
void CKDevelop::slotOptionsCreateSearchDatabase(){
  bool foundGlimpse = CToolClass::searchInstProgram("glimpseindex");
  bool foundHtDig = CToolClass::searchInstProgram("htdig");
  if(!foundGlimpse && !foundHtDig){
    KMessageBox::error( 0,
                        i18n("KDevelop needs either \"glimpseindex\" or \"htdig\" to work properly.\n\tPlease install one!"),
                        i18n("Program Not Found!"));
    return;
  }

  QString qtDocu = config->readEntry("doc_qt",QT_DOCDIR);
  QString kdeDocu = config->readEntry("doc_kde",KDELIBS_DOCDIR);

    QDialog parentDlg(this, "create_doc_database_parentdlg", true);
  parentDlg.setCaption(i18n("Create Search Database..."));
  CCreateDocDatabaseDlg embeddedDlg(&parentDlg,"create_doc_database_dlg",&shell_process,kdeDocu,qtDocu,foundGlimpse, foundHtDig, false);

  KButtonBox bb( &parentDlg);
  bb.addStretch();
  QPushButton* ok_button  = bb.addButton( i18n("OK") );
  QPushButton* cancel_button  = bb.addButton( i18n("Cancel") );
  ok_button->setDefault(true);
  connect(cancel_button, SIGNAL(clicked()), &parentDlg, SLOT(reject()));
  connect(ok_button, SIGNAL(clicked()), &embeddedDlg, SLOT(slotOkClicked()));

    QVBoxLayout* vbl = new QVBoxLayout(&parentDlg, 15, 6);
    vbl->addWidget(&embeddedDlg);
    vbl->addWidget(&bb);

  if (parentDlg.exec()){
    slotStatusMsg(i18n("Creating Search Database..."));
  }

  return;

}

///////////////////////////////////////////////////////////////////////////////////////
// BOOKMARK-Menu slots
///////////////////////////////////////////////////////////////////////////////////////

void CKDevelop::slotBookmarksToggle()
{
  debug("CKDevelop::slotBookmarksToggle !\n");

    m_docViewManager->doBookmarksToggle();
}

void CKDevelop::slotBookmarksClear(){

  debug("CKDevelop::slotBookmarksClear !\n");

    m_docViewManager->doBookmarksClear();
}

void CKDevelop::slotBookmarksNext()
{
  debug("CKDevelop::slotBookmarksNext !\n");

    m_docViewManager->doBookmarksNext();
}

void CKDevelop::slotBookmarksPrevious()
{
  debug("CKDevelop::slotBookmarksPrevious !\n");

    m_docViewManager->doBookmarksPrevious();
}

void CKDevelop::openBrowserBookmark(const QString& file)
{
  slotStatusMsg(i18n("Opening bookmark..."));
  slotURLSelected(file);
  slotStatusMsg(i18n("Ready."));
}

///////////////////////////////////////////////////////////////////////////////////////
// HELP-Menu slots
///////////////////////////////////////////////////////////////////////////////////////
void CKDevelop::slotHelpBack()
{
  slotStatusMsg(i18n("Switching to last page..."));
  QString str = history_list.prev();
  if (str != 0){
    CDocBrowser* pCurBrowser = m_docViewManager->currentBrowserDoc();
    if (pCurBrowser)
      pCurBrowser->showURL(str);
  }

  //KDEBUG1(KDEBUG_INFO,CKDEVELOP,"COUNT HISTORYLIST: %d",history_list.count());
  slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotHelpForward()
{
  slotStatusMsg(i18n("Switching to next page..."));
  QString str = history_list.next();
  if (str != 0) {
    CDocBrowser* pCurBrowser = m_docViewManager->currentBrowserDoc();
    if (pCurBrowser)
      pCurBrowser->showURL(str);
  }

  slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotHelpHistoryBack(int id)
{
  slotStatusMsg(i18n("Opening history page..."));

    int index = history_prev->indexOf(id);
  QString str = history_list.at(index);
  if (str != 0) {
    CDocBrowser* pCurBrowser = m_docViewManager->currentBrowserDoc();
    if (pCurBrowser)
      pCurBrowser->showURL(str);
  }

  slotStatusMsg(i18n("Ready."));

}

void CKDevelop::slotHelpHistoryForward( int id)
{
  slotStatusMsg(i18n("Opening history page..."));

    int index = history_next->indexOf(id);
  int cur=history_list.at()+1;
  QString str = history_list.at(cur+index);
  if (str != 0) {
    CDocBrowser* pCurBrowser = m_docViewManager->currentBrowserDoc();
    if (pCurBrowser)
      pCurBrowser->showURL(str);
  }

  slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotHelpBrowserReload()
{
  slotStatusMsg(i18n("Reloading page..."));
  CDocBrowser* pCurBrowser = m_docViewManager->currentBrowserDoc();
  if (pCurBrowser) {
    pCurBrowser->view()->setFocus();
    pCurBrowser->showURL(pCurBrowser->currentURL(), true);
  }
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
    QString  temp;
    static const char  *digits = "0123456789ABCDEF";
    const char  *p;

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

void CKDevelop::slotHelpManpage(QString text)
{
  slotStatusMsg(i18n("Show Manpage on..."));
  slotURLSelected(text);
  slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotHelpSearchText(QString text){
  int pos;

  useGlimpse = CToolClass::searchInstProgram("glimpse");
  useHtDig = CToolClass::searchInstProgram("htsearch");

  if (!useGlimpse && !useHtDig)
  {
    KMessageBox::error(0,
                        i18n("KDevelop needs either \"glimpse\" or \"htsearch\" to work properly.\n\tPlease install one!"),
                        i18n("Program Not Found!"));
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
    KMessageBox::error(this,i18n("You must select a text for searching the documentation!"));
    return;
  }
  //  kdDebug() << ":" << text << ":" << endl;

  doc_search_display_text = text.copy(); // save the text
  text=realSearchText2regExp(text);  // change the text for using with regexp
  doc_search_text = text.copy();

  config->setGroup("Doc_Location");
  QString engine=config->readEntry("searchengine","htdig");

  slotStatusMsg(i18n("Searching selected text in documentation..."));
  if(engine=="glimpse" && useGlimpse && !QFile::exists(locateLocal("appdata", ".glimpse_index")))
  {
    if (!useHtDig) {
      if(KMessageBox::questionYesNo(this,
                    i18n("KDevelop couldn't find the search database.\n Do you want to generate it now?"),
                    i18n("Error...")) == KMessageBox::Yes)
      {
        slotOptionsCreateSearchDatabase();
      }
      return;
    }
    useGlimpse = false;
  }
  enableCommand(ID_HELP_BROWSER_STOP);
  search_output = ""; // delete all from the last search
  search_process.clearArguments();
  if (engine=="glimpse" && useGlimpse && QFile::exists(locateLocal("appdata", ".glimpse_index")))
  {
    search_process << "glimpse";
    search_process << "-H" << locateLocal("appdata", "");
    search_process << "-U" << "-c" << "-y" << "'" + text +"'";

    search_process.start(KShellProcess::NotifyOnExit,KShellProcess::AllOutput);
  }
  if (useHtDig && engine=="htdig" )
  {
    search_process << "htsearch -c " +
                        locate("appdata", "tools/htdig.conf") +
                        " \"format=&matchesperpage=30&words=" +
                        encodeURL(text) +"\" | sed -e '/file:\\/\\/localhost/s//file:\\/\\//g' > " +
                        locateLocal("appdata", "search_result.html");
    search_process.start(KShellProcess::NotifyOnExit,KShellProcess::AllOutput);
  }
}

void CKDevelop::slotHelpSearchText()
{
  QString text;
  m_docViewManager->doSearchText(text);
  slotHelpSearchText(text);
}

void CKDevelop::slotManpage()
{
  slotStatusMsg(i18n("Show Manpage on..."));
  CManpageTextDlg manpageDlg(this,"Manpage");
  if (manpageDlg.exec())
  {
    QString manpage = manpageDlg.manpageText();
    if (!manpage.isEmpty())
      slotHelpManpage(manpage);
  }
}

void CKDevelop::slotHelpSearch()
{
  slotStatusMsg(i18n("Searching for Help on..."));
  CFindDocTextDlg* help_srch_dlg=new CFindDocTextDlg(this,"Search_for_Help_on");
  connect(help_srch_dlg,SIGNAL(signalFind(QString)),this,SLOT(slotHelpSearchText(QString)));
  help_srch_dlg->exec();
  delete help_srch_dlg;
}


void CKDevelop::showDocHelp(const QString& filename)
{
  QString file = DocTreeKDevelopBook::locatehtml(filename);
  if (!file.isEmpty())
    slotURLSelected(file);
}

void CKDevelop::slotHelpContents(){
  showDocHelp("index.html");
}

void CKDevelop::slotHelpProgramming(){
  showDocHelp("programming/index.html");
}

void CKDevelop::slotHelpTutorial(){
  showDocHelp("tutorial/index.html");
}

void CKDevelop::slotHelpKDELibRef(){
  showDocHelp("kde_libref/index.html");
}

void CKDevelop::slotHelpReference() {
  showDocHelp("reference/C/cref.html");
}

void CKDevelop::showHelpTipOfDay(bool force){
  KTipDialog::showTip(locate("data", "kdevelop/tips"), force);
}

void CKDevelop::slotHelpHomepage(){
    new KRun("http://www.kdevelop.org");
}


void CKDevelop::slotHelpAPI(){
  if(project){
    QString api_file=prj->getProjectDir() + prj->getProjectName().lower() +  "-api/index.html";
    //MB
    KToggleAction* pToggleAction = dynamic_cast<KToggleAction*>
                                   (actionCollection()->action("project_api_doxygen"));

    if (pToggleAction && pToggleAction->isChecked())
    {
      QString api_dir =  prj->getProjectDir() + "/";
      QString doxconf = api_dir +  prj->getProjectName().lower()+".doxygen";
      if(!QFileInfo(doxconf).exists())
      {
           KMessageBox::error(0,
                       i18n("Doxygen configuration file not found\n"
                             "Generate a valid one:\n"
                             "Project->API Doc Tool->Configure doxygen"));
        return;
      }
      api_file=api_dir +  prj->getProjectName().lower() +"-api/html/index.html";
    }
    //MB end
    if(!QFileInfo(api_file).exists()){
//      int result=KMessageBox::yesNo( this, i18n("No Project API documentation !"), i18n("The Project API documentation is not present.\n"
//                                                                      "Would you like to generate it now ?"), KMessageBox::QUESTION);
//      if(result==1){
//        slotProjectAPI();
//      }
//      else{
        return;  // replaced by right mouse button handling to generate the API in the DocTreeView
//      }
    }
    else{
      slotStatusMsg(i18n("Switching to project API Documentation..."));
      slotURLSelected(api_file);
      slotStatusMsg(i18n("Ready."));
    }
  }
}
void CKDevelop::slotHelpManual(){
  if(project){

    QString name = prj->getSGMLFile().copy();
    QFileInfo finfo(name);

    QString doc_file = finfo.dirPath() + "/" + finfo.baseName()+ ".html";
    if(!QFileInfo(doc_file).exists()){
      // try docbook file projectdir/doc/HTML/index.html
      doc_file = prj->getProjectDir()+"/doc/en/HTML/index.html";
      if(!QFileInfo(doc_file).exists())
        return;   // replaced by right mouse button handling to generate the help manual in DocTreeView
      else
        slotURLSelected(doc_file);
    }
    else{
      slotStatusMsg(i18n("Switching to project Manual..."));
      slotURLSelected(doc_file);
      slotStatusMsg(i18n("Ready."));
    }
  }
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
  KComboBox* classCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_CLASS_CHOICE);
  QString classname = classCombo->currentText();

  CVGotoDeclaration( classname, "", THCLASS, THCLASS );
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
  KComboBox* classCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_CLASS_CHOICE);
  KComboBox* methodCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_METHOD_CHOICE);
  QString classname = classCombo->currentText();
  QString methodname = methodCombo->currentText();

  if(classname==i18n("(Globals)"))
      CVGotoDeclaration( classname, methodname, THFOLDER, THGLOBAL_FUNCTION );
  else
    CVGotoDeclaration( classname, methodname, THCLASS, THPUBLIC_METHOD );
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
  KComboBox* classCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_CLASS_CHOICE);
  KComboBox* methodCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_METHOD_CHOICE);
  QString classname = classCombo->currentText();
  QString methodname = methodCombo->currentText();

  if( methodname.isEmpty() && !classname==i18n("(Globals)") ){
      CVGotoDefinition( classname, "", THCLASS, THCLASS );
  }
  else{
    if(classname==i18n("(Globals)"))
      CVGotoDefinition( classname, methodname, THFOLDER, THGLOBAL_FUNCTION );
    else
      CVGotoDefinition( classname, methodname, THCLASS, THPUBLIC_METHOD );
  }
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
  KComboBox* classCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_CLASS_CHOICE);
  QString classname = classCombo->currentText();

  if (classname.isEmpty() || classname==i18n("(Globals)"))
    return;

  ParsedClass* aClass;
  aClass = API::getInstance()->classStore()->getClassByName ( classname );

  CClassPropertiesDlgImpl* dlg = class_tree->createCTDlg(aClass, (int) CTPADDMETH);
  dlg -> show();
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
  KComboBox* classCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_CLASS_CHOICE);
  QString classname = classCombo->currentText();

  if  (classname.isEmpty() || classname==i18n("(Globals)"))
    return;

  ParsedClass* aClass;
  aClass = API::getInstance()->classStore()->getClassByName ( classname );

  CClassPropertiesDlgImpl* dlg = class_tree->createCTDlg(aClass, (int) CTPADDATTR);
  dlg -> show();
}

/*------------------------- CKDevelop::slotClassbrowserNewSignal()
 * slotClassbrowserNewSignal()
 *   Event when the user wants to create a new signal from the
 *   browser toolbar/menu.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::slotClassbrowserNewSignal()
{
  KComboBox* classCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_CLASS_CHOICE);
  QString classname = classCombo->currentText();

  if (classname.isEmpty() || classname==i18n("(Globals)"))
    return;

  ParsedClass* aClass;
  aClass = API::getInstance()->classStore()->getClassByName ( classname );

  CClassPropertiesDlgImpl* dlg = class_tree->createCTDlg(aClass, (int) CTPADDSIGNAL);
  dlg -> show();
}

/*------------------------- CKDevelop::slotClassbrowserNewSlot()
 * slotClassbrowserNewSlot()
 *   Event when the user wants to create a new slot from the
 *   browser toolbar/menu.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::slotClassbrowserNewSlot()
{
  KComboBox* classCombo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_CLASS_CHOICE);
  QString classname = classCombo->currentText();

  if (classname.isEmpty() || classname==i18n("(Globals)"))
    return;
  ParsedClass* aClass;
  aClass = API::getInstance()->classStore()->getClassByName ( classname );

  CClassPropertiesDlgImpl* dlg = class_tree->createCTDlg(aClass, (int) CTPADDSLOT);
  dlg -> show();
}

/////////////////////////////////////////////////////////////////////
// Other slots and functions needed
/////////////////////////////////////////////////////////////////////

void CKDevelop::slotStatusMsg(const QString& text)
{
  ///////////////////////////////////////////////////////////////////
  // change status message permanently
  statProg->reset();
  QString status = text.simplifyWhiteSpace();
  if (!status.isEmpty())
  {
    int actualWidth = m_statusLabel->fontMetrics().width(status);
    int realLetterCount = status.length();
    int newLetterCount = (m_statusLabel->width() * realLetterCount) / actualWidth;
    if (newLetterCount > 3)
      newLetterCount -= 4;
    status = status.left(newLetterCount);
  }
  m_statusLabel->setText(status);
}


void CKDevelop::slotStatusHelpMsg(const QString& text)
{
  ///////////////////////////////////////////////////////////////////
  // change status message of whole statusbar temporary (text, msec)
  statusBar()->message(text, 2000);
}

void CKDevelop::enableCommand(int id_)
{
  menuBar()->setItemEnabled(id_,true);
  accel->setItemEnabled(id_,true);

//  menuBar()->setItemEnabled(id_,true);
//  toolBar()->setItemEnabled(id_,true);
  toolBar(ID_BROWSER_TOOLBAR)->setItemEnabled(id_,true);
}

void CKDevelop::disableCommand(int id_)
{
  menuBar()->setItemEnabled(id_,false);
  accel->setItemEnabled(id_,false);

//  menuBar()->setItemEnabled(id_,false);
//  toolBar()->setItemEnabled(id_,false);
  toolBar(ID_BROWSER_TOOLBAR)->setItemEnabled(id_,false);
}

void CKDevelop::slotNewStatus()
{
//  kdDebug() << "*** slotNewStatus()" << endl;
  int config = 0;
  if (m_docViewManager->currentEditView()) {
//    this cant be done without another API change in Kate,
//    maybe we can get around this (rokrau 08/08/01)
//    config = m_docViewManager->currentEditView()->config();
    if (m_docViewManager->currentEditDoc()->isModified()) {
      enableCommand(ID_FILE_SAVE);
    } else {
      disableCommand(ID_FILE_SAVE);
    }
  }
//  statusBar()->changeItem(config & cfOvr ? i18n("OVR") : i18n("INS"),ID_STATUS_INS_OVR);
  // set new caption... maybe the file content is changed
  // and we need to set a [modified] in the caption
  setMainCaption();
}

void CKDevelop::slotCPPMarkStatus(Kate::View *,bool bMarked)
{
  if (m_docViewManager->curDocIsCppFile())
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

void CKDevelop::slotHEADERMarkStatus(Kate::View *, bool bMarked)
{
  if (m_docViewManager->curDocIsHeaderFile())
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

void CKDevelop::slotMarkStatus(Kate::View *, bool bMarked)
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


void CKDevelop::slotBROWSERMarkStatus(KHTMLPart *, bool bMarked)
{
  if (m_docViewManager->curDocIsBrowser())
  {
      if(bMarked){
        enableCommand(ID_EDIT_COPY);
      }
      else{
        disableCommand(ID_EDIT_COPY);
      }
  }
}

void CKDevelop::slotClipboardChanged(Kate::View *, bool bContents)
{
  QString text=QApplication::clipboard()->text();
  if(!bContents || m_docViewManager->curDocIsBrowser())
    disableCommand(ID_EDIT_PASTE);
  else
    enableCommand(ID_EDIT_PASTE);
}

void CKDevelop::slotNewLineColumn()
{
  if (!m_docViewManager->currentEditView()) return;
  QString linenumber;
  uint line, column;
  m_docViewManager->currentEditView()->cursorPosition(&line,&column);
  linenumber = i18n("Line: %1 Col: %2").arg(line+1).arg(column+1);
  statusBar()->changeItem(linenumber.data(), ID_STATUS_LN_CLM);
}
void CKDevelop::slotNewUndo()
{
  if (!m_docViewManager->currentEditDoc()) return;
  uint undoCount = m_docViewManager->currentEditDoc()->undoCount();
  uint redoCount = m_docViewManager->currentEditDoc()->redoCount();
  //undo
  if(undoCount){
    enableCommand(ID_EDIT_UNDO);
  }
  else{
    disableCommand(ID_EDIT_UNDO);
  }
  //redo
  if(redoCount){
    enableCommand(ID_EDIT_REDO);
  }
  else{
    disableCommand(ID_EDIT_REDO);
  }

}


void CKDevelop::slotURLSelected(const QString& url)
{
  if (url.isEmpty())
    return;

  QString url_str = url;

  // add file: directive only if it is an absolute path
  if (url_str.left(1)=="/")
     url_str=QString("file:") + url;

  // Call the DocViewManager
  m_docViewManager->doSelectURL(url_str);

  QString str = history_list.current();
  //if it's a url-request from the search result jump to the correct point
  if (str.contains("kdevelop/search_result.html")){
    prev_was_search_result=true; // after this time, jump to the searchkey
  }
}

void CKDevelop::slotURLonURL(const QString& url )
{
        // in some cases KHTMLView return "file:/file:/...."
        //  this will be here workarounded... and also on
        //  showURL in cdocbrowser.cpp
  QString corr=url, url_str=url;

  if (corr.left(6)=="file:/")
    corr=corr.mid(6, corr.length());
  if (corr.left(5)=="file:")
    url_str=corr;

  if ( url_str ) {
    int actualWidth = m_statusLabel->fontMetrics().width(url_str);
    int realLetterCount = url_str.length();
    int newLetterCount = (m_statusLabel->width() * realLetterCount) / actualWidth;
    if (newLetterCount > 3) newLetterCount -= 4;
    m_statusLabel->setText(url_str.left(newLetterCount));
  }
  else
    m_statusLabel->setText(i18n("Ready.") );
}

void CKDevelop::slotDocumentDone()
{
  CDocBrowser* pCurBrowserDoc = m_docViewManager->currentBrowserDoc();
  if (!pCurBrowserDoc)
    return;

  QString actualURL = pCurBrowserDoc->currentURL();
  QString actualTitle = pCurBrowserDoc->currentTitle();
  int cur =  history_list.at()+1; // get the current index
  int found =  history_list.find(actualURL); // get the current index
  int pos = actualURL.findRev('#');
  QString url_wo_ref=actualURL; // without ref

  if(prev_was_search_result){
    pCurBrowserDoc->findTextBegin();
#if (QT_VERSION < 300)
    pCurBrowserDoc->findTextNext(QRegExp(doc_search_text),true);
#else
    pCurBrowserDoc->findTextNext(doc_search_text,true,false,true);
#endif
  }

  if (m_docViewManager->curDocIsBrowser())
    setMainCaption(BROWSER);

  if (pos!=-1)
    url_wo_ref = actualURL.left(pos);

  // insert into the history-list
  // the following if-statement isn't necessary, because
  //   slotDocumentDone isn't called in the other cases [use of KFMclient for non file://....htm(l)]
  if (actualURL.left(7) != "http://" && url_wo_ref.right(4).find("htm", FALSE)>-1)
  {
    // http aren't added to the history list ...
    if (found == -1)
    {
      if(cur == 0 )
      {
        history_list.append(actualURL);
        history_title_list.append(actualTitle);
      }
      else
      {
        history_list.insert(cur,actualURL);
        history_title_list.insert(cur, actualTitle);
      }
    }
    else
    {
      // the desired URL was already found in the list
      if (actualURL.contains("kdevelop/search_result.html") && history_title_list.at(found)!=actualTitle)
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
      {
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
          cur=found;
      }
    }

    // set now the pointer of the history list
    history_list.at(cur);

    // reorganize the prev- and the next-historylist
    history_next->clear();
    history_prev->clear();

    int i;
    for ( i =0 ; i < cur; i++)
       history_prev->insertItem(history_title_list.at(i));

    for (i = cur+1 ; i < (int) history_list.count(); i++)
       history_next->insertItem(history_title_list.at(i));

    // disable the back button if were at the start of the list
    if (cur == 0)
      disableCommand(ID_HELP_BACK);
    else
      enableCommand(ID_HELP_BACK);

    // disable the forward button if we're at the end of the list
    if (cur >= ((int) history_list.count())-1)
      disableCommand(ID_HELP_FORWARD);
    else
      enableCommand(ID_HELP_FORWARD);
  }

  prev_was_search_result=false;
  disableCommand(ID_HELP_BROWSER_STOP);
}

void CKDevelop::slotReceivedStdout(KProcess*,char* buffer,int buflen)
{
  messages_widget->insertAtEnd(QString::fromLocal8Bit(buffer,buflen));
}

void CKDevelop::slotReceivedStderr(KProcess*,char* buffer,int buflen){
  messages_widget->insertAtEnd(QString::fromLocal8Bit(buffer,buflen), CMakeOutputWidget::Diagnostic);
}

void CKDevelop::slotApplReceivedStdout(KProcess*,char* buffer,int buflen){
  stdin_stdout_widget->insertAtEnd(QString::fromLocal8Bit(buffer,buflen));
}

void CKDevelop::slotApplReceivedStderr(KProcess*,char* buffer,int buflen){
  stderr_widget->insertAtEnd(QString::fromLocal8Bit(buffer,buflen));
}

void CKDevelop::slotApplReceivedStdout(const char* buffer)
{
  slotApplReceivedStdout(0, (char*)buffer, strlen(buffer));
}

void CKDevelop::slotApplReceivedStderr(const char* buffer)
{
    slotApplReceivedStderr(0, (char*)buffer, strlen(buffer));
}

#if defined(GDB_MONITOR) || defined(DBG_MONITOR)
void CKDevelop::slotDebugReceivedStdout(const QString& buffer)
{
  dbg_widget->insertAtEnd(buffer);
}
#else
void CKDevelop::slotDebugReceivedStdout(const QString& )
{ }
#endif

void CKDevelop::slotSearchReceivedStdout(KProcess* /*proc*/,char* buffer,int buflen){
  search_output = search_output + QString::fromLocal8Bit(buffer,buflen);
}
void CKDevelop::slotSearchProcessExited(KProcess*)
{
  disableCommand(ID_HELP_BROWSER_STOP);

  // Figure out the filename of the file we will create if we are using glimpse,
  // or the file that should have the results created by HtDig.
  QString filename = locateLocal("appdata", "search_result.html");

  // Since we prefer glimspe when we started the search we have to
  // prefer glimpse here as well!!
  if (useGlimpse)
  {
    QStrList list;
    QString str;
    int nextpos;
    int pos=0;
    while((nextpos = search_output.find('\n', pos)) != -1)
    {
      str = search_output.mid(pos,nextpos-pos);
      list.append(str);
      pos = nextpos+1;
    }

    if (list.isEmpty()){

       KMessageBox::information(0,i18n("\"%1\" not found in documentation!").arg(doc_search_display_text),
                                  i18n("Not Found!"));
      return;
    }

    int max;
    QStrList sort_list;
    QString found_str;

    // sort on the numeric count at the end of each line.
    // The higher the hit count, the earlier it appears in the list.
    for(int i=0;i<30;i++)
    {
      max =0;
      found_str = "";
      for(str = list.first();str != 0;str = list.next()){
        if (searchToolGetNumber(str) >= max){
          found_str = str.copy();
          max = searchToolGetNumber(str);
        }
      }
      if (found_str != ""){
        sort_list.append(found_str);
        list.remove(found_str);
      }
    }

    QFile file(filename);
    QTextStream stream(&file);
    file.open(IO_WriteOnly);

    stream << "<HTML>";
    stream << "<HEAD><TITLE> - " << i18n("Search for: ") << doc_search_display_text;
    stream << "</TITLE></HEAD><H1>Search String: '" << doc_search_display_text << "'</H1><HR><BODY BGCOLOR=\"#ffffff\"><BR> <TABLE><TR><TH>";
    stream << i18n("Title") << "<TH>" << i18n("Hits") << "\n";
    QString numstr;
    for(str = sort_list.first(); str != 0; str = sort_list.next() ){
      stream << "<TR><TD><A HREF=\""+searchToolGetURL(str)+"\">"+
                searchToolGetTitle(str)+"</A><TD>"+
                numstr.setNum(searchToolGetNumber(str)) + "\n";
    }

    stream << "\n</TABLE></BODY></HTML>";

    file.close();
    slotURLSelected(filename);
    return;
  }
  else
  {
    ASSERT (useHtDig);
    filename = locateLocal("appdata", "search_result.html");
    if (QFile::exists(filename))
      slotURLSelected(filename);
  }
}

QString CKDevelop::searchToolGetTitle(QString str){
  int pos = str.find(' ');
  pos = str.find(' ',pos);
  int end_pos = str.findRev(':');
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

/*
void CKDevelop::slotKeyPressedOnStdinStdoutWidget(int key){
  char a = key;
  appl_process.writeStdin(&a,1);
}
*/
//void CKDevelop::slotClickedOnMessagesWidget(){
//  TErrorMessageInfo info;
//  int x,y;
//
//  messages_widget->cursorPosition(&x,&y);
//  info = error_parser->getInfo(x+1);
//  if(info.filename != ""){
////    if(!bKDevelop)
////      switchToKDevelop();
//    messages_widget->setCursorPosition(info.makeoutputline,0);
//    switchToFile(info.filename,info.errorline-1);
//  }
//  else{
//     XBell(kapp->getDisplay(),100); // not a next found, beep
//  }
//    // switchToFile(error_filename);
////     m_docViewManager->currentEditView()->setCursorPosition(error_line-1,0);
////     m_docViewManager->currentEditView()->setFocus();
//  // int x,y;
////   int error_line;
////   QString text;
////   QString error_line_str;
////   QString error_filename;
////   int pos1,pos2; // positions in the string
////   QRegExp reg(":[0-9]*:"); // is it an error line?, I hope it works
//
//
// //  text = messages_widget->textLine(x);
////   if((pos1=reg.match(text)) == -1) return; // not an error line
//
////   // extract the error-line
////   pos2 = text.find(':',pos1+1);
////   error_line_str = text.mid(pos1+1,pos2-pos1-1);
////   error_line = error_line_str.toInt();
//
////   // extract the filename
////   pos2 = text.findRev(' ',pos1);
////   if (pos2 == -1) {
////     pos2 = 0; // the filename is at the begining of the string
////   }
////   else { pos2++; }
//
////   error_filename = text.mid(pos2,pos1-pos2);
//
////   // switch to the file
////   if (error_filename.find('/') == -1){ // it is a file outer the projectdir ?
////     error_filename = prj->getProjectDir() + prj->getSubDir() + error_filename;
////   }
////   if (QFile::exists(error_filename)){
//
//    //  }
//
//
//}
void CKDevelop::slotProcessExited(KProcess* proc)
{
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

    // 0x0 access test.
    if (prj == 0 && next_job != "load_new_prj" ) return;

    if (next_job=="doc_refresh")
    {
      doc_tree->refresh(prj);
      next_job="";
    }

    else if (next_job == "make_end"  && process.exitStatus() == 0)
    {
      // if we can run the application, so we can clear the Makefile.am-changed-flag
      prj->clearMakefileAmChanged();
      next_job = "";
    }

    else if (next_job == make_cmd)
    { // rest from the rebuild all
             KComboBox* compile_combo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_COMPILE_CHOICE);
            QString conf=compile_combo->currentText();
      QString makefileDir;
      if(conf==i18n("(Default)"))
                    makefileDir = prj->getProjectDir();
            else{
                    makefileDir = m_pKDevSession->getVPATHSubdir(conf);
            }
      QDir::setCurrent(makefileDir);
//      error_parser->setStartDir(makefileDir);
      process.clearArguments();
      if(!prj->getMakeOptions().isEmpty()){
        process << make_cmd << prj->getMakeOptions();
      }
      else{
        process << make_cmd;
      }
      setToolMenuProcess(false);
      process.start(KProcess::NotifyOnExit,KProcess::AllOutput);
      next_job = "make_end";
      ready=false;
    }

    else if (next_job == "debug"  && process.exitStatus() == 0)
    {
      // if we can debug the application, so we can clear the Makefile.am-changed-flag
      prj->clearMakefileAmChanged();

      slotStartDebug();
      next_job = "";
      ready = false;
    }

    else if (next_job == "debug_with_args"  && process.exitStatus() == 0)
    {
      // if we can debug the application, so we can clear the Makefile.am-changed-flag
      prj->clearMakefileAmChanged();

      slotStartDebugRunWithArgs();
      next_job = "";
      ready = false;
    }

    else if ((next_job == "run"  || next_job == "run_with_args") && process.exitStatus() == 0)
    {
      // if we can run the application, so we can clear the Makefile.am-changed-flag
      prj->clearMakefileAmChanged();

      slotStartRun(next_job=="run_with_args");
      next_job = "";
      ready = false;
    }

    else if (next_job == "refresh")
    { // rest from the add projectfile
      refreshTrees();
      next_job = "";
    }
    else if (next_job == "fv_refresh")
    { // update fileview trees...
      log_file_tree->refresh(prj);
      real_file_tree->refresh(prj);
      next_job = "";
    }
    else if( next_job == "load_new_prj")
    {
      if(project) {  //now that we know that a new project will be built we can close the previous one   {
        QString old_project = prj->getProjectFile();
        if(!slotProjectClose()) {      //the user may have pressed cancel in which case the state is undetermined
          CProject* pProj = prepareToReadProjectFile(old_project);
          if (pProj != 0L) {
            readProjectFile(old_project, pProj);
            slotViewRefresh();
          }
        }
        else {
          QDir dir(QDir::current());
          CProject* pProj = prepareToReadProjectFile(QDir::currentDirPath()+"/"+ dir.dirName()+".kdevprj");
          if (pProj != 0L) {
            readProjectFile(QDir::currentDirPath()+"/"+ dir.dirName()+".kdevprj", pProj);
            slotViewRefresh();    // a new project started, this is legitimate
          }
        }
      }
      else {
        QDir dir(QDir::current());
        CProject* pProj = prepareToReadProjectFile(QDir::currentDirPath()+"/"+ dir.dirName()+".kdevprj");
        if (pProj != 0L) {
          readProjectFile(QDir::currentDirPath()+"/"+ dir.dirName()+".kdevprj", pProj);
          slotViewRefresh();    // a new project started, this is legitimate
        }
      }
      next_job = "";
    }
    else if (next_job == "all") {
                  RunMake(CMakefile::toplevel,"");
      next_job = "";
    }
      else if (next_job == "autoconf+configure+make") {
            slotBuildAutoconf();
      next_job = "configure+make";
        }
      else if (next_job == "configure+make") {
            slotBuildConfigure();
      next_job = "all";
        }
        else {
      next_job = "";
    }
  }
  else {
    result= i18n("*** process exited with error(s) ***\n");
    next_job = "";
  }

  if (!result.isEmpty())
  {
//     int x,y;
//     messages_widget->cursorPosition(&x,&y);
//     messages_widget->insertAt(result, x, y);
    messages_widget->insertAtEnd(result);
  }

  if (ready && proc->normalExit()) {

    if (proc->exitStatus()) {
      KNotifyClient::event(QString::fromLatin1("BuildError"), i18n("The build process has finished with errors"));
    } else {
      KNotifyClient::event(QString::fromLatin1("BuildSuccess"), i18n("The build process has finished successfully"));
    }
  }

  if(beep && ready){

      XBell(kapp->getDisplay(),100); //beep :-)
      beep = false;
  }

}

// this requires some work using stateChanged()
// (rokrau 03/21/02)
void CKDevelop::slotViewSelected(QWidget* /*pView*/ /*, int docType */)
{
	Kate::View* pView = m_docViewManager->currentEditView();
	Kate::Document* pDoc = m_docViewManager->currentEditDoc();
  if (!(m_docViewManager->curDocIsBrowser()))
  {
   // enableCommand(ID_FILE_SAVE);  is handled by setMainCaption()
    enableCommand(ID_FILE_SAVE_AS);
    enableCommand(ID_FILE_CLOSE);
    enableCommand(ID_FILE_REVERT);

    enableCommand(ID_FILE_PRINT);

//  QString text=QApplication::clipboard()->text();
//  if(text.isEmpty())
//    disableCommand(ID_EDIT_PASTE);
//  else
//    enableCommand(ID_EDIT_PASTE);

    enableCommand(ID_EDIT_INSERT_FILE);
    enableCommand(ID_EDIT_SEARCH);
    enableCommand(ID_EDIT_REPEAT_SEARCH);
    enableCommand(ID_EDIT_REPLACE);
    enableCommand(ID_EDIT_SPELLCHECK);
    enableCommand(ID_EDIT_INDENT);
    enableCommand(ID_EDIT_UNINDENT);
    enableCommand(ID_EDIT_COMMENT);
    enableCommand(ID_EDIT_UNCOMMENT);
    enableCommand(ID_EDIT_SELECT_ALL);
    enableCommand(ID_EDIT_DESELECT_ALL);
    enableCommand(ID_EDIT_INVERT_SELECTION);
    enableCommand(ID_EDIT_EXPAND_TEXT);
    enableCommand(ID_EDIT_COMPLETE_TEXT);
  }

  if (m_docViewManager->curDocIsHeaderFile()){
    if(bAutoswitch && doc_tree->parentWidget()->isVisible()) {
      if ( bDefaultCV)
        makeWidgetDockVisible(class_tree->parentWidget());
      else
        makeWidgetDockVisible(log_file_tree->parentWidget());
    }
    disableCommand(ID_BUILD_COMPILE_FILE);
    slotNewUndo();
    slotNewStatus();
//    setMainCaption();
    slotNewLineColumn();
  }
  if (m_docViewManager->curDocIsCppFile()){
    if(bAutoswitch && doc_tree->parentWidget()->isVisible()) {
      if ( bDefaultCV)
        makeWidgetDockVisible(class_tree->parentWidget());
      else
        makeWidgetDockVisible(log_file_tree->parentWidget());
    }
// needs to be fixed (rokrau 02/17/02)
//    if(project && build_menu->isItemEnabled(ID_BUILD_MAKE)){
//      enableCommand(ID_BUILD_COMPILE_FILE);
//    }
//
    slotNewUndo();
    slotNewStatus();
//    setMainCaption();
    slotNewLineColumn();
  }

  if (!(m_docViewManager->curDocIsBrowser()))
  {
    if (pDoc) {
      int undoCount = pDoc->undoCount();
      int redoCount = pDoc->undoCount();
      //undo
// to reenable using stateChanged
// (rokrau 03/21/02)
      if(undoCount){
//        enableCommand(ID_EDIT_UNDO);
      }
      else{
//        disableCommand(ID_EDIT_UNDO);
      }
      //redo
      if(redoCount){
//        enableCommand(ID_EDIT_REDO);
      }
      else{
//        disableCommand(ID_EDIT_REDO);
      }
      QString str;
      str = pDoc->selection();
      if(str.isEmpty()){
        disableCommand(ID_EDIT_CUT);
        disableCommand(ID_EDIT_COPY);
      }
      else{
        enableCommand(ID_EDIT_CUT);
        enableCommand(ID_EDIT_COPY);
      }
    }
    if (pView) pView->setFocus();
  }
  else if (m_docViewManager->curDocIsBrowser())
  {
    disableCommand(ID_BUILD_COMPILE_FILE);

    disableCommand(ID_FILE_SAVE);
    disableCommand(ID_FILE_SAVE_AS);
    disableCommand(ID_FILE_CLOSE);
    disableCommand(ID_FILE_REVERT);

    disableCommand(ID_FILE_PRINT);

    disableCommand(ID_EDIT_UNDO);
    disableCommand(ID_EDIT_REDO);
    disableCommand(ID_EDIT_CUT);
    disableCommand(ID_EDIT_PASTE);
    disableCommand(ID_EDIT_INSERT_FILE);
    // disableCommand(ID_EDIT_SEARCH);
    disableCommand(ID_EDIT_REPEAT_SEARCH);
    disableCommand(ID_EDIT_REPLACE);
    disableCommand(ID_EDIT_SPELLCHECK);
    disableCommand(ID_EDIT_INDENT);
    disableCommand(ID_EDIT_UNINDENT);
    disableCommand(ID_EDIT_COMMENT);
    disableCommand(ID_EDIT_UNCOMMENT);
    disableCommand(ID_EDIT_SELECT_ALL);
    disableCommand(ID_EDIT_DESELECT_ALL);
    disableCommand(ID_EDIT_INVERT_SELECTION);
    disableCommand(ID_EDIT_EXPAND_TEXT);
    disableCommand(ID_EDIT_COMPLETE_TEXT);
  }

  if (m_docViewManager->curDocIsBrowser())
  {
    if(bAutoswitch)
      makeWidgetDockVisible(doc_tree);
    m_docViewManager->currentBrowserView()->setFocus();

    if (m_docViewManager->currentBrowserDoc()->hasSelection())
      enableCommand(ID_EDIT_COPY);
    else
      disableCommand(ID_EDIT_COPY);

    setMainCaption(BROWSER);
  }
  else {
    setMainCaption();
  }
}

void CKDevelop::slotLogFileTreeSelected(QString file){
  switchToFile(file);
}

void CKDevelop::slotRealFileTreeSelected(QString file){
  switchToFile(file);
}

void CKDevelop::slotUpdateFileFromVCS(QString file){
    switchToFile(file,-1,-1, false,false); // force not reload and no messagebox if modified on disc, because slotFileSave() will do it
    slotFileSave();
    prj->getVersionControl()->update(file);
    switchToFile(file,true,false);
}
void CKDevelop::slotCommitFileToVCS(QString file){
    switchToFile(file,-1,-1,false,false);
    slotFileSave();
    prj->getVersionControl()->commit(file);
    switchToFile(file,true,false);
}

void CKDevelop::slotUpdateDirFromVCS(QString dir){

    slotFileSaveAll();

    prj->getVersionControl()->update(dir);

    m_docViewManager->reloadModifiedFiles();
}

void CKDevelop::slotCommitDirToVCS(QString dir){

    slotFileSaveAll();

    prj->getVersionControl()->commit(dir);

    m_docViewManager->reloadModifiedFiles();
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
  QString text = doc_tree->selectedText();

  if(!QFile::exists(url_file)){
    if( text == i18n("Qt-Library")){
      if(KMessageBox::questionYesNo(0,
                                    i18n("KDevelop couldn't find the Qt documentation.\n Do you want to set the correct path?"),
                                    i18n("File Not Found!")) == KMessageBox::Yes) {
        slotOptionsKDevelop();
      }
      return;
    }
    if(text == i18n("KDE-Core-Library")     || text == i18n("KDE-UI-Library") ||
       text == i18n("KDE-KFile-Library")    || text == i18n("KDE-KHTML-Library") ||
       text == i18n("KDE-KFM-Library")      || text == i18n("KDE-KDEutils-Library") ||
       text == i18n("KDE-KAB-Library")      || text == i18n("KDE-KSpell-Library")){
      if(KMessageBox::questionYesNo(0,
                            i18n("KDevelop couldn't find the KDE API-Documentation.\nDo you want to generate it now?"),
                            i18n("File Not Found!")) == KMessageBox::Yes) {
        slotOptionsUpdateKDEDocumentation();
      }
      return;
    }
  }
  slotURLSelected(url_file);

}

void CKDevelop::slotTCurrentTab(int item){
  switch( item) {
  case CV:
    makeWidgetDockVisible( class_tree);
  break;
  case LFV:
    makeWidgetDockVisible( log_file_tree);
  break;
  case RFV:
    makeWidgetDockVisible( real_file_tree);
  break;
  case DOC:
    makeWidgetDockVisible( doc_tree);
  break;
  }
}

void CKDevelop::slotGrepDialogItemSelected(QString filename,int linenumber){
  setActiveWindow();
  switchToFile(filename,linenumber);
}

void CKDevelop::slotToolbarClicked(int item){
  switch (item) {
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
//#warning FIXME KHTML changes.
//    m_docViewManager->currentBrowserDoc()->cancelAllRequests();
    shell_process.kill();
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
  case ID_HELP_WHATS_THIS:
    QWhatsThis::enterWhatsThisMode();
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

/** Reimplemented from base class QextMdiMainFrm.
 *  Dispatches this 'event' to m_docViewMan which will delete the closed view
 */
void CKDevelop::closeWindow(QextMdiChildView *pWnd, bool /*layoutTaskBar*/)
{
  m_docViewManager->closeView( pWnd);
}

QString CKDevelop::getProjectName()
{
  return (prj) ? prj->getProjectName() : QString("");
}

/**
 * Overridden from its base class QextMdiMainFrm. Adds additional entries to the "Window" menu.
 */
void CKDevelop::fillWindowMenu()
{
  QextMdiMainFrm::fillWindowMenu();
  windowMenu()->insertItem(i18n("New &Window"), this, SLOT(slotCreateNewViewWindow()), 0, -1, 0);
    if (mdiMode() != QextMdi::TabPageMode) {
      windowMenu()->removeItemAt(6);
      windowMenu()->removeItemAt(5);
    }
    else {
      windowMenu()->removeItemAt(5);
      windowMenu()->removeItemAt(4);
    }
}

/**
 * The slot for the main menu entry "Window->New Window".
 * Creates and shows a new MDI view window depending on the last focused view type
 */
void CKDevelop::slotCreateNewViewWindow()
{
  slotStatusMsg(i18n("Creating new view window for the current document..."));
  if (m_pWinList->isEmpty()) {
    // if no view is open at all, we open a browser view, but we need the appropriate document first!
    CDocBrowser* pDoc = m_docViewManager->createCDocBrowser(DocTreeKDevelopBook::locatehtml("about/intro.html"));
    m_docViewManager->createBrowserView(pDoc, true);
  }
  else {
    m_docViewManager->doCreateNewView();
  }
  slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotDockWidgetHasUndocked(KDockWidget*)
{
  if (m_bToggleToolViewsIsPending) return;

  // check if any tree view widget is invisible now, if yes -> set tree toolbar-button off
  if (!isToolViewVisible(class_tree)
   || !isToolViewVisible(log_file_tree)
   || !isToolViewVisible(real_file_tree)
   || !isToolViewVisible(doc_tree)
   || ((dbgController != 0L) && !isToolViewVisible(var_viewer))
     ) {
// FIXME (rokrau 02/17/02)
//    if (view_menu)
//        view_menu->setItemChecked(ID_VIEW_TREEVIEW, false);
//    toolBar()->setButton(ID_VIEW_TREEVIEW, false);
  }

  // check if any output view widget is invisible now, if yes -> set output toolbar-button off
  if (!isToolViewVisible(messages_widget)
   || !isToolViewVisible(stdin_stdout_widget)
   || !isToolViewVisible(stderr_widget)
   || !isToolViewVisible(konsole_widget)
   || !isToolViewVisible(brkptManager)
   || ((dbgController != 0L) && !isToolViewVisible(disassemble))
   || ((dbgController != 0L) && !isToolViewVisible(frameStack))
#if defined(GDB_MONITOR) || defined(DBG_MONITOR)
   || ((dbgController != 0L) && !isToolViewVisible(dbg_widget))
#endif
     ) {
// FIXME (rokrau 02/17/02)
//    if (view_menu)
//      view_menu->setItemChecked(ID_VIEW_OUTPUTVIEW, false);
//    toolBar()->setButton(ID_VIEW_OUTPUTVIEW, false);
  }
}

bool CKDevelop::isToolViewVisible(QWidget* pToolView)
{
  // pointer access checks
  if (pToolView == 0L) return false;
  if (pToolView->parentWidget() == 0L) return false;
  QWidget* pDockCandidate = pToolView->parentWidget()->parentWidget();
  if (pDockCandidate == 0L) return false;
  KDockWidget* pDock = dynamic_cast<KDockWidget*>(pDockCandidate);
  if (pDock == 0L) return false;

  // actual check
  bool bVisible = pDock->isVisible();
  QWidget* pTabWidget = pDock->parentDockTabGroup();
  bool bIsVisible = bVisible || (pTabWidget && pTabWidget->isVisible());
  return bIsVisible;
}

/** checks if any of the tool views is visible */
bool CKDevelop::isAnyToolViewVisible(){
  return ( isToolViewVisible(class_tree)
        || isToolViewVisible(log_file_tree)
        || isToolViewVisible(real_file_tree)
        || isToolViewVisible(doc_tree)
        || ((dbgController != 0L) && isToolViewVisible(var_viewer)));
}

/** checks if any of the output view widget is visible */
bool CKDevelop::isAnyOutputWindowVisible(){
  return ( isToolViewVisible(messages_widget)
        || isToolViewVisible(stdin_stdout_widget)
        || isToolViewVisible(stderr_widget)
        || isToolViewVisible(konsole_widget)
        || isToolViewVisible(brkptManager)
        || ((dbgController != 0L) && isToolViewVisible(disassemble))
        || ((dbgController != 0L) && isToolViewVisible(frameStack))
#if defined(GDB_MONITOR) || defined(DBG_MONITOR)
        || ((dbgController != 0L) && isToolViewVisible(dbg_widget))
#endif
          );
}

void CKDevelop::fillToggleTreeViewsMenu()
{
//  toggletreeviews_popup->clear();
//
//  toggletreeviews_popup->insertItem(i18n("All &Tree Tool-Views"),this, SLOT(slotViewTTreeView()),0,ID_VIEW_TREEVIEW);
//  toggletreeviews_popup->insertSeparator();
//
//  toggletreeviews_popup->insertItem(i18n("&Classes"), this, SLOT(slotViewTClassesView()));
//  bool bClassesChecked = isToolViewVisible(class_tree);
//  toggletreeviews_popup->setItemChecked(toggletreeviews_popup->idAt(2), bClassesChecked);
//
//  toggletreeviews_popup->insertItem(i18n("&Groups"), this, SLOT(slotViewTGroupsView()));
//  bool bGroupsChecked = isToolViewVisible(log_file_tree);
//  toggletreeviews_popup->setItemChecked(toggletreeviews_popup->idAt(3), bGroupsChecked);
//
//  toggletreeviews_popup->insertItem(i18n("&Files"), this, SLOT(slotViewTFilesView()));
//  bool bFilesChecked = isToolViewVisible(real_file_tree);
//  toggletreeviews_popup->setItemChecked(toggletreeviews_popup->idAt(4), bFilesChecked);
//
//  toggletreeviews_popup->insertItem(i18n("&Books"), this, SLOT(slotViewTBooksView()));
//  bool bBooksChecked = isToolViewVisible(doc_tree);
//  toggletreeviews_popup->setItemChecked(toggletreeviews_popup->idAt(5), bBooksChecked);
//
//  toggletreeviews_popup->insertItem(i18n("&Watch"), this, SLOT(slotViewTWatchView()));
//  bool bWatchChecked = isToolViewVisible(var_viewer);
//  toggletreeviews_popup->setItemChecked(toggletreeviews_popup->idAt(6), bWatchChecked);
//  toggletreeviews_popup->setItemEnabled(toggletreeviews_popup->idAt(6), (dbgController != 0L));
//
//  if (dbgController != 0L) {  // if not debugging, don't consider the watch view
//    if (bClassesChecked && bGroupsChecked && bFilesChecked && bBooksChecked && bWatchChecked) {
//      toggletreeviews_popup->setItemChecked(toggletreeviews_popup->idAt(0), true);   //only if all are on
//    }
//  }
//  else {
//    if (bClassesChecked && bGroupsChecked && bFilesChecked && bBooksChecked) {
//      toggletreeviews_popup->setItemChecked(toggletreeviews_popup->idAt(0), true);   //only if all are on
//    }
//  }
}

void CKDevelop::slotViewTClassesView()
{
        KDockWidget* pDock = (KDockWidget*)class_tree->parentWidget()->parentWidget();
        KToggleAction* pAction = dynamic_cast<KToggleAction*>
                                 (actionCollection()->action("view_tree_classes"));
        if (!pAction) return;
        if (pAction->isChecked())
                pDock->undock();
        else {
                if (! isAnyToolViewVisible()) {
                     slotViewTTreeView();
                     slotActivateTView_Class();
                } else
                     pDock->dockBack();
         }
        adjustTTreesToolButtonState();
}

void CKDevelop::slotViewTGroupsView()
{
        KDockWidget* pDock = (KDockWidget*)log_file_tree->parentWidget()->parentWidget();
        KToggleAction* pAction = dynamic_cast<KToggleAction*>
                                 (actionCollection()->action("view_tree_groups"));
        if (!pAction) return;
        if (pAction->isChecked())
        pDock->undock();
       else {
         if (! isAnyToolViewVisible()) {
           slotViewTTreeView();
           slotActivateTView_LFV();
         } else
           pDock->dockBack();
      }
        adjustTTreesToolButtonState();
}

void CKDevelop::slotViewTFilesView()
{
        KDockWidget* pDock = (KDockWidget*)real_file_tree->parentWidget()->parentWidget();
        KToggleAction* pAction = dynamic_cast<KToggleAction*>
                                 (actionCollection()->action("view_tree_file"));
        if (!pAction) return;
        if (pAction->isChecked())
                pDock->undock();
  else {
    if (! isAnyToolViewVisible()) {
      slotViewTTreeView();
      slotActivateTView_RFV();
    } else
      pDock->dockBack();
  }
        adjustTTreesToolButtonState();
}

void CKDevelop::slotViewTBooksView()
{
        KDockWidget* pDock = (KDockWidget*)doc_tree->parentWidget()->parentWidget();
        KToggleAction* pAction = dynamic_cast<KToggleAction*>
                                 (actionCollection()->action("view_tree_books"));
        if (!pAction) return;
        if (pAction->isChecked())
                pDock->undock();
  else {
    if (! isAnyToolViewVisible()) {
      slotViewTTreeView();
      slotActivateTView_Doc();
    } else
      pDock->dockBack();
  }
        adjustTTreesToolButtonState();
}

void CKDevelop::slotViewTWatchView()
{
        KDockWidget* pDock = (KDockWidget*)var_viewer->parentWidget()->parentWidget();
        KToggleAction* pAction = dynamic_cast<KToggleAction*>
                                 (actionCollection()->action("view_tree_watch"));
        if (!pAction) return;
        if (pAction->isChecked())
                pDock->undock();
  else {
    if (! isAnyToolViewVisible()) {
      slotViewTTreeView();
      slotActivateTView_VAR();
    } else
      pDock->dockBack();
  }
        adjustTTreesToolButtonState();
}

void CKDevelop::fillToggleOutputViewsMenu()
{
//  toggleoutputviews_popup->clear();
//
//  toggleoutputviews_popup->insertItem(i18n("All &Output Tool-Views"),this,SLOT(slotViewTOutputView()),0,ID_VIEW_OUTPUTVIEW);
//  toggleoutputviews_popup->insertSeparator();
//
//  toggleoutputviews_popup->insertItem(i18n("&Messages"), this, SLOT(slotViewOMessagesView()));
//  bool bMessagesChecked = isToolViewVisible(messages_widget);
//  toggleoutputviews_popup->setItemChecked(toggleoutputviews_popup->idAt(2), bMessagesChecked);
//
//  toggleoutputviews_popup->insertItem(i18n("&StdOut"), this, SLOT(slotViewOStdOutView()));
//  bool bStdOutChecked = isToolViewVisible(stdin_stdout_widget);
//  toggleoutputviews_popup->setItemChecked(toggleoutputviews_popup->idAt(3), bStdOutChecked);
//
//  toggleoutputviews_popup->insertItem(i18n("S&tdErr"), this, SLOT(slotViewOStdErrView()));
//  bool bStdErrChecked = isToolViewVisible(stderr_widget);
//  toggleoutputviews_popup->setItemChecked(toggleoutputviews_popup->idAt(4), bStdErrChecked);
//
//  toggleoutputviews_popup->insertItem(i18n("&Konsole"), this, SLOT(slotViewOKonsoleView()));
//  bool bKonsoleChecked = isToolViewVisible(konsole_widget);
//  toggleoutputviews_popup->setItemChecked(toggleoutputviews_popup->idAt(5), bKonsoleChecked);
//
//  toggleoutputviews_popup->insertItem(i18n("&Breakpoints"), this, SLOT(slotViewOBreakpointView()));
//  bool bBreakpointsChecked = isToolViewVisible(brkptManager);
//  toggleoutputviews_popup->setItemChecked(toggleoutputviews_popup->idAt(6), bBreakpointsChecked);
//
//  toggleoutputviews_popup->insertItem(i18n("&Disassemble"), this, SLOT(slotViewODisassembleView()));
//  bool bDisassembleChecked = isToolViewVisible(disassemble);
//  toggleoutputviews_popup->setItemChecked(toggleoutputviews_popup->idAt(7), bDisassembleChecked);
//  toggleoutputviews_popup->setItemEnabled(toggleoutputviews_popup->idAt(7), (dbgController != 0L));
//
//  toggleoutputviews_popup->insertItem(i18n("&Call Stack"), this, SLOT(slotViewOFrameStackView()));
//  bool bCallStackChecked = isToolViewVisible(frameStack);
//  toggleoutputviews_popup->setItemChecked(toggleoutputviews_popup->idAt(8), bCallStackChecked);
//  toggleoutputviews_popup->setItemEnabled(toggleoutputviews_popup->idAt(8), (dbgController != 0L));
//
//#if defined(GDB_MONITOR) || defined(DBG_MONITOR)
//  toggleoutputviews_popup->insertItem(i18n("D&ebugger"), this, SLOT(slotViewODebuggerView()));
//  bool bDebuggerChecked = isToolViewVisible(dbg_widget);
//  toggleoutputviews_popup->setItemChecked(toggleoutputviews_popup->idAt(9), bDebuggerChecked);
//  toggleoutputviews_popup->setItemEnabled(toggleoutputviews_popup->idAt(9), (dbgController != 0L));
//#endif
//
//  if (dbgController != 0L) {  // if not debugging, don't consider the disassemble, callstack and debugger views
//#if defined(GDB_MONITOR) || defined(DBG_MONITOR)
//    if (bMessagesChecked && bStdOutChecked && bStdErrChecked && bKonsoleChecked && bBreakpointsChecked && bDisassembleChecked && bCallStackChecked && bDebuggerChecked) {
//#else
//    if (bMessagesChecked && bStdOutChecked && bStdErrChecked && bKonsoleChecked && bBreakpointsChecked && bDisassembleChecked && bCallStackChecked) {
//#endif
//      toggleoutputviews_popup->setItemChecked(toggleoutputviews_popup->idAt(0), true);   //only if all are on
//    }
//  }
//  else {
//    if (bMessagesChecked && bStdOutChecked && bStdErrChecked && bKonsoleChecked && bBreakpointsChecked) {
//      toggleoutputviews_popup->setItemChecked(toggleoutputviews_popup->idAt(0), true);   //only if all are on
//    }
//  }
}

void CKDevelop::slotViewOMessagesView()
{
        KDockWidget* pDock = (KDockWidget*)messages_widget->parentWidget()->parentWidget();
        KToggleAction* pAction = dynamic_cast<KToggleAction*>
                                 (actionCollection()->action("view_out_msg"));
        if (!pAction) return;
        if (pAction->isChecked())
                pDock->undock();
        else
                pDock->dockBack();
                if (! isAnyOutputWindowVisible() ) {
                    slotViewTOutputView();
                    slotActivateOView_Messages();
                } else
                   pDock->dockBack();
        adjustTOutputToolButtonState();

}

void CKDevelop::slotViewOStdOutView()
{
        KDockWidget* pDock = (KDockWidget*)stdin_stdout_widget->parentWidget()->parentWidget();
        KToggleAction* pAction = dynamic_cast<KToggleAction*>
                                 (actionCollection()->action("view_out_stdout"));
        if (!pAction) return;
        if (pAction->isChecked())
                pDock->undock();
        else
                if (! isAnyOutputWindowVisible() ) {
                        slotViewTOutputView();
                        slotActivateOView_StdInStdOut();
                } else
                        pDock->dockBack();
        adjustTOutputToolButtonState();
}

void CKDevelop::slotViewOStdErrView()
{
        KDockWidget* pDock = (KDockWidget*)stderr_widget->parentWidget()->parentWidget();
        KToggleAction* pAction = dynamic_cast<KToggleAction*>
                                 (actionCollection()->action("view_out_stderr"));
        if (!pAction) return;
        if (pAction->isChecked())
                pDock->undock();
        else
                if (! isAnyOutputWindowVisible() ) {
                         slotViewTOutputView();
                         slotActivateOView_StdErr();
                } else
                        pDock->dockBack();
         adjustTOutputToolButtonState();
}

void CKDevelop::slotViewOKonsoleView()
{
        KDockWidget* pDock = (KDockWidget*)konsole_widget->parentWidget()->parentWidget();
        KToggleAction* pAction = dynamic_cast<KToggleAction*>
                                 (actionCollection()->action("view_out_konsole"));
        if (!pAction) return;
        if (pAction->isChecked())
                pDock->undock();
        else
        if (! isAnyOutputWindowVisible() ) {
                slotViewTOutputView();
                slotActivateOView_Konsole();
       } else
                pDock->dockBack();
       adjustTOutputToolButtonState();
}

void CKDevelop::slotViewOBreakpointView()
{
        KDockWidget* pDock = (KDockWidget*)brkptManager->parentWidget()->parentWidget();
        KToggleAction* pAction = dynamic_cast<KToggleAction*>
                                 (actionCollection()->action("view_out_break"));
        if (!pAction) return;
        if (pAction->isChecked())
                pDock->undock();
        else
    if (! isAnyOutputWindowVisible() ) {
      slotViewTOutputView();
      slotActivateOView_BrkptManager();
    } else
      pDock->dockBack();
  adjustTOutputToolButtonState();
}

void CKDevelop::slotViewODisassembleView()
{
        KDockWidget* pDock = (KDockWidget*)disassemble->parentWidget()->parentWidget();
        KToggleAction* pAction = dynamic_cast<KToggleAction*>
                                 (actionCollection()->action("view_out_disasm"));
        if (!pAction) return;
        if (pAction->isChecked())
                pDock->undock();
        else
    if (! isAnyOutputWindowVisible() ) {
      slotViewTOutputView();
      slotActivateOView_Disassemble();
    } else
      pDock->dockBack();
  adjustTOutputToolButtonState();
}

void CKDevelop::slotViewOFrameStackView()
{
        KDockWidget* pDock = (KDockWidget*)frameStack->parentWidget()->parentWidget();
        KToggleAction* pAction = dynamic_cast<KToggleAction*>
                                 (actionCollection()->action("view_out_stack"));
        if (!pAction) return;
        if (pAction->isChecked())
                pDock->undock();
  else
    if (! isAnyOutputWindowVisible() ) {
      slotViewTOutputView();
      slotActivateOView_FrameStack();
    } else
      pDock->dockBack();
  adjustTOutputToolButtonState();
}

void CKDevelop::slotViewODebuggerView()
{
        KDockWidget* pDock = (KDockWidget*)dbg_widget->parentWidget()->parentWidget();
        KToggleAction* pAction = dynamic_cast<KToggleAction*>
                                 (actionCollection()->action("view_out_dbg"));
        if (!pAction) return;
        if (pAction->isChecked())
                pDock->undock();
        else
    if (! isAnyOutputWindowVisible() ) {
      slotViewTOutputView();
      slotActivateOView_Dbg();
    } else
      pDock->dockBack();
  adjustTOutputToolButtonState();
}

void CKDevelop::slotActivateTView_Class()
{
  dockManager->findWidgetParentDock(class_tree->parentWidget())->makeDockVisible();
  class_tree->setFocus();
}

void CKDevelop::slotActivateTView_LFV()
{
  dockManager->findWidgetParentDock(log_file_tree->parentWidget())->makeDockVisible();
  log_file_tree->setFocus();
}

void CKDevelop::slotActivateTView_RFV()
{
  dockManager->findWidgetParentDock(real_file_tree->parentWidget())->makeDockVisible();
  real_file_tree->setFocus();
}

void CKDevelop::slotActivateTView_Doc()
{
  dockManager->findWidgetParentDock(doc_tree->parentWidget())->makeDockVisible();
  doc_tree->setFocus();
}

void CKDevelop::slotActivateTView_VAR()
{
  if (dbgController != 0L) {  // when debugging, only
    dockManager->findWidgetParentDock(var_viewer->parentWidget())->makeDockVisible();
    var_viewer->setFocus();
  }
}

void CKDevelop::slotActivateOView_Messages()
{
  dockManager->findWidgetParentDock(messages_widget->parentWidget())->makeDockVisible();
}

void CKDevelop::slotActivateOView_StdInStdOut()
{
  dockManager->findWidgetParentDock(stdin_stdout_widget->parentWidget())->makeDockVisible();
}

void CKDevelop::slotActivateOView_StdErr()
{
  dockManager->findWidgetParentDock(stderr_widget->parentWidget())->makeDockVisible();
}

void CKDevelop::slotActivateOView_Konsole()
{
  dockManager->findWidgetParentDock(konsole_widget->parentWidget())->makeDockVisible();
  konsole_widget->setFocus();
}

void CKDevelop::slotActivateOView_BrkptManager()
{
  if (brkptManager && dbgInternal)
  {
    dockManager->findWidgetParentDock(brkptManager->parentWidget())->makeDockVisible();
    brkptManager->setFocus();
  }
}

void CKDevelop::slotActivateOView_FrameStack()
{
  if (dbgController != 0L) {  // when debugging, only
    dockManager->findWidgetParentDock(frameStack->parentWidget())->makeDockVisible();
    frameStack->setFocus();
  }
}

void CKDevelop::slotActivateOView_Disassemble()
{
  if (dbgController != 0L) {  // when debugging, only
    dockManager->findWidgetParentDock(disassemble->parentWidget())->makeDockVisible();
    disassemble->setFocus();
  }
}

void CKDevelop::slotActivateOView_Dbg()
{
#if defined(GDB_MONITOR) || defined(DBG_MONITOR)
  if (dbgController != 0L) {  // when debugging, only
    dockManager->findWidgetParentDock(dbg_widget->parentWidget())->makeDockVisible();
    dbg_widget->setFocus();
  }
#endif
}

void CKDevelop::statusCallback(int id_){
  switch(id_)
  {
//    ON_STATUS_MSG(ID_FILE_REVERT,                           i18n("Discard all changes and reload file"))

//    ON_STATUS_MSG(ID_EDIT_CUT,                              i18n("Cuts the selected section and puts it to the clipboard"))
//    ON_STATUS_MSG(ID_EDIT_COPY,                             i18n("Copys the selected section to the clipboard"))
//    ON_STATUS_MSG(ID_EDIT_PASTE,                            i18n("Pastes the clipboard contents to current position"))
//
//    ON_STATUS_MSG(ID_EDIT_INSERT_FILE,                      i18n("Inserts a file at the current position"))
//
//    ON_STATUS_MSG(ID_EDIT_SEARCH,                           i18n("Searches the file for an expression"))
//    ON_STATUS_MSG(ID_EDIT_REPEAT_SEARCH,                    i18n("Repeats the last search"))
//    ON_STATUS_MSG(ID_EDIT_REPLACE,                          i18n("Searches and replace expression"))
//    ON_STATUS_MSG(ID_EDIT_SEARCH_IN_FILES,                  i18n("Opens the search in files dialog to search for expressions over several files"))
//    ON_STATUS_MSG(ID_EDIT_TAGS_SWITCH,                      i18n("Switch between header and source file"))
//
//    ON_STATUS_MSG(ID_EDIT_RUN_TO_CURSOR,                    i18n("Run program to this cursor position"))
//    ON_STATUS_MSG(ID_EDIT_STEP_OUT_OFF,                     i18n("Run the program until this function/method ends"))
//    ON_STATUS_MSG(ID_EDIT_ADD_WATCH_VARIABLE,               i18n("Try to display this variable whenever the application execution is paused"))
//
//
//    ON_STATUS_MSG(ID_EDIT_INDENT,                           i18n("Moves the selection to the right"))
//    ON_STATUS_MSG(ID_EDIT_UNINDENT,                         i18n("Moves the selection to the left"))
//    ON_STATUS_MSG(ID_EDIT_COMMENT,                          i18n("Adds // to the beginning of each selected line"))
//    ON_STATUS_MSG(ID_EDIT_UNCOMMENT,                        i18n("Removes // from the beginning of each selected line"))
//
//    ON_STATUS_MSG(ID_EDIT_SELECT_ALL,                       i18n("Selects the whole document contents"))
//    ON_STATUS_MSG(ID_EDIT_DESELECT_ALL,                     i18n("Deselects the whole document contents"))
//    ON_STATUS_MSG(ID_EDIT_INVERT_SELECTION,                 i18n("Inverts the current selection"))


//    ON_STATUS_MSG(ID_VIEW_GOTO_LINE,                        i18n("Goes to Line Number..."))
//    ON_STATUS_MSG(ID_VIEW_NEXT_ERROR,                       i18n("Switches to the file and line the next error was reported"))
//    ON_STATUS_MSG(ID_VIEW_PREVIOUS_ERROR,                   i18n("Switches to the file and line the previous error was reported"))
//
//    ON_STATUS_MSG(ID_VIEW_TREEVIEW,                         i18n("Enables/Disables the treeview"))
//    ON_STATUS_MSG(ID_VIEW_OUTPUTVIEW,                       i18n("Enables/Disables the outputview"))
//
//    ON_STATUS_MSG(ID_VIEW_TOOLBAR,                          i18n("Enables/Disables the standard toolbar"))
//    ON_STATUS_MSG(ID_VIEW_BROWSER_TOOLBAR,                  i18n("Enables/Disables the browser toolbar"))
//    ON_STATUS_MSG(ID_VIEW_STATUSBAR,                        i18n("Enables/Disables the statusbar"))
//    ON_STATUS_MSG(ID_VIEW_MDIVIEWTASKBAR,                   i18n("Enables/Disables the MDI-view taskbar"))
//
//    ON_STATUS_MSG(ID_VIEW_REFRESH,                          i18n("Refreshes current view"))
//    ON_STATUS_MSG(ID_VIEW_IN_KFM,                           i18n("Opens the current document in the KFM browser"))

//    ON_STATUS_MSG(ID_PROJECT_KAPPWIZARD,                    i18n("Generates a new project with Application Wizard"))
//    ON_STATUS_MSG(ID_PROJECT_OPEN,                          i18n("Opens an existing project"))
//    ON_STATUS_MSG(ID_PROJECT_CLOSE,                         i18n("Closes the current project"))
//    ON_STATUS_MSG(ID_PROJECT_ADD_FILE_EXIST,                i18n("Adds an existing file to the project"))
//    ON_STATUS_MSG(ID_PROJECT_ADD_NEW_TRANSLATION_FILE,      i18n("Adds a new language for internationalization to the project"))
//    ON_STATUS_MSG(ID_PROJECT_REMOVE_FILE,                   i18n("Removes file from the project"))
//
//    ON_STATUS_MSG(ID_PROJECT_NEW_CLASS,                     i18n("Creates a new Class frame structure and files"))
//    ON_STATUS_MSG(ID_PROJECT_GENERATE,                      i18n("Creates a project file for an existing automake project"))
//
//    ON_STATUS_MSG(ID_PROJECT_FILE_PROPERTIES,               i18n("Shows the current file properties"))
//    ON_STATUS_MSG(ID_PROJECT_OPTIONS,                       i18n("Sets project and compiler options"))
//    ON_STATUS_MSG(ID_PROJECT_MESSAGES,                      i18n("Invokes make to create the message file by extracting all i18n() macros"))
//  //MB
//    ON_STATUS_MSG(ID_PROJECT_DOC_TOOL,                      i18n("Switches the documentation tool (kdoc/doxygen)"))
//    ON_STATUS_MSG(ID_PROJECT_MAKE_PROJECT_API,              i18n("Creates the Project's API Documentation"))
//  //MB end
//    ON_STATUS_MSG(ID_PROJECT_MAKE_USER_MANUAL,              i18n("Creates the Project's User Manual with the sgml-file"))
//    ON_STATUS_MSG(ID_PROJECT_MAKE_DISTRIBUTION,             i18n("Creates distribution packages from the current project"))
//    ON_STATUS_MSG(ID_PROJECT_MAKE_DISTRIBUTION_SOURCE_TGZ,  i18n("Creates a tar.gz file from the current project sources"))
//
//    ON_STATUS_MSG(ID_PROJECT_MAKE_TAGS, i18n("Creates a tags file from the current project sources"))
//    ON_STATUS_MSG(ID_PROJECT_LOAD_TAGS, i18n("Load a tags file generated for the current project sources"))

//    ON_STATUS_MSG(ID_BUILD_COMPILE_FILE,                    i18n("Compiles the current sourcefile"))
//    ON_STATUS_MSG(ID_BUILD_MAKE,                            i18n("Invokes make-command"))
//    ON_STATUS_MSG(ID_BUILD_REBUILD_ALL,                     i18n("Rebuilds the program"))
//    ON_STATUS_MSG(ID_BUILD_CLEAN_REBUILD_ALL,               i18n("Invokes make clean and rebuild all"))
//    ON_STATUS_MSG(ID_BUILD_STOP,                            i18n("Stops make immediately"))
//    ON_STATUS_MSG(ID_BUILD_RUN,                             i18n("Invokes make-command and runs the program"))
//    ON_STATUS_MSG(ID_BUILD_RUN_WITH_ARGS,                   i18n("Lets you set run-arguments to the binary and invokes the make-command"))
//    ON_STATUS_MSG(ID_BUILD_DISTCLEAN,                       i18n("Invokes make distclean and deletes all compiled files"))
//    ON_STATUS_MSG(ID_BUILD_MAKECLEAN,                       i18n("Invokes make clean which deletes all object and metaobject files"))
//    ON_STATUS_MSG(ID_BUILD_AUTOCONF,                        i18n("Invokes automake and co."))
//    ON_STATUS_MSG(ID_BUILD_CONFIGURE,                       i18n("Invokes ./configure"))

//    ON_STATUS_MSG(ID_DEBUG_START,                           i18n("Invokes the debugger on the current project executable"))
//    ON_STATUS_MSG(ID_DEBUG_START_OTHER,                     i18n("Various startups for the debugger"))
//    ON_STATUS_MSG(ID_DEBUG_SET_ARGS,                        i18n("Lets you debug your project app after specifying arguments for your app."))
//    ON_STATUS_MSG(ID_DEBUG_CORE,                            i18n("Examine a core file"))
//    ON_STATUS_MSG(ID_DEBUG_NAMED_FILE,                      i18n("Debug an app other than the current project executable"))
//    ON_STATUS_MSG(ID_DEBUG_ATTACH,                          i18n("Attach to running process"))
//    ON_STATUS_MSG(ID_DEBUG_RUN,                             i18n("Continues app execution"))
//    ON_STATUS_MSG(ID_DEBUG_RUN_CURSOR,                      i18n("Continues app execution until reaching the current cursor position"))
//    ON_STATUS_MSG(ID_DEBUG_STOP,                            i18n("Kills the app and exits the debugger"))
//    ON_STATUS_MSG(ID_DEBUG_STEP,                            i18n("Step into"))
//    ON_STATUS_MSG(ID_DEBUG_STEP_INST,                       i18n("Step instr"))
//    ON_STATUS_MSG(ID_DEBUG_NEXT,                            i18n("Step over"))
//    ON_STATUS_MSG(ID_DEBUG_NEXT_INST,                       i18n("Step over instr"))
//    ON_STATUS_MSG(ID_DEBUG_FINISH,                          i18n("Run to end of function"))
//    ON_STATUS_MSG(ID_DEBUG_MEMVIEW,                         i18n("Various views into the app"))
//    ON_STATUS_MSG(ID_DEBUG_BREAK_INTO,                      i18n("Interuppt the app"))

    ON_STATUS_MSG(ID_TOOLS_DESIGNER,                        i18n("Start QT's designer (dialog editor)"))

    ON_STATUS_MSG(ID_OPTIONS_EDITOR,                        i18n("Sets the Editor's behavoir"))
    ON_STATUS_MSG(ID_OPTIONS_EDITOR_COLORS,                 i18n("Sets the Editor's colors"))
    ON_STATUS_MSG(ID_OPTIONS_SYNTAX_HIGHLIGHTING_DEFAULTS,  i18n("Sets the highlighting default colors"))
    ON_STATUS_MSG(ID_OPTIONS_SYNTAX_HIGHLIGHTING,           i18n("Sets the highlighting colors"))
    ON_STATUS_MSG(ID_OPTIONS_DOCBROWSER,                    i18n("Configures the Browser options"))
    ON_STATUS_MSG(ID_OPTIONS_TOOLS_CONFIG_DLG,              i18n("Configures the Tools-Menu entries"))
//    ON_STATUS_MSG(ID_OPTIONS_PRINT,                         i18n("Configures printing options"))
//    ON_STATUS_MSG(ID_OPTIONS_PRINT_ENSCRIPT,                i18n("Configures the printer to use enscript"))
//    ON_STATUS_MSG(ID_OPTIONS_PRINT_A2PS,                    i18n("Configures the printer to use a2ps"))
    ON_STATUS_MSG(ID_OPTIONS_KDEVELOP,                      i18n("Configures KDevelop"))

    ON_STATUS_MSG(ID_BOOKMARKS_SET,                         i18n("Sets a bookmark to the current window file"))
    ON_STATUS_MSG(ID_BOOKMARKS_TOGGLE,                      i18n("Toggles a bookmark to the current window file"))
    ON_STATUS_MSG(ID_BOOKMARKS_NEXT,                        i18n("Goes to the next bookmark in the current window file"))
    ON_STATUS_MSG(ID_BOOKMARKS_PREVIOUS,                    i18n("Goes to the previous bookmark in the current window file"))
    ON_STATUS_MSG(ID_BOOKMARKS_CLEAR,                       i18n("Clears the bookmarks for the current window"))

    ON_STATUS_MSG(ID_HELP_BACK,                             i18n("Switches to last browser page"))
    ON_STATUS_MSG(ID_HELP_FORWARD,                          i18n("Switches to next browser page"))

    ON_STATUS_MSG(ID_HELP_BROWSER_RELOAD,                   i18n("Reloads the current browser page"))
    ON_STATUS_MSG(ID_HELP_BROWSER_STOP,                     i18n("Cancels the document request"))


    ON_STATUS_MSG(ID_HELP_SEARCH_TEXT,                      i18n("Searches the selected text in the documentation"))
    ON_STATUS_MSG(ID_HELP_SEARCH,                           i18n("Lets you search individually for an expression"))

    ON_STATUS_MSG(ID_HELP_CONTENTS,                         i18n("Switches to KDevelop's User Manual"))
    ON_STATUS_MSG(ID_HELP_PROGRAMMING,                      i18n("Switches to the KDevelop Programming Handbook"))
    ON_STATUS_MSG(ID_HELP_TUTORIAL,                         i18n("Switches to the KDE Tutorials Handbook"))
    ON_STATUS_MSG(ID_HELP_KDELIBREF,                        i18n("Switches to the KDE Library Reference Guide Handbook"))
    ON_STATUS_MSG(ID_HELP_KDE2_DEVGUIDE,                    i18n("Switches to the KDE 2 Developer's Guide Handbook"))
    ON_STATUS_MSG(ID_HELP_REFERENCE,                        i18n("Switches to the C/C++-Reference"))

    ON_STATUS_MSG(ID_HELP_TIP_OF_DAY,                       i18n("Opens the Tip of the Day dialog with hints for using KDevelop"))
    ON_STATUS_MSG(ID_HELP_HOMEPAGE,                         i18n("Enter the KDevelop Homepage"))
    ON_STATUS_MSG(ID_HELP_BUG_REPORT,                       i18n("Sends a bug-report email to the KDevelop Team"))

    ON_STATUS_MSG(ID_HELP_PROJECT_API,                      i18n("Switches to the project's API-Documentation"))
    ON_STATUS_MSG(ID_HELP_USER_MANUAL,                      i18n("Switches to the project's User-Manual"))


    ON_STATUS_MSG(ID_HELP_DLGNOTES,                         i18n("Some information about the dialog editor..."))
    ON_STATUS_MSG(ID_HELP_ABOUT,                            i18n("Programmer's Hall of Fame..."))

    ON_STATUS_MSG(ID_CV_WIZARD,                             i18n("Switches to declaration/implementation"))
    ON_STATUS_MSG(ID_CV_VIEW_DECLARATION,                   i18n("Switches to the method's declaration"))
    ON_STATUS_MSG(ID_CV_VIEW_DEFINITION,                    i18n("Switches to the method's definition"))
    ON_STATUS_MSG(ID_CV_GRAPHICAL_VIEW,                     i18n("Opens the graphical inheritance tree"))
    ON_STATUS_MSG(ID_CV_CLASS_TOOL,                         i18n("Opens the classtool dialog"))
    ON_STATUS_MSG(ID_CV_CLASS_BASE_CLASSES,                 i18n("Displays the inherited classes of the current class"))
    ON_STATUS_MSG(ID_CV_CLASS_DERIVED_CLASSES,              i18n("Displays the classes who inherit the current class"))
    ON_STATUS_MSG(ID_CV_FOLDER_NEW,                         i18n("Creates a new folder"))
    ON_STATUS_MSG(ID_CV_FOLDER_DELETE,                      i18n("Deletes the current folder"))
    ON_STATUS_MSG(ID_CV_CLASS_DELETE,                       i18n("Deletes the current class"))
    ON_STATUS_MSG(ID_CV_VIEW_CLASS_DECLARATION,             i18n("Goes to the class declaration"))
    ON_STATUS_MSG(ID_CV_VIEW_CLASS_DEFINITION,				i18n("Goes to the class definition"))
    ON_STATUS_MSG(ID_CV_METHOD_NEW,                         i18n("Opens the New Method dialog"))
    ON_STATUS_MSG(ID_CV_METHOD_DELETE,                      i18n("Deletes the current class method"))
    ON_STATUS_MSG(ID_CV_ATTRIBUTE_NEW,                      i18n("Creates a new attribute for the current class"))
    ON_STATUS_MSG(ID_CV_ATTRIBUTE_DELETE,                   i18n("Deletes the current class attribute"))
    ON_STATUS_MSG(ID_CV_IMPLEMENT_VIRTUAL,                  i18n("Creates a virtual method"))
    ON_STATUS_MSG(ID_CV_ADD_SLOT_SIGNAL,                    i18n("Adds a signal/slot mechanism"))

    // LFV popups
    ON_STATUS_MSG(ID_LFV_NEW_GROUP,                         i18n("Lets you create a new logical file group"))
    ON_STATUS_MSG(ID_LFV_REMOVE_GROUP,                      i18n("Removes the selected logical file group"))
     ON_STATUS_MSG(ID_LFV_GROUP_PROP,                       i18n("Shows the group's properties"))
    ON_STATUS_MSG(ID_LFV_SHOW_PATH_ITEM,                    i18n("Displays the absolute / relative path"))
    ON_STATUS_MSG(ID_FILE_DELETE,                           i18n("Deletes the selected file"))

    // RFV popups
    ON_STATUS_MSG(ID_RFV_SHOW_NONPRJFILES,                  i18n("Show files that aren't registered as project files"))
    ON_STATUS_MSG(ID_PROJECT_CVS_UPDATE,                    i18n("Updates file/directory from repository"))
    ON_STATUS_MSG(ID_PROJECT_CVS_COMMIT,                    i18n("Commits file/directory to the repository"))
    ON_STATUS_MSG(ID_PROJECT_CVS_ADD,                       i18n("Adds file/directory to the repository"))
    ON_STATUS_MSG(ID_PROJECT_CVS_REMOVE,                    i18n("Deletes file from disk and removes it from the repository"))

    default: slotStatusMsg(i18n("Ready"));
  }
}

void CKDevelop::setAutomaticCompletion( bool enable )
{
    config->setGroup("CodeCompletion");
    config->writeEntry("automatic_completion",enable);
    config->sync();
}

void CKDevelop::setAutomaticArgsHint( bool enable )
{
    config->setGroup("CodeCompletion");
    config->writeEntry("automatic_argshint",enable);
    config->sync();
}

bool CKDevelop::getAutomaticCompletion()
{
    config->setGroup("CodeCompletion");
    return config->readBoolEntry("automatic_completion", true);
}

bool CKDevelop::getAutomaticArgsHint()
{
    config->setGroup("CodeCompletion");
    return config->readBoolEntry("automatic_argshint", true);
}

QextMdiChildView *CKDevelop::wrapper(QWidget *view, const QString &name)
{
  QextMdiChildView* pMDICover = new QextMdiChildView(name);
  QBoxLayout* pLayout = new QHBoxLayout( pMDICover, 0, -1, "layout");
  view->reparent(pMDICover, QPoint(0,0));
  pLayout->addWidget(view);
  pMDICover->setName(name);
  QString shortName = name;
  int length = shortName.length();
  shortName = shortName.right(length - (shortName.findRev('/') +1));
  pMDICover->setTabCaption(shortName);
  pMDICover->setCaption(name);

  m_widgetMap.insert(view, pMDICover);

  return pMDICover;
}

void CKDevelop::embedPartView(QWidget *view, const QString &name)
{
    kdDebug() << "CKDevelop::embedPartView()" << endl;
    QextMdiChildView *child = wrapper(view, name);

    unsigned int mdiFlags = QextMdi::StandardAdd | QextMdi::Maximize;

    addWindow(child, QPoint(0,0), mdiFlags);

    connect(view, SIGNAL(destroyed()), this, SLOT(slotWidgetDeleted()));
}

void CKDevelop::embedSelectView(QWidget *view, const QString &name)
{
    QextMdiChildView *child = wrapper(view, name);
    addToolWindow(child, KDockWidget::DockCenter, class_tree, 25, name, name);
    connect(view, SIGNAL(destroyed()), this, SLOT(slotWidgetDeleted()));
    m_selectViews.append(child);
}
void CKDevelop::embedOutputView(QWidget *view, const QString &name)
{
    QextMdiChildView *child = wrapper(view, name);
    addToolWindow(child, KDockWidget::DockCenter, messages_widget, 25, name, name);
    connect(view, SIGNAL(destroyed()), this, SLOT(slotWidgetDeleted()));
    m_outputViews.append(child);
}

void CKDevelop::removeView(QWidget *view)
{
    kdDebug() << "CKDevelop::removeView()" << endl;
    QextMdiChildView *wrapper = m_widgetMap[view];
    if (wrapper)
    {
        closeWindow(wrapper);

        m_selectViews.remove(wrapper);
        m_outputViews.remove(wrapper);

        m_widgetMap.remove(view);
    }
}

void CKDevelop::raiseView(QWidget *view)
{
    kdDebug() << "CKDevelop::raiseView()" << endl;
    QextMdiChildView *wrapper = m_widgetMap[view];
    if (wrapper)
    {
        wrapper->activate();
        activateView(wrapper);
    }
}

void CKDevelop::lowerView(QWidget *view)
{
    // ignored in MDI mode!
    kdDebug() << "CKDevelop::lowerView()" << endl;
}

void CKDevelop::loadSettings()
{
}

KMainWindow * CKDevelop::main()
{
    return this;
}

CKDevelop* CKDevelop::getInstance()
{
    if( !m_instance ){
        new CKDevelop();
    }
    return m_instance;
}

void CKDevelop::slotWidgetDeleted()
{
    QWidget *w = (QWidget*)sender();

    removeView(w);
}
