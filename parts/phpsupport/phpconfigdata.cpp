/***************************************************************************
                          phpconfigdata.cpp  -  description
                             -------------------
    begin                : Thu Jun 14 2001
    copyright            : (C) 2001 by 
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "phpconfigdata.h"
#include "domutil.h"
#include <iostream>

PHPConfigData::PHPConfigData(QDomDocument* dom){
  //  cerr << endl << "kdevelop (phpsupport): enter PHPConfigData::PHPConfigData";
  
  document = dom;
  invocationMode = (InvocationMode) DomUtil::readIntEntry(*dom, "/kdevphpsupport/general/invocationMode");

  webURL = DomUtil::readEntry(*dom, "/kdevphpsupport/webInvocation/weburl");
  webFileMode = (WebFileMode)DomUtil::readIntEntry(*dom, "/kdevphpsupport/webInvocation/webFileMode");
  webDefaultFile = DomUtil::readEntry(*dom, "/kdevphpsupport/webInvocation/defaultFile");

  phpExePath = DomUtil::readEntry(*dom, "/kdevphpsupport/shell/phpexe");  

  
}
PHPConfigData::~PHPConfigData(){
}

void PHPConfigData::storeConfig(){
  DomUtil::writeIntEntry(*document, "/kdevphpsupport/general/invocationMode",(int) invocationMode);

  DomUtil::writeEntry(*document, "/kdevphpsupport/webInvocation/weburl",webURL);
  DomUtil::writeIntEntry(*document, "/kdevphpsupport/webInvocation/webFileMode",(int) webFileMode);
  DomUtil::writeEntry(*document, "/kdevphpsupport/webInvocation/defaultFile",webDefaultFile);

  DomUtil::writeEntry(*document, "/kdevphpsupport/shell/phpexe",phpExePath);  
  
}

bool PHPConfigData::validateConfig(){
  bool valid = false;
  if(invocationMode == Web || invocationMode == Shell) {
    valid = true;
  }
  if(valid){
    if(invocationMode == Web){
      if(!(webURL != "" && (webFileMode == Default || webFileMode == Current))){
	valid = false;
      }
    }
    if(invocationMode == Shell){
      if(phpExePath == ""){
	valid = false;
      }
    }
  }
  return valid;
}
