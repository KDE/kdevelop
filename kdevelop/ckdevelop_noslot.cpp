/***************************************************************************
                    noslot.cpp - some no slot functions in CKDevelop
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

#include <qdir.h>
#include "ckdevelop.h"

#include "cclassview.h"
#include "cdocbrowser.h"
//#include "ceditwidget.h"
#include "clogfileview.h"
#include "crealfileview.h"
#include "ctoolclass.h"
#include "debug.h"
#include "doctreeview.h"
#include "./dbg/brkptmanager.h"
#include "./dbg/vartree.h"
#include "./dbg/framestack.h"
#include "./dbg/disassemble.h"
#include "ckonsolewidget.h"
#include "docviewman.h"
//#include "kwdoc.h"
#include <kate/document.h>
#include <kate/view.h>


#include "coutputwidget.h"

#include <kaction.h>
#include <kstdaction.h>

#include <khtmlview.h>
#include <kcursor.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kprogress.h>
#include <kstatusbar.h>
#include <kstddirs.h>
#include <krun.h>

#include <kurl.h>
#include <kopenwith.h>
#include <kmimetype.h>

#include <qmessagebox.h>
#include <qprogressbar.h>
#include <qprogressdialog.h>

#include <qdatetime.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


SaveAllDialog::SaveAllResult 
CKDevelop::doProjectSaveAllDialog(const QString &filename)
{
  SaveAllDialog dialog(filename, prj);
  dialog.exec();
  return dialog.result();
}


/*---------------------------------------- isProjectDirty()
 * bool CKDevelop::isProjectDirty()
 *
 *  search all edit_infos for any files that have the modified flag set
 *  and if one of these files are younger than the target file
 *
 *  restrictions: this needs a project to be opened!!!
 *
 * Returns:
 *       returns true if there is at least one file that has been modified.
 *-----------------------------------------------------------------*/
bool CKDevelop::isProjectDirty()
{
  bool isClean=true;

  QString prjDir=prj->getProjectDir();
  QFileInfo bin_info(prjDir + prj->getSubDir() + prj->getBinPROGRAM());
  QStrList listAllPrjFiles;
  const char *filename;

  prj->getAllFiles(listAllPrjFiles);
  if (!bin_info.exists())
    isClean=false;

  // check if a Makefile.am has changed
  if (prj->getMakefileAmChanged())
    isClean=false;

  // m_docViewManager->synchronizeDocAndInfo();

  for(filename=listAllPrjFiles.first(); 
      isClean && filename != 0; 
      filename=listAllPrjFiles.next())
  {
    // only check valid names and don't check files 
    // like *kdevprj or AUTHORS etc.
    if (*filename!='\0' && CProject::getType(filename)!=DATA &&
           CProject::getType(filename)!=KDEV_DIALOG)
    {
      Kate::Document* pDoc = m_docViewManager->findKWriteDoc(prjDir+filename);
      QFileInfo src_info(prjDir + filename);

      if (pDoc)
      {
        // here we are... having the file already opened
        if (pDoc->isModified() 
        // i took this out because we dont have a way to check the date
        // the document was modified last anylonger, it is probably the
        // same to check the filename's QFileInfo i suppose. So this may
        // very likely break something. (rokrau 6/28/01)
        /*|| bin_info.lastModified() < pDoc->getLastFileModifDate()*/)
          isClean=false;
      }
      /* here only the check if the file would be younger than the target file
         i. e. the project binary
         this should be checked always... even if the file is already loaded
         so we can check if the source was modified outside the buffer
      */
      if (isClean && bin_info.lastModified() < src_info.lastModified())
        isClean=false;
    }
  }

  return !isClean;
}

/*---------------------------------------- CKDevelop::setMainCaption()
 * setMainCaption()
 *
 *  make the caption of the main window and sets the generated title
 *
 * Parameters:
 *
 * Returns:
 *    nothing
 *-----------------------------------------------------------------*/
void CKDevelop::setMainCaption(int item)
{
  QString capt=QString::null;
//  Kate::View* pCEW = m_docViewManager->currentEditView();
  Kate::Document* pDoc = m_docViewManager->currentEditDoc();
  switch(item)
  {
  case BROWSER:
    {
      CDocBrowser* pCurBrowserDoc = m_docViewManager->currentBrowserDoc();
      KHTMLView* pCurBrowserView = m_docViewManager->currentBrowserView();
      if (pCurBrowserDoc) {
        setCaption(pCurBrowserDoc->currentTitle());
        QextMdiChildView* pMDICover = (QextMdiChildView*) pCurBrowserView->parentWidget();
        if (pMDICover) {
          pMDICover->setCaption(pCurBrowserDoc->currentURL());
          pMDICover->setTabCaption(pCurBrowserDoc->currentTitle());
        }
      }
    }
    break;

  default:
    if (pDoc && activeWindow()) {
      //capt = QFileInfo(pCEW->getName()).fileName();
      QString name=pDoc->docName();
      int len=name.length();
      int ip=name.findRev("/",-1);
      capt = name.right(len-ip-1);
    }
    if (prj && capt.isNull()) {
      capt = prj->getProjectName();
    }
    if (pDoc && pDoc->isModified())
    {
      enableCommand(ID_FILE_SAVE);
      setCaption(capt,true);
    }
    else
    {
      disableCommand(ID_FILE_SAVE);
      setCaption(capt,false);
    }
    break;
  }
}


