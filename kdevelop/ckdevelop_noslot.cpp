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

/*---------------------------------------- getInfoFromFilename
 * TEditInfo *CKDevelop::projectIsDirty(const QString &filename)
 *
 * searches for the edit_info-element
 *
 * Returns:
 *       returns either 0l if the filename wasn't found
 *       or the pointer of the first occurence inside
 *       the TEditInfoList
 *-----------------------------------------------------------------*/
TEditInfo *CKDevelop::getInfoFromFilename(const QString &filename)
{
  TEditInfo *pRetVal=0l;
  bool bSearch=true;
  QDir dir=QFileInfo(filename).dir(true);
  QString fullname=dir.canonicalPath()+"/"+QFileInfo(filename).fileName();
  // search the current file which would be changed
  for(pRetVal=edit_infos.first(); bSearch && pRetVal != 0l;)
  {
    if (pRetVal->filename == fullname )
      bSearch=false;
    else
      pRetVal=edit_infos.next();
  }
  return pRetVal;
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

  setInfoModified(header_widget->getName(), header_widget->isModified());
  setInfoModified(cpp_widget->getName(), cpp_widget->isModified());


  for(filename=listAllPrjFiles.first(); isClean && filename != 0; filename=listAllPrjFiles.next())
  {
    // only check valid names and don't check files like *kdevprj or AUTHORS etc.
    if (*filename!='\0' && CProject::getType(filename)!=DATA &&
           CProject::getType(filename)!=KDEV_DIALOG)
    {
      TEditInfo *actual_info=getInfoFromFilename(prjDir+filename);
      QFileInfo src_info(prjDir + filename);

      if (actual_info)
      {
        // here we are... having the file already opened
        if (actual_info->modified || bin_info.lastModified()<actual_info->last_modified)
          isClean=false;
      }
      /* here only the check if the file would be younger than the target file
         i. e. the project binary
         this should be checked always... even if the file is already loaded
         so we can check if the source was modified outside the buffer
      */
      if (isClean && bin_info.lastModified()<src_info.lastModified())
        isClean=false;
    }
  }

  return !isClean;
}

void CKDevelop::removeFileFromEditlist(const char *filename){
  TEditInfo* actual_info;

  QString corrAbsFilename = (isUntitled(filename)) ? QString(filename) : QFileInfo(filename).absFilePath();

//search the actual edit_info and remove it
  for(actual_info=edit_infos.first();actual_info != 0;){
    TEditInfo* next_info=edit_infos.next();
    if (actual_info->filename == corrAbsFilename){ // found
//      KDEBUG(KDEBUG_INFO,CKDEVELOP,"remove edit_info begin\n");
      menu_buffers->removeItem(actual_info->id);
      if(edit_infos.removeRef(actual_info)){
//	KDEBUG(KDEBUG_INFO,CKDEVELOP,"remove edit_info end\n");
      }
    }
    actual_info=next_info;
  }

  // was this file in the cpp_widget?
  if (cpp_widget->getName() == corrAbsFilename)
  {
    for(actual_info=edit_infos.first();actual_info != 0;actual_info=edit_infos.next()){
      // subject of change if another widget will be implemented for CORBA or KOM or YACC ecc.
      if ( CProject::getType( actual_info->filename ) == CPP_SOURCE)
      { // found
        cpp_widget->setText(actual_info->text);
        cpp_widget->toggleModified(actual_info->modified);
        cpp_widget->setName(actual_info->filename);
  //    KDEBUG1(KDEBUG_INFO,CKDEVELOP,"FOUND A NEXT %s",actual_info->filename.data());
        return;
      }
    }

    // if not found a successor create an new file
    actual_info = new TEditInfo;
    actual_info->modified=false;
    QString sCFilename= (project && prj->getProjectType()=="normal_c") ? i18n("Untitled.c") : i18n("Untitled.cpp");
    actual_info->id = menu_buffers->insertItem(sCFilename,-2,0);
    actual_info->filename = sCFilename;

    edit_infos.append(actual_info);

    cpp_widget->clear();
    cpp_widget->setName(actual_info->filename);
  }

  // was this file in the header_widget?
  if (header_widget->getName() == corrAbsFilename)
  {
    for(actual_info=edit_infos.first();actual_info != 0;actual_info=edit_infos.next()){
      // subject of change if another widget will be implemented for CORBA or KOM or YACC ecc.
      if ( CProject::getType( actual_info->filename ) != CPP_SOURCE)
      { // found
        header_widget->setText(actual_info->text);
        header_widget->toggleModified(actual_info->modified);
        header_widget->setName(actual_info->filename);
  //    KDEBUG1(KDEBUG_INFO,CKDEVELOP,"FOUND A NEXT %s",actual_info->filename.data());
        return;
      }
    }

    // if not found a successor create an new file
    actual_info = new TEditInfo;
    actual_info->modified=false;
    actual_info->id = menu_buffers->insertItem(i18n("Untitled.h"),-2,0);
    actual_info->filename = i18n("Untitled.h");

    edit_infos.append(actual_info);

    header_widget->clear();
    header_widget->setName(actual_info->filename);
  }

}

