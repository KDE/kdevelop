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

#include "ckdevelop.h"
#include "projectmanager.h"
#include "core.h"
#include "./ctags/ctagsdialog_impl.h"
#include "caddexistingfiledlg.h"
#include "caddnewtranslationdlg.h"
#include "cclassview.h"
//#include "ceditwidget.h"

#include "cfilepropdlg.h"
#include "cgeneratenewfile.h"
#include "ckappwizard.h"
#include "ckdevaccel.h"
#include "clogfileview.h"
#include "cmakemanualdlg.h"
#include "cnewclassdlg.h"
#include "cnewfiledlg.h"
#include "coutputwidget.h"
#include "cprjoptionsdlg.h"
#include "crealfileview.h"
#include "ctoolclass.h"
#include "debug.h"
#include "kpp.h"
#include "docviewman.h"
#include <kate/document.h>
#include <kate/view.h>
#include "kdevsession.h"

#include <qprogressdialog.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qstyle.h>

#include <kcombobox.h>
#include <kdebug.h>
#include <kcursor.h>
#include <kbuttonbox.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kmessagebox.h>

#include <kurlrequester.h>
#include <kemailsettings.h>

#include "ctags/cctags.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*********************************************************************
 *                                                                   *
 *                              SLOTS                                *
 *                                                                   *
 ********************************************************************/

bool CKDevelop::slotProjectClose()
{
  if( !project )
    return true;

  slotStatusMsg(i18n("Closing project..."));

  if( !m_docViewManager->doProjectClose() ) {
    slotStatusMsg(i18n("Ready."));
    return false;
  }

  log_file_tree->storeState(prj);

  // save the session
  KComboBox* compile_combo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_COMPILE_CHOICE);
  m_pKDevSession->setLastCompile(compile_combo->currentText());
  QString sessionFileName = prj->getProjectFile();
  sessionFileName = sessionFileName.left( sessionFileName.length() - 7);
  sessionFileName += "kdevses";
  m_pKDevSession->saveToFile(sessionFileName);
  prj->writeProject();

  class_tree->clear();
  log_file_tree->clear();
  real_file_tree->clear();
  messages_widget->clear();
  stdin_stdout_widget->clear();
  stderr_widget->clear();
  file_open_popup->clear();
  file_open_list.clear();

  if (dbgController)
    slotDebugStop();

  m_docViewManager->doCloseAllDocs();

  stateChanged("project_open",StateReverse);
  stateChanged("build_project",StateReverse);
  stateChanged("build",StateReverse);

  project=false;
  delete prj;
  prj = 0;

  refreshTrees();

  setMainCaption();

  // DISABLED robe 7/4/2002
  // ProjectManager::getInstance()->closeProject();
  // Core::getInstance()->doEmitProjectClosed();

  return true;
}

void CKDevelop::slotProjectAddNewFile(){
  newFile(true);
}

void CKDevelop::slotAddExistingFiles(){
  QString temp_template;
  KShellProcess add_process("/bin/sh");
  CGenerateNewFile genfile;
  bool copy = false;
  ProjectFileType type = DATA;
  bool new_subdir=false; // if a new subdir was added to the project, we must do a rebuildmakefiles
  QString token;
  QStrList files;
  QString str_files = add_dlg->source_edit->lineEdit()->text();

  connect(&add_process,SIGNAL(receivedStdout(KProcess*,char*,int)),
        this,SLOT(slotReceivedStdout(KProcess*,char*,int)) );

  connect(&add_process,SIGNAL(receivedStderr(KProcess*,char*,int)),
      this,SLOT(slotReceivedStderr(KProcess*,char*,int)) );

  connect(&add_process,SIGNAL(processExited(KProcess*)),
      this,SLOT(slotProcessExited(KProcess*) )) ;


  QStringList fileList;
  fileList = QStringList::split ( ",", str_files, FALSE );
  for ( QStringList::Iterator it = fileList.begin(); it != fileList.end(); ++it )
    files.append((*it));

  QString dest = add_dlg->destination_edit->lineEdit()->text();
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
  progress.setCaption(i18n("Please Wait..."));
  progress.show();

  i=0;
  progress.setProgress( i);
  QStrList lNewFiles;
  for(file = files.first(); file !=0;file = files.next()){
    i++;
    copy = false;
    progress.setProgress( i );
    if (!QFile::exists((const char*)file)) {
        KMessageBox::error(this, i18n("%1\n\nFile does not exist!").arg(file));
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
      add_process.clearArguments();
      add_process << QString("echo '")+
                     i18n("Preparing file: ") << source_name << "';";
    // give a little message

      add_process << "cat"; // copy is your friend :-) ...cat, too

      if (add_dlg->isTemplateChecked())
      {
       if (CProject::getType(file)==CPP_HEADER)
        {
         temp_template = genfile.genHeaderFile(locate("data","temp_template"), prj,source_name);
         add_process << temp_template;
        }
        else
        {
          if (CProject::getType(file)==CPP_SOURCE)
          {
             temp_template = genfile.genCPPFile(locate("data","temp_template"), prj, source_name);
             add_process << temp_template;
          }
        }
      }
      add_process << file;
      add_process << ">";

      add_process << temp_name;
      add_process.start(KProcess::Block,KProcess::AllOutput); // blocked because it is important
      add_process.clearArguments();
      add_process << QString("echo '")+
                     i18n("Add to project: ") << source_name << "';";
    // give a little message
      add_process << "mv";
      add_process << temp_name;
      add_process << dest_name;
      add_process.start(KProcess::Block,KProcess::AllOutput);
    }
    else
    if(QFile::exists(dest_name)){
      int result=KMessageBox::warningYesNoCancel(this,
                                  i18n("\nThe file\n\n%1\n\n"
                                        "already exists.\nDo you want overwrite the old one?\n")
                                        .arg(source_name),
                                  i18n("File Exists!"));
      if(result==KMessageBox::Yes)
        copy = true;
      if(result==KMessageBox::No)
        copy = false;
      if(result==KMessageBox::Cancel){
        setCursor( KCursor::arrowCursor() );
        break;;
      }
    }
    else {
      copy = true;
    }

    if(copy){
      add_process.clearArguments();
      add_process << QString("echo '")+
                     i18n("Copy and add to project: ") << source_name << "';";
    // give a little message
      add_process << "cat"; // copy is your friend :-) ...cat, too

      if (add_dlg->isTemplateChecked())
      {
       if (CProject::getType(file)==CPP_HEADER)
        {
         temp_template = genfile.genHeaderFile(locate("data","temp_template"), prj,source_name);
         add_process << temp_template;
        }
        else
        {
          if (CProject::getType(file)==CPP_SOURCE)
          {
            temp_template = genfile.genCPPFile(locate("data","temp_template"), prj, source_name);
            add_process << temp_template;
          }
        }
      }
      add_process << file;
      add_process << ">";

      add_process << dest+source_name;
      add_process.start(KProcess::Block,KProcess::AllOutput); // blocked because it is important
    }

    new_subdir = addFileToProject(dest_name,type,false) || new_subdir; // no refresh
    lNewFiles.append(dest_name);
  }

  // disconnect all of add_process

  disconnect(&add_process);

  progress.setProgress( files.count() );
  // if (type != DATA)               // don't load data files (has to be tested if wanted)
  switchToFile(dest_name);
  refreshTrees(&lNewFiles);


  if(new_subdir){
    newSubDir();
  }
   if(prj->getProjectType()=="normal_gnome" && !new_subdir){
    slotBuildAutoconf();
  }

}

