/***************************************************************************
                          kdejavaprojectspace.cpp
                             -------------------
    begin                : Sat May 13 2000
    copyright            : (C) 2000 by Sandy Meier
    email                : smeier@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kdejavaprojectspace.h"
#include "kdevmakefrontend.h"
#include "main.h"
#include <kinstance.h>
#include <kiconloader.h>
#include <ksimpleconfig.h>
#include <kstddirs.h>
#include <klocale.h>
#include <kaction.h>
#include <kpopupmenu.h>
#include <kdebug.h>
#include <qfileinfo.h>
#include <kdebug.h>
#include <qdir.h>
#include <ctoolclass.h>
#include <qdatetime.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <qtextstream.h>
#include <kdevactions.h>
#include <kpopupmenu.h>
#include <kaboutdata.h>
#include <ktrader.h>
#include <klibloader.h>
#include <qlineedit.h>
#include <iostream.h>
#include "kdevmakefrontend.h"
#include "KDevComponentManager.h"



KDEJavaProjectSpace::KDEJavaProjectSpace(QObject* parent,const char* name)
		: AutomakeProjectSpace(parent,name)
{
	kdDebug(9020) << "KDEJavaProjectSpace created" << endl;
	
	setInstance(KDEJavaProjectSpaceFactory::instance());

	KStandardDirs* std_dirs = KGlobal::dirs();
	

	// projectspace
	m_projectspaceTemplate = std_dirs->findResource("data","kdevelop/projectspaces/kdejava_projectspace.tar.gz");
	m_language = "Java";
	m_pAboutData=0;


	setXMLFile("kdevkdejavaprojectspaceui.rc");
}

KDEJavaProjectSpace::~KDEJavaProjectSpace(){
}

KDevMakeFrontend* KDEJavaProjectSpace::makeFrontend(){
	return static_cast<KDevMakeFrontend*>(componentManager()->component("KDevMakeFrontend"));
}

void KDEJavaProjectSpace::setupGUI(){

  AutomakeProjectSpace::setupGUI(); // set actions for "set active project"
  kdDebug(9020) << "Building KDEJavaProjectSpace GUI" << endl;
  KAction *pAction;
  pAction = new KAction( i18n("Add new &Translation File..."), "locale", 0,
			 this, SLOT( slotProjectAddNewTranslationFile() ),
			 actionCollection(), "project_add_translation");
  pAction->setEnabled(true);
  pAction->setStatusText( i18n("Adds a new language for internationalization to the project") );

  //++++++++++++++++++++++++++
  pAction = new KAction( i18n("&Make"), "make", 0,
			 this, SLOT( slotBuildMake()),
			 actionCollection(), "build_make");
  pAction->setEnabled(true);
  pAction->setStatusText( i18n("Invokes make-command") );
  pAction->setWhatsThis( i18n("Make\n\n"
			      "Invokes the make-command set in the "
			      "options-menu for the current project "
			      "after saving all files. "
			      "This will compile all changed sources "
			      "since the last compilation was invoked.\n"
			      "The output window opens to show compiler "
			      "messages. If errors occur, clicking on the "
			      "error line will open the file where the "
			      "error was found and sets the cursor to the "
			      "error line.") );

  //+++++++++++++++++++++++++++
  pAction = new KAction( i18n("&Rebuild all"), "rebuild", 0,
			 this, SLOT( slotBuildRebuildAll()),
			 actionCollection(), "build_rebuild_all");
  pAction->setEnabled(true);
  pAction->setStatusText( i18n("Rebuilds the project") );
  pAction->setWhatsThis( i18n("Rebuild all\n\n"
					  "After saving all files, rebuild all "
					  "invokes the make-command set with the "
					  "clean-option to remove all object files. "
					  "Then, configure creates new Makefiles and "
					  "the make-command will rebuild the project.") );

  //+++++++++++++++++++++++++++
  pAction = new KAction( i18n("&Clean/Rebuild all"), 0,
			 this, SLOT( slotBuildCleanRebuildAll()),
			 actionCollection(), "build_clean_rebuild_all");
  pAction->setEnabled(true);
  pAction->setStatusText( i18n("Invokes make clean and rebuild all") );

  //+++++++++++++++++++++++++++
  pAction = new KAction( i18n("&Execute"),"run", 0,
			 this, SLOT( slotBuildExecute()),
			 actionCollection(), "build_execute");
  pAction->setEnabled(true);
  pAction->setStatusText( i18n("Invokes make-command and runs the program") );
  pAction->setWhatsThis( i18n("Execute\n\n"
			      "After saving all files,the make-command is "
			      "called to build the project. Then the binary "
			      "is executed out of the project directory.\n"
			      "Be aware that this function is only valid for "
			      "programs and that references to e.g. pixmaps "
			      "or html help files that are supposed to be "
			      "installed will cause some strange behavoir "
			      "like testing the helpmenu will open an error "
			      "message that the index.html file is not found.") );
  //++++++++++++++++++++++++++++
  pAction = new KAction( i18n("Execute &with Arguments..."),"run", 0,
			 this, SLOT( slotBuildExecuteWithArgs()),
			 actionCollection(), "build_execute_with_args");
  pAction->setEnabled(true);
  pAction->setStatusText( i18n("Lets you set run-arguments to the binary and invokes the make-command") );

  //+++++++++++++++++++++++++++
  pAction = new KAction( i18n("DistC&lean"), 0,
			 this, SLOT( slotBuildDistClean()),
			 actionCollection(), "build_dist_clean");
  pAction->setEnabled(true);
  pAction->setStatusText( i18n("Invokes make distclean and deletes all compiled files") );
  //+++++++++++++++++++++++++++
  pAction = new KAction( i18n("&Autoconf and automake"), 0,
			 this, SLOT( slotBuildAutoconf()),
			 actionCollection(), "build_autoconf");
  pAction->setEnabled(true);
  pAction->setStatusText( i18n("Invokes automake and co.") );


  //+++++++++++++++++++++++++++
  pAction = new KAction( i18n("C&onfigure..."), 0,
			 this, SLOT( slotBuildConfigure()),
			 actionCollection(), "build_configure");
  pAction->setEnabled(true);
  pAction->setStatusText( i18n("Invokes ./configure") );
}

// slots

void KDEJavaProjectSpace::slotProjectAddNewTranslationFile(){
    kdDebug(9020) << "Add new &Translation File (KDE)... actived" << endl;
}


void KDEJavaProjectSpace::updateAdminFiles(){
  cerr << "\nkdevelop (KDE Java ProjectSpace) enter KDEJavaProjectSpace::updateAdminFiles()";
  QStringList dirs = allDirectories();
  QStringList makefileDirs; // all directories under the projectspace root (automake can only handle this)

  QFile file(m_path + "/configure.in.in");
    // save
    if ( file.open(IO_WriteOnly) ){
      QTextStream tw( &file );
      tw << "dnl *************************************************\n";
      tw << "dnl generated by KDevelop (http://www.kdevelop.org)\n";
      tw << "dnl KDE Java ProjectSpace plugin (c) 2000 by KDevelop Team\n";
      tw << "dnl\n";
      tw << "dnl Please do not edit this file!\n";
      tw << "dnl *************************************************\n\n";
      tw << "AM_INIT_AUTOMAKE(" + m_name + "," + m_version +")\n\n";

      tw << "dnl  This are created by configure\n";
      tw << "AC_OUTPUT(\n";


      for ( QStringList::Iterator it = dirs.begin(); it != dirs.end(); ++it ) {
	if((*it).startsWith(m_path)){
	  makefileDirs.append((*it));
	}
      }
      int length = m_path.length();
      for ( QStringList::Iterator it = makefileDirs.begin(); it != makefileDirs.end(); ++it ) {
	if((*it) == m_path){ // the root dir
	  tw << "Makefile" << " \\" << "\n";
	}
	else{
	  tw << (*it).mid(length+1) << "/Makefile" << " \\" << "\n";
	}
      }
      tw << ")";
      file.close();
    }
    else {
      cerr << "\nERROR! couldn't open file to write:" << file.name();
    }

    QString makefile;
    QStringList makefileContent; // the complete Makefile.am
    QString kdevelopBegin = "####### kdevelop will overwrite this part!!! (begin)##########";
    QString kdevelopEnd =   "####### kdevelop will overwrite this part!!! (end)############";
    QString line;
    bool kdevelopArea = false;
    QList<Project>* pProjects = allProjects();
    Project* pProject=0;

    // update all Makefiles
    for ( QStringList::Iterator it = makefileDirs.begin(); it != makefileDirs.end(); ++it ) {
      makefile = (*it)+ "/Makefile.am";
      //read the makefile
      cerr << "\nUPDATE makefile: " << makefile;
      makefileContent.clear();
      file.setName(makefile);
      if(file.open(IO_ReadOnly)){ // read the makefileam,without the kdevelop area
	QTextStream stream( &file );
	while(!stream.eof()){
	  line = stream.readLine();
	  if(line == kdevelopBegin){
	    kdevelopArea = true;
	  }
	  else if (line == kdevelopEnd){
	    kdevelopArea = false;
	  }
	  else if(!kdevelopArea){
	    makefileContent.append(line);
	  }
	}
      }
      else {
	cerr << "\nERROR! couldn't open file to read:" << file.name();
      }
      file.close();

      if(file.open(IO_WriteOnly)){
	QTextStream stream( &file );
	
	// write the kdevelop area
	stream << kdevelopBegin << "\n";
	
	// every project can write it targets to the Makefile
	stream << "\nbin_PROGRAMS = ";
	for(pProject = pProjects->first();pProject != 0;pProject = pProjects->next()){
	  pProject->updateMakefile((*it),stream,"bin_PROGRAMS");
	}
	stream << "\ninstall-data-local:";
	for(pProject = pProjects->first();pProject != 0;pProject = pProjects->next()){
	  pProject->updateMakefile((*it),stream,"install-data-local:");
	}
	stream << "\nuninstall-local:";
	for(pProject = pProjects->first();pProject != 0;pProject = pProjects->next()){
	  pProject->updateMakefile((*it),stream,"uninstall-local:");
	}
	for(pProject = pProjects->first();pProject != 0;pProject = pProjects->next()){
	  pProject->updateMakefile((*it),stream);
	}
	stream << "\n" << kdevelopEnd << "\n";
	
	
	// write the rest
	for ( QStringList::Iterator it = makefileContent.begin(); it != makefileContent.end(); ++it ) {
	  stream << (*it) + "\n";
	}
      } // end if
      else {
	cerr << "\nERROR! couldn't open file to write:" << file.name();
      }
      file.close();
    }
}

QStringList KDEJavaProjectSpace::allDirectories(){
  QList<Project>* pProjects = allProjects();
  Project* pProject=0;
  QStringList dirs;
  QFileInfo info;

  dirs.append(m_path);
  for(pProject=pProjects->first();pProject!=0;pProject = pProjects->next()){
    QStringList files = pProject->allAbsoluteFileNames();
    for ( QStringList::Iterator it = files.begin(); it != files.end(); ++it ) {
      info.setFile(*it);
      if(dirs.contains(info.dirPath()) == 0 ){ // not in the list
	dirs.append(info.dirPath());
      }
    }
  }
  for ( QStringList::Iterator it = dirs.begin(); it != dirs.end(); ++it ) {
    cerr << "\nDIR:" << *it;
  }
  return dirs;
}

 /** add the data to the psElement (Projectspace)*/
