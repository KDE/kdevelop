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

#include "kdevelop.h"
#include "kdevelopfactory.h"


//KGuiCmdManager cmdMngr; //manager for user -> gui commands

//static const char *description=I18N_NOOP("The KDE Integrated Development Environment");
//static const char *version="2.0pre";


static KCmdLineOptions options[] =
{
   { "setup", I18N_NOOP("Setup KDevelop"), 0 },
   { "+[file]", I18N_NOOP("The file to open"), 0 },
   { 0, 0, 0 }
};


int main(int argc, char* argv[])
{

  KCmdLineArgs::init( argc, argv, KDevelopFactory::aboutData() );
  KCmdLineArgs::addCmdLineOptions( options );
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  KApplication a;
//  KStartupLogo* start_logo=0L;
//  KGuiCmdApp a(argc, argv);
  //	a.connectToKWM();

  // commands
//  KWrite::addCursorCommands(cmdMngr);
//  KWrite::addEditCommands(cmdMngr);
//  KWrite::addFindCommands(cmdMngr);
//  KWrite::addStateCommands(cmdMngr);
//  cmdMngr.makeDefault();
//  cmdMngr.readConfig(a.config());

//  a.config()->setGroup("General Options");
//  bool bStartLogo= a.config()->readBoolEntry("Logo",true);
//  bool bInstall=a.config()->readBoolEntry("Install",true);
//
//  if (args->isSet("setup")) bInstall = true; // start the setupwizard
//
//  if(bStartLogo){
//    start_logo= new KStartupLogo;
//    start_logo->show();
//  }

  if (a.isRestored()){
    RESTORE(KDevelop);
  }
  else{
//    if(bInstall){
//      CKDevInstall* install=new CKDevInstall(0,"install");
//	if (bStartLogo) {
//	start_logo->close();
//	delete start_logo;
//	bStartLogo = false;
//      }
//      install->show();
//      delete install;
//    }
  KDevelop *kdevelop = new KDevelop("kdevelop gui");
//    CKDevelop* kdevelop = new CKDevelop();
//    if(bInstall){
//      kdevelop->refreshTrees();  // this is because of the new documentation
//    }
    kdevelop->show();

//
//    if (!args->isSet("setup"))
//	  kdevelop->slotProjectOpenCmdl(argv[1]);
//
//    if(bStartLogo){
//      start_logo->close();
//      delete start_logo;
//    }
//
//    a.config()->setGroup("TipOfTheDay");
//    bool showTip=a.config()->readBoolEntry("show_tod",true);
//    if(showTip){
//    	kdevelop->slotHelpTipOfDay();
//    }
  }
  args->clear();
  
  int rc = a.exec();
  return rc;
}