void CKDevelop::slotProjectAddExistingFiles(){
  add_dlg = new CAddExistingFileDlg(this,"test",prj);

  add_dlg->destination_edit->lineEdit()->setText(prj->getProjectDir()+ prj->getSubDir());

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

QString CKDevelop::prepareConfigureCommand()
{
        QStringList configs;
  QString shellcommand;
  QString args, cppflags, cflags, cxxflags, addcxxflags, ldflags;

  configs=m_pKDevSession->getCompileConfigs();
  if(!configs.isEmpty())
  { // only default used - run only in srcdir
    for (QStringList::Iterator it=configs.begin() ; it!=configs.end() ; ++it)
    {
      QString vpath=m_pKDevSession->getVPATHSubdir( (*it) );
      QDir dir(vpath);
      // change to VPATH subdir, create it if not existant
      if(!dir.exists())
        dir.mkdir(vpath);

      if(it==configs.begin()) // first loop, leave out &&
        shellcommand+=" echo \""+i18n("Running configure in build directory %1").arg(vpath);
      else
        shellcommand+=" && echo \""+i18n("Running configure in build directory %1").arg(vpath);
      shellcommand+="\" ";

      shellcommand +=" && cd "+vpath+" && ";
      config->setGroup("Compilearch "+
          m_pKDevSession->getArchitecture(*it)+"-"+m_pKDevSession->getPlatform(*it) );
      shellcommand += "CPP=\""+ config->readEntry("CPP","cpp") + "\" ";
      shellcommand += "CC=\"" + config->readEntry("CC","gcc") + "\" ";
      shellcommand += "CXX=\"" + config->readEntry("CXX","g++") + "\" ";

      cppflags=m_pKDevSession->getCPPFLAGS(*it).simplifyWhiteSpace();
      cflags=m_pKDevSession->getCFLAGS(*it).simplifyWhiteSpace();
      cxxflags=m_pKDevSession->getCXXFLAGS(*it).simplifyWhiteSpace();
      addcxxflags=m_pKDevSession->getAdditCXXFLAGS(*it).simplifyWhiteSpace();
      ldflags=m_pKDevSession->getLDFLAGS(*it).simplifyWhiteSpace();

      // this has to get over and over again per configuration
      shellcommand += "CPPFLAGS=\"" + cppflags + "\" ";
      // if the project type is normal_c, the cflags were stored in the cxxflags
      // of the old project type. Therefore, when the project is normal_c, continue
      // to export the CXXFLAGS as CFLAGS.
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
      // the configure script is always in the project directory, no matter where we are

      args=m_pKDevSession->getConfigureArgs( (*it) ).simplifyWhiteSpace();
      // this check is only to handle a strange bug
      //   if vpath==project dir the rule for .ui files won't be accepted (Walter)
      if (vpath!=prj->getProjectDir())
        shellcommand += prj->getProjectDir() +"/configure "+ args;
      else
        shellcommand += "./configure "+ args;

    }
  }
  else
  {
     cxxflags=prj->getCXXFLAGS().simplifyWhiteSpace();
     addcxxflags=prj->getAdditCXXFLAGS().simplifyWhiteSpace();
     ldflags=prj->getLDFLAGS().simplifyWhiteSpace();


     shellcommand+=" echo \""+i18n("Running configure in source directory")+"\" ";
     shellcommand+=" && cd "+prj->getProjectDir()+" && ";
     // export CC, CXX, CPP
     config->setGroup("Compiler");
     QString arch=config->readEntry("Architecture","i386");
     QString platf=config->readEntry("Platform","linux");

     config->setGroup("Compilearch "+arch+"-"+platf);
     shellcommand += " CPP=\"" + config->readEntry("CPP","cpp")+ "\" ";
     shellcommand += " CC=\"" + config->readEntry("CC","gcc")+ "\" ";
     shellcommand += " CXX=\"" + config->readEntry("CXX","g++")+ "\" ";
     shellcommand += " CPPFLAGS=\"" + cppflags + "\" ";

     // if the project type is normal_c, the cflags were stored in the cxxflags
     // of the old project type. Therefore, when the project is normal_c, continue
     // to export the CXXFLAGS as CFLAGS.
     if(prj->getProjectType()=="normal_c")
     {
        shellcommand += "CFLAGS=\"" + cflags + " " + cxxflags + " " + addcxxflags + "\" " ;
     }
     else
     {
        shellcommand += "CFLAGS=\"" + cflags + "\" ";
        shellcommand += "CXXFLAGS=\"" + cxxflags + " " + addcxxflags + "\" ";
     }

     shellcommand  += " LDFLAGS=\"" + ldflags + "\" ";
     // the configure script is always in the project directory, no matter where we are
     args=prj->getConfigureArgs();
     shellcommand += "./configure " + args;
  }

  return shellcommand;
}

void CKDevelop::slotProjectOptions(){
        QStringList configs;
        QString shellcommand="";
        // get the current config to set it as the config in the project options dialog
         KComboBox* compile_combo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_COMPILE_CHOICE);
         QString curr=compile_combo->currentText();
  CPrjOptionsDlg prjdlg(prj,m_pKDevSession, curr, this,"optdialog");
  // DISABLED robe 7/4/2002
  //Core::getInstance()->doEmitProjectConfigWidget( &prjdlg );
  if(prjdlg.exec()){
                 // refill the compile configs combobox
                 curr=compile_combo->currentText();
                 compile_combo->clear();
                 configs=m_pKDevSession->getCompileConfigs();
                 configs.prepend(i18n("(Default)"));
                 compile_combo->insertStringList(configs);
                 int idx=configs.findIndex(curr); // find the index for the last config
                 compile_combo->setCurrentItem(idx);
                 compile_combo->setEnabled(true);

                //////////////////////////////////////
                bool cfgure=false; // run configure for all configurations ?
    if (prjdlg.needConfigureInUpdate())
    {
      cfgure=true;
      prj->updateConfigureIn();
      KMessageBox::information(0,i18n("You have modified certain project information.\nWe will regenerate configure now."));
      setToolMenuProcess(false);
      slotStatusMsg(i18n("Running automake/autoconf and configure..."));
      messages_widget->start();
      showOutputView(true);
      QDir::setCurrent(prj->getProjectDir());
      QString makefile("Makefile.dist");
      if(!QFileInfo(QDir::current(), makefile).exists())
        makefile="Makefile.cvs";
      shellcommand += make_cmd + " -f "+makefile+" && ";
    }
    if(prjdlg.needMakefileUpdate())
    {
      cfgure=true;
      prj->updateMakefilesAm();
    }

    if(cfgure)
    { // yes, run configure for either only the default config (builddir=srcdir)
      // or for all other configurations to update makefiles.
      QDir::setCurrent(prj->getProjectDir());

      shellcommand+=prepareConfigureCommand();

      setToolMenuProcess(false);
      messages_widget->start();
      showOutputView(true);
      shell_process.clearArguments();
      shell_process << shellcommand;
      debug("run: %s\n", shellcommand.data());
      shell_process.start(KProcess::NotifyOnExit,KProcess::AllOutput);
    }
  }
}