bool KDEJavaProjectSpace::writeGlobalConfig(QDomDocument& doc,QDomElement& psElement){
  AutomakeProjectSpace::writeGlobalConfig(doc,psElement);
  QDomElement kdeElement = psElement.appendChild(doc.createElement("KDEJavaProjectSpace")).toElement();
  return true;
}
bool KDEJavaProjectSpace::writeUserConfig(QDomDocument& doc,QDomElement& psElement){
  AutomakeProjectSpace::writeUserConfig(doc,psElement);
  QDomElement kdeElement = psElement.appendChild(doc.createElement("KDEJavaProjectSpace")).toElement();
  return true;
}

bool KDEJavaProjectSpace::readGlobalConfig(QDomDocument& doc,QDomElement& psElement){
  AutomakeProjectSpace::readGlobalConfig(doc,psElement);
  QDomElement kdeElement = psElement.namedItem("KDEJavaProjectSpace").toElement();
  if(kdeElement.isNull()){
    kdDebug(9020) << "KDEJavaProjectSpace::readGlobalConfig not \"KDEJavaProjectSpace\" tag found!" << endl;
    return false;
  }
  else{
    kdDebug(9020) << "KDEJavaProjectSpace::readGlobalConfig  \"KDEJavaProjectSpace\" tag found!" << endl;
  }
  return true;
}


