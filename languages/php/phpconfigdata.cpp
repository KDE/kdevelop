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

  webURL = DomUtil::readEntry(*document, "/kdevphpsupport/webInvocation/weburl");

  phpExePath = DomUtil::readEntry(*document, "/kdevphpsupport/shell/phpexe"); 
  
  phpIncludePath = DomUtil::readEntry(*document, "/kdevphpsupport/options/phpincludepath");
  phpStartupFile = DomUtil::readEntry(*document, "/kdevphpsupport/options/defaultFile");
  phpStartupFileMode = (StartupFileMode)DomUtil::readIntEntry(*document, "/kdevphpsupport/options/startupFileMode");

  m_codeCompletion = DomUtil::readBoolEntry(*document,"kdevphpsupport/codeHelp/codeCompletion");
  m_codeHinting = DomUtil::readBoolEntry(*document,"kdevphpsupport/codeHelp/codeHinting");;
  m_realtimeParsing = DomUtil::readBoolEntry(*document,"kdevphpsupport/codeHelp/realtimeParsing");
  
}
PHPConfigData::~PHPConfigData(){
}

void PHPConfigData::storeConfig(){
  DomUtil::writeIntEntry(*document, "/kdevphpsupport/general/invocationMode",(int) invocationMode);

  DomUtil::writeEntry(*document, "/kdevphpsupport/webInvocation/weburl",webURL);

  DomUtil::writeEntry(*document, "/kdevphpsupport/shell/phpexe",phpExePath);
  
  DomUtil::writeEntry(*document, "/kdevphpsupport/options/phpincludepath",phpIncludePath);
  DomUtil::writeEntry(*document, "/kdevphpsupport/options/defaultFile",phpStartupFile);
  DomUtil::writeIntEntry(*document, "/kdevphpsupport/options/startupFileMode",(int) phpStartupFileMode);

  DomUtil::writeBoolEntry(*document,"kdevphpsupport/codeHelp/codeCompletion",m_codeCompletion);
  DomUtil::writeBoolEntry(*document,"kdevphpsupport/codeHelp/codeHinting",m_codeHinting);
  DomUtil::writeBoolEntry(*document,"kdevphpsupport/codeHelp/realtimeParsing",m_realtimeParsing);
  emit configStored(); // inform all others
  
}

bool PHPConfigData::validateConfig(){
  bool valid = false;
  if(invocationMode == Web || invocationMode == Shell) {
    valid = true;
  }
  if(valid){
    if(invocationMode == Web){
      if(!(!webURL.isEmpty() && (phpStartupFileMode == Default || phpStartupFileMode == Current))){
	valid = false;
      }
    }
    if(invocationMode == Shell){
      if(phpExePath.isEmpty()){
	valid = false;
      }
    }
  }
  return valid;
}
#include "phpconfigdata.moc"