void CKDevelop::slotProjectNewClass(const char* folder)
{
  CNewClassDlg dlg(prj,folder,this,"newclass");
  if(dlg.exec()){
    QString source_file=dlg.getImplFile() ;
    QString header_file=dlg.getHeaderFile();
    switchToFile(source_file);
    switchToFile(header_file);
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
  }
}

void CKDevelop::slotProjectNewClass(){
  CNewClassDlg dlg(prj,this,"newclass");
  if(dlg.exec())
  {
    QString source_file=dlg.getImplFile() ;
    QString header_file=dlg.getHeaderFile();
    switchToFile(source_file);
    switchToFile(header_file);
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

void CKDevelop::slotShowFileProperties(const QString& rel_name){
  CFilePropDlg dlg(this,"DLG",prj,rel_name);
  dlg.exec();
}

void CKDevelop::slotProjectOpen()
{
  if( !slotProjectClose() )
    return;

  slotStatusMsg(i18n("Opening project..."));

  config->setGroup("General Options");
  QString defDir=config->readEntry("ProjectDefaultDir", QDir::homeDirPath());

  slotOpenProject( KFileDialog::getOpenURL( defDir, "*.kdevprj") );
}

void CKDevelop::slotProjectOpenRecent( const KURL& url )
{
  if( !slotProjectClose() )
    return;

  // FIXME: Remote Files?
  if( !QFile::exists(url.path()) ) {
    if( KMessageBox::Yes == KMessageBox::questionYesNo(
          this, i18n( "This project no longer exists. "
                      "Do you want to remove it from the list?"),
          i18n("File Not Found: %1").arg( url.url() ) ) )
      pRecentProjects->removeURL(url);
    return;
  }

  slotOpenProject( url );
}

void CKDevelop::slotOpenProject( const KURL& url )
{
  if( !slotProjectClose() )
    return;

  kdDebug() << "CKDevelop::slotOpenProject(): " << url.prettyURL() << endl;

  QString prjname = url.path();

  QFileInfo info( prjname );

  // if the new project file is not valid, do nothing
  if (!info.isFile())
    return;

  // Make sure we have the right permissions to read and write to the prj file
  if (!(info.isWritable() && info.isReadable())) {
    KMessageBox::error( this,
      i18n("Unable to open \"%1\" because you do not have "
           "read/write permissions for this file.").arg( url.prettyURL() ),
      i18n("Permission Denied") );

    slotStatusMsg(i18n("Ready."));
    return;
  }

  CProject* pProj = prepareToReadProjectFile( url.path() );

  if( !pProj ) {
   KMessageBox::error( this,
    i18n("\"%1\" is not a valid KDevelop project file.").arg( url.prettyURL() ),
    i18n("Invalid Project") );

    slotStatusMsg(i18n("Ready."));
    return;
  }

  // first restore the project session stored in a .kdevses file
  QString projSessionFileName = prjname.left(prjname.length()-7); // without ".kdevprj"
  projSessionFileName += "kdevses"; // suffix for a KDeveop session file
  if( !m_pKDevSession->restoreFromFile(projSessionFileName) ) {
    debug("error during restoring of the KDevelop session !\n");
    slotStatusMsg(i18n("Ready."));
    return;
  }

  // fill in the configs into the toolbar
  KComboBox* compile_combo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_COMPILE_CHOICE);
  compile_combo->clear();
  QStringList configs=m_pKDevSession->getCompileConfigs();
  configs.prepend(i18n("(Default)"));
  compile_combo->insertStringList(configs);
  compile_combo->setEnabled(true);
  int idx = configs.findIndex(m_pKDevSession->getLastCompile());
  if(idx==-1) {
    idx=configs.findIndex(i18n("(Default)"));
    compile_combo->setCurrentItem(idx);
  }

  // now parse the .kdevprj file
  readProjectFile(pProj->getProjectFile(), pProj);
  slotViewRefresh();

  // load CTags database
  if (bCTags) {
    slotProjectLoadTags();
  }

  stateChanged("project_open");
  stateChanged("build_project");

  slotStatusMsg(i18n("Ready."));

  pRecentProjects->addURL(url);
//  shuffleProjectToTop(id);
  // DISABLED robe 7/4/2002
  // Core::getInstance()->doEmitProjectOpened();
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
  KEMailSettings emailSettings;
  QString authorName = config->readEntry("author_name","");
  if (authorName.isEmpty())
    authorName = emailSettings.getSetting (KEMailSettings::RealName);
  QString emailAddress = config->readEntry("author_email", "");
  if (emailAddress.isEmpty())
    emailAddress = emailSettings.getSetting (KEMailSettings::EmailAddress);

  CKAppWizard kappw(this,"zutuz", authorName, emailAddress);

  kappw.setCaption(i18n("ApplicationWizard"));
  kappw.exec();
  QString file = kappw.getProjectFile();

  if(kappw.generatedProject())
  {
    config->setGroup("General Options");
    config->writeEntry("author_name",kappw.getAuthorName());
    config->writeEntry("author_email",kappw.getAuthorEmail());
    config->sync();

    if(project)        //now that we know that a new project will be built we can close the previous one
    {
      old_project = prj->getProjectFile();
      if(!slotProjectClose())                //the user may have pressed cancel in which case the state is undetermined
      {
        CProject* pProj = prepareToReadProjectFile(old_project);
        if (pProj != 0L) {
          readProjectFile(old_project, pProj);
          slotViewRefresh();
        }
        return;
      }
    }

    CProject* pProj = prepareToReadProjectFile(file);
    if (pProj == 0L)
       return;

    readProjectFile(file, pProj);
    KComboBox* compile_combo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_COMPILE_CHOICE);
    compile_combo->clear(); // on startup, the combo contains "compile configuration"

    QStringList configs;
    configs.append(i18n("(Default)"));
    compile_combo->insertStringList(configs);
    int idx=configs.findIndex(i18n("(Default)")); // find the index for the last config
    compile_combo->setCurrentItem(idx);
    compile_combo->setEnabled(true);

    slotViewRefresh();        // a new project started, this is legitimate

    /* try now to open the file main.(cpp|c), if the option "StartupEditing"
       says so...
     */
    if (bStartupEditing)
    {
      QStrList sources=pProj->getSources();
      QString srcname, fname;
      for(srcname=sources.first(); srcname!=0 && fname.isEmpty();
         srcname=sources.next())
      {
        QFileInfo fi(srcname);
        if (fi.fileName()=="main.c" || fi.fileName()=="main.cpp")
          fname=srcname;
      }

      if (!fname.isEmpty() && QFile::exists(fname))
      {
        switchToFile(fname);
      }
    }
  }
  slotStatusMsg(i18n("Ready."));
}

