/***************************************************************************
                          project.h  -  description
                             -------------------
    begin                : Sat May 13 2000
    copyright            : (C) 2000 by Sandy Meier
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

#ifndef PROJECT_H
#define PROJECT_H

#include <qstring.h>
#include <qstringlist.h>
#include <qlist.h>
#include "registeredfile.h"
#include <ksimpleconfig.h>
#include <qdom.h>
#include <qobject.h>
#include <qtextstream.h>

class KAboutData;

/**-base class for all project plugins, contains  special config widgets for
     PMC (Project Managment Console) , generate/modify Makefile.am's,
     - abstract
  *@author Sandy Meier
  */

class Project : public QObject {
  Q_OBJECT
    
    public: 
  Project(QObject* parent=0,const char* name=0);
  virtual ~Project();
  /*____some get methods_____ */
  
  /** */
  QStringList allSources();

  /** returns all files*/
  QStringList allAbsoluteFileNames();
  RegisteredFile* file(QString relFileName);
  RegisteredFile* fileAbsolute(QString absFileName);

  QString version();
  QString name();
  QString projectFile();
  QString userProjectFile();

  /** store the project version. */
  void setVersion(QString version);
  void setName(QString name);
  void setAbsolutePath(QString path);
  void setRelativePath(QString path);
  QString absolutePath();
  QString relativePath();
  /** generate/modifiy the Makefile*/
  virtual void updateMakefile();

  virtual void addFile(RegisteredFile* file);
  /** include in distribution,install no*/
  virtual void addFile(QString abs_filename);
  virtual void removeFile(RegisteredFile* file);
  virtual void removeFile(QString absFilename);

  virtual bool writeGlobalConfig(QDomDocument& doc,QDomElement& projectElement);
  virtual bool writeUserConfig(QDomDocument& doc,QDomElement& projectElement);
  virtual bool readGlobalConfig(QDomDocument& doc,QDomElement& projectElement);
  virtual bool readUserConfig(QDomDocument& doc,QDomElement& projectElement);

  /** generates the Makefile entry for the main target (lib,binary) */
  virtual void updateMakefileEntries(QTextStream& stream);

  /* */
  virtual void showAllFiles();
  virtual void dump();
  /** returns some data about this Component, should be static?*/
  virtual KAboutData* aboutPlugin();

  /** a factory to create new projects, use KTrader*/
  static Project* createNewProject(QString projecttypeName,QObject* parent=0);

  protected:
  QList<RegisteredFile>* m_pFiles;
  QString m_name;
  QString m_version;
  QString m_absPath;
  /** absolute*/
  QString m_userProjectFile;
  /** absolute */
  QString m_projectFile;
  QString m_relPath;
};

#endif
