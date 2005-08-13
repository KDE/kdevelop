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

#ifndef PHPCONFIGDATA_H
#define PHPCONFIGDATA_H

#include <qstring.h>
#include <qdom.h>
#include <qobject.h>

/**
  *@author Sandy Meier
  */

class PHPConfigData : public QObject {
Q_OBJECT

signals:
   void  configStored(); 

public: 
   enum InvocationMode {Web=1,Shell=2};
   enum StartupFileMode {Current=1,Default=2};

   PHPConfigData(QDomDocument* document);
   ~PHPConfigData();

   /** returns true if the configuration is ok, false if something is missing
   */
   bool validateConfig();
   /** write the configuration to the DOM document(project file)
   */
   void storeConfig();

   InvocationMode getInvocationMode() {
      return invocationMode;
   }

   void setInvocationMode(InvocationMode mode) {
      invocationMode = mode;
   }

   // web
   QString getWebURL() {
      return webURL;
   }
   void setWebURL(QString weburl) {
      webURL = weburl;
   }

   // shell
   QString getPHPExecPath() {
      return phpExePath;
   }

   void setPHPExePath(QString path) {
      phpExePath = path;
   }

   // config
   QString getPHPIniPath() {
      return phpIniPath;
   }

   void setPHPIniPath(QString path) {
      phpIniPath = path;
   }

   // options
   QString getPHPIncludePath() {
      return phpIncludePath;
   }
   void setPHPIncludePath(QString path) {
      phpIncludePath = path;
   }

   QString getStartupFile() {
      return phpStartupFile;
   }
   void setStartupFile(QString defaultFile) {
      phpStartupFile = defaultFile;
   }

   StartupFileMode getStartupFileMode() {
      return phpStartupFileMode;
   }

   void setStartupFileMode(StartupFileMode mode) {
      phpStartupFileMode = mode;
   }

   // code help
   void setCodeCompletion(bool enable) {
      m_codeCompletion = enable; 
   }

   bool getCodeCompletion() {
      return m_codeCompletion;
   }

   void setCodeHinting(bool enable) {
      m_codeHinting = enable;
   }

   bool getCodeHinting() {
      return m_codeHinting;
   }

   void setRealtimeParsing(bool enable) {
      m_realtimeParsing = enable;
   }

   bool getRealtimeParsing() {
      return m_realtimeParsing;
   }

private:
   QDomDocument* document;
   InvocationMode invocationMode;
   // web
   QString webURL;

   // shell
   QString phpExePath;
   QString phpIniPath;
   QString phpStartupFile;

   // options
   QString phpIncludePath;
   QString phpDefaultFile;
   StartupFileMode phpStartupFileMode;

   // code help
   bool m_codeCompletion;
   bool m_codeHinting;
   bool m_realtimeParsing;
};

#endif