/** calls kimport to generate a new project by
requesting a project directory, writes project file
and loads the new project */
void CKDevelop::slotProjectGenerate()
{
  QString kimport = locate("appdata", "tools/kimport");
  if (kimport.isEmpty())
    return;

  slotStatusMsg(i18n("Generating project file..."));
  messages_widget->start();

  QString dir;
  dir = KFileDialog::getExistingDirectory(QDir::currentDirPath());
  if (dir.isEmpty())
  {
    slotStatusMsg(i18n("Ready."));
    return;
  }

  if (dir.right(1) != "/" ){
    dir = dir + "/";
  }
  QString qt_testfile=dir+"Makefile.am"; // test if the path contains a Makefile.am
  if(!QFileInfo(qt_testfile).exists()){
    if (KMessageBox::questionYesNo(this,i18n("The chosen path does not lead to a\n"
                                 "directory containing a Makefile.am\n"
                                 "to create a Project file from.\n"
                                 "Are you sure this is the correct path?\n"),
                            i18n("The selected Path May Not Correct!")) != KMessageBox::Yes)
    {
      slotStatusMsg(i18n("Ready."));
      return;
    }
  }

  QDir::setCurrent(dir);
  QDir directory(dir);
  QString relDir=directory.dirName();
  QString file =relDir+".kdevprj";

  if(QFileInfo(file).exists())
  {
    if (KMessageBox::questionYesNo(this, i18n("In the path you've given\n"
                                                "already contains a KDevelop Project file!\n"
                                                "Overwrite?"),
                                          i18n("File Exists!")) != KMessageBox::Yes)
    {
      slotStatusMsg(i18n("Ready."));
      return;
    }
  }

  showOutputView(true);
  setToolMenuProcess(false);
//  error_parser->toogleOff();

  shell_process.clearArguments();
  shell_process << "echo"
                << "'"+dir+"'"
                << "&&";
  shell_process << "cd"
                << "'"+dir+"'"
                << "&&";
  shell_process <<  "perl"
                << kimport
                << "-o="+file
                << "-b="+relDir;
  shell_process.start(KProcess::NotifyOnExit, KProcess::AllOutput);
  beep = true;
  next_job="load_new_prj";
}

