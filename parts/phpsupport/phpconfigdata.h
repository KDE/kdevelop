/***************************************************************************
    begin                : Thu Jun 14 2001
    copyright            : (C) 2001 by Sandy Meier
    email                : smeier@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
  enum WebFileMode {Current=1,Default=2};
  
  
  PHPConfigData(QDomDocument* document);
  ~PHPConfigData();
  
  /**
     returns true if the configuration is ok, false if something is missing
  */
  bool validateConfig();
  /** write the configuration to the DOM document(project file)
   */
  void storeConfig();
  
  InvocationMode getInvocationMode(){
    return invocationMode;
  }
  void setInvocationMode(InvocationMode mode){
    invocationMode = mode;
  }
  
  // web
  WebFileMode getWebFileMode(){
    return webFileMode;
  }
  QString getWebURL(){
    return webURL;
  }
  QString getWebDefaultFile(){
    return webDefaultFile;
  }
  void setWebFileMode(WebFileMode mode){
    webFileMode = mode;
  }
  void setWebURL(QString weburl){
    webURL = weburl;
  }
  void setWebDefaultFile(QString defaultFile){
    webDefaultFile = defaultFile;
  }
  
  // shell
  QString getPHPExecPath(){
    return phpExePath;
  }
  void setPHPExePath(QString path){
    phpExePath = path;
  }

  // code help
  void setCodeCompletion(bool enable){
    m_codeCompletion = enable; 
  }
  bool getCodeCompletion(){
    return m_codeCompletion;
  }
  void setCodeHinting(bool enable){
    m_codeHinting = enable;
  }
  bool getCodeHinting(){
    return m_codeHinting;
  }
  void setRealtimeParsing(bool enable){
    m_realtimeParsing = enable;
  }
  bool getRealtimeParsing(){
    return m_realtimeParsing;
  }
  
  
 private:
  QDomDocument* document;
  InvocationMode invocationMode;
  // web
  WebFileMode webFileMode;
  QString webURL;
  QString webDefaultFile;
  
  // shell
  QString phpExePath;
  
  // code help
  bool m_codeCompletion;
  bool m_codeHinting;
  bool m_realtimeParsing;
  
};

#endif
