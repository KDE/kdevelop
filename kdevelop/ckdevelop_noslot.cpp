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
#include "ceditwidget.h"
#include "clogfileview.h"
#include "crealfileview.h"
#include "ctabctl.h"
#include "ctoolclass.h"
#include "debug.h"
#include "doctreeview.h"
#include "./dbg/brkptmanager.h"
#include "ckonsolewidget.h"
#include "docviewman.h"
#include "kwdoc.h"
#include "docviewman.h"

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

void CKDevelop::setupRecentProjectMenu()
{
	QStringList list=config->readListEntry("Recent Projects");
  for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
    addRecentProject((*it));
}

void CKDevelop::saveRecentProjectMenu()
{
	QStringList list;
  for (uint index=0; index < recent_projects_menu->count(); index++)
  {
    int id = recent_projects_menu->idAt(index);
    QString menuFile = recent_projects_menu->text(id);
    list.append(menuFile);
  }
	config->writeEntry("Recent Projects", list);
}

void CKDevelop::shuffleProjectToTop(int id)
{
  QString file = recent_projects_menu->text(id);
  if (!file.isEmpty())
  {
    recent_projects_menu->removeItem(id);
    recent_projects_menu->insertItem(file, (int) -1, (int)0);
  }
}

QString CKDevelop::getProjectAsString(int id)
{
  return recent_projects_menu->text(id);
}

