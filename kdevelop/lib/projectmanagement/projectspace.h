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
#include "KDevCompiler.h"


// Forward declarations;
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
  //++++++ from the KDevComponent interface +++++++
  virtual QList<KAction> kdevNodeActions(KDevNode* pNode);
  virtual void setupComponent();
  virtual void setupGUI();

  /** nesessary to bootstrap a ProjectSpace*/
  static QString projectSpacePluginName(QString fileName);
  /** returns the default files groups, can be used in ProjectView...*/
  virtual QList<FileGroup> defaultFileGroups();

  //++++++++ project related methods +++++++++++
  /** returns all registered projects*/
  QList<Project>* allProjects();
  /** returns a lsit of all registered directories in this projectspace */
  virtual QStringList allDirectories();
  Project* project(QString projectName);
  void addProject(Project* prj);
  void removeProject(QString name);
  void setCurrentProject(Project* prj);
  void setCurrentProject(QString name);
  Project* currentProject();
  QStringList allProjectNames();
  
  //++++++++ general projectspace properties ++++++++
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
  /** The author's initials. Stored in the user file*/
  void setInitials(QString initials);
  /** set the email, stored in the user file */
  void setEmail(QString email);
  void setCompany(QString company);
  /** method to fill up a string template with actual projectspace info */
  void setInfosInString(QString& text);
  /** returns the name of the projectspace*/
  QString name();
  /** Fetch the name of the version control system */
  QString VCSystem();
  /** Fetch the authors name. stored in the user projectspace file*/
  QString author();
  /** Fetch the author's initials. stored in the *_local files*/
  QString initials();
  /** Fetch the authors eMail-address,  stored in the user projectspace file */
  QString email();
  QString company();
  QString programmingLanguage();

  // the compilers that ps uses
  QList<KDevCompiler>* compilers();

  // returns the name compiler named 'name' used by this project space
  KDevCompiler* compilerByName(const QString &name);

  // register a compiler for this project space
  void addCompiler(KDevCompiler::CompilerID cid);

  /** generate default files, 
   * is used by the application wizard to generated all needed files, 
   * for instance the admin dir in automake projects*/
  virtual void generateDefaultFiles();

  /** this methods update all project administration files:
      Makefiles, configure.in and so*/
  virtual void updateAdminFiles();
	
  virtual QDomDocument* writeGlobalDocument();
  virtual QDomDocument* writeUserDocument();
  virtual bool saveConfig();

  virtual bool readConfig(QString abs_filename);
  QDomDocument* readGlobalDocument();
  QDomDocument* readUserDocument();

  virtual void dump();

signals:
  void sigAddedFileToProject(KDevFileNode* pNode);
  void sigRemovedFileFromProject(KDevFileNode* pNode);
  void sigAddedProject(KDevNode* pNode);
	void sigProjectChanged();

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

  // compilers we need for the project
	QList<KDevCompiler>* m_compilers;

  // static
  /** projectspace template, name*/
  QString m_projectspaceTemplate;

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
  QString m_email;      // Author e-mail address.
  QString m_company;    // Company name.
  QString m_author;     // Author name.
  QString m_initials;     // Author Initials.

 private:
  void fillActiveProjectPopupMenu();
  
};

#endif
