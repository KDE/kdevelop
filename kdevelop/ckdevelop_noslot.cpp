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
#include "./kdlgedit/kdlgedit.h"
#include <iostream.h>

#include <qprogressdialog.h>

#include <kmsgbox.h>
#include <kcursor.h>

#include "debug.h"
#include "ckdevelop.h"
#include "cclassview.h"
#include "kswallow.h"
#include "ctoolclass.h"
#include "cdocbrowser.h"
#include "doctreeview.h"
#include "clogfileview.h"
#include "crealfileview.h"
#include "./kdlgedit/kdlgdialogs.h"
#include "./kdlgedit/kdlgreadmedlg.h"

void CKDevelop::addRecentProject(const char* file)
{
  if(recent_projects.find(file) == -1){
    if( recent_projects.count() < 5)
      recent_projects.insert(0,file);
    else{
      recent_projects.remove(4);
      recent_projects.insert(0,file);
    }
    recent_projects_menu->clear();
		uint i;
    for ( i =0 ; i < recent_projects.count(); i++){
      recent_projects_menu->insertItem(recent_projects.at(i));
    }
	}
}

void CKDevelop::removeFileFromEditlist(const char *filename){
  TEditInfo* actual_info;

//search the actual edit_info and remove it
  for(actual_info=edit_infos.first();actual_info != 0;){
    TEditInfo* next_info=edit_infos.next();
    if (actual_info->filename == filename){ // found
//      KDEBUG(KDEBUG_INFO,CKDEVELOP,"remove edit_info begin\n");
      menu_buffers->removeItem(actual_info->id);
      if(edit_infos.removeRef(actual_info)){
//	KDEBUG(KDEBUG_INFO,CKDEVELOP,"remove edit_info end\n");
      }
    }
    actual_info=next_info;
  }

  // was this file in the cpp_widget?
  if (cpp_widget->getName() == filename)
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
  if (header_widget->getName() == filename)
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
    if (bKDevelop)
    {
      switch(tab_item)
      {
          case BROWSER:
	     kdev_caption=browser_widget->currentTitle()+ " - KDevelop " + version ;
                  break;
          case TOOLS:
	     kdev_caption=QString(i18n("Tools")) +" - KDevelop " + version ;
                  break;
          default:
	      kdev_caption=(project) ? (const char *) (prj->getProjectName()+" - KDevelop ") : "KDevelop ";
	      kdev_caption+= version +
             	" - ["+ QFileInfo(edit_widget->getName()).fileName()+"] ";
	      if (edit_widget->isModified())
                  {
                     enableCommand(ID_FILE_SAVE);
	        kdev_caption+= " *";
                  }
                  else
                  {
                    disableCommand(ID_FILE_SAVE);
                  }
                  break;
      }
      setCaption(kdev_caption);
    }
    else
    {
        // not using KDevelop but KDlgEdit
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
  int message_result=1; // simulate ok state... this could change by one of the following messageboxes

  oldName=edit_widget->getName();
  if (bAutosave)
    saveTimer->stop();

  do
  {
    if (!isUntitled(oldName))
      name = KFileDialog::getSaveFileName(oldName,0, this,oldName);
    else
      name = KFileDialog::getSaveFileName((const char *)
                                          ((project) ?  QString(prj->getProjectDir()+oldName) : oldName),
                                          0,this,oldName);

    if (name.isNull()){
    // KDEBUG(KDEBUG_INFO,CKDEVELOP,"Cancel");
      if (bAutosave)
       saveTimer->start(saveTimeout);
     return false;
    }

    // check if the extension is changed and the widget or program to view must change
    if (CProject::getType(name)!=CProject::getType(edit_widget->getName()))
      message_result = KMsgBox::yesNoCancel(this,i18n("Save as new type of document?"),
                                                                  i18n("Do you really want to save the file\n"
                                                                  "as another type of document?"),
                                                                  KMsgBox::QUESTION);
    if(message_result==1 && QFile::exists(name))
    {
      message_result=KMsgBox::yesNoCancel(this,i18n("File exists!"),
                    QString(i18n("\nThe file\n\n"))+name+
		i18n("\n\nalready exists.\nDo you want overwrite the old one?\n"));
    }
    
  } while (message_result == 2); // repeat it on 'no'


  if (message_result==3){
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
    switchToFile(name, true);

    if (oldName!=name)
    {
      // here we are... and any Untitled-file was saved with another name
      //   and now we can remove the untitled file
      if (isUntitled(oldName))
      {
          removeFileFromEditlist(oldName);
      }
    }

    slotViewRefresh();
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

/*------------------------------------------ CKDevelop::refreshTrees()
 * refreshTrees()
 *   Refresh the all trees. Includes re-parsing all files.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::refreshTrees(){
  doc_tree->refresh(prj);
  if (!project){
    return; // no project
  }

  // Update the classview.
  slotStatusMsg(i18n("Scanning project files..."));
	setCursor(KCursor::waitCursor());
  statProg->show();
  class_tree->refresh(prj);
  statProg->reset();
  statProg->hide();

  // Update the classcombo.
  CVRefreshClassCombo();

  // Update LFV.
  log_file_tree->storeState(prj);
  log_file_tree->refresh(prj);

  // Update RFV.
  real_file_tree->refresh(prj);

  kdlg_dialogs_view->refresh(prj);

  kdev_statusbar->repaint();
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
      // If this is a sourcefile we parse it and update the classview.
      if( info->type == CPP_SOURCE || info->type == CPP_HEADER )
	{
	  class_tree->addFile( prj->getProjectDir() + info->rel_name );
	  CVRefreshClassCombo();
	}
      
      // Update LFV.
      log_file_tree->storeState(prj);
      log_file_tree->refresh(prj);
      
      // Update RFV.
      real_file_tree->refresh(prj);
      // update dialogs tree
      kdlg_dialogs_view->refresh(prj);
      
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

void CKDevelop::switchToFile(QString filename, bool bForceReload,bool bShowModifiedBox){
  lastfile = edit_widget->getName();
  lasttab = s_tab_view->getCurrentTab();

  TEditInfo* info;
  TEditInfo* actual_info;

  // check if the file exists
  if(!QFile::exists(filename) && !isUntitled(filename)){
    KMsgBox::message(this,i18n("Attention"),filename +i18n("\n\nFile does not exist!"));
    return;
  }

  if(filename.right(8) == ".kdevdlg"){
    switchToKDlgEdit();
    kdlgedit->slotOpenDialog(filename);
    return;
  }
  
  // load kiconedit if clicked/loaded  an icon
  if((filename).right(4) == ".xpm"){
    if(!CToolClass::searchProgram("kiconedit")){
      return;
    }
    showOutputView(false);
    s_tab_view->setCurrentTab(TOOLS);
    swallow_widget->sWClose(false);
    swallow_widget->setExeString("kiconedit " + filename);
    swallow_widget->sWExecute();
    swallow_widget->init();
    return;
  }
  //load ktranslator if clicked/loaded an po file
  if((filename).right(3) == ".po"){
    if(CToolClass::searchInstProgram("ktranslator")){
    showOutputView(false);
    s_tab_view->setCurrentTab(TOOLS);
    swallow_widget->sWClose(false);
    swallow_widget->setExeString("ktranslator "+ filename);
    swallow_widget->sWExecute();
    swallow_widget->init();
    return;
		}
  }
  //load ktranslator if clicked/loaded an po file
  if((filename).right(4) == ".gif" || (filename).right(4) == ".bmp" || (filename).right(4) == ".xbm"){
    if(!CToolClass::searchInstProgram("kpaint")){
			return;
		}
    showOutputView(false);
    s_tab_view->setCurrentTab(TOOLS);
    swallow_widget->sWClose(false);
    swallow_widget->setExeString("kpaint "+ filename);
    swallow_widget->sWExecute();
    swallow_widget->init();
    return;
  }
  if((filename).right(3) == ".ps"){
    if(!CToolClass::searchInstProgram("kghostview")){
			return;
		}
    showOutputView(false);
    s_tab_view->setCurrentTab(TOOLS);
    swallow_widget->sWClose(false);
    swallow_widget->setExeString("kghostview "+ filename);
    swallow_widget->sWExecute();
    swallow_widget->init();
    return;
  }

  
  // set the correct edit_widget
  if (CProject::getType(filename) == CPP_SOURCE){
    edit_widget = cpp_widget;
//    s_tab_view->setCurrentTab(CPP);
    
    if(build_menu->isItemEnabled(ID_BUILD_MAKE))			
      enableCommand(ID_BUILD_COMPILE_FILE);
  }
  else{
    edit_widget = header_widget;
//    s_tab_view->setCurrentTab(HEADER);
    disableCommand(ID_BUILD_COMPILE_FILE);
  }
  
  // search the current file which would be changed
  
  for(actual_info=edit_infos.first();actual_info != 0;actual_info=edit_infos.next()){
    if (actual_info->filename == edit_widget->getName() ){
      break;
    }
  }
  
  if(actual_info == 0){
//    KDEBUG(KDEBUG_FATAL,CKDEVELOP,"actual_info in switchtoFile() is NULL!!!!!");
  }


  // handle file if it was modified on disk by another editor/cvs
  QFileInfo file_info(edit_widget->fileName());

  if((file_info.lastModified() != actual_info->last_modified )&& bShowModifiedBox){
      if(QMessageBox::warning(this,i18n("File modified"),"The file " + filename +" was modified outside this editor.\nOpen the file from disk and delete the current Buffer?",QMessageBox::Yes,QMessageBox::No) == QMessageBox::Yes){
	  bForceReload = true;
	  actual_info->last_modified = file_info.lastModified();
      }
  }
  if (!bShowModifiedBox){
     actual_info->last_modified = file_info.lastModified(); 
  }
  
  if (!bForceReload && filename == edit_widget->getName()){
      //    cerr << endl <<endl << "Filename:" << filename 
      // << "EDITNAME:" << edit_widget->getName() <<"no action---:" << endl;
      s_tab_view->setCurrentTab((edit_widget==header_widget) ? HEADER : CPP);
      edit_widget->setFocus();
      return;
  }

    // rescue the old file
  actual_info->text = edit_widget->text();
  actual_info->modified = edit_widget->isModified();
  actual_info->cursor_line = edit_widget->currentLine();
  actual_info->cursor_col = edit_widget->currentColumn();
  // output_widget->append("auszuwechseldes file:" + actual_info->filename);

  // already in the list ?
  for(info=edit_infos.first();info != 0;info=edit_infos.next()){
    if (info->filename == filename ) { // if found in list
      
      //      cerr << "******already****\n" << info->text << "**************\n";
      if (bForceReload)
      {
	  QFileInfo fileinfo(filename);
               edit_widget->clear();
               edit_widget->loadFile(filename,1);
               info->modified=false;
               info->cursor_line=info->cursor_col=0;
               info->text = edit_widget->text();
      }
      else
      {
         edit_widget->setName(filename);     // inserted to stop flickering of caption
         edit_widget->setText(info->text);
      }
      edit_widget->setName(filename);
      edit_widget->toggleModified(info->modified);
      edit_widget->setCursorPosition(info->cursor_line,info->cursor_col);

      //      output_widget->append ("File: was was already there");
      //      setMainCaption();  is handled by setCurrentTab()
      s_tab_view->setCurrentTab((edit_widget==header_widget) ? HEADER : CPP);
      edit_widget->setFocus();
      return;
    }
  }
  // not found -> generate a new edit_info,loading
  
  // build a new info
  QFileInfo fileinfo(filename);
  info = new TEditInfo;
  
  info->id = menu_buffers->insertItem(fileinfo.fileName(),-2,0); // insert at first index
  info->filename = filename.copy(); // a bugfix,that takes me 30 mins :-( -Sandy 
  info->modified = false;
  info->cursor_line = 0;
  info->cursor_col = 0;
  info->last_modified = fileinfo.lastModified();

  // update the widget
//  KDEBUG1(KDEBUG_INFO,CKDEVELOP,"switchToFile: %s",filename.data());
  edit_widget->clear();
  edit_widget->loadFile(filename,1);
  edit_widget->setName(filename);
//  edit_widget->setFocus();
  info->text = edit_widget->text();
  edit_infos.append(info); // add to the list
  //      setMainCaption();  is handled by setCurrentTab()
  s_tab_view->setCurrentTab((edit_widget==header_widget) ? HEADER : CPP);
  edit_widget->setFocus();
}

void CKDevelop::switchToFile(QString filename, int lineNo){
  lasttab = s_tab_view->getCurrentTab();
  switchToFile( filename, false);
  edit_widget->setCursorPosition( lineNo, 0 );
}

void CKDevelop::switchToKDevelop(){

  kdlg_caption = caption();
  setCaption(kdev_caption);

  bKDevelop=true;
  this->setUpdatesEnabled(false);

  //////// change the mainview ////////
  kdlg_tabctl->hide();
  kdlg_top_panner->hide();
  s_tab_view->show();
  t_tab_view->show();

  top_panner->hide();
  top_panner->deactivate();
  top_panner->activate(t_tab_view,s_tab_view);// activate the top_panner
  top_panner->show();
  //////// change the bars ///////////
  kdlg_menubar->hide();
  kdev_menubar->show();
  setMenu(kdev_menubar);

  kdlg_statusbar->hide();
  kdev_statusbar->show();
  setStatusBar(kdev_statusbar);

  toolBar(ID_KDLG_TOOLBAR)->hide();
  toolBar()->show();
  toolBar(ID_BROWSER_TOOLBAR)->show();

  setKeyAccel();  // initialize Keys
  ///////// reset bar status ////////////
  if(view_menu->isItemChecked(ID_VIEW_STATUSBAR))
    kdev_statusbar->show();
  else
    kdev_statusbar->hide();

  if(view_menu->isItemChecked(ID_VIEW_TOOLBAR))
    enableToolBar(KToolBar::Show);
  else
    enableToolBar(KToolBar::Hide);

  if(view_menu->isItemChecked(ID_VIEW_BROWSER_TOOLBAR))
    enableToolBar(KToolBar::Show,ID_BROWSER_TOOLBAR);
  else
    enableToolBar(KToolBar::Hide,ID_BROWSER_TOOLBAR);

  ///////// reset the views status ///////////////
  if(view_menu->isItemChecked(ID_VIEW_TREEVIEW))
    showTreeView();
  else
    showTreeView(false);
  if(view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW))
    showOutputView();
  else
    showOutputView(false);

  this->setUpdatesEnabled(true);
  this->repaint();

}
void CKDevelop::switchToKDlgEdit(){
//   KConfig *config = kapp->getConfig();  

  config->setGroup("KDlgEdit");
  if (config->readEntry("KDlgEdit_ShowReadme").lower() != "false")
    {
      KDlgReadmeDlg *readmedlg = new KDlgReadmeDlg(this);
      readmedlg->exec();

      if (!readmedlg->isShowAgain())
        config->writeEntry("KDlgEdit_ShowReadme","false");

      delete readmedlg;
    }


  kdev_caption = caption();
  setCaption(kdlg_caption);
  bKDevelop=false;
  this->setUpdatesEnabled(false);
  //////// change the mainview ////////
  s_tab_view->hide();
  t_tab_view->hide();
  kdlg_tabctl->show();
  kdlg_top_panner->show();

  top_panner->hide();
  top_panner->deactivate();
  top_panner->activate(kdlg_tabctl,kdlg_top_panner);// activate the top_panner
  top_panner->show();

  //////// change the bars ///////////
  kdev_menubar->hide();
  kdlg_menubar->show();
  setMenu(kdlg_menubar);

  kdev_statusbar->hide();
  kdlg_statusbar->show();
  setStatusBar(kdlg_statusbar);

  toolBar()->hide();
  toolBar(ID_BROWSER_TOOLBAR)->hide();
  toolBar(ID_KDLG_TOOLBAR)->show();

  ///////// reset bar status ////////////
  if(kdlg_view_menu->isItemChecked(ID_VIEW_STATUSBAR))
    kdlg_statusbar->show();
  else
    kdlg_statusbar->hide();

  if(kdlg_view_menu->isItemChecked(ID_KDLG_VIEW_TOOLBAR))
    enableToolBar(KToolBar::Show, ID_KDLG_TOOLBAR);
  else
    enableToolBar(KToolBar::Hide, ID_KDLG_TOOLBAR);

  setKeyAccel();  // initialize Keys

  // this toolbar toogle is for placing the panner devider correctly
  enableToolBar(KToolBar::Toggle, ID_KDLG_TOOLBAR);
  enableToolBar(KToolBar::Toggle, ID_KDLG_TOOLBAR);

  ///////// reset the views status ///////////////
  if(kdlg_view_menu->isItemChecked(ID_VIEW_TREEVIEW))
    showTreeView();
  else
    showTreeView(false);

  if(kdlg_view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW))
    showOutputView();
  else
    showOutputView(false);
  
  if(!kdlg_tabctl->isTabEnabled("widgets_view")){
    kdlg_tabctl->setCurrentTab(1); // set Dialogs enabled if no dialog was choosen
  }
  kdlg_tabctl->setFocus();
  this->setUpdatesEnabled(true);
  this->repaint();
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
    enableCommand(ID_BUILD_DEBUG);
    enableCommand(ID_BUILD_MAKE);
    enableCommand(ID_BUILD_REBUILD_ALL);
    enableCommand(ID_BUILD_CLEAN_REBUILD_ALL);
    enableCommand(ID_BUILD_DISTCLEAN);
    enableCommand(ID_BUILD_AUTOCONF);
    enableCommand(ID_BUILD_CONFIGURE);
    if (prj->getProjectType() != "normal_kde" && prj->getProjectType() != "mini_kde"){
      disableCommand(ID_PROJECT_MESSAGES);
    }
    else{
      enableCommand(ID_PROJECT_MESSAGES);
    }
    enableCommand(ID_PROJECT_MAKE_PROJECT_API);
    enableCommand(ID_PROJECT_MAKE_USER_MANUAL);
		enableCommand(ID_PROJECT_MAKE_DISTRIBUTION);

  }

  if (!enable)
  {
    // set the popupmenus enable or disable
    disableCommand(ID_BUILD_COMPILE_FILE);
    disableCommand(ID_BUILD_RUN_WITH_ARGS);
    disableCommand(ID_BUILD_RUN);
    disableCommand(ID_BUILD_DEBUG);
    disableCommand(ID_BUILD_MAKE);
    disableCommand(ID_BUILD_REBUILD_ALL);
    disableCommand(ID_BUILD_CLEAN_REBUILD_ALL);
    disableCommand(ID_BUILD_DISTCLEAN);
    disableCommand(ID_BUILD_AUTOCONF);
    disableCommand(ID_BUILD_CONFIGURE);
    disableCommand(ID_PROJECT_MESSAGES);
    disableCommand(ID_PROJECT_MAKE_PROJECT_API);
    disableCommand(ID_PROJECT_MAKE_USER_MANUAL);
    disableCommand(ID_PROJECT_MAKE_DISTRIBUTION);
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
  if(id == 1){
    project_menu->setItemChecked(ID_PROJECT_WORKSPACES_1,true);
    project_menu->setItemChecked(ID_PROJECT_WORKSPACES_2,false);
    project_menu->setItemChecked(ID_PROJECT_WORKSPACES_3,false);
  }
  if(id == 2){
    project_menu->setItemChecked(ID_PROJECT_WORKSPACES_1,false);
    project_menu->setItemChecked(ID_PROJECT_WORKSPACES_2,true);
    project_menu->setItemChecked(ID_PROJECT_WORKSPACES_3,false);
  }
  if(id == 3){
    project_menu->setItemChecked(ID_PROJECT_WORKSPACES_1,false);
    project_menu->setItemChecked(ID_PROJECT_WORKSPACES_2,false);
    project_menu->setItemChecked(ID_PROJECT_WORKSPACES_3,true);
  }
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
    if(show){
      if(view_menu->isItemChecked(ID_VIEW_TREEVIEW)){
        return; // it's already visible){
      }
      else{
				slotViewTTreeView();
      }
    }
    else{
      if(!view_menu->isItemChecked(ID_VIEW_TREEVIEW)){
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
void CKDevelop::readOptions(){
  config->setGroup("General Options");

	/////////////////////////////////////////
	// GEOMETRY
  QSize size=config->readSizeEntry("Geometry");
	if(!size.isEmpty())
		resize(size);
	else
		setGeometry(QApplication::desktop()->width()/2-400, QApplication::desktop()->height()/2-300, 800, 600);

	/////////////////////////////////////////
	// BAR STATUS
	KMenuBar::menuPosition kdev_menu_bar_pos=(KMenuBar::menuPosition)config->readNumEntry("KDevelop MenuBar Position", KMenuBar::Top);
	kdev_menubar->setMenuBarPos(kdev_menu_bar_pos);
	KMenuBar::menuPosition kdlg_menu_bar_pos=(KMenuBar::menuPosition)config->readNumEntry("KDlgEdit MenuBar Position", KMenuBar::Top);
	kdev_menubar->setMenuBarPos(kdlg_menu_bar_pos);


  KToolBar::BarPosition tool_bar_pos=(KToolBar::BarPosition)config->readNumEntry("ToolBar Position", KToolBar::Top);
  toolBar()->setBarPos(tool_bar_pos);
	bool std_toolbar=	config->readBoolEntry("show_std_toolbar", true);
	if(std_toolbar){
	  view_menu->setItemChecked(ID_VIEW_TOOLBAR, true);
    enableToolBar(KToolBar::Show,0);
  }
  else{
    enableToolBar(KToolBar::Hide,0);
  }
	// Browser Toolbar
  KToolBar::BarPosition browser_tool_bar_pos=(KToolBar::BarPosition)config->readNumEntry("Browser ToolBar Position", KToolBar::Top);
  toolBar(ID_BROWSER_TOOLBAR)->setBarPos(browser_tool_bar_pos);
	bool browser_toolbar=config->readBoolEntry("show_browser_toolbar",true);
	if(browser_toolbar){
	  view_menu->setItemChecked(ID_VIEW_BROWSER_TOOLBAR, true);
    enableToolBar(KToolBar::Show,ID_BROWSER_TOOLBAR);
  }
  else{
    enableToolBar(KToolBar::Hide,ID_BROWSER_TOOLBAR);
  }
	
	// Dialogedit Toolbar	
  KToolBar::BarPosition kdlg_tool_bar_pos=(KToolBar::BarPosition)config->readNumEntry("KDlgEdit ToolBar Position", KToolBar::Top);
  toolBar(ID_KDLG_TOOLBAR)->setBarPos(kdlg_tool_bar_pos);
	bool kdlg_toolbar=config->readBoolEntry("show_kdlg_toolbar", true);
  if(kdlg_toolbar){
		kdlg_view_menu->setItemChecked(ID_KDLG_VIEW_TOOLBAR,true);
    enableToolBar(KToolBar::Show,ID_KDLG_TOOLBAR);
  }
  else{
    enableToolBar(KToolBar::Hide,ID_KDLG_TOOLBAR);
  }
	// Statusbar
	bool statusbar=config->readBoolEntry("show_statusbar",true);
	if(statusbar){
	  view_menu->setItemChecked(ID_VIEW_STATUSBAR, true);
    kdlg_view_menu->setItemChecked(ID_VIEW_STATUSBAR,true);
	}
	else{
		enableStatusBar();
	}
	
	/////////////////////////////////////////
	// Outputwindow, TreeView, KDevelop/KDlgEdit
	view->setSeparatorPos(config->readNumEntry("view_panner_pos",80));
	bool outputview= config->readBoolEntry("show_output_view", true);
	if(outputview){
	  view_menu->setItemChecked(ID_VIEW_OUTPUTVIEW, true);
    kdlg_view_menu->setItemChecked(ID_VIEW_OUTPUTVIEW,true);
		toolBar()->setButton(ID_VIEW_OUTPUTVIEW, true);
		toolBar(ID_KDLG_TOOLBAR)->setButton(ID_VIEW_OUTPUTVIEW, true);
    output_view_pos=view->separatorPos();
	}
	else{
    output_view_pos=config->readNumEntry("output_view_pos", 80);
	}
	
  top_panner->setSeparatorPos(config->readNumEntry("top_panner_pos", 213));
	bool treeview=config->readBoolEntry("show_tree_view", true);
	if(treeview){
	  view_menu->setItemChecked(ID_VIEW_TREEVIEW, true);
    kdlg_view_menu->setItemChecked(ID_VIEW_TREEVIEW, true);
		toolBar()->setButton(ID_VIEW_TREEVIEW, true);
		toolBar(ID_KDLG_TOOLBAR)->setButton(ID_VIEW_TREEVIEW, true);
    tree_view_pos=top_panner->separatorPos();
	}
  else{
    tree_view_pos=config->readNumEntry("tree_view_pos", 213);
  }
	

  kdlg_top_panner->setSeparatorPos(config->readNumEntry("kdlg_top_panner_pos", 80));
	if(config->readBoolEntry("show_properties_view",true)){
	  kdlg_view_menu->setItemChecked(ID_KDLG_VIEW_PROPVIEW,true);
    properties_view_pos=kdlg_top_panner->separatorPos();
	}	
  else{
    properties_view_pos=config->readNumEntry("properties_view_pos", 80);
  }


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
	recent_projects.setAutoDelete(TRUE);
	config->readListEntry("Recent Projects",recent_projects);
	
	uint i;
	for ( i =0 ; i < recent_projects.count(); i++){
    recent_projects_menu->insertItem(recent_projects.at(i));
  }

	doc_bookmarks_list.setAutoDelete(TRUE);
	doc_bookmarks_title_list.setAutoDelete(TRUE);
	
	config->readListEntry("doc_bookmarks",doc_bookmarks_list);
	config->readListEntry("doc_bookmarks_title",doc_bookmarks_title_list);
	for ( i =0 ; i < doc_bookmarks_title_list.count(); i++){
    doc_bookmarks->insertItem(Icon("mini/html.xpm"),doc_bookmarks_title_list.at(i));
  }
	
  QString filename;
  filename = config->readEntry("browser_file","");
  if(!filename.isEmpty()){
    slotURLSelected(browser_widget,filename,1,"test");
  }
  else{
    QString strpath = KApplication::kde_htmldir().copy() + "/";
    QString file;
    // first try the locale setting
    file = strpath + klocale->language() + '/' + "kdevelop/welcome/index.html";

    if( !QFileInfo( file ).exists() ){
      // not found: use the default
      file = strpath + "default/" + "kdevelop/welcome/index.html";
    }
    slotURLSelected(browser_widget,"file:" + file,1,"test");
  }

  bool switchKDevelop=config->readBoolEntry("show_kdevelop",true);  // if true, kdevelop, else kdialogedit
  if(switchKDevelop){
    switchToKDevelop();
  }
  else{
    switchToKDlgEdit();
  }
}

void CKDevelop::saveOptions(){
	
  config->setGroup("General Options");
  config->writeEntry("Geometry", size() );
  
  config->writeEntry("KDevelop MenuBar Position", (int)kdev_menubar->menuBarPos());
  config->writeEntry("KDlgEdit MenuBar Position", (int)kdlg_menubar->menuBarPos());
  config->writeEntry("ToolBar Position",  (int)toolBar()->barPos());
  config->writeEntry("Browser ToolBar Position", (int)toolBar(ID_BROWSER_TOOLBAR)->barPos());
  config->writeEntry("KDlgEdit ToolBar Position", (int)toolBar(ID_KDLG_TOOLBAR)->barPos());
  
  config->writeEntry("view_panner_pos",view->separatorPos());
  config->writeEntry("top_panner_pos",top_panner->separatorPos());
  config->writeEntry("kdlg_top_panner_pos",kdlg_top_panner->separatorPos());

  config->writeEntry("tree_view_pos",tree_view_pos);
  config->writeEntry("output_view_pos",output_view_pos);
  config->writeEntry("properties_view_pos", properties_view_pos);

  config->writeEntry("show_tree_view",view_menu->isItemChecked(ID_VIEW_TREEVIEW));
  config->writeEntry("show_output_view",view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW));
  config->writeEntry("show_properties_view",kdlg_view_menu->isItemChecked(ID_KDLG_VIEW_PROPVIEW));

  config->writeEntry("show_std_toolbar",view_menu->isItemChecked(ID_VIEW_TOOLBAR));
  config->writeEntry("show_browser_toolbar",view_menu->isItemChecked(ID_VIEW_BROWSER_TOOLBAR));
  config->writeEntry("show_kdlg_toolbar",kdlg_view_menu->isItemChecked(ID_KDLG_VIEW_TOOLBAR));

  config->writeEntry("show_statusbar",view_menu->isItemChecked(ID_VIEW_STATUSBAR));
  config->writeEntry("LastActiveTab", s_tab_view->getCurrentTab());
  config->writeEntry("LastActiveTree", t_tab_view->getCurrentTab());

  config->writeEntry("show_kdevelop",bKDevelop);

  config->writeEntry("lfv_show_path",log_file_tree->showPath());

  config->writeEntry("Autosave",bAutosave);
  config->writeEntry("Autosave Timeout",saveTimeout);

  config->writeEntry("Make",make_cmd);

  config->setGroup("Files");
  config->writeEntry("browser_file",history_list.current());
  config->writeEntry("doc_bookmarks", doc_bookmarks_list);
  config->writeEntry("doc_bookmarks_title", doc_bookmarks_title_list);
  config->writeEntry("Recent Projects", recent_projects);
  
  config->sync();
}

bool CKDevelop::queryExit(){
  saveOptions();
  return true;
}

bool CKDevelop::queryClose(){
  swallow_widget->sWClose(false); // close the tools in the tools-tab
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
    int message_result=1;
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
      message_result = KMsgBox::yesNo(this,i18n("Exit KDevelop"),
                                         i18n("There is unsaved data.\n"
					      "Do you really want to quit?"),
                                         KMsgBox::QUESTION);
    }
    return message_result==1;
  }
  return true;
}

void CKDevelop::readProperties(KConfig* sess_config){
  QString filename;
  filename = sess_config->readEntry("project_file","");
  
  QFile file(filename);
  if (file.exists()){
    if(!(readProjectFile(filename))){
      KMsgBox::message(0,filename,"This is a Project-File from KDevelop 0.1\nSorry,but it's incompatible with KDevelop >= 0.2.\nPlease use only new generated projects!");
      refreshTrees();
    }
    else{
      QProgressDialog *progressDlg= new QProgressDialog(NULL, "progressDlg", true );
      connect(class_tree,SIGNAL(setStatusbarProgressSteps(int)),progressDlg,SLOT(setTotalSteps(int)));
      connect(class_tree,SIGNAL(setStatusbarProgress(int)),progressDlg,SLOT(setProgress(int)));
      progressDlg->setCaption(i18n("Starting..."));
      progressDlg->setLabelText( i18n("Initializing last project...\nPlease wait...\n") );
      progressDlg->setProgress(0);
      progressDlg->show();
      refreshTrees();
      delete progressDlg;
    }
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
  else{
    refreshTrees(); // this refresh only the documentation tab,tree
  }
}

void CKDevelop::saveProperties(KConfig* sess_config){
	
  if(project){
    sess_config->writeEntry("project_file",prj->getProjectFile());
    sess_config->writeEntry("cpp_file",cpp_widget->getName());
    sess_config->writeEntry("header_file",header_widget->getName());
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
	setUnsavedData ( true );
	break;
      }
    }
  }
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



