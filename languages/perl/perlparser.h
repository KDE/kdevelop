/***************************************************************************
                          perlparser.h  -  description
                             -------------------
    begin                : Sun Nov 2 2003
    copyright            : (C) 2003 by luc
    email                : luc@lieve
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PERLPARSER_H
#define PERLPARSER_H

#include "kdevlanguagesupport.h"
#include <codemodel.h>


/**perl source to classview parser
  *@author luc
  */

class perlparser {
  public: 
  perlparser(KDevCore* core,CodeModel* model, QString interpreter);
  perlparser();
  ~perlparser();

  void initialParse();
  void parse(const QString &fileName);
  void parseLines(QStringList* lines,const QString& fileName);
  const QStringList UseFiles();
  QString findLib( const QString& lib);

  private:
  //global functions to add to ClassStore
  void addPackage(const QString& fileName ,int lineNr , const QString& name);
  void addAttributetoScript(const QString& fileName ,int lineNr ,const QString& name);
  void addAttributetoPackage(const QString& fileName ,int lineNr ,const QString& name);
  void addClass(const QString& fileName ,int lineNr );
  void addConstructor(const QString& fileName ,int lineNr ,const QString& methodname);
  void addGlobalSub(const QString& fileName ,int lineNr , const QString& name, bool privatesub);
  void addScriptSub(const QString& fileName ,int lineNr , const QString& name, bool privatesub);
  void addClassMethod(const QString& fileName ,int lineNr , const QString& name , bool privatesub);
  void addPackageSub(const QString& fileName ,int lineNr , const QString& name, bool privatesub);
  void addParentClass(const QString& parent);
  void addScript(const QString& fileName ,int lineNr , const QString& name);
  void addUseLib(const QString& lib);
  void getPerlINC();

  bool    m_inpackage;
  bool    m_inscript;
  bool    m_inclass;

  QString m_lastsub;
  QString m_lastparentclass;
  QString m_lastattr;
  QString m_lastpackagename;
  QString m_lastscriptname;

  NamespaceDom m_lastscript;
  NamespaceDom m_lastpackage;
  ClassDom m_lastclass;

  //CodeModel
  CodeModel* m_model;
  KDevCore* m_core;
  FileDom m_file;

  //this willhav a list of INC paths
  QStringList m_INClist;
  //this will get a list off all files "use" in the perl files
  //and need additional parsing to include the classes in the classview
  QStringList m_usefiles;

  QString m_interpreter;
  
  
};

#endif
