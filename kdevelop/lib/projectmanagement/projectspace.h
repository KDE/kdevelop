/***************************************************************************
                          projectspace.h  -  description
                             -------------------
    begin                : Sat May 13 2000
    copyright            : (C) 2000 by Sandy Meier (KDevelop Team)
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

#ifndef PROJECTSPACE_H
#define PROJECTSPACE_H

#include "plugin.h"
#include "project.h"
#include "kdevcomponent.h"
#include <qlist.h>
#include <string.h>
#include <ksimpleconfig.h>
#include <qdom.h>

/**handles toplevel dir, configure.in, and all projects
    data are stored in NAME.kdevpsp and NAME_local.kdevpsp
  *@author Sandy Meier
  */

class ProjectSpace : public KDevComponent, public Plugin  {
  Q_OBJECT
    public: 
  ProjectSpace(QObject* parent=0,const char* name=0,QString file="");
  ~ProjectSpace();

  /** nesessary to bootstrap a ProjectSpace*/
  static QString projectSpacePluginName(QString fileName);

  void addProject(Project* prj);
  void removeProject(QString name);

  void setCurrentProject(Project* prj);
  void setCurrentProject(QString name);
  Project* currentProject();
  
  
  /** set the projectspace name*/
  void setName(QString name);
  void setAbsolutePath(QString path);
  QString absolutePath();
  void setVersion(QString version);
  
  /** Store the name of version control system */
  void setVCSystem(QString vcsystem);
  
  /** stored in the *_local files*/
  void setAuthor(QString name);

  /** set the email, stored in the *_local file */
  void setEmail(QString email);

  void setCompany(QString company);

  /** method to fill up a string template with actual projectspace info
   */
  void setInfosInString(QString& text);

  /*_____some get methods_____*/
	// member
  /** returns the name of the projectspace*/
  QString getName();

  /** Fetch the name of the version control system */
  QString getVCSystem();

  /** Fetch the authors name. stored in the *_local files*/
  QString getAuthor();

  /** Fetch the authors eMail-address,  stored in the *_local files */
  QString getEmail();
  
  QString getCompany();
  QString getProgrammingLanguage();
  QStringList allProjectNames();
  
  /***/
  virtual void generateDefaultFiles();
  virtual void modifyDefaultFiles();
	
  virtual bool writeXMLConfig();
  /** add the data to the psElement (Projectspace)*/
  virtual bool writeGlobalConfig(QDomDocument& doc,QDomElement& psElement);
  virtual bool writeUserConfig(QDomDocument& doc,QDomElement& psElement);

  virtual bool readXMLConfig(QString abs_filename);
  virtual bool readGlobalConfig(QDomDocument& doc,QDomElement& psElement);
  virtual bool readUserConfig(QDomDocument& doc,QDomElement& psElement);
  

  virtual void dump();


protected:
  /** ProjectSpace name*/
  QString m_name;
  /** the current absolute path to the projectspace */
  QString m_path;
	
  /** the programming language for the projectspace
      needed to load the correct languagesupport*/
  
  QString m_language;
  // static
  /** projectspace template, name*/
  QString m_projectspace_template;
  /** Version control object */
//  VersionControl *vc;

  /** all projects in the ProjectSpace*/
  QList<Project>* m_projects;

  /** current active project*/
  Project* m_current_project;
  /** absolute*/
  QString m_user_projectspace_file;
  /** absolute */
  QString m_projectspace_file;
  QString m_version;

  // current User profile
  QString m_email;
  QString m_company;
  QString m_author;
 
 
  
};

#endif
