/*
   Copyright (C) 2005 by Nicolas Escuder <n.escuder@intra-links.com>
   Copyright (C) 2001 by smeier@kdevelop.org

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   version 2, License as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kstandarddirs.h"
#include "phpconfigdata.h"
#include "domutil.h"
#include <iostream>

PHPConfigData::PHPConfigData(QDomDocument* dom) {
  //  cerr << endl << "kdevelop (phpsupport): enter PHPConfigData::PHPConfigData";

   QString file = KStandardDirs::findExe("php");

   if (file.isEmpty())
      file = "/usr/local/bin/php";

   document = dom;
   invocationMode = (InvocationMode) DomUtil::readIntEntry(*dom, "/kdevphpsupport/general/invocationMode", Web);

   webURL = DomUtil::readEntry(*document, "/kdevphpsupport/webInvocation/weburl");

   phpExePath = DomUtil::readEntry(*document, "/kdevphpsupport/shell/phpexe", file);
   phpIniPath = DomUtil::readEntry(*document, "/kdevphpsupport/shell/phpini");

   phpIncludePath = DomUtil::readEntry(*document, "/kdevphpsupport/options/phpincludepath");
   phpStartupFile = DomUtil::readEntry(*document, "/kdevphpsupport/options/defaultFile");
   phpStartupFileMode = (StartupFileMode)DomUtil::readIntEntry(*document, "/kdevphpsupport/options/startupFileMode", Current);

   m_codeCompletion = DomUtil::readBoolEntry(*document,"kdevphpsupport/codeHelp/codeCompletion", true);
   m_codeHinting = DomUtil::readBoolEntry(*document,"kdevphpsupport/codeHelp/codeHinting", true);
   m_realtimeParsing = DomUtil::readBoolEntry(*document,"kdevphpsupport/codeHelp/realtimeParsing", true);
}

PHPConfigData::~PHPConfigData() {
}

void PHPConfigData::storeConfig() {
   DomUtil::writeIntEntry(*document, "/kdevphpsupport/general/invocationMode",(int) invocationMode);

   DomUtil::writeEntry(*document, "/kdevphpsupport/webInvocation/weburl",webURL);

   DomUtil::writeEntry(*document, "/kdevphpsupport/shell/phpexe",phpExePath);
   DomUtil::writeEntry(*document, "/kdevphpsupport/shell/phpini",phpIniPath);

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
   if (invocationMode == Web || invocationMode == Shell) {
      valid = true;
   }
   if (valid) {
      if (invocationMode == Web) {
         if (!(!webURL.isEmpty() && (phpStartupFileMode == Default || phpStartupFileMode == Current))) {
            valid = false;
         }
      }
      if (invocationMode == Shell) {
         if (phpExePath.isEmpty()) {
            valid = false;
         }
      }
   }
   return valid;
}
#include "phpconfigdata.moc"
