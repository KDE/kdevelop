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
#include "kstartuplogo.h"
#include "ckdevinstall.h"
#include "ckdevelop.h"
#include "kwrite/kwview.h"





KGuiCmdManager cmdMngr; //manager for user -> gui commands



int main(int argc, char* argv[]) {
  
  KStartupLogo* start_logo=0L;
  KGuiCmdKWMModuleApp a(argc,argv,"kdevelop");//KWMModuleApplication a(argc,argv,"kdevelop");
	a.connectToKWM();

  // commands
  KWrite::addCursorCommands(cmdMngr);
  KWrite::addEditCommands(cmdMngr);
  KWrite::addFindCommands(cmdMngr);
  KWrite::addStateCommands(cmdMngr);
  cmdMngr.makeDefault();
  cmdMngr.readConfig(a.getConfig());

  // Resource types
  KGlobal::dirs()->addResourceType("kdev_template", KStandardDirs::kde_default("data")
                                   + kapp->name() + "/templates/");
  KGlobal::dirs()->addResourceType("kdev_pic", KStandardDirs::kde_default("data")
                                   + kapp->name() + "/pics/");
  

  a.getConfig()->setGroup("General Options");
  bool bStartLogo= a.getConfig()->readBoolEntry("Logo",true);
  bool bInstall=a.getConfig()->readBoolEntry("Install",true);
  if (argc > 1 ){
    if( QString(argv[1]) == "--setup") bInstall = true; // start the setupwizard
  }
  
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
      start_logo->close();
      install->show();
      delete install;
    }
    CKDevelop* kdevelop = new CKDevelop( argc > 1 );
    if(bInstall){
      kdevelop->refreshTrees();  // this is because of the new documentation
    }
    kdevelop->show();
    
    a.getConfig()->setGroup("General Options");
    kdevelop->slotSCurrentTab(a.getConfig()->readNumEntry("LastActiveTab",BROWSER));
    kdevelop->slotTCurrentTab(a.getConfig()->readNumEntry("LastActiveTree",DOC));
    
    if(!a.getConfig()->readBoolEntry("show_kdevelop",true))
      kdevelop->setKDlgCaption();
    
    if (argc > 1){ 
      if (QString(argv[1]) != "--setup")
	  kdevelop->slotProjectOpenCmdl(argv[1]);
    }
    if(bStartLogo){
      start_logo->close();
      delete start_logo;
    }
  
    a.getConfig()->setGroup("TipOfTheDay");
    bool showTip=a.getConfig()->readBoolEntry("show_tod",true);
    if(showTip){
    	kdevelop->slotHelpTipOfDay();
    }
  }
  
  int rc = a.exec();
  return rc;
}



