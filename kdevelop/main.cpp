/***************************************************************************
                     main.cpp - the main-function
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


#include <kstddirs.h>
#include <kglobal.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include "kstartuplogo.h"
#include "ckdevinstall.h"
#include "ckdevelop.h"
#include "kwrite/kwview.h"



KGuiCmdManager cmdMngr; //manager for user -> gui commands

static const char *description=I18N_NOOP("The KDE Integrated Development Environment");
static const char *version="2.0pre";


static KCmdLineOptions options[] =
{
   { "setup", I18N_NOOP("Setup KDevelop"), 0 },
   { "+[file]", I18N_NOOP("The file to open"), 0 },
   { 0, 0, 0 }
};


int main(int argc, char* argv[])
{
  KAboutData aboutData( "kdevelop", I18N_NOOP("KDevelop"),
    version, description, KAboutData::License_GPL,
    "(c) 1998-2000, The KDevelop Authors");
  aboutData.addAuthor("Sandy Meier","Maintainer", "smeier@rz.uni-potsdam.de");
  aboutData.addAuthor("Stefab Heidrich","KAppWizard, Printing", "sheidric@rz.uni-potsdam.de");
  aboutData.addAuthor("Ralf Nolden","KDevelop<->Dialogeditor Interface, Configuration Functionality, Online Hhelp", "");
  aboutData.addAuthor("Jonas Nordin","Classviewer and Classparser", "jonas.nordin@syncom.se");
  aboutData.addAuthor("Pascal Krahmer","Dialogeditor", "pascal@beast.de");
  aboutData.addAuthor("Jörgen Olsson","Graphical Classviewer", "jorgen@trej.net");
  aboutData.addAuthor("Stefan Bartel","Real-File-Viewer, Project Options", "bartel@rz.uni-potsdam.de");
  aboutData.addAuthor("Bernd Gehrmann","CVS Support", "bernd@physik.hu-berlin.de");
  aboutData.addAuthor("Walter Tasin","Many, Many Bugfixes, General Enhancements", "tasin@e-technik.fh-muenchen.de");
  aboutData.addAuthor("John Birch","Internal Debugger", "jbb@ihug.co.nz");
  
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );
  
  
  KStartupLogo* start_logo=0L;
  KGuiCmdApp a(argc, argv);
  //	a.connectToKWM();

  // commands
  KWrite::addCursorCommands(cmdMngr);
  KWrite::addEditCommands(cmdMngr);
  KWrite::addFindCommands(cmdMngr);
  KWrite::addStateCommands(cmdMngr);
  cmdMngr.makeDefault();
  cmdMngr.readConfig(a.config());

  // Resource types
  KGlobal::dirs()->addResourceType("kdev_template", KStandardDirs::kde_default("data")
                                   + kapp->name() + "/templates/");
  KGlobal::dirs()->addResourceType("kdev_pic", KStandardDirs::kde_default("data")
                                   + kapp->name() + "/pics/");
  

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  
  a.config()->setGroup("General Options");
  bool bStartLogo= a.config()->readBoolEntry("Logo",true);
  bool bInstall=a.config()->readBoolEntry("Install",true);
  
  if (args->isSet("setup")) bInstall = true; // start the setupwizard
  
  if(bStartLogo){
    start_logo= new KStartupLogo;
    start_logo->show();
  }

  if (a.isRestored()){
    RESTORE(CKDevelop);
  }
  else{
    if(bInstall){
      CKDevInstall* install=new CKDevInstall(0,"install");
	if (bStartLogo) {
	start_logo->close();
	delete start_logo;
	bStartLogo = false;
      }
      install->show();
      delete install;
    }
    CKDevelop* kdevelop = new CKDevelop();
    if(bInstall){
      kdevelop->refreshTrees();  // this is because of the new documentation
    }
    kdevelop->show();
    
    if (!args->isSet("setup"))
	  kdevelop->slotProjectOpenCmdl(argv[1]);
    
    if(bStartLogo){
      start_logo->close();
      delete start_logo;
    }
  
    a.config()->setGroup("TipOfTheDay");
    bool showTip=a.config()->readBoolEntry("show_tod",true);
    if(showTip){
    	kdevelop->slotHelpTipOfDay();
    }
  }
  args->clear();
  
  int rc = a.exec();
  return rc;
}



