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

#include "qobjectplugin.h"
#include <qstring.h>
#include <qstringlist.h>
#include <qlist.h>
#include "registeredfile.h"
#include <ksimpleconfig.h>
#include <qdom.h>

/**-base class for all project plugins, contains  special config widgets for
     PMC (Project Managment Console) , generate/modify Makefile.am's,
     - abstract
  *@author Sandy Meier
  */

class Project : public QObjectPlugin  {
  Q_OBJECT
    
    public: 
  Project(QObject* parent=0,const char* name=0);
  virtual ~Project();
  /*____some get methods_____ */
  
  /** */
  QStringList getAllSources();

  /** returns all files*/
  QStringList getAllFileNames();
  RegisteredFile getFileProperties(QString filename);

  QString getVersion();
  QString getName();
  QString getProjectFile();
  QString getUserProjectFile();

  /** store the project version. */
  void setVersion(QString version);
  void setName(QString name);
  void setAbsolutePath(QString path);
  void setRelativePath(QString path);
  QString getAbsolutePath();
  QString relativePath();
  /** generate/modifiy the Makefile*/
  virtual void updateMakefile();

  virtual void addFile(RegisteredFile* file);
  /** include in distribution,install no*/
  virtual void addFile(QString abs_filename);
  virtual void removeFile(RegisteredFile* file);

  virtual bool writeGlobalConfig(QDomDocument& doc,QDomElement& projectElement);
  virtual bool writeUserConfig(QDomDocument& doc,QDomElement& projectElement);
  virtual bool readGlobalConfig(QDomDocument& doc,QDomElement& projectElement);
  virtual bool readUserConfig(QDomDocument& doc,QDomElement& projectElement);

  /* */
  virtual void showAllFiles();
  virtual void dump();
  protected:
  QList<RegisteredFile>* m_files;
  QString m_name;
  QString m_version;
  QString m_abs_path;
  /** absolute*/
  QString m_user_project_file;
  /** absolute */
  QString m_project_file;
  QString m_relPath;
};

#endif