/*---------------------------------------- CKDevelop::isUntitled()
 * isUntitled()
 *
 *  static method
 *  checks if the passed name
 *  is defined by KDevelop (should be changed by the user)
 *
 * Parameters:
 *  filename to check
 * Returns:
 *   true if filename isn´t still defined by user
 *        (means some Untitled-name)
 *-----------------------------------------------------------------*/
bool CKDevelop::isUntitled(const char* name)
{
  QString s=(name) ? name : "";
  return (s==i18n("Untitled.h") || s==i18n("Untitled.c") || s==i18n("Untitled.cpp") || s==i18n("Untitled.m"));
}

/*---------------------------------------- CKDevelop::fileSaveAs()
 * fileSaveAs()
 *
 *  makes file "save As" handling
 *  and returns true if it succeeded
 *
 * Parameters:
 *  none
 * Returns:
 *   true if the file was saved
 *-----------------------------------------------------------------*/
bool CKDevelop::fileSaveAs(){

  QString name, oldName;
  int message_result=KMessageBox::Yes; // simulate ok state... this could change by one of the following messageboxes

  Kate::View* pView=m_docViewManager->currentEditView();
  if (!pView)
    return false;

  oldName = pView->getDoc()->docName();
  if (bAutosave)
    saveTimer->stop();

  do
  {
    if (!oldName.isNull() && !isUntitled(oldName))
      name = KFileDialog::getSaveFileName(oldName,QString::null, 0,oldName);
    else
      name = KFileDialog::getSaveFileName(((project) ?  QString(prj->getProjectDir()+oldName) : oldName),
                                          QString::null,0,oldName);

    if (name.isNull()) {
    // KDEBUG(KDEBUG_INFO,CKDEVELOP,"Cancel");
      if (bAutosave)
       saveTimer->start(saveTimeout);
     return false;
    }

    // check if the extension is changed and the widget or program to view must change
    if (CProject::getType(name)!=CProject::getType(pView->getDoc()->docName()))
      message_result = KMessageBox::warningYesNoCancel(this,
                            i18n("Do you really want to save the file\nas another type of document?"),
                            i18n("Save as new type of document?"));

    if(message_result == KMessageBox::Yes && QFile::exists(name)) {
      message_result=KMessageBox::warningYesNoCancel(this,
                        i18n("\nThe file\n\n%1\n\n"
                              "already exists.\nDo you want overwrite the old one?\n").arg(name),
                        i18n("File Exists!"));
    }
    
  } while (message_result == KMessageBox::No); // repeat it on 'no'


  if (message_result==KMessageBox::Cancel) {
     //KDEBUG(KDEBUG_INFO,CKDEVELOP,"Cancel on new type question");
      if (bAutosave)
       saveTimer->start(saveTimeout);
     return false;
  }


  // search if we can find a doc with the new desired filename  ...
  // means already loaded
  Kate::Document* pActualDoc = m_docViewManager->findKWriteDoc(name);
  Kate::Document* pOldDoc = m_docViewManager->findKWriteDoc(oldName);
    
  // now that all cancel possibilities are handled simulate a changed file
  // m_docViewManager->currentEditView()->toggleModified(true);

  // i hope this works, sometimes i dont understand why saving a document
  // must be so obfuscated (rokrau 6/28/01)
  pView->getDoc()->setDocName(name);
  if (!pView->save()) {
  // if saving failed
      if (bAutosave)
       saveTimer->start(saveTimeout);
     return false;
  }

  if (pActualDoc != 0 && pActualDoc == pOldDoc) {
    // here we are ... saving the file with the same name
    //   so only the modified-flags have to be changed
    pActualDoc->setModified(false);
    //m_docViewManager->currentEditView()->setModified(false);
  }
  else {
    // now open this file as new file in edit_infos
    //    if an widget still contains the file then update the contents in the widget from file
    switchToFile(name,-1,-1, true);

    if (oldName!=name) {
      // here we are... and any Untitled-file was saved with another name
      //   and now we can remove the untitled file
      if (isUntitled(oldName)) {
// m_docViewManager->removeFileFromEditlist(oldName);
      }
    }
    
    QStrList lToRefresh;
    lToRefresh.append(oldName);
    lToRefresh.append(name);
    refreshTrees(&lToRefresh);
//    slotViewRefresh();
  }

  if (bAutosave)
     saveTimer->start(saveTimeout);

  return true;
}

/*---------------------------------- CKDevelop::realSearchText2regExp()
 * realSearchText2regExp()
 *    converts a string to a regular expression.
 *    you will need the function if you want to search even this text.
 *    chars which are special chars in a regular expression are escaped with '\\'
 *
 * Parameters:
 *   szOldText   the search text you want to convert
 *   bForGrep    set this true if you will change the special chars from grep
 *
 * Returns:
 *   a string object with the escaped string
 *-----------------------------------------------------------------*/
