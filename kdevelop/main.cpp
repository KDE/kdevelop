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
#include <kwmmapp.h>

int main(int argc, char* argv[]) {

    KWMModuleApplication a(argc,argv,"kdevelop");  
    a.connectToKWM();
    
    if (a.isRestored()){
	RESTORE(CKDevelop);
    }
    else {
	CKDevelop* kdevelop = new CKDevelop;
	a.setMainWidget(kdevelop);
	a.setTopWidget(kdevelop);
	kdevelop->show();
	a.getConfig()->setGroup("General Options");
	kdevelop->slotSCurrentTab(a.getConfig()->readNumEntry("LastActiveTab",BROWSER));
    }
    
    int rc = a.exec();
    return rc;
}