void CKDevelop::addRecentProject(const QString& file)
{
  for (uint index=0; index < recent_projects_menu->count(); index++)
  {
    int id = recent_projects_menu->idAt(index);
    QString menuFile = recent_projects_menu->text(id);
    if (menuFile == file)
    {
//      shuffleProjectToTop(id);
      return;
    }
  }

  if (recent_projects_menu->count() > 7)
    recent_projects_menu->removeItemAt(7);

  // Add it at the top of the menu
  recent_projects_menu->insertItem(file, (int)-1, (int)0);
}

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
      KWriteDoc* pDoc = m_docViewManager->findKWriteDoc(prjDir+filename);
      QFileInfo src_info(prjDir + filename);

      if (pDoc)
      {
        // here we are... having the file already opened
        if (pDoc->isModified() 
	    || bin_info.lastModified() < pDoc->getLastFileModifDate())
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
  QString capt;
  CEditWidget* pCEW = m_docViewManager->currentEditView();
  switch(item)
  {
  case BROWSER:
    {
      CDocBrowser* pCurBrowserDoc = m_docViewManager->currentBrowserDoc();
      KHTMLView* pCurBrowserView = m_docViewManager->currentBrowserView();
      if (pCurBrowserDoc) {
        setCaption(pCurBrowserDoc->currentTitle());
        QextMdiChildView* pMDICover = (QextMdiChildView*) pCurBrowserView->parentWidget();
        pMDICover->setCaption(pCurBrowserDoc->currentURL());
        pMDICover->setTabCaption(pCurBrowserDoc->currentTitle());
      }
    }
    break;

  default:
    if (pCEW) {
      capt = QFileInfo(pCEW->getName()).fileName();
    }
    else {
      capt = "";
    }
    if (project) {
      capt = prj->getProjectName() + " - " + capt;
    }
    if (pCEW && pCEW->isModified())
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
  return (s==i18n("Untitled.h") || s==i18n("Untitled.c") || s==i18n("Untitled.cpp"));
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

  if (!m_docViewManager->currentEditView())
    return false;

  oldName = m_docViewManager->currentEditView()->getName();
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
    if (CProject::getType(name)!=CProject::getType(m_docViewManager->currentEditView()->getName()))
      message_result = KMessageBox::warningYesNoCancel(this,
                            i18n("Do you really want to save the file\nas another type of document?"),
                            i18n("Save as new type of document?"));

    if(message_result == KMessageBox::Yes && QFile::exists(name)) {
      message_result=KMessageBox::warningYesNoCancel(this,
                        i18n("\nThe file\n\n%1\n\n"
                              "already exists.\nDo you want overwrite the old one?\n").arg(name),
                        i18n("File exists!"));
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
  KWriteDoc* pActualDoc = m_docViewManager->findKWriteDoc(name);
  KWriteDoc* pOldDoc = m_docViewManager->findKWriteDoc(oldName);
    
  // now that all cancel possibilities are handled simulate a changed file
  // m_docViewManager->currentEditView()->toggleModified(true);

  if (!m_docViewManager->currentEditView()->KWrite::writeFile(name)) {
  // if saving failed
      if (bAutosave)
       saveTimer->start(saveTimeout);
     return false;
  }

  if (pActualDoc != 0 && pActualDoc == pOldDoc) {
    // here we are ... saving the file with the same name
    //   so only the modified-flags have to be changed
    pActualDoc->setModified(false);
    m_docViewManager->currentEditView()->toggleModified(false);
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
  	QStrList lHeaderList(FALSE);	//no deep copies
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
void CKDevelop::refreshTrees(QStrList * iFileList){
//	time_t lStart = time(NULL);
//	clock_t lStartClock = clock();
  kapp->processEvents(100);
  doc_tree->refresh(prj);
  if (!project){
    return; // no project
  }

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
//		time_t lStart = time(NULL);
//		clock_t lStartClock = clock();
    kapp->processEvents(100);
		class_tree->refresh(prj);
//		kdDebug() << "refresh classview took " << (time(NULL) - lStart) << "ms to complete" << endl;
// 	 	kdDebug() << "refresh classview took " << (clock() - lStartClock) << "clocktick to complete" << endl;

	}

  statProg->reset();
//  statProg->hide();

  // Update the classcombo.
  kapp->processEvents(100);
  CVRefreshClassCombo();

  // Update LFV.
  kapp->processEvents(100);
  log_file_tree->storeState(prj);
  kapp->processEvents(100);
  log_file_tree->refresh(prj);

  // Update RFV.
  kapp->processEvents(100);
  real_file_tree->refresh(prj);

  kapp->processEvents(100);

  statusBar()->repaint();
  setCursor(KCursor::arrowCursor());	
  // update the file_open_menu
  file_open_list=prj->getHeaders();
  QStrList sources=prj->getSources();
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
//  kdDebug() << "refreshTree took " << (time(NULL) - lStart) << "ms to complete" << endl;
//  kdDebug() << "refrehTree took " << (clock() - lStartClock) << "clocktick to complete" << endl;
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

  if( project )
  {
    kapp->processEvents(100);
    // If this is a sourcefile we parse it and update the classview.
    if( info->type == CPP_SOURCE || info->type == CPP_HEADER )
		{
			class_tree->addFile( prj->getProjectDir() + info->rel_name );
			CVRefreshClassCombo();
		}

    // Update LFV.
    kapp->processEvents(100);
    log_file_tree->storeState(prj);
    log_file_tree->refresh(prj);

    // Update RFV.
    kapp->processEvents(100);
    real_file_tree->refresh(prj);
    // update dialogs tree
    kapp->processEvents(100);
  }
  // refresh the file_open_list
  file_open_list=prj->getHeaders();
  QStrList sources=prj->getSources();
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

  debug("switching to %s !\n", filename.data());

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

//FB  lastfile = pCurEditWidget->getName();
//FB  lasttab = s_tab_view->getCurrentTab();

  debug("getting file info !\n");

  QFileInfo fileInfo(filename);
  // check if the file exists
  if(!fileInfo.exists() && !isUntitled(filename)){
    KMessageBox::error(this, i18n("%1\n\nFile does not exist!").arg(filename));
    return;
  }

  QString ext = fileInfo.extension(false);

  debug("url stuffing !\n");

  KURL url;
  url.setFileName(filename);
  QString type =  KMimeType::findByURL(url, 0, true)->name();
  if (!(type.startsWith("text/")
    || type.startsWith("application/x-perl") || type.startsWith("application/x-python")
    || type.startsWith("application/x-shellscript") || type.startsWith("application/x-desktop") ) )  // open with krun
  {
    new KRun( url );
    return;
  }

  // Enable or disable command
  if (CProject::getType(filename) == CPP_SOURCE){
    if(build_menu->isItemEnabled(ID_BUILD_MAKE))			
      enableCommand(ID_BUILD_COMPILE_FILE);
  }
  else
  {
    disableCommand(ID_BUILD_COMPILE_FILE);
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
    disableCommand(ID_BUILD_STOP);
  else
    enableCommand(ID_BUILD_STOP);

  if (enable && project){

    if (m_docViewManager->curDocIsCppFile()){
      enableCommand(ID_BUILD_COMPILE_FILE);
    }
    enableCommand(ID_BUILD_RUN);
    enableCommand(ID_BUILD_RUN_WITH_ARGS);
    enableCommand(ID_DEBUG_START);
    enableCommand(ID_DEBUG_START_OTHER);
    enableCommand(ID_BUILD_MAKE);
    enableCommand(ID_BUILD_REBUILD_ALL);
    enableCommand(ID_BUILD_CLEAN_REBUILD_ALL);
    enableCommand(ID_BUILD_DISTCLEAN);
    enableCommand(ID_BUILD_MAKECLEAN);
    enableCommand(ID_BUILD_AUTOCONF);
    enableCommand(ID_BUILD_CONFIGURE);
    QString type=prj->getProjectType();
    if (!(prj->isKDEProject()||prj->isKDE2Project()||prj->isQt2Project()))
    {
      disableCommand(ID_PROJECT_MESSAGES);
    }
    else{
      enableCommand(ID_PROJECT_MESSAGES);
    }

    enableCommand(ID_PROJECT_CLOSE);
    enableCommand(ID_PROJECT_NEW_CLASS);
    enableCommand(ID_PROJECT_ADD_FILE_EXIST);
    enableCommand(ID_PROJECT_ADD_NEW_TRANSLATION_FILE);
    enableCommand(ID_PROJECT_FILE_PROPERTIES);
    enableCommand(ID_PROJECT_OPTIONS);

    enableCommand(ID_FILE_NEW);

  //MB
    enableCommand(ID_PROJECT_DOC_TOOL);
  //MB end
    enableCommand(ID_PROJECT_MAKE_PROJECT_API);
    enableCommand(ID_PROJECT_MAKE_USER_MANUAL);
    enableCommand(ID_PROJECT_MAKE_DISTRIBUTION);
    enableCommand(ID_PROJECT_MAKE_TAGS);
    enableCommand(ID_PROJECT_LOAD_TAGS);
  }
  else
//  if (!enable)
  {
    // set the popupmenus enable or disable
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

    disableCommand(ID_FILE_NEW);
  }

  if(bAutosave)
  {
    if(enable)
      saveTimer->start(saveTimeout); // restart autosaving if enabled after a process finished
    else
      saveTimer->stop();  // stop the autosaving if make or something is running
  }
}

void CKDevelop::switchToWorkspace(int id){
  workspace = id;
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
  TWorkspace ws = prj->getWorkspace(id);
  if(ws.show_output_view){
    showOutputView(true);
  }
  else{showOutputView(false);}

  if(ws.show_treeview){
    showTreeView(true);
  }
  else{showTreeView(false);}
}

void CKDevelop::showTreeView(bool show){
  if(bAutoswitch)
  {
    if(show)
    {
      // This is a hack to get around some startup problems
//temp-disabled      if(treedock->isVisible())
//temp-disabled      {
//temp-disabled        view_menu->setItemChecked(ID_VIEW_TREEVIEW, true);
//temp-disabled        return;
//temp-disabled      }
//temp-disabled      else
//temp-disabled      {
//temp-disabled        slotViewTTreeView();
//temp-disabled      }
    }
    else
    {
      if(!view_menu->isItemChecked(ID_VIEW_TREEVIEW))
      {
        return; // it's already unvisible){
      }
      else{
				slotViewTTreeView();
      }
    }
  }
}
void CKDevelop::showOutputView(bool show){
  if(bAutoswitch){
    if(show){
      if(view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW)){
        return; // it's already visible
      }
      else{
        slotViewTOutputView();
      }
    }
    else{
      if(!view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW)){
        return; //it's already unvisible
      }
      else{
        slotViewTOutputView();
      }
    }
  }
}
void CKDevelop::readOptions()
{
  //default geometry on first startup, saved geometry is set by applyMainWindowSettings afterwards
//  setGeometry(QApplication::desktop()->width()/2-400, QApplication::desktop()->height()/2-300, 800, 600);
//  showMaximized();
  applyMainWindowSettings(config);

  config->setGroup("General Options");

  if(config->readBoolEntry("show_std_toolbar", true))
    view_menu->setItemChecked(ID_VIEW_TOOLBAR, true);
  if(config->readBoolEntry("show_browser_toolbar",true))
    view_menu->setItemChecked(ID_VIEW_BROWSER_TOOLBAR, true);
	if (config->readBoolEntry("show_statusbar",true))
    view_menu->setItemChecked(ID_VIEW_STATUSBAR, true);
	if (config->readBoolEntry("show_mdi_view_taskbar",true))
    view_menu->setItemChecked(ID_VIEW_MDIVIEWTASKBAR, true);

	/////////////////////////////////////////
	// Outputwindow, TreeView, KDevelop
  /////////////////////	

	config->setGroup("dock_setting_default");
	// settings just turned around...true is false and false is true ;-)
	bool outputview= config->readBoolEntry("Output-View:visible", true);
	if(outputview){
	  view_menu->setItemChecked(ID_VIEW_OUTPUTVIEW, true);
		toolBar()->setButton(ID_VIEW_OUTPUTVIEW, true);
	}
	bool treeview=config->readBoolEntry("Tree-View:visible", true);
	if(treeview){
	  view_menu->setItemChecked(ID_VIEW_TREEVIEW, true);
		toolBar()->setButton(ID_VIEW_TREEVIEW, true);
	}
/////////////////////

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
  setupRecentProjectMenu();

  //MB
	doctool = config->readNumEntry("doc_tool_type");
  // must be done here - cause the call comes AFTER the initialization of Project menue :(
  if (doctool == DT_KDOC || doctool == 0)
  {
    doctool_menu->setItemChecked(ID_PROJECT_DOC_TOOL_KDOC,true);
    doctool_menu->setItemChecked(ID_PROJECT_DOC_TOOL_DOXYGEN,false);
	  doctool_menu->setItemEnabled(ID_PROJECT_DOC_TOOL_CONF_DOXYGEN,false);
  }
  if (doctool == DT_DOX)
  {
    doctool_menu->setItemChecked(ID_PROJECT_DOC_TOOL_KDOC,false);
    doctool_menu->setItemChecked(ID_PROJECT_DOC_TOOL_DOXYGEN,true);
	  doctool_menu->setItemEnabled(ID_PROJECT_DOC_TOOL_CONF_DOXYGEN,true);
  }
	//MB end
	
	doc_bookmarks_list.setAutoDelete(TRUE);
	doc_bookmarks_title_list.setAutoDelete(TRUE);
	
	config->readListEntry("doc_bookmarks",doc_bookmarks_list);
	config->readListEntry("doc_bookmarks_title",doc_bookmarks_title_list);
	for ( uint i =0 ; i < doc_bookmarks_title_list.count(); i++){
    doc_bookmarks->insertItem(SmallIconSet("html"),doc_bookmarks_title_list.at(i));
  }

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

  config->writeEntry("show_std_toolbar",view_menu->isItemChecked(ID_VIEW_TOOLBAR));
  config->writeEntry("show_browser_toolbar",view_menu->isItemChecked(ID_VIEW_BROWSER_TOOLBAR));

  config->writeEntry("show_statusbar",view_menu->isItemChecked(ID_VIEW_STATUSBAR));
  config->writeEntry("show_mdi_view_taskbar",view_menu->isItemChecked(ID_VIEW_MDIVIEWTASKBAR));
//!!!!!!!!!  config->writeEntry("LastActiveTree", t_tab_view->getCurrentTab());

  // set the mode of the tab headers
  if(view_tab_menu->isItemChecked(ID_VIEW_TAB_TEXT))
    config->writeEntry("tabviewmode", 1);
  else if(view_tab_menu->isItemChecked(ID_VIEW_TAB_ICONS))
    config->writeEntry("tabviewmode", 2);
  else
    config->writeEntry("tabviewmode", 3);

  config->writeEntry("lfv_show_path",log_file_tree->showPath());

  config->writeEntry("Autosave",bAutosave);
  config->writeEntry("Autosave Timeout",saveTimeout);

  config->writeEntry("Make",make_cmd);

  config->setGroup("Files");
  config->writeEntry("browser_file",history_list.current());
  config->writeEntry("doc_bookmarks", doc_bookmarks_list);
  config->writeEntry("doc_bookmarks_title", doc_bookmarks_title_list);
  saveRecentProjectMenu();

  //MB serializes menuoptions
  config->writeEntry("doc_tool_type",doctool);
  //MB end
  writeDockConfig();
  config->sync();
}

bool CKDevelop::queryExit(){
  kdDebug() << "queryExit()..." << endl;
  if (!bStartupIsPending) {
    saveOptions();
  }
  return true;
}

bool CKDevelop::queryClose(){
  kdDebug() << "queryClose()..." << endl;

  if (bStartupIsPending) return true;

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
  else{
    int message_result = KMessageBox::Yes;

    config->writeEntry("project_file","");

    // m_docViewManager->synchronizeDocAndInfo();

    int save = m_docViewManager->noDocModified();

    if (!save)
    {
      message_result = KMessageBox::questionYesNo(this,
                            i18n("There is unsaved data.\nDo you really want to quit?"),
                            i18n("Exit KDevelop"));
    }
    return (message_result==KMessageBox::Yes);
  }
  return true;
}

void CKDevelop::readProperties(KConfig* sess_config){
  readOptions();
  QString filename;
//  filename = sess_config->readEntry("project_file","");
  filename = kapp->sessionConfig()->readEntry("project_file","");

  QFile file(filename);
  if (file.exists())
  {
    if(!(readProjectFile(filename)))
    {
      KMessageBox::error(0,
                        i18n("Unable to read the project file. Perhaps its\n"
                              "an old unsupported project file, or you do not\n"
                              "have read/write permissions for this project"),
                              filename);
      return;
    }
    else
    {  // projectfile successfully read
      filename = sess_config->readEntry("header_file",i18n("Untitled.h"));
      QFile _file(filename);

      if (QFile::exists(filename)){
        switchToFile(filename);

      }

      filename = sess_config->readEntry("cpp_file", i18n("Untitled.cpp"));
      if (QFile::exists(filename)){
        switchToFile(filename);
      }
    }
  }
  refreshTrees();  // always used
}

void CKDevelop::saveProperties(KConfig* sess_config){
	
  if(project){
    kapp->sessionConfig()->writeEntry("project_file",prj->getProjectFile());
//FB    kapp->sessionConfig()->writeEntry("cpp_file",cpp_widget->getName());
//FB    kapp->sessionConfig()->writeEntry("header_file",header_widget->getName());
    prj->setCurrentWorkspaceNumber(workspace);
    saveCurrentWorkspaceIntoProject();
    prj->writeProject();
  }	
  if(bAutosave)
    slotFileSaveAll();
  else{
    // TEditInfo* info;
    // for(info=edit_infos.first();info != 0;info=edit_infos.next()){
    //   if(info->modified){
    //     #warning FIXME missing method	setUnsavedData ( true );
    //     break;
    //   }
    // }
  }
  saveOptions();
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
  // collect all files belonging to the project
  QString files;
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
  shell_process << m_CTagsCmdLine.file_tags();
  shell_process << m_CTagsCmdLine.c_types();
  shell_process << m_CTagsCmdLine.fortran_types();
  shell_process << m_CTagsCmdLine.exclude();
  shell_process << m_CTagsCmdLine.fields();
  shell_process << "-f" ;
  shell_process << tagfile ;
  shell_process << files ;
  // run it
  shell_process.start(KProcess::NotifyOnExit,KProcess::AllOutput);
}