QString CKDevelop::realSearchText2regExp(const char *szOldText, bool bForGrep)
{
  QString sRegExpString="";
  char ch;
  int i;
  bool bFound;
  char *szChangingChars= (bForGrep) ? (char*)"[]\\^$" : (char*)"$^*[]|()\\;,#<>-.~{}" ;

  if (szOldText) {
    while ((ch=*szOldText++)!='\0') {
       bFound=false;
       for (i=0; !bFound && szChangingChars[i]!='\0';)
      {
         if (szChangingChars[i]==ch)
             bFound=true;
         else
             i++;
      }
      if (bFound)
          sRegExpString+='\\';
      sRegExpString+=ch;
    }
  }
  return sRegExpString;
}

void CKDevelop::refreshClassViewByFileList(QStrList *iFileList)
{
      //first we'll separate the headers and the source files
      QStrList lHeaderList(FALSE);    //no deep copies
      QStrList lSourceList(FALSE);
      ProjectFileType lCurFileType;
      for (const char* lCurFile = iFileList->first(); lCurFile; lCurFile = iFileList->next())
      {
          lCurFileType = prj->getType(lCurFile);
          switch(lCurFileType)
          {
              case CPP_HEADER:
                  lHeaderList.append(lCurFile);
                  break;
              case FTN_SOURCE:
              case CPP_SOURCE:
                  lSourceList.append(lCurFile);
                  break;
              //skip all the other files
              default:
                  break;
          }
      }
      class_tree->refresh(lHeaderList, lSourceList);
}