void CKDevelop::slotProjectAddNewTranslationFile(){
  CAddNewTranslationDlg dlg(this,0,prj);
  QString file;
  if (dlg.exec()){
    file = dlg.getLangFile();
    if(!prj->isQt2Project()){
      file = prj->getProjectDir() + "po/" + file;
      QFile nfile(file); // create a empty file
      nfile.open(IO_WriteOnly);
      nfile.close();
      addFileToProject(file, PO);
    }
    else{
      QString ts,qm;
      ts = prj->getProjectDir() +prj->getSubDir() + file + QString(".ts");
      QFile ts_file(ts); // create a empty file
      ts_file.open(IO_WriteOnly);
      ts_file.close();
      addFileToProject(ts, QT_TS);

      qm = prj->getProjectDir() +prj->getSubDir() + file + QString(".qm");
      QFile qm_file(qm); // create a empty file
      qm_file.open(IO_WriteOnly);
      qm_file.close();
      addFileToProject(qm, DATA);
    }
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
  real_file_tree->refresh( prj );
}

void CKDevelop::slotProjectMessages(){
  if(!prj->isQt2Project()){
    if(!CToolClass::searchProgram("xgettext")){
      return;
    }
  }
  else{
    if(!(CToolClass::searchProgram("lupdate") && CToolClass::searchProgram("lrelease")) ){
      return;
    }
  }
  slotDebugStop();

//  error_parser->toogleOff();
  showOutputView(true);
  setToolMenuProcess(false);
  slotFileSaveAll();
  if(!prj->isQt2Project())
    slotStatusMsg(i18n("Creating pot-file in /po..."));
  else
    slotStatusMsg(i18n("Updating translation files..."));
  messages_widget->start();
//  error_parser->toogleOff();
  shell_process.clearArguments();
  //shellprocess << make_cmd;
  if(prj->isQt2Project()){
    shell_process << QString("cd '")+prj->getProjectDir() + prj->getSubDir()+ "' && ";
    shell_process << "lupdate Makefile.am && lrelease Makefile.am" ;
  }
  else if(prj->isKDE2Project()){
    shell_process << QString("cd '")+prj->getProjectDir() +"' && ";
    shell_process <<  make_cmd + " package-messages";

  }
  else{
    shell_process << QString("cd '")+prj->getProjectDir() + prj->getSubDir()+ "' && ";
    shell_process << make_cmd + " messages && cd ../po && " + make_cmd + " merge";
  }
  next_job="fv_refresh";
  shell_process.start(KProcess::NotifyOnExit, KProcess::AllOutput);
  beep = true;
}

void CKDevelop::slotProjectAPI(){

// FIXME (rokrau 02/17/02)

    config->setGroup("Doc_Location");
    QString idx_path, link;
    idx_path = config->readEntry("doc_kde", KDELIBS_DOCDIR)
            + "/kdoc-reference";
    if (!idx_path.isEmpty())
    {
      QDir d;
      d.setPath(idx_path);
      if(d.exists())
      {
        QString libname;
        const QFileInfoList *fileList = d.entryInfoList(); // get the file info list
        QFileInfoListIterator it( *fileList ); // iterator
        QFileInfo *fi; // the current file info
        while ( (fi=it.current()) )
        {  // traverse all kdoc reference files
          libname=fi->fileName();  // get the filename
          if(fi->isFile())
          {
            libname=fi->baseName();  // get only the base of the filename as library name
            if (libname != QString("libkmid"))
            { // workaround for a strange behaviour of kdoc: don't try libkmid
              link+=" -l"+libname;
            }
          }
          ++it; // increase the iterator
        }
      }
      else
        idx_path="";
    }

    QDir d(prj->getProjectDir());
    int dirlength = d.absPath().length()+1;

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
    QDir::setCurrent(prj->getProjectDir());
    shell_process.clearArguments();
    shell_process << "kdoc";
    shell_process << "-p -d '" + prj->getProjectDir() + prj->getProjectName().lower() +  "-api'";
    if (!idx_path.isEmpty() && !link.isEmpty())
    {
      shell_process << "-L" << idx_path;
      shell_process << link;
    }

    bool bCreateKDoc;
    config->setGroup("General Options");
    bCreateKDoc = config->readBoolEntry("CreateKDoc", false);
    if (bCreateKDoc)
     shell_process << QString("-n ")+prj->getProjectName();

    if (!sources.isEmpty())
        shell_process << sources;

    next_job="fv_refresh";
    shell_process.start(KShellProcess::NotifyOnExit,KShellProcess::AllOutput);
    beep=true;
}

//MB
#include <vector>

void CKDevelop::slotConfigureDoxygen(){
    // check for Doxyfile
    KShellProcess process;
  QString dir =prj->getProjectDir();
  QString file= prj->getProjectDir()+"/"+prj->getProjectName().lower()+".doxygen";
  if(!QFileInfo(file).exists())
  {
    // create default
    process.clearArguments();
    process << QString("cd '")+ dir + "' && ";
    process << "doxygen -s -g "+prj->getProjectName().lower()+".doxygen";
    process.start(KProcess::Block,KProcess::AllOutput);

    // fill file with default projectname directories, etc.
     QFile f( file );
    if ( !f.open( IO_ReadOnly ) )
        return;
    QTextStream t(&f);
    std::vector<QString> vec;
    while ( !t.eof() )
    {
      QString s = t.readLine();
        vec.push_back(s);
        if (s.find("#") == 0)
            continue;
        if (s.find("OUTPUT_DIRECTORY") == 0)
            vec.back() = QString("OUTPUT_DIRECTORY\t=")+dir+prj->getProjectName().lower()+"-api/";
        if (s.find("TAB_SIZE") == 0)
            vec.back() = "TAB_SIZE\t=4";
        if (s.find("WARN_IF_UNDOCUMENTED") == 0)
            vec.back() = "WARN_IF_UNDOCUMENTED\t=NO";
        if (s.find("INPUT") == 0)
            vec.back() = QString("INPUT\t=") + dir;
        if (s.find("FILE_PATTERNS") == 0)
            vec.back().append("*.h \\ \n *.hh \\ \n *.H \\ \n " );
        if (s.find("RECURSIVE") == 0)
            vec.back() = "RECURSIVE\t=YES";
         if (s.find("ALPHABETICAL_INDEX") == 0)
            vec.back() = "ALPHABETICAL_INDEX\t=YES";
        if (s.find("GENERATE_LATEX") == 0)
            vec.back() = "GENERATE_LATEX\t=NO";
        if (s.find("GENERATE_RTF") == 0)
            vec.back() = "GENERATE_RTF\t=NO";
        if (s.find("GENERATE_MAN") == 0)
            vec.back() = "GENERATE_MAN\t=NO";
    }
    f.close();
    t.unsetDevice();
    f.open(IO_WriteOnly);
    t.setDevice(&f);
    for (unsigned i=0; i<vec.size(); ++i)
        t << vec[i] << "\n";
    f.close();
    t.unsetDevice();
     }
    // doxywizard ?
    if(!CToolClass::searchInstProgram("doxywizard")) // no dialog
    {
       KMessageBox::error(0,
            QString("doxwizard ") +i18n(" is not necessary, but you have to edit your Configuration for doxygen by hand.\nMaybe you should look for a newer Version at:\n\n\t http://www.stack.nl/~dimitri/doxygen/download.html\n\n"),
                            i18n("Program not found -- doxywizard "));
      return;
    }

  KShellProcess    shell_process;
  shell_process.clearArguments();
  shell_process << QString("cd '")+ dir + "' && ";
  shell_process << "doxywizard "+prj->getProjectName().lower()+".doxygen";
  shell_process.start(KProcess::DontCare);
}

//MB end

void CKDevelop::slotProjectManual(){

 if(prj->isKDE2Project()){
    slotDebugStop();
      showOutputView(true);
      setToolMenuProcess(false);
      slotFileSaveAll();
      slotStatusMsg(i18n("Creating project Manual..."));
      messages_widget->start();
    shell_process.clearArguments();
    shell_process << "cd '"+prj->getProjectDir()+"/doc"+"' && ";
    shell_process << make_cmd;
    next_job="fv_refresh";
    shell_process.start(KProcess::NotifyOnExit,KProcess::AllOutput);
    beep=true;
    return;
  }

  CMakeManualDlg dlg(this,"tesr",prj->getSGMLFile());
  if(dlg.exec()){

  slotDebugStop();
    showOutputView(true);
    setToolMenuProcess(false);
    //  slotFileSaveAll();
    slotStatusMsg(i18n("Creating project Manual..."));
    messages_widget->start();

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
        shell_process.clearArguments();
        shell_process << "cd '"+info.dirPath()+"' && ";
    if(ksgml){
        shell_process << "ksgml2html";
        shell_process << info.fileName();
        shell_process << "en";
    }
    else{
        shell_process << "sgml2html";
        shell_process << info.fileName();
    }
        next_job="fv_refresh";
        shell_process.start(KProcess::NotifyOnExit,KProcess::AllOutput);
        beep=true;
    }
}

void CKDevelop::slotProjectMakeDistSourceTgz(){

  slotDebugStop();
  showOutputView(true);
//  error_parser->toogleOff();
  setToolMenuProcess(false);
  slotFileSaveAll();
  slotStatusMsg(i18n("Running make dist..."));
  messages_widget->start();
  QDir::setCurrent(prj->getProjectDir());
  shell_process.clearArguments();
  shell_process << make_cmd << " dist";
  next_job="fv_refresh";
  shell_process.start(KProcess::NotifyOnExit,KProcess::AllOutput);
  beep = true;
}

void CKDevelop::slotProjectMakeDistRPM(){
 kdDebug() << "Lets build an RPM!!!" << endl;


 rpmbuilder = new Kpp(0,"KPP",true);
 rpmbuilder->setProjectRoot(prj->getProjectDir());
 connect(rpmbuilder, SIGNAL(finished()), this, SLOT(slotdoneWithKpp()));
 connect(rpmbuilder, SIGNAL(building()), this, SLOT(slotrpmBuildProcess()));
 connect(rpmbuilder, SIGNAL(stdERR(QString)), this, SLOT(slotGetRPMBuildSTDERR(QString)));
 connect(rpmbuilder, SIGNAL(stdOUT(QString)), this, SLOT(slotGetRPMBuildSTDOUT(QString)));
 connect(rpmbuilder, SIGNAL(newSpec(QString)), this, SLOT(slotAddSpec(QString)));
    QStrList shortInfo = prj->getShortInfo();
    QString qsShortInfo = "";
    for ( uint idx= 0; idx < shortInfo.count();  idx++ )
        qsShortInfo += shortInfo.at(idx);
        rpmbuilder->setProjectData(
                prj->getProjectName(),
                                prj->getVersion(),
                                prj->getAuthor(),
                                prj->getEmail(),
                                prj->getConfigureArgs(),
                                qsShortInfo,
                prj->getKPPRPMVersion(),
                prj->getKPPLicenceType(),
                prj->getKPPURL(),
                prj->getKPPAppGroup(),
                prj->getKPPBuildRoot(),
                prj->getKPPSummary(),
                prj->getKPPIcon());
    rpmbuilder->startBuild();
}

void CKDevelop::slotConfigMakeDistRPM()
{
  slotStatusMsg(i18n("Configuring RPMS..."));

 rpmbuilder = new Kpp(0,"KPP",true);
 rpmbuilder->setProjectRoot(prj->getProjectDir());
 connect(rpmbuilder, SIGNAL(finished()), this, SLOT(slotdoneWithKpp()));
 connect(rpmbuilder, SIGNAL(building()), this, SLOT(slotrpmBuildProcess()));
 connect(rpmbuilder, SIGNAL(stdERR(QString)), this, SLOT(slotGetRPMBuildSTDERR(QString)));
 connect(rpmbuilder, SIGNAL(stdOUT(QString)), this, SLOT(slotGetRPMBuildSTDOUT(QString)));
 connect(rpmbuilder, SIGNAL(newSpec(QString)), this, SLOT(slotAddSpec(QString)));
    QStrList shortInfo = prj->getShortInfo();
    QString qsShortInfo = "";
    for ( uint idx= 0; idx < shortInfo.count();  idx++ )
        qsShortInfo += shortInfo.at(idx);
 rpmbuilder->setProjectData(    prj->getProjectName(),
                                prj->getVersion(),
                                prj->getAuthor(),
                                prj->getEmail(),
                                prj->getConfigureArgs(),
                                qsShortInfo,
                prj->getKPPRPMVersion(),
                prj->getKPPLicenceType(),
                prj->getKPPURL(),
                prj->getKPPAppGroup(),
                prj->getKPPBuildRoot(),
                prj->getKPPSummary(),
                prj->getKPPIcon());
    rpmbuilder->show();
}


void CKDevelop::slotProjectLoadTags()
{
  kdDebug() << "in slotProjectLoadTags()\n";
  slotStatusMsg(i18n("Loading tags file..."));
  // must have a project for this
  CProject* pPrj = getProject();
  if (!pPrj) {
    bCTags=false;
    return;
  }
  // check whether to reload a CTagsDataBase
  if (bCTags && pPrj->ctagsDataBase().is_initialized()) {
      slotProjectMakeTags();
      // change status if reload failes for some reason
      if (!pPrj->ctagsDataBase().reload())
          bCTags=false;
      kdDebug() << "reloading tags file completed\n";
      return;
  }
  QString filename = pPrj->getProjectDir() + "/tags";
  if (!QFileInfo(filename).exists()) {
    // warn user that the tag file does not exist
    if (KMessageBox::questionYesNo(this,
      i18n("There is no tags file in the project directory\n"
           "Would you like to create a tags file for your project now?\n")) == KMessageBox::Yes)
    {
      slotProjectMakeTags();
    }
    else
    {
      KMessageBox::sorry(this,
        i18n("Some ctags based features like\n"
        "\"Goto Definition\" and \"Goto Declaration\"\n"
        "will not be available\n"));
      bCTags = false;
      return;
    }
  }
  if (pPrj->ctagsDataBase().load(filename)) {
      bCTags = true;
      kdDebug() << "loading tags file completed\n";
  }
  else {
      bCTags = false;
      kdDebug() << "loading tags file failed\n";
  }
  slotStatusMsg(i18n("Ready."));
}

void CKDevelop::slotProjectMakeTags()
{
  slotStatusMsg(i18n("Creating tags file..."));
  if(!CToolClass::searchInstProgram("ctags")) // no dialog
  {
    KMessageBox::sorry(0,
      i18n("KDevelop needs \"Exuberant Ctags\" \n for tags based search functionality."),
      i18n("You can find it at: http://ctags.sourceforge.net"));
    return;
  }
  create_tags();
}

void CKDevelop::slotAddSpec(QString path)
{

        QString namelow = (prj->getProjectName()).lower();
        QStrList myList;
        prj->getAllFiles(myList);
        //if(myList.contains(namelow+".spec"))
        {
          kdDebug() << "I need to add " << path << " to the project..." << endl;
          TFileInfo fileInfo;
          fileInfo.rel_name = namelow + ".spec";
          fileInfo.type = DATA;
          fileInfo.dist = true;
          fileInfo.install = false;
          fileInfo.install_location = "";
          prj->addFileToProject (namelow + ".spec",fileInfo);
        }
}
void CKDevelop::slotGetRPMBuildSTDOUT(QString sstdout){
        kdDebug() << sstdout << endl;
                messages_widget->insertAtEnd(sstdout);
}
void CKDevelop::slotGetRPMBuildSTDERR(QString sstderr){
        kdDebug() << sstderr << endl;
                stderr_widget->insertAtEnd(sstderr);
}
void CKDevelop::slotdoneWithKpp(){
 rpmbuilder->hide();

  setToolMenuProcess(true);
  slotStatusMsg(i18n("RPMS Finished"));
  beep = true;
    //save prefs here
    prj->setKPPAppGroup(rpmbuilder->getAppGroup());
    prj->setKPPBuildRoot(rpmbuilder->getBuildRoot());
    prj->setKPPIcon(rpmbuilder->getIcon());
    prj->setKPPLicenceType(rpmbuilder->getLicense());
    prj->setKPPRPMVersion(rpmbuilder->getVersion());
    prj->setKPPSummary(rpmbuilder->getSummary());
    prj->setKPPURL(rpmbuilder->getURL());

 delete rpmbuilder;
}

void CKDevelop::slotrpmBuildProcess(){
  slotDebugStop();
  showOutputView(true);
//  error_parser->toogleOff();
  setToolMenuProcess(false);
  slotFileSaveAll();
  slotStatusMsg(i18n("Building RPMS..."));
  messages_widget->start();
  beep = true;
  rpmbuilder->hide();
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
void CKDevelop::newFile(bool add_to_project, const char* dir/*=0*/){
  ProjectFileType type;
  bool new_subdir=false;
  QString complete_filename;
  CNewFileDlg dlg(prj,this,"test",true,0);
  if(dir)
    dlg.setLocation(dir);

  dlg.setUseTemplate();
  if (add_to_project){
    dlg.setAddToProject();
  }
  if(!dlg.exec()) return; // cancel

  complete_filename = dlg.location() + dlg.fileName();

  // Get the filetype.
  type = CProject::getType( complete_filename );
  // load into the widget
  switchToFile(complete_filename);

  // add the file to the project if necessary
  if (dlg.addToProject() == true){
    new_subdir = addFileToProject(complete_filename,type);
  }
  if(new_subdir){
    newSubDir();
  }
  if(prj->getProjectType()=="normal_gnome" && !new_subdir){
    slotBuildAutoconf();
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
  //  kdDebug() << "getProDir():" << prj->getProjectDir() << endl;
  //  kdDebug() << "*rel_name2*:" << rel_name << endl;

  TFileInfo info;
  info.rel_name = rel_name;
  info.type = type;
  info.dist = ( type != PO );

  info.install=false;
  info.install_location = "";
  new_subdir = prj->addFileToProject(rel_name,info);

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

CProject* CKDevelop::prepareToReadProjectFile(QString file)
{
  CProject* lNewProject = new CProject(file);
    if (!(lNewProject->prepareToReadProject())) {
    delete lNewProject;
    return 0L;
    }
    return lNewProject;
}

void CKDevelop::readProjectFile(QString file, CProject* lNewProject)
{
    // everything seems to be OK
  lNewProject->readProject();

  QString str;

  // Ok - valid project file - we hope
  project=true;
  prj = lNewProject;

  // set the top level and autoconf makefiles
  lNewProject->setTopMakefile();
  lNewProject->setCvsMakefile();

// TODO: Add function to read last opened files from project to restore project workspace
  // the whole Workspace concept was never really implemented I suppose
  // so let's mark it for removal
  //switchToWorkspace(prj->getCurrentWorkspaceNumber());

  // doc menu
  enableCommand(ID_HELP_PROJECT_API);
  enableCommand(ID_HELP_USER_MANUAL);
  // build menu
  setToolMenuProcess(true);

  // prj menu
  enableCommand(ID_PROJECT_CLOSE);
  enableCommand(ID_PROJECT_ADD_FILE_EXIST);

  if (prj->isKDEProject() || prj->isKDE2Project() || prj->isQt2Project()){
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

  enableCommand(ID_PROJECT_REMOVE_FILE);
//  enableCommand(ID_PROJECT_WORKSPACES);
  enableCommand(ID_BUILD_AUTOCONF);
  enableCommand(ID_PROJECT_MAKE_DISTRIBUTION);
  enableCommand(ID_CV_TOOLBAR_COMPILE_CHOICE);

  if (prj->getProjectType()!="normal_c" && prj->getProjectType()!="normal_objc")  // activate class wizard unless it is a normal C project
  {
    enableCommand(ID_PROJECT_NEW_CLASS);
    enableCommand(ID_CV_WIZARD);
    enableCommand(ID_CV_GRAPHICAL_VIEW);
    enableCommand(ID_CV_TOOLBAR_CLASS_CHOICE);
    enableCommand(ID_CV_TOOLBAR_METHOD_CHOICE);
  }
}


void  CKDevelop::saveCurrentWorkspaceIntoProject(){
  TWorkspace current;

  // save the current workspace
  current.id = workspace;
  prj->writeWorkspace(current);
}

void CKDevelop::newSubDir(){
  if(prj->getProjectType() == "normal_empty"){
    return; // no makefile handling
  }
  KMessageBox::information(0,i18n("You have added a new subdir to the project.\nWe will regenerate all Makefiles now."));
  setToolMenuProcess(false);
  slotStatusMsg(i18n("Running automake/autoconf and configure..."));
  messages_widget->start();
  showOutputView(true);
  QDir::setCurrent(prj->getProjectDir());
  shell_process.clearArguments();

  QString shell = getenv("SHELL");
  QString flagclabel;
  QString flagcpplabel;
  if(shell == "/bin/bash"){
      flagclabel= "CFLAGS=\"";
      flagcpplabel = "CXXFLAGS=\"";
  }
  else{
    flagclabel="env CFLAGS=\"";
    flagcpplabel= "env CXXFLAGS=\"";
  }
        QStringList configs;
        QString shellcommand="";
        // get the current config to set it as the config in the project options dialog
         KComboBox* compile_combo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_COMPILE_CHOICE);
         QString curr=compile_combo->currentText();
                 // refill the compile configs combobox
                 curr=compile_combo->currentText();
                 compile_combo->clear();
                 configs=m_pKDevSession->getCompileConfigs();
                 configs.prepend(i18n("(Default)"));
                 compile_combo->insertStringList(configs);
                 int idx=configs.findIndex(curr); // find the index for the last config
                 compile_combo->setCurrentItem(idx);
                 compile_combo->setEnabled(true);

                //////////////////////////////////////
    prj->updateConfigureIn();
    KMessageBox::information(0,i18n("You have added a new subdir to the project.\nWe will regenerate all Makefiles now."));
    setToolMenuProcess(false);
    slotStatusMsg(i18n("Running automake/autoconf and configure..."));
    messages_widget->start();
    showOutputView(true);
    QDir::setCurrent(prj->getProjectDir());
    QString makefile("Makefile.dist");
    if(!QFileInfo(QDir::current(), makefile).exists())
        makefile="Makefile.cvs";
    shellcommand += make_cmd + " -f "+makefile+" && ";

    shellcommand += prepareConfigureCommand();

    setToolMenuProcess(false);
    messages_widget->start();
    showOutputView(true);
    shell_process.clearArguments();
          shell_process << shellcommand;
                debug("run: %s\n", shellcommand.data());
          shell_process.start(KProcess::NotifyOnExit,KProcess::AllOutput);
}

/***************************************************************************/

SaveAllDialog::SaveAllDialog(const QString& filename, CProject* prj) :
  KDialog(0, 0, true)
{
  setCaption(i18n("Save Changed Project Files?"));

  QBoxLayout *topLayout = new QVBoxLayout(this, 5);

  QHBoxLayout * lay = new QHBoxLayout(topLayout);
  lay->setSpacing(KDialog::spacingHint()*2);
  lay->setMargin(KDialog::marginHint()*2);
  lay->addStretch(1);
  QLabel *label1 = new QLabel(this);
#if (QT_VERSION < 300)
  label1->setPixmap(QMessageBox::standardIcon(QMessageBox::Warning, kapp->style().guiStyle()));
#else
  label1->setPixmap(QMessageBox::standardIcon(QMessageBox::Warning));
#endif
  lay->add( label1 );
  QString title;
  if (prj) {
     title = prj->getProjectName();
  }
  else {
     title = "";
  }
  lay->add(  new QLabel(  i18n( "The project\n\n%1\n\n"
                                "contains changed files. Save modified file\n\n%2"
                                " ?\n\n").arg(title).arg(filename),
                          this) );


  lay->addStretch(1);

  KButtonBox *buttonbox = new KButtonBox(this, Qt::Horizontal, 5);
  QPushButton *yes      = buttonbox->addButton(i18n("Yes"));
  QPushButton *no       = buttonbox->addButton(i18n("No"));
  QPushButton *saveAll  = buttonbox->addButton(i18n("Save All"));
  QPushButton *cancel   = buttonbox->addButton(i18n("Cancel"));
  buttonbox->layout();
  topLayout->addWidget(buttonbox);

  connect(yes,      SIGNAL(clicked()),  SLOT(yes()));
  connect(no,       SIGNAL(clicked()),  SLOT(no()));
  connect(saveAll,  SIGNAL(clicked()),  SLOT(saveAll()));
  connect(cancel,   SIGNAL(clicked()),  SLOT(cancel()));

  topLayout->activate();
}

/***************************************************************************/

SaveAllDialog::~SaveAllDialog()
{
}

/***************************************************************************/

SaveAllDialog::SaveAllResult SaveAllDialog::result()
{
  return m_result;
}

/***************************************************************************/

void SaveAllDialog::yes()
{
  m_result = Yes;
  accept();
}

/***************************************************************************/

void SaveAllDialog::no()
{
  m_result = No;
  accept();
}

/***************************************************************************/

void SaveAllDialog::saveAll()
{
  m_result = SaveAll;
  accept();
}

/***************************************************************************/

void SaveAllDialog::cancel()
{
  m_result = Cancel;
  accept();
}

/***************************************************************************/
/**
 * Switch between corresponding source and header files. Assumes that
 * the files exist and that they have the same basename.
 */
void CKDevelop::slotTagSwitchTo()
{
  Kate::View* pEditView = m_docViewManager->currentEditView();
  if (!pEditView) return;
  slotStatusMsg(i18n("Switch between Source and Header Files..."));
  bool useCTags = (bCTags && hasProject())?true:false ;
  bool bFoundInCTags=false;

  // docName is missing in KTextEditor interface so we need to
  // do this hack (rokrau 03/21/02)
  Kate::Document* pDoc = dynamic_cast<Kate::Document*>
                         (pEditView->document());
  QFileInfo curFileInfo = QFileInfo(pDoc->docName());
  QString curFileName = curFileInfo.fileName();
  QString curFileDir = curFileInfo.dirPath();
  QString curFileExt = curFileInfo.extension(FALSE);
  QString switchToName = curFileInfo.baseName();

  QStringList srcExtensions, headerExtensions, *extensionList=0l;;
  QString newExtension;

  srcExtensions << ".cpp" << ".cxx" << ".C" << ".cc" << ".ecpp" << ".c" << ".ec"
                << ".inl" << ".m";
  headerExtensions << ".h" << ".hpp" << ".hxx" << ".H" << ".hh" << ".tlh";

  kdDebug() << "in CKDevelop::slotTagSwitchTo():" << endl;
  kdDebug() << "current filename: " << curFileDir << "/" << curFileName << " (" << switchToName<< ")" << endl;

  if (m_docViewManager->curDocIsHeaderFile())
  {
    extensionList=&srcExtensions;
  }

  if (m_docViewManager->curDocIsCppFile())
  {
    extensionList=&headerExtensions;
  }

  if (extensionList)
  {
    if (useCTags)
    {
      int ntags=0;
      for (QStringList::Iterator it=extensionList->begin();
           newExtension.isEmpty() && it!=extensionList->end(); ++it)
      {
           // should be fixed... it cannot find a tag, but they exist
           ctags_dlg->searchTags(switchToName+(*it),&ntags);
           if (ntags)
           {
              newExtension=*it;
              bFoundInCTags=true;
           }
      }
    }

    if (!newExtension.isEmpty())
    {
      switchToName = switchToName + newExtension;
    }
    else
    {
      for (QStringList::Iterator it=extensionList->begin();
           newExtension.isEmpty() && it!=extensionList->end(); ++it)
      {
        if (QFile::exists(curFileDir+"/"+switchToName+(*it)))
        {
          newExtension=*it;
        }
      }

      if (!newExtension.isEmpty())
        switchToName = switchToName + newExtension;
    }
  }

  if (!newExtension.isEmpty())
  {

    kdDebug() << "switch to filename: " << switchToName << endl;

    // we can do this the easy...
    if (bFoundInCTags)
    {
      kdDebug() << "lookup file using CTags database, fast." << endl;
      ctags_dlg->slotGotoFile(switchToName);
    }
    // ...or the hard way
    else
    {
      kdDebug() << "file was found in tree." << endl;
      switchToFile(curFileDir+"/"+switchToName);
    }
  }
  slotStatusMsg(i18n("Ready."));
}
void CKDevelop::slotTagSearch()
{
  slotStatusMsg(i18n("Searching in CTags Database..."));
  ctags_dlg->slotClear();
  ctags_dlg->setTagType(searchTagsDialogImpl::all);
  ctags_dlg->show();
  ctags_dlg->raise();
  slotStatusMsg(i18n("Ready."));
}
void CKDevelop::slotSwitchDocTool()
{
        KRadioAction* pDoxyAction = dynamic_cast<KRadioAction*>
                                     (actionCollection()->action("project_api_doxygen"));
        KRadioAction* pKdocAction = dynamic_cast<KRadioAction*>
                                     (actionCollection()->action("project_api_kdoc"));
        KAction* pDoxyConfAction = actionCollection()->action("project_api_doxyconf");
        // kdoc used, can we switch to doxygen ?
        if (pDoxyAction->isChecked())
        {
                if(!CToolClass::searchInstProgram("doxygen"))
                {
                        KMessageBox::error(0,
                        i18n(" This option requires Doxygen to work. Look for it at:\n\n http://www.stack.nl/~dimitri/doxygen/download.html\n"),
                        i18n("Program not found -- doxygen"));
                        // no doxygen found
                        return;
                }
                // yes, we have it
                //pKdocAction->setChecked(false);
                //pDoxyAction->setChecked(true);
                if (hasProject()) pDoxyConfAction->setEnabled(true);
                doctool = DT_DOX;
                return;
        } else // kdoc
        {
                //pKdocAction->setChecked(false);
                //pDoxyAction->setChecked(true);
                pDoxyConfAction->setEnabled(false);
                doctool = DT_KDOC;
        }
}
//MB end