bool KDEJavaProjectSpace::readUserConfig(QDomDocument& doc,QDomElement& psElement){
  AutomakeProjectSpace::readUserConfig(doc,psElement);
  QDomElement kdeElement = psElement.namedItem("KDEJavaProjectSpace").toElement();
  if(kdeElement.isNull()){
    kdDebug(9020) << "KDEJavaProjectSpace::readUserConfig not \"KDEJavaProjectSpace\" tag found!" << endl;
    return false;
  }
  else{
    kdDebug(9020) << "KDEJavaProjectSpace::readUserConfig  \"KDEJavaProjectSpace\" tag found!" << endl;
  }
  return true;
}


KAboutData* KDEJavaProjectSpace::aboutPlugin(){
  if (m_pAboutData == 0){
    m_pAboutData= new KAboutData( "KDE Java", I18N_NOOP("KDE Java ProjectSpace"),
				  "0.1", "kk",
				  KAboutData::License_GPL,
				  "(c) 1998-2001, The KDevelop Team",
				  "text",
				  "http://www.kdevelop.org");

    m_pAboutData->addAuthor("Richard Dale",I18N_NOOP("Developer"), "Richard_Dale@tipitina.demon.co.uk");
  }
  return m_pAboutData;
}
QList<FileGroup> KDEJavaProjectSpace::defaultFileGroups(){
  QList<FileGroup> list;
  FileGroup* pGroup = new FileGroup();
  pGroup->setName(i18n("Source Files"));
  pGroup->setFilter("*.java");
  list.append(pGroup);

  pGroup = new FileGroup();
  pGroup->setName(i18n("Dialogs Files"));
  pGroup->setFilter("*.ui");
  list.append(pGroup);

  pGroup = new FileGroup();
  pGroup->setName(i18n("Resource Files"));
  pGroup->setFilter("*.rc;*.png;*.gif;*.jpg;");
  list.append(pGroup);

  return list;
}
void KDEJavaProjectSpace::updateMakefilesAm(){
}
void KDEJavaProjectSpace::slotBuildMake(){
  updateAdminFiles();
    makeFrontend()->executeMakeCommand("make");
}
void KDEJavaProjectSpace::slotBuildRebuildAll(){
}
void KDEJavaProjectSpace::slotBuildCleanRebuildAll(){
    makeFrontend()->executeMakeCommand("make clean");
}
void KDEJavaProjectSpace::slotBuildExecute(){
  updateAdminFiles();
}
void KDEJavaProjectSpace::slotBuildExecuteWithArgs(){
}
void KDEJavaProjectSpace::slotBuildDistClean(){
    makeFrontend()->executeMakeCommand("make distclean");
}
void KDEJavaProjectSpace::slotBuildAutoconf(){
}
void KDEJavaProjectSpace::slotBuildConfigure(){
    makeFrontend()->executeMakeCommand("./configure");
}


#include "kdejavaprojectspace.moc"
