/***************************************************************************
                          kdeprojectspace.cpp
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

#include "kdeprojectspace.h"
#include "main.h"
#include <kinstance.h>
#include <iostream.h>
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



KDEProjectSpace::KDEProjectSpace(QObject* parent,const char* name)
		: AutomakeProjectSpace(parent,name){
	cerr << "\nKDEProjectSpace created\n";
	
	setInstance(KDEProjectSpaceFactory::instance());

	KStandardDirs* std_dirs = KGlobal::dirs();
	

	// projectspace
	m_projectspaceTemplate = std_dirs->findResource("data","kdevelop/projectspaces/kde_projectspace.tar.gz");
	m_language = "C++";
	m_pAboutData=0;


	setXMLFile("kdevkdeprojectspaceui.rc");
	//setXMLFile("/home/kde2/kdevelop/kdevelop/parts/kdeprojectspace/kdevkdeprojectspace.rc"); //test
}
KDEProjectSpace::~KDEProjectSpace(){
}

void KDEProjectSpace::setupGUI(){

  AutomakeProjectSpace::setupGUI(); // set actions for "set active project"
  cerr << "Building KDEProjectSpace GUI" << endl;
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

void KDEProjectSpace::slotProjectAddNewTranslationFile(){
  cerr << "\n Add new &Translation File (KDE)... actived";
}
void KDEProjectSpace::modifyDefaultFiles(){
  AutomakeProjectSpace::modifyDefaultFiles();
}

 /** add the data to the psElement (Projectspace)*/
bool KDEProjectSpace::writeGlobalConfig(QDomDocument& doc,QDomElement& psElement){
  AutomakeProjectSpace::writeGlobalConfig(doc,psElement);
  QDomElement kdeElement = psElement.appendChild(doc.createElement("KDEProjectSpace")).toElement();
  return true;
}
bool KDEProjectSpace::writeUserConfig(QDomDocument& doc,QDomElement& psElement){
  AutomakeProjectSpace::writeUserConfig(doc,psElement);
  QDomElement kdeElement = psElement.appendChild(doc.createElement("KDEProjectSpace")).toElement();
  return true;
}

bool KDEProjectSpace::readGlobalConfig(QDomDocument& doc,QDomElement& psElement){
  AutomakeProjectSpace::readGlobalConfig(doc,psElement);
  QDomElement kdeElement = psElement.namedItem("KDEProjectSpace").toElement();
  if(kdeElement.isNull()){
    cerr << "\nKDEProjectSpace::readGlobalConfig not \"KDEProjectSpace\" tag found!";
    return false;
  }
  else{
    cerr << "\nKDEProjectSpace::readGlobalConfig  \"KDEProjectSpace\" tag found!";
  }
  return true;
}
bool KDEProjectSpace::readUserConfig(QDomDocument& doc,QDomElement& psElement){
  AutomakeProjectSpace::readUserConfig(doc,psElement);
  QDomElement kdeElement = psElement.namedItem("KDEProjectSpace").toElement();
  if(kdeElement.isNull()){
    cerr << "\nKDEProjectSpace::readUserConfig not \"KDEProjectSpace\" tag found!";
    return false;
  }
  else{
    cerr << "\nKDEProjectSpace::readUserConfig  \"KDEProjectSpace\" tag found!";
  }
  return true;
}

KAboutData* KDEProjectSpace::aboutPlugin(){
  if (m_pAboutData == 0){
    m_pAboutData= new KAboutData( "KDE", I18N_NOOP("KDE ProjectSpace"),
				  "0.1", "kk",
				  KAboutData::License_GPL,
				  "(c) 1998-2000, The KDevelop Team",
				  "text",
				  "http://www.kdevelop.org");

    m_pAboutData->addAuthor("Sandy Meier",I18N_NOOP("Developer"), "smeier@kdevelop.org");
  }
  return m_pAboutData;
}
QList<FileGroup> KDEProjectSpace::defaultFileGroups(){
  QList<FileGroup> list;
  FileGroup* pGroup = new FileGroup();
  pGroup->setName(i18n("Source Files"));
  pGroup->setFilter("*.cpp;*.c;*.cc;*.cxx;*.C");
  list.append(pGroup);

  pGroup = new FileGroup();
  pGroup->setName(i18n("Header Files"));
  pGroup->setFilter("*.h;*.hxx");
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
void KDEProjectSpace::updateMakefilesAm(){
}
void KDEProjectSpace::slotBuildMake(){
    emit executeMakeCommand("make");
}
void KDEProjectSpace::slotBuildRebuildAll(){
}
void KDEProjectSpace::slotBuildCleanRebuildAll(){
  emit executeMakeCommand("make clean");
}
void KDEProjectSpace::slotBuildExecute(){
}
void KDEProjectSpace::slotBuildExecuteWithArgs(){
}
void KDEProjectSpace::slotBuildDistClean(){
  emit executeMakeCommand("make distclean");
}
void KDEProjectSpace::slotBuildAutoconf(){
}
void KDEProjectSpace::slotBuildConfigure(){
  emit executeAppCommand("./configure");
}

#include "kdeprojectspace.moc"
