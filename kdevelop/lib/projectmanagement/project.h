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

/**-base class for all project plugins, contains  special config widgets for
     PMC (Project Managment Console) , generate/modify Makefile.am's,
     - every project has 2 own project files
     NAME.kdevprj2 for global relevant options,
     .NAME.kdevprj2 for local user options
     - abstract
  *@author Sandy Meier
  */

class Project : public QObjectPlugin  {
  Q_OBJECT
    
    public: 
  Project(QObject* parent=0,const char* name=0,QString filename="");
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
  QString getAbsolutePath();
  /** generate/modifiy the Makefile*/
  virtual void updateMakefile();

  virtual void addFile(RegisteredFile* file);
  /** include in distribution,install no*/
  virtual void addFile(QString abs_filename);
  virtual void removeFile(RegisteredFile* file);

  virtual bool readConfig(QString abs_filename);
  virtual bool readGeneralConfig(KSimpleConfig* config);
  virtual bool readUserConfig(KSimpleConfig* config);
  
  virtual bool writeConfig();	
  virtual bool writeGeneralConfig(KSimpleConfig* config);
  virtual bool writeUserConfig(KSimpleConfig* config);

  /* */
  virtual void showAllFiles();
  protected:
  QList<RegisteredFile>* m_files;
  QString m_name;
  QString m_version;
  QString m_abs_path;
  /** absolute*/
  QString m_user_project_file;
  /** absolute */
  QString m_project_file;
};

#endif
