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

#include "project.h"
#include "kdevcomponent.h"
#include <qlist.h>
#include <string.h>
#include <ksimpleconfig.h>
#include <qdom.h>
#include "kaction.h"
class KAboutData;
class KDevFileNode;

class FileGroup {
 public:
  void setName(QString name);
  QString name();
  void setFilter(QString filter);
  QString filter();

 protected:
  QString m_name;
  QString m_filter;
};

/**handles toplevel dir, configure.in, and all projects
    data are stored in NAME.kdevpsp and .NAME.kdevpsp
  *@author Sandy Meier
  */

class ProjectSpace : public KDevComponent {
  Q_OBJECT
    public: 
  ProjectSpace(QObject* parent=0,const char* name=0);
  ~ProjectSpace();

  // from KDevComponent
  void languageSupportOpened(KDevLanguageSupport *ls);
  void languageSupportClosed();
  virtual QList<KAction>* kdevNodeActions(KDevNode* pNode);


  /** nesessary to bootstrap a ProjectSpace*/
  static QString projectSpacePluginName(QString fileName);
  /** factory to create new ProjectSpaces */
  static ProjectSpace* createNewProjectSpace(const QString& name,QObject* parent=0);

  /** returns the default files groups, can be used in ProjectView...*/
  virtual QList<FileGroup> defaultFileGroups();
  /** returns all registered projects*/
  QList<Project>* allProjects();
  Project* project(QString projectName);

  virtual void setupGUI();
  void addProject(Project* prj);
  void removeProject(QString name);
  void setCurrentProject(Project* prj);
  void setCurrentProject(QString name);
  Project* currentProject();
  
  /** set the projectspace name*/
  void setName(QString name);
  void setAbsolutePath(QString path);
  QString absolutePath();
  /** returns the absolute filelocation*/
  QString projectSpaceFile();
  void setVersion(QString version);
  /** Store the name of version control system */
  void setVCSystem(QString vcsystem);
  /** stored in the user file*/
  void setAuthor(QString name);
  /** set the email, stored in the user file */
  void setEmail(QString email);
  void setCompany(QString company);

  /** method to fill up a string template with actual projectspace info
   */
  void setInfosInString(QString& text);

  /*_____some get methods_____*/
	// member
  /** returns the name of the projectspace*/
  QString name();
  /** Fetch the name of the version control system */
  QString VCSystem();
  /** Fetch the authors name. stored in the *_local files*/
  QString author();
  /** Fetch the authors eMail-address,  stored in the *_local files */
  QString email();
  QString company();
  QString programmingLanguage();
  QStringList allProjectNames();
  
  /***/
  virtual void generateDefaultFiles();
  virtual void modifyDefaultFiles();
	
  virtual QDomDocument* writeGlobalDocument();
  virtual QDomDocument* writeUserDocument();
  virtual bool saveConfig();

  virtual bool readConfig(QString abs_filename);
  QDomDocument* readGlobalDocument();
  QDomDocument* readUserDocument();

  virtual void dump();

 signals:
  void sigAddedFileToProject(KDevFileNode* pNode);

  protected slots:
    virtual void slotProjectSetActivate( int id);
  virtual void slotProjectAddExistingFiles();
  virtual void slotRenameFile(KDevNode* pNode);
  virtual void slotDeleteFile(KDevNode* pNode);
  virtual void slotRemoveFileFromProject(KDevNode* pNode);
  virtual void slotMoveFileTo(KDevNode* pNode);
  virtual void slotCopyFileTo(KDevNode* pNode);
protected:

  virtual bool readGlobalConfig(QDomDocument& doc,QDomElement& psElement);
  virtual bool readUserConfig(QDomDocument& doc,QDomElement& psElement);
  /** add the data to the psElement (Projectspace)*/
  virtual bool writeGlobalConfig(QDomDocument& doc,QDomElement& psElement);
  virtual bool writeUserConfig(QDomDocument& doc,QDomElement& psElement);
  
  QDomDocument* m_pUserDoc;
  QDomDocument* m_pGlobalDoc;
  
  /** ProjectSpace name*/
  QString m_name;
  /** the current absolute path to the projectspace */
  QString m_path;
	
  /** the programming language for the projectspace
      needed to load the correct languagesupport*/
  
  QString m_language;
  // static
  /** projectspace template, name*/
  QString m_projectspaceTemplate;
  /** Version control object */
//  VersionControl *vc;

  /** all projects in the ProjectSpace*/
  QList<Project>* m_pProjects;

  /** current active project*/
  Project* m_pCurrentProject;
  /** absolute*/
  QString m_userProjectspaceFile;
  /** absolute */
  QString m_projectspaceFile;
  QString m_version;

  // current User profile
  QString m_email;
  QString m_company;
  QString m_author;

  // the current languagesupport
  KDevLanguageSupport* m_pLanguageSupport;
 
 private:
  void fillActiveProjectPopupMenu();
  
};

#endif
