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

#include "ckdevelop.h"
#include "kstartuplogo.h"
#include "ktipofday.h"
#include "ckdevinstall.h"
#include <kwmmapp.h>


int main(int argc, char* argv[]) {

    KStartupLogo* start_logo=0L;
    KTipofDay* tipdlg=0L;
    KWMModuleApplication a(argc,argv,"kdevelop");
    a.getConfig()->setGroup("General Options");
    bool bStartLogo= a.getConfig()->readBoolEntry("Logo",true);
    bool bInstall=a.getConfig()->readBoolEntry("Install",true);
    if(bStartLogo){
      start_logo= new KStartupLogo;
      start_logo->show();
    }
    a.connectToKWM();

    if (a.isRestored()){
    	RESTORE(CKDevelop);
    }
    else {
      CKDevelop* kdevelop = new CKDevelop;
      a.setMainWidget(kdevelop);
      a.setTopWidget(kdevelop);
      if(bInstall){
        CKDevInstall* install=new CKDevInstall(kdevelop,"install");
        install->show();
        delete install;
	kdevelop->refreshTrees();  // this is because of the new documentation
      }
      kdevelop->show();
      a.getConfig()->setGroup("General Options");
      kdevelop->slotSCurrentTab(a.getConfig()->readNumEntry("LastActiveTab",BROWSER));
      kdevelop->slotTCurrentTab(a.getConfig()->readNumEntry("LastActiveTree",DOC));
      if (argc > 1)
        kdevelop->slotProjectOpenCmdl(argv[1]);
      
      a.getConfig()->setGroup("TipOfTheDay");
      bool showTip=a.getConfig()->readBoolEntry("show_tod",true);
      if(showTip){
        tipdlg=new KTipofDay;
        tipdlg->exec();
      }
    }
    if(bStartLogo){
      start_logo->close();
    }
    delete start_logo;
    delete tipdlg;
    int rc = a.exec();
    return rc;
}


































