/*------------------------------------------ CKDevelop::refreshTrees()
 * refreshTrees()
 *   Refresh the all trees. Includes re-parsing all files.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::refreshTrees(QStrList * iFileList)
{
  CProject* pPrj=getProject();
  if (!pPrj)
    return;
  kapp->processEvents(100);
  doc_tree->refresh(pPrj);
  // Update the classview.
  slotStatusMsg(i18n("Scanning project files..."));
    setCursor(KCursor::waitCursor());
  statProg->show();
  if (iFileList)
  {
    refreshClassViewByFileList(iFileList);
  }
  else
  {
    kapp->processEvents(100);
    class_tree->refresh(pPrj);
  }
  statProg->reset();

  // Update the classcombo.
  kapp->processEvents(100);
  CVRefreshClassCombo();

  // Update LFV.
  kapp->processEvents(100);
  log_file_tree->storeState(pPrj);
  kapp->processEvents(100);
  slotStatusMsg(i18n("Refreshing tree views..."));
  log_file_tree->refresh(pPrj);

  // Update RFV.
  kapp->processEvents(100);
  real_file_tree->refresh(pPrj);

  kapp->processEvents(100);

  statusBar()->repaint();
  setCursor(KCursor::arrowCursor());    
  // update the file_open_menu
  file_open_list=pPrj->getHeaders();
  QStrList sources=pPrj->getSources();
  uint j;
  for( j=0; j< sources.count(); j++){
    file_open_list.append(sources.at(j));
  }
  // create the file_open_popup for the toolbar
  file_open_popup->clear();
  uint i;
  for ( i =0 ; i < file_open_list.count(); i++){
    QFileInfo fileInfo (file_open_list.at(i));
    file_open_popup->insertItem(fileInfo.fileName());
  }
  slotStatusMsg(i18n("Ready."));
}
 
/*------------------------------------------ CKDevelop::refreshTrees()
 * refreshTrees()
 *   Refresh the file trees with regard to a certain file.
 *
 * Parameters:
 *   aFilename   The relative filename.
 *   aType       Type of file.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::refreshTrees(TFileInfo *info)
{
  CProject* pPrj=getProject();
  if (!pPrj)
    return;

  kapp->processEvents(100);
  // If this is a sourcefile we parse it and update the classview.
  if( info->type == CPP_SOURCE || info->type == CPP_HEADER )
      {
          class_tree->addFile( pPrj->getProjectDir() + info->rel_name );
          CVRefreshClassCombo();
      }

  // Update LFV.
  kapp->processEvents(100);
  log_file_tree->storeState(pPrj);
  log_file_tree->refresh(pPrj);

  // Update RFV.
  kapp->processEvents(100);
  real_file_tree->refresh(pPrj);
  // update dialogs tree
  kapp->processEvents(100);

  // refresh the file_open_list
  file_open_list=pPrj->getHeaders();
  QStrList sources=pPrj->getSources();
  uint j;
  for( j=0; j< sources.count(); j++){
    file_open_list.append(sources.at(j));
  }
  // create the file_open_popup for the toolbar
  file_open_popup->clear();
  uint i;
  for ( i =0 ; i < file_open_list.count(); i++){
    QFileInfo fileInfo (file_open_list.at(i));
    file_open_popup->insertItem(fileInfo.fileName());
  }
}

void CKDevelop::switchToFile( QString filename, int line, int col,
                              bool bForceReload, bool bShowModifiedBox)
{
  if (!isUntitled(filename)) {
    // We consider only symbolic links in directories here,
    // not links in files or hardlinks. The _real_ solution
    // is to use devno/inode anyway.
    QFileInfo fi(filename);
    QString dirpart = fi.dir(true).canonicalPath();
    QString filepart = fi.fileName();
//    kdDebug() << "Path: " << dirpart << endl;
//    kdDebug() << "File: " << filepart << endl;
    filename = dirpart + "/" + filepart;

    // tell the konsole about our new path
    konsole_widget->setDirectory(dirpart);
  }

  QFileInfo fileInfo(filename);
  // check if the file exists
  if(!fileInfo.exists() && !isUntitled(filename)){
    KMessageBox::error(this, i18n("%1\n\nFile does not exist!").arg(filename));
    return;
  }

  QString ext = fileInfo.extension(false);

  KURL url;
  url.setFileName(filename);
  QString type =  KMimeType::findByURL(url, 0, true)->name();
  if (!(type.startsWith("text/")
    || type.startsWith("application/x-perl") || type.startsWith("application/x-python")
    || type.startsWith("application/x-shellscript") || type.startsWith("application/x-desktop")
    || type.startsWith("application/x-kdevelop-project") ) )  // open with krun
  {
    bool bStartWithKRun = true;
    if (type.startsWith("image/x-xpm") || type.startsWith("application/x-gettext") || type.startsWith("application/x-linguist") 
     || type.startsWith("application/x-designer")) {
      if (KMessageBox::questionYesNo(this, i18n("Do you want to load it as ASCII file?"), i18n("Load decision")) == KMessageBox::Yes) {
        bStartWithKRun = false;
      }
    }
    if (bStartWithKRun) {
      new KRun( url );
      return;
    }
  }

        // Enable or disable compile command
        if (CProject::getType(filename) == CPP_SOURCE){
                KAction* pAction=actionCollection()->action("build_make");
                if (pAction && pAction->isEnabled()) {
                        actionCollection()->action("build_compile")->setEnabled(true);
                }
        }
        else
        {
                actionCollection()->action("build_compile")->setEnabled(false);
        }

  // Ask the doc manager
  m_docViewManager->doSwitchToFile(filename, line, col,
                                  bForceReload,  bShowModifiedBox);

  // Need to get the breakpoints displayed in this file (if any)
  if (brkptManager)
    brkptManager->refreshBP(filename);

}

//void CKDevelop::switchToFile(QString filename, int lineNo){
//  lasttab = s_tab_view->getCurrentTab();
//  switchToFile( filename, false, lineNo, 0);
//  pCurEditWidget->setCursorPosition( lineNo, 0 );
//}

void CKDevelop::startDesigner()
{
  KShellProcess designer_process("/bin/sh");
  const QString oldGroup = config->group();
  config->setGroup("QT2");
  QString qt2dir = QString ("QTDIR=")+ config->readEntry("qt2dir",getenv("QTDIR")) +" ";
  config->setGroup(oldGroup);
  designer_process << qt2dir << "designer";
  if(!designer_process.start(KProcess::DontCare)) {
    debug("QtDesigner didn't start!");
  }
  return;
}


void CKDevelop::setToolMenuProcess(bool enable){

        if (enable)
//                stateChanged("build_stop",StateReverse);
                //disableCommand(ID_BUILD_STOP);
//        else
//                stateChanged("build_stop");
                //enableCommand(ID_BUILD_STOP);

        if (enable && project){

//                if (m_docViewManager->curDocIsCppFile())
//                        stateChanged("build_compile");
                        //enableCommand(ID_BUILD_COMPILE_FILE);

                enableCommand(ID_CV_TOOLBAR_COMPILE_CHOICE); // enable switching project configs

//                stateChanged("build",StateReverse);

//    enableCommand(ID_BUILD_RUN);
//    enableCommand(ID_BUILD_RUN_WITH_ARGS);
//    enableCommand(ID_DEBUG_START);
//    enableCommand(ID_DEBUG_START_OTHER);
//    enableCommand(ID_BUILD_MAKE);
//    enableCommand(ID_BUILD_REBUILD_ALL);
//    enableCommand(ID_BUILD_CLEAN_REBUILD_ALL);
//    enableCommand(ID_BUILD_DISTCLEAN);
//    enableCommand(ID_BUILD_MAKECLEAN);
//    enableCommand(ID_BUILD_AUTOCONF);
//    enableCommand(ID_BUILD_CONFIGURE);

                QString type=prj->getProjectType();
//                if (!(prj->isKDEProject()||prj->isKDE2Project()||prj->isQt2Project()))
//                        stateChanged("project_make_msg");
                        //disableCommand(ID_PROJECT_MESSAGES);
//                else
//                        stateChanged("project_make_msg",StateReverse);
                        //enableCommand(ID_PROJECT_MESSAGES);
//                stateChanged("project");

//    enableCommand(ID_PROJECT_CLOSE);
//    enableCommand(ID_PROJECT_NEW_CLASS);
//    enableCommand(ID_PROJECT_ADD_FILE_EXIST);
//    enableCommand(ID_PROJECT_ADD_NEW_TRANSLATION_FILE);
//    enableCommand(ID_PROJECT_FILE_PROPERTIES);
//    enableCommand(ID_PROJECT_OPTIONS);

//                stateChanged("file_new");

//    enableCommand(ID_FILE_NEW);

  //MB
//    enableCommand(ID_PROJECT_DOC_TOOL);
  //MB end
//    enableCommand(ID_PROJECT_MAKE_PROJECT_API);
//    enableCommand(ID_PROJECT_MAKE_USER_MANUAL);
//    enableCommand(ID_PROJECT_MAKE_DISTRIBUTION);
//    enableCommand(ID_PROJECT_MAKE_TAGS);
//    enableCommand(ID_PROJECT_LOAD_TAGS);
  }
  else        
//  if (!enable)
  {
    // set the popupmenus enable or disable
          disableCommand(ID_CV_TOOLBAR_COMPILE_CHOICE); // disable switching project configs during an operation
    disableCommand(ID_BUILD_COMPILE_FILE);
    disableCommand(ID_BUILD_RUN_WITH_ARGS);
    disableCommand(ID_BUILD_RUN);
    disableCommand(ID_DEBUG_START);
    disableCommand(ID_DEBUG_START_OTHER);
    disableCommand(ID_BUILD_MAKE);
    disableCommand(ID_BUILD_REBUILD_ALL);
    disableCommand(ID_BUILD_CLEAN_REBUILD_ALL);
    disableCommand(ID_BUILD_DISTCLEAN);
    disableCommand(ID_BUILD_MAKECLEAN);
    disableCommand(ID_BUILD_AUTOCONF);
    disableCommand(ID_BUILD_CONFIGURE);
    disableCommand(ID_PROJECT_MESSAGES);
  //MB
    disableCommand(ID_PROJECT_DOC_TOOL);
  //MB end
    disableCommand(ID_PROJECT_MAKE_PROJECT_API);
    disableCommand(ID_PROJECT_MAKE_USER_MANUAL);
    disableCommand(ID_PROJECT_MAKE_DISTRIBUTION);
    disableCommand(ID_PROJECT_MAKE_TAGS);
    disableCommand(ID_PROJECT_LOAD_TAGS);

    disableCommand(ID_PROJECT_CLOSE);
    disableCommand(ID_PROJECT_NEW_CLASS);
    disableCommand(ID_PROJECT_ADD_FILE_EXIST);
    disableCommand(ID_PROJECT_ADD_NEW_TRANSLATION_FILE);
    disableCommand(ID_PROJECT_FILE_PROPERTIES);
    disableCommand(ID_PROJECT_OPTIONS);

//    KAction* pFileNewAction = actionCollection()->action("file_new");
//    pFileNewAction->setEnabled(false);
    //disableCommand(ID_FILE_NEW);
  }

  if(bAutosave)
  {
    if(enable)
      saveTimer->start(saveTimeout); // restart autosaving if enabled after a process finished
    else
      saveTimer->stop();  // stop the autosaving if make or something is running
  }
}


//void CKDevelop::switchToWorkspace(int id){
//  workspace = id;
//  if(id == 1){
//    project_menu->setItemChecked(ID_PROJECT_WORKSPACES_1,true);
//    project_menu->setItemChecked(ID_PROJECT_WORKSPACES_2,false);
//    project_menu->setItemChecked(ID_PROJECT_WORKSPACES_3,false);
//  }
//  if(id == 2){
//    project_menu->setItemChecked(ID_PROJECT_WORKSPACES_1,false);
//    project_menu->setItemChecked(ID_PROJECT_WORKSPACES_2,true);
//    project_menu->setItemChecked(ID_PROJECT_WORKSPACES_3,false);
//  }
//  if(id == 3){
//    project_menu->setItemChecked(ID_PROJECT_WORKSPACES_1,false);
//    project_menu->setItemChecked(ID_PROJECT_WORKSPACES_2,false);
//    project_menu->setItemChecked(ID_PROJECT_WORKSPACES_3,true);
//  }
//  prj->getWorkspace(id);
//}

void CKDevelop::showOutputView(bool show)
{
        if (bAutoswitch) {
                if (show) {
                        if (isToolViewVisible(messages_widget)) {
                                // if it's a tab page, raise the messages_widget
                                makeWidgetDockVisible(messages_widget->parentWidget());
                        }
                        else {
                                slotViewTOutputView();
                        }
                }
                else {
                        //KToggleAction* pViewToolbarAction = dynamic_cast<KToggleAction*>
                        //                                    (actionCollection()->action("view_toolbar"));
                        //if (pViewToolbarAction && !pViewToolbarAction->isChecked()) {
//      if (!view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW)) {
                        //        return; //it's already unvisible
                        //}
                        //else {
                        //        slotViewTOutputView();
                        //}
                }
        }
}

void CKDevelop::adjustTTreesToolButtonState()
{
  bool bClassesChecked = isToolViewVisible(class_tree);
  bool bGroupsChecked = isToolViewVisible(log_file_tree);
  bool bFilesChecked = isToolViewVisible(real_file_tree);
  bool bBooksChecked = isToolViewVisible(doc_tree);
  bool bWatchChecked = isToolViewVisible(var_viewer);

  if (dbgController != 0L) {  // if not debugging, don't consider the watch view
//    toolBar()->setButton(ID_VIEW_TREEVIEW, (bClassesChecked && bGroupsChecked && bFilesChecked && bBooksChecked && bWatchChecked));
  }
  else {
//    toolBar()->setButton(ID_VIEW_TREEVIEW, (bClassesChecked && bGroupsChecked && bFilesChecked && bBooksChecked));
  }
}

void CKDevelop::adjustTOutputToolButtonState()
{
  bool bMessagesChecked = isToolViewVisible(messages_widget);
  bool bStdOutChecked = isToolViewVisible(stdin_stdout_widget);
  bool bStdErrChecked = isToolViewVisible(stderr_widget);
  bool bKonsoleChecked = isToolViewVisible(konsole_widget);
  bool bBreakpointsChecked = isToolViewVisible(brkptManager);
  bool bDisassembleChecked = isToolViewVisible(disassemble);
  bool bCallStackChecked = isToolViewVisible(frameStack);
#if defined(GDB_MONITOR) || defined(DBG_MONITOR)
  bool bDebuggerChecked = isToolViewVisible(dbg_widget);
#endif

  if (dbgController != 0L) {  // if not debugging, don't consider the disassemble, callstack and debugger views
#if defined(GDB_MONITOR) || defined(DBG_MONITOR)
//    toolBar()->setButton(ID_VIEW_OUTPUTVIEW, (bMessagesChecked && bStdOutChecked && bStdErrChecked && bKonsoleChecked && bBreakpointsChecked && bDisassembleChecked && bCallStackChecked && bDebuggerChecked));
#else
//    toolBar()->setButton(ID_VIEW_OUTPUTVIEW, (bMessagesChecked && bStdOutChecked && bStdErrChecked && bKonsoleChecked && bBreakpointsChecked && bDisassembleChecked && bCallStackChecked));
#endif
  }
  else {
//    toolBar()->setButton(ID_VIEW_OUTPUTVIEW, (bMessagesChecked && bStdOutChecked && bStdErrChecked && bKonsoleChecked && bBreakpointsChecked));
  }
}

void CKDevelop::readOptions()
{
        applyMainWindowSettings(config);

        kdDebug() << "in CKDevelop::readOptions():\n" ;

        config->setGroup("General Options");
        KToggleAction* pToggleAction = dynamic_cast<KToggleAction*>
                                       (actionCollection()->action("view_toolbar"));
        if(config->readBoolEntry("show_std_toolbar", true) && pToggleAction) {
                pToggleAction->setChecked(true);
        }
        else {
                pToggleAction->setChecked(false);
        }
        pToggleAction = dynamic_cast<KToggleAction*>
                        (actionCollection()->action("view_browser"));
        if(config->readBoolEntry("show_browser_toolbar",true) && pToggleAction) {
                pToggleAction->setChecked(true);
        }
        else {
                pToggleAction->setChecked(false);
        }
        pToggleAction = dynamic_cast<KToggleAction*>
                        (actionCollection()->action("view_status"));
        if (config->readBoolEntry("show_statusbar",true) && pToggleAction) {
                pToggleAction->setChecked(true);
        }
        else {
                pToggleAction->setChecked(false);
        }
        pToggleAction = dynamic_cast<KToggleAction*>
                        (actionCollection()->action("view_mdi"));
        if (config->readBoolEntry("show_mdi_view_taskbar",m_pTaskBar->isSwitchedOn())
            && pToggleAction) {
                showViewTaskBar();
                pToggleAction->setChecked(true);
        }
        else {
                hideViewTaskBar();
                pToggleAction->setChecked(false);
        }

//  config->setGroup("General Options");
        int mode=config->readNumEntry("tabviewmode", 3);
        KRadioAction* pRadioAction;
        switch (mode){
                case 1:
                        pRadioAction = dynamic_cast<KRadioAction*>
                                       (actionCollection()->action("view_tab_text"));
                        if (pRadioAction) pRadioAction->setChecked(true);
                break;
                case 2:
                        pRadioAction = dynamic_cast<KRadioAction*>
                                       (actionCollection()->action("view_tab_icons"));
                        if (pRadioAction) pRadioAction->setChecked(true);
                break;
                case 3:
                        pRadioAction = dynamic_cast<KRadioAction*>
                                       (actionCollection()->action("view_tab_texticons"));
                        if (pRadioAction) pRadioAction->setChecked(true);
                break;
        }

        // read setting whether to use the ctags search database
        bCTags = config->readBoolEntry("use_ctags", false);
  config->setGroup("General Options");
    /////////////////////////////////////////
    // RUNTIME VALUES AND FILES
  bAutosave=config->readBoolEntry("Autosave",true);
  saveTimeout=config->readNumEntry("Autosave Timeout",5*60*1000);
  saveTimer=new QTimer(this);
  connect(saveTimer,SIGNAL(timeout()),SLOT(slotFileSaveAll()));
  if(bAutosave){
    saveTimer->start(saveTimeout);
  }
  else{
    saveTimer->stop();
  }
  bAutoswitch=config->readBoolEntry("Autoswitch",true);
  bStartupEditing=config->readBoolEntry("StartupEditing",true);
  bDefaultCV=config->readBoolEntry("DefaultClassView",true);
  make_cmd=config->readEntry("Make","make");
  //  make_with_cmd=config->readEntry("MakeWith","");

        config->setGroup("General Options");
        /////////////////////////////////////////
        // RUNTIME VALUES AND FILES
        bAutosave=config->readBoolEntry("Autosave",true);
        saveTimeout=config->readNumEntry("Autosave Timeout",5*60*1000);
        saveTimer=new QTimer(this);
        connect(saveTimer,SIGNAL(timeout()),SLOT(slotFileSaveAll()));
        if(bAutosave){
                saveTimer->start(saveTimeout);
        }
        else{
                saveTimer->stop();
        }
        bAutoswitch=config->readBoolEntry("Autoswitch",true);
        bDefaultCV=config->readBoolEntry("DefaultClassView",true);
        make_cmd=config->readEntry("Make","make");
//  make_with_cmd=config->readEntry("MakeWith","");

        config->setGroup("Files");
        pRecentProjects->loadEntries(config,"Recent Projects");


        doctool = config->readNumEntry("doc_tool_type");
        KToggleAction* pDoxyAction = dynamic_cast<KToggleAction*>
                                     (actionCollection()->action("project_api_doxygen"));
        KToggleAction* pKdocAction = dynamic_cast<KToggleAction*>
                                     (actionCollection()->action("project_api_kdoc"));
        KAction* pDoxyConfAction = actionCollection()->action("project_api_doxyconf");

        // must be done here - cause the call comes AFTER the initialization of Project menue :(
        if (pDoxyAction && pKdocAction && pDoxyConfAction) {
                if (doctool == DT_KDOC || doctool == 0)
                {
                        pKdocAction->setChecked(true);
                        //pDoxyAction->setChecked(false);
                        if (hasProject())
                                pDoxyConfAction->setEnabled(false);
                }
                else if (doctool == DT_DOX)
                {
                        pDoxyAction->setChecked(true);
                        //pKdocAction->setChecked(false);
                        if (hasProject())
                                pDoxyConfAction->setEnabled(true);
                }
        }

        m_docViewManager->readBookmarkConfig(config);
        /*
        doc_bookmarks_list.setAutoDelete(TRUE);
        doc_bookmarks_title_list.setAutoDelete(TRUE);
    
        config->readListEntry("doc_bookmarks",doc_bookmarks_list);
        config->readListEntry("doc_bookmarks_title",doc_bookmarks_title_list);
        for ( uint i =0 ; i < doc_bookmarks_title_list.count(); i++){
    doc_bookmarks->insertItem(SmallIconSet("html"),doc_bookmarks_title_list.at(i));
        }
        */

   // restore MDI mode
   config->setGroup("General Options");
   int mdiModeNum = config->readNumEntry( "MDI mode", (int)QextMdi::ChildframeMode);
   QextMdi::MdiMode mdiMode;
   mdiMode = (QextMdi::MdiMode)mdiModeNum;
   switch (mdiMode) {
   case QextMdi::ToplevelMode:
      {
         int childFrmModeHt = config->readNumEntry( "Childframe mode height", 400);
         resize( width(), childFrmModeHt);
         switchToToplevelMode();
      }
      break;
   case QextMdi::ChildframeMode:
      break;
   case QextMdi::TabPageMode:
      {
         int childFrmModeHt = config->readNumEntry( "Childframe mode height", 400);
         resize( width(), childFrmModeHt);
         switchToTabPageMode();
      }
      break;
   default:
      break;
   }
   bool bMaximizeMode = config->readBoolEntry( "Maximize mode", true);  // maximized is default
   setEnableMaximizedChildFrmMode(bMaximizeMode);
}