/*---------------------------------------- setInfoModified()
 * setInfoModified(const QString &sFilename, bool bModified)
 *
 *  search all edit_infos for the file named "sFilename", the first
 *  match will change 'modified'
 *
 * Parameters:
 *  sFilename   filename to search in the EditInfos
 *  bModified   sets editinfo->modified
 * Returns:
 *       returns true if a struct-element was changed
 *-----------------------------------------------------------------*/
bool CKDevelop::setInfoModified(const QString &sFilename, bool bModified)
{
  bool bChanged=false;
  TEditInfo* actual_info;

  for(actual_info=edit_infos.first();!bChanged && actual_info != 0;actual_info=edit_infos.next())
  {
   if ( actual_info->filename == sFilename)
      { // found
        actual_info->modified=bModified;
        bChanged=true;
      }
  }

  return bChanged;
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
void CKDevelop::setMainCaption(int tab_item)
{
  QString capt;
  switch(tab_item)
  {
    case BROWSER:
      setCaption(browser_widget->currentTitle());
      break;

    default:
      capt=(project) ? (prj->getProjectName()+" - "+QFileInfo(edit_widget->getName()).fileName()) :
              QFileInfo(edit_widget->getName()).fileName();
      if (edit_widget->isModified())
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
  TEditInfo* actual_info=0;
  TEditInfo* old_info=0;
  TEditInfo* search_info;
  int message_result=KMessageBox::Yes; // simulate ok state... this could change by one of the following messageboxes

  oldName=edit_widget->getName();
  if (bAutosave)
    saveTimer->stop();

  do
  {
    if (!isUntitled(oldName))
      name = KFileDialog::getSaveFileName(oldName,QString::null, 0,oldName);
    else
      name = KFileDialog::getSaveFileName(((project) ?  QString(prj->getProjectDir()+oldName) : oldName),
                                          QString::null,0,oldName);

    if (name.isNull()){
    // KDEBUG(KDEBUG_INFO,CKDEVELOP,"Cancel");
      if (bAutosave)
       saveTimer->start(saveTimeout);
     return false;
    }

    // check if the extension is changed and the widget or program to view must change
    if (CProject::getType(name)!=CProject::getType(edit_widget->getName()))
      message_result = KMessageBox::warningYesNoCancel(this,
                            i18n("Do you really want to save the file\nas another type of document?"),
                            i18n("Save as new type of document?"));

    if(message_result==KMessageBox::Yes && QFile::exists(name))
    {
      message_result=KMessageBox::warningYesNoCancel(this,
                        i18n("\nThe file\n\n%1\n\n"
                              "already exists.\nDo you want overwrite the old one?\n").arg(name),
                        i18n("File exists!"));
    }
    
  } while (message_result == KMessageBox::No); // repeat it on 'no'


  if (message_result==KMessageBox::Cancel){
     //KDEBUG(KDEBUG_INFO,CKDEVELOP,"Cancel on new type question");
      if (bAutosave)
       saveTimer->start(saveTimeout);
     return false;
  }


   // search if we can find the new desired filename in edit_infos ...
   // means already loaded
  for(search_info=edit_infos.first(); search_info!=0 && (actual_info == 0 || old_info == 0);
      search_info=edit_infos.next()){
     if (search_info->filename == name)
       actual_info=search_info;
     if (search_info->filename == oldName)
       old_info=search_info;
  }

  // now that all cancel possibilities are handled simulate a changed file
  // edit_widget->toggleModified(true);

  if (!edit_widget->KWrite::writeFile(name)){
  // if saving failed
      if (bAutosave)
       saveTimer->start(saveTimeout);
     return false;
  }

  if (actual_info != 0l && actual_info==old_info)
  {
    // here we are ... saving the file with the same name
    //   so only the modified-flags have to be changed
    actual_info->modified = false;
    edit_widget->toggleModified(false);
  }
  else
  {
    // now open this file as new file in edit_infos
    //    if an widget still contains the file then update the contents in the widget from file
    switchToFile(name,-1,-1, true);

    if (oldName!=name)
    {
      // here we are... and any Untitled-file was saved with another name
      //   and now we can remove the untitled file
      if (isUntitled(oldName))
      {
          removeFileFromEditlist(oldName);
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

  if (szOldText)
  {
    while ((ch=*szOldText++)!='\0')
     {
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

  lastfile = edit_widget->getName();
  lasttab = s_tab_view->getCurrentTab();

  QFileInfo fileInfo(filename);
  // check if the file exists
  if(!fileInfo.exists() && !isUntitled(filename)){
    KMessageBox::error(this, i18n("%1\n\nFile does not exist!").arg(filename));
    return;
  }


  QString ext = fileInfo.extension(false);

  // Load QtDesigner if clicked/loaded an User Interface file (.ui)
//  if ( ext == "ui") {
//    if(!CToolClass::searchProgram("designer")){
//      return;
//    }

//		KProcess designer_process;	
//		designer_process << "designer" << "-client" << filename;
//		KShellProcess designer_process("/bin/sh");
//		const QString oldGroup = config->group();
//		config->setGroup("QT2");
//		QString qt2dir = QString ("QTDIR=")+ config->readEntry("qt2dir",getenv("QTDIR")) +" ";
//		config->setGroup(oldGroup);
//		designer_process << qt2dir << "designer" << "-client" << filename;
//		if(!designer_process.start(KProcess::DontCare)) {
//    	debug("QtDesigner didn't start!");
//		}
//    return;
//  }
//
//  // Load Qt linguist if the filename is a ts file
//  if( ext == "ts") {
//    if(!CToolClass::searchProgram("linguist")){
//      return;
//    }
//
//		KProcess linguist_process;
//		linguist_process << "linguist" << filename;
//		if(!linguist_process.start(KProcess::DontCare)) {
//    	debug("Qt Linguist didn't start!");
//		}
//    return;
//  }
//
//  // Load Qt linguist if the filename is a ts file
//  if( ext == "po") {
//    if(CToolClass::searchInstProgram("kbabel")){
//   		KProcess linguist_process;
//   		linguist_process << "kbabel" << filename;
//   		if(!linguist_process.start(KProcess::DontCare)) {
//       	debug("KBabel didn't start!");
//   		}
//      return;
//   	}
//  }

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

//FB----------anfang-vom-abschnitt-altes-behandeln----------

  // set the correct edit_widget
  if (CProject::getType(filename) == CPP_SOURCE){
    edit_widget = cpp_widget;

    if(build_menu->isItemEnabled(ID_BUILD_MAKE))			
      enableCommand(ID_BUILD_COMPILE_FILE);
  }
  else
  {
    edit_widget = header_widget;
    disableCommand(ID_BUILD_COMPILE_FILE);
  }

  TEditInfo* info;
  TEditInfo* actual_info;

  // We need to look in the list of "open" files for the file that
  // is currently held in the edit_widget. This file needs to
  // be taken out of the editor_widget and stored, so that we can
  // reuse the edit_widget for the new file.
  for (actual_info=edit_infos.first(); actual_info != 0; actual_info=edit_infos.next())
  {
    if (actual_info->filename == edit_widget->getName())
      break;
  }

  // Make sure that we found the file in the editor_widget in our list
  // If we haven't then this should be fatal.
  if(actual_info == 0)
    return;

  // handle file if it was modified on disk by another editor/cvs
  QFileInfo file_info(edit_widget->getName());
  if((file_info.lastModified() != actual_info->last_modified )&& bShowModifiedBox)
  {
    if(KMessageBox::questionYesNo(this, i18n("The file %1 was modified outside this editor.\n"
                                              "Open the file from disk and delete the current Buffer?")
                                              .arg(edit_widget->getName()),
                                        i18n("File modified"))==KMessageBox::Yes){
      bForceReload = true;
      actual_info->last_modified = file_info.lastModified();
    }
  }

  if (!bShowModifiedBox){
     actual_info->last_modified = file_info.lastModified(); 
  }

  if (!bForceReload && filename == edit_widget->getName()){
    if (line != -1)
      edit_widget->setCursorPosition(line, col);

      //    cerr << endl <<endl << "Filename:" << filename
      // << "EDITNAME:" << edit_widget->getName() <<"no action---:" << endl;
      s_tab_view->setCurrentTab((edit_widget==header_widget) ? HEADER : CPP);
      edit_widget->setFocus();
      return;
  }

  // store the old file
  actual_info->text = edit_widget->text();
  actual_info->modified = edit_widget->isModified();
  actual_info->cursor_line = edit_widget->currentLine();
  actual_info->cursor_col = edit_widget->currentColumn();
  // output_widget->append("auszuwechseldes file:" + actual_info->filename);

  bool found = false;

//FB---------ende-der-behandlung-der-alten-datei--------

  // See if we already have the file wanted in our list of saved files.
  for(info=edit_infos.first();info != 0;info=edit_infos.next())
  {
     // if found in list
    if (info->filename == filename )
    {
      found = true;
      break;
    }
  }

  // get the document type
  int docType = DocViewMan::Header;
  if (CProject::getType(filename) == CPP_SOURCE)
    docType = DocViewMan::Source;

  // Not found or needing a reload causes the file to be read from disk and the
  // info reset.
  if (!found || bForceReload)
  {
    QFileInfo fileinfo(filename);
    if (!found)
    {
      // not found -> generate a new edit_info,loading
      info = new TEditInfo;
      info->id = menu_buffers->insertItem(fileinfo.fileName(),-2,0); // insert at first index
      edit_infos.append(info); // add to the list
    }

    info->filename = filename;
    info->modified = false;
    info->cursor_line = 0;
    info->cursor_col = 0;
    info->last_modified = fileinfo.lastModified();

//FB    edit_widget->clear();
    int docId = m_docViewManager->findDoc( filename);
    if (docId == -1) {
      docId = m_docViewManager->createDoc( docType, filename);
      if (docId != -1)
        edit_widget = (CEditWidget*) m_docViewManager->createView( docId);
    }
    else {
      // view doesn't already exist, create it
      edit_widget = (CEditWidget*) m_docViewManager->viewsOfDoc( docId).getFirst();
      // update the widget
    }
    m_docViewManager->loadDoc( docId, filename,1);
//FB    edit_widget->loadFile(filename,1);
  }
  else
  {
    // The file is in the list so use the saved text
    edit_widget->setText(info->text);
  }

  // update the pointers
  if (docType == DocViewMan::Source)
    cpp_widget = edit_widget;
  else
    header_widget = edit_widget;

  edit_widget->toggleModified(info->modified);

  // If the caller wanted to be positioned at a particular place in the file
  // then they have supplied the line and col. Otherwise we use the
  // current info values (0 if new) for the placement.
  if (line == -1)
    edit_widget->setCursorPosition(info->cursor_line,info->cursor_col);
  else
    edit_widget->setCursorPosition(line, col);

  edit_widget->setName(filename);
  info->text = edit_widget->text();
  s_tab_view->setCurrentTab((edit_widget==header_widget) ? HEADER : CPP);
  edit_widget->setFocus();

  // Need to get the breakpoints displayed in this file (if any)
	if (brkptManager)
	  brkptManager->refreshBP(filename);

//      if (bForceReload)
//      {
//        QFileInfo fileinfo(filename);
//        edit_widget->clear();
//        edit_widget->loadFile(filename,1);
//        info->modified=false;
//        info->cursor_line=info->cursor_col=0;
//        info->text = edit_widget->text();
//      }
//      else
//      {
////         edit_widget->setName(filename);     // inserted to stop flickering of caption
//         edit_widget->setText(info->text);
//      }
//
//      if (line == -1)
//        edit_widget->setCursorPosition(info->cursor_line,info->cursor_col);
//      else
//        edit_widget->setCursorPosition(line, col);
//
//      edit_widget->toggleModified(info->modified);
//      edit_widget->setName(filename);
//
//      s_tab_view->setCurrentTab((edit_widget==header_widget) ? HEADER : CPP);
//      edit_widget->setFocus();
//
//      // Need to get the breakpoints displayed in this file (if any)
//			if (brkptManager)
//	      brkptManager->refreshBP(filename);
//      return;
//    }
}

//void CKDevelop::switchToFile(QString filename, int lineNo){
//  lasttab = s_tab_view->getCurrentTab();
//  switchToFile( filename, false, lineNo, 0);
//  edit_widget->setCursorPosition( lineNo, 0 );
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

    if(s_tab_view->getCurrentTab() == CPP){
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
      if(treedock->isVisible())
      {
        view_menu->setItemChecked(ID_VIEW_TREEVIEW, true);
        return;
      }
      else
      {
        slotViewTTreeView();
      }
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
	
  QString filename = config->readEntry("browser_file","");
  if (filename.isEmpty())
		filename = DocTreeKDevelopBook::locatehtml("about/intro.html");

  if(!filename.isEmpty())
    slotURLSelected(filename,1,"test");
}

void CKDevelop::saveOptions(){
	
  saveMainWindowSettings (config);
  config->setGroup("General Options");

  config->writeEntry("show_std_toolbar",view_menu->isItemChecked(ID_VIEW_TOOLBAR));
  config->writeEntry("show_browser_toolbar",view_menu->isItemChecked(ID_VIEW_BROWSER_TOOLBAR));

  config->writeEntry("show_statusbar",view_menu->isItemChecked(ID_VIEW_STATUSBAR));
  config->writeEntry("LastActiveTab", s_tab_view->getCurrentTab());
  config->writeEntry("LastActiveTree", t_tab_view->getCurrentTab());

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
  saveOptions();
  return true;
}

bool CKDevelop::queryClose(){
  config->setGroup("Files");
  if(project){
    config->writeEntry("project_file",prj->getProjectFile());
    config->writeEntry("cpp_file",cpp_widget->getName());
    config->writeEntry("header_file",header_widget->getName());
    prj->setCurrentWorkspaceNumber(workspace);
    saveCurrentWorkspaceIntoProject();
    prj->writeProject();
    if(!slotProjectClose()){ // if not ok,pressed cancel
      return false; //not close!
    }
  }
  else{
    TEditInfo* actual_info;
    int message_result=KMessageBox::Yes;
    int save=true;

    config->writeEntry("project_file","");

    setInfoModified(header_widget->getName(), header_widget->isModified());
    setInfoModified(cpp_widget->getName(), cpp_widget->isModified());

    for(actual_info=edit_infos.first();save && actual_info != 0;
		actual_info=edit_infos.next())
    {
       if (actual_info->modified)
         save=false;
    }

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
    kapp->sessionConfig()->writeEntry("cpp_file",cpp_widget->getName());
    kapp->sessionConfig()->writeEntry("header_file",header_widget->getName());
    prj->setCurrentWorkspaceNumber(workspace);
    saveCurrentWorkspaceIntoProject();
    prj->writeProject();
  }	
  if(bAutosave)
    slotFileSaveAll();
  else{
    TEditInfo* info;
    for(info=edit_infos.first();info != 0;info=edit_infos.next()){
      if(info->modified){
//#warning FIXME missing method	setUnsavedData ( true );
	break;
      }
    }
  }
  saveOptions();
}

bool  CKDevelop::isFileInBuffer(QString abs_filename){
  TEditInfo* info;
  for(info=edit_infos.first();info != 0;info=edit_infos.next()){
    if (info->filename == abs_filename ){
      return true;
    }
  }
  return false;
}

/** additionally adapt the position of MDI system buttons when in maximized mode */
void CKDevelop::resizeEvent( QResizeEvent *pRSE)
{
   QextMdiMainFrm::resizeEvent( pRSE);
   setSysButtonsAtMenuPosition();
}
