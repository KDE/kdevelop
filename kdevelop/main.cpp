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
#include "ckdevinstall.h"

#include <dcopclient.h>
#include <kaboutdata.h>
#include <kapp.h>
#include <kglobal.h>
#include <klocale.h>


int main(int argc, char* argv[]) {
  
 KApplication a(argc,argv,"kdevelop");

  KGlobal::config()->setGroup("General Options");
  bool bInstall=KGlobal::config()->readBoolEntry("Install",true);
  if (argc > 1 ){
    if( QString(argv[1]) == "--setup") bInstall = true; // start the setupwizard
  }

  if (a.isRestored()){
    RESTORE(CKDevelop);
  }
  else{
    if(bInstall){
      CKDevInstall* install=new CKDevInstall(0,"install");
      install->show();
      delete install;
    }

    CKDevelop* kdevelop = new CKDevelop();

		// We don't want this happening - It shouldn't - but (indirectly) KHTMLPart connects :(
		if (a.dcopClient() && a.dcopClient()->isAttached())
			a.dcopClient()->suspend();

    kdevelop->completeStartup( argc > 1 );

    if(bInstall){
      kdevelop->refreshTrees();  // this is because of the new documentation
    }
    
    KGlobal::config()->setGroup("General Options");
    kdevelop->slotSCurrentTab(KGlobal::config()->readNumEntry("LastActiveTab",BROWSER));
    kdevelop->slotTCurrentTab(KGlobal::config()->readNumEntry("LastActiveTree",DOC));
    
    if (argc > 1){
      if (QString(argv[1]) != "--setup")
        kdevelop->slotProjectOpenCmdl(argv[1]);
    }  
  }
  
  int rc = a.exec();
  return rc;
}