void CKDevelop::saveOptions(){
    
        saveMainWindowSettings (config);
        config->setGroup("General Options");
        // toolbar visible
        KToggleAction* pToggleAction = dynamic_cast<KToggleAction*>
                                       (actionCollection()->action("view_toolbar"));
        if (pToggleAction)
                config->writeEntry("show_std_toolbar",pToggleAction->isChecked());
        // browser toolbar visible
        pToggleAction = dynamic_cast<KToggleAction*>
                        (actionCollection()->action("view_browser"));
        if (pToggleAction)
                config->writeEntry("show_browser_toolbar",pToggleAction->isChecked());
        // status bar visible
        pToggleAction = dynamic_cast<KToggleAction*>
                        (actionCollection()->action("view_status"));
        if (pToggleAction)
                config->writeEntry("show_statusbar",pToggleAction->isChecked());
        // mdi taskbar visible
        pToggleAction = dynamic_cast<KToggleAction*>
                        (actionCollection()->action("view_mdi"));
        if (pToggleAction)
                config->writeEntry("show_mdi_view_taskbar",pToggleAction->isChecked());

        // write setting whether to use the ctags search database
        config->writeEntry("use_ctags", bCTags);

        // set the mode of the tab headers
        KRadioAction*
        pRadioAction = dynamic_cast<KRadioAction*>
                       (actionCollection()->action("view_tab_text"));
        if (pRadioAction && pRadioAction->isChecked())
                config->writeEntry("tabviewmode", 1);
        pRadioAction = dynamic_cast<KRadioAction*>
                       (actionCollection()->action("view_tab_icons"));
        if (pRadioAction && pRadioAction->isChecked())
                config->writeEntry("tabviewmode", 2);
        pRadioAction = dynamic_cast<KRadioAction*>
                       (actionCollection()->action("view_tab_texticons"));
        if (pRadioAction && pRadioAction->isChecked())
                config->writeEntry("tabviewmode", 3);

        // write current chosen MDI mode
  config->writeEntry("MDI mode", mdiMode());

  config->writeEntry("lfv_show_path",log_file_tree->showPath());

  config->writeEntry("Autosave",bAutosave);
  config->writeEntry("StartupEditing",bStartupEditing);
  
  config->writeEntry("Autosave Timeout",saveTimeout);

  config->writeEntry("Make",make_cmd);

  config->setGroup("Files");
  config->writeEntry("browser_file",history_list.current());

        m_docViewManager->writeBookmarkConfig(config);
/*
  config->writeEntry("doc_bookmarks", doc_bookmarks_list);
  config->writeEntry("doc_bookmarks_title", doc_bookmarks_title_list);
*/

  pRecentProjects->saveEntries(config,"Recent Projects");

  //MB serializes menuoptions
  config->writeEntry("doc_tool_type",doctool);
  //MB end
  writeDockConfig(config, "docking_version_2_0");
  config->sync();
}

bool CKDevelop::queryExit(){
  kdDebug() << "queryExit()..." << endl;
  if (!bStartupIsPending) {
    QApplication::sendPostedEvents();
    saveOptions();
  }
  return true;
}

bool CKDevelop::queryClose(){
  kdDebug() << "queryClose()..." << endl;

  if (bStartupIsPending) return false;

  config->setGroup("Files");
  if(project){
    config->writeEntry("project_file",prj->getProjectFile());
    prj->setCurrentWorkspaceNumber(workspace);
    saveCurrentWorkspaceIntoProject();
    prj->writeProject();
    if(!slotProjectClose()){ // if not ok,pressed cancel
      return false; //not close!
    }
  }
  else {
    config->writeEntry("project_file","");

    int message_result = KMessageBox::Yes;
    int save = m_docViewManager->noDocModified();
    if (!save) {
      message_result = KMessageBox::questionYesNo(this,
                            i18n("There is unsaved data.\nDo you really want to quit?"),
                            i18n("Exit KDevelop"));
    }
    if (message_result==KMessageBox::Yes) {
      // close all documents
      m_docViewManager->doCloseAllDocs();
    }
    else {
      return false;
    }
  }
  return true;
}

bool  CKDevelop::isFileInBuffer(QString abs_filename){
  return (m_docViewManager->findKWriteDoc(abs_filename) != 0);
}

/** additionally adapt the position of MDI system buttons when in maximized mode */
void CKDevelop::resizeEvent( QResizeEvent *pRSE)
{
   QextMdiMainFrm::resizeEvent( pRSE);
   setSysButtonsAtMenuPosition();
}

/** create a tags file for the current CProject */
void CKDevelop::create_tags()
{
  kdDebug() << "creating tags file\n";
  QString files;
  if (m_CTagsCmdLine.onlyProjectFiles()) {
      // collect all files belonging to the project
      QStrListIterator isrc(getProject()->getSources());
      while (isrc)
      {
          files = files + *isrc + " ";
          ++isrc;
      }
      QStrListIterator ihdr(getProject()->getHeaders());
      while (ihdr)
      {
          files = files + *ihdr + " ";
          ++ihdr;
      }
  }
  else {
      files = getProject()->getProjectDir();
      // plus scan external dependencies' files
      // NOTE these are taken from the *.P files in the .deps subdirectory created
      // and updated by the make process
      // (external dependencies let you browse non-project #include symbols)
      // TODO ".deps" directories may not always be in first level subdirectories or may not at all exist
      // TODO let user config this as an option
      // TODO is this portable? (grep -e / max command line length)
      files = files + " `grep -h -e \"^/.* :$\" */.deps/*.P | sort -u | cut -d \" \" -f 1`";
  }
  // set the name of the output file
  QString tagfile = getProject()->getProjectDir() + "/tags";
  // set the ctags command
  shell_process.clearArguments();
  shell_process << m_CTagsCmdLine.command();
  shell_process << m_CTagsCmdLine.append();
  shell_process << m_CTagsCmdLine.sort();
  shell_process << m_CTagsCmdLine.totals();
  shell_process << m_CTagsCmdLine.excmd_pattern();
  shell_process << m_CTagsCmdLine.file_scope();
  shell_process << m_CTagsCmdLine.c_types();
  shell_process << m_CTagsCmdLine.fortran_types();
  shell_process << m_CTagsCmdLine.fields();
  shell_process << m_CTagsCmdLine.extra();
  QStringList list = m_CTagsCmdLine.exclude();
  QStringList::Iterator it = list.begin();
  for ( ; it !=  list.end(); ++it ) {
      shell_process << (*it);
  }
  shell_process << "-f" ;
  shell_process << tagfile ;
  shell_process << m_CTagsCmdLine.recurse();
  shell_process << files ;
  // run it
  shell_process.start(KProcess::Block/*NotifyOnExit*/,KProcess::AllOutput);
}

/** return the instance of the CTags search dialog */
searchTagsDialogImpl* CKDevelop::getCTagsDialog() const
{
  return ctags_dlg;
}
