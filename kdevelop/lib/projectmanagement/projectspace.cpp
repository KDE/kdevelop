/***************************************************************************
			 projectspace.cpp  -  description
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

#include "projectspace.h"
#include "pluginloader.h"
#include <kprocess.h>
#include <kstddirs.h>
#include <iostream.h>
#include "plugin.h"
#include <ksimpleconfig.h>
#include <kdebug.h>
#include <qdir.h>
#include <ctoolclass.h>
#include <qfileinfo.h>
#include <qdatetime.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <qtextstream.h>


ProjectSpace::ProjectSpace(QObject* parent,const char* name,QString file) : KDevComponent(parent,name){
  m_projects = new QList<Project>;
  if (file != ""){
    readConfig(file);
  }
}
ProjectSpace::~ProjectSpace(){
}

QString ProjectSpace::projectSpacePluginName(QString fileName){
  QFile file(fileName);
  if (!file.open(IO_ReadOnly)){
    KMessageBox::sorry(0, i18n("Can't open the file %1")
		       .arg(fileName));
    return "";
  }
  QDomDocument doc;
  if (!doc.setContent(&file)){
    KMessageBox::sorry(0,
		       i18n("The file %1 does not contain valid XML").arg(fileName));
      return "";
  }
  QDomElement psElement = doc.documentElement(); // get the Projectspace
  return psElement.attribute("pluginName");
}
void ProjectSpace::addProject(QString file){
  
}
void ProjectSpace::addProject(Project* prj){
  cerr << endl << "enter ProjectSpace::addProject";
  m_projects->append (prj);
  m_current_project = prj;
}
void ProjectSpace::setCurrentProject(Project* prj){
  m_current_project = prj;
}

void ProjectSpace::removeProject(QString name){
}
void ProjectSpace::generateDefaultFiles(){
  KShellProcess proc("/bin/sh");
  
  // create the directories
  proc << "mkdirhier";
  proc << m_path;
  proc.start(KProcess::Block,KProcess::AllOutput);
  
  // untar/unzip the projectspace
  proc.clearArguments();
  QString args = "xzvf " + m_projectspace_template + " -C " + m_path;
  kdDebug(9000)  << "ProjectSpace::generateDefaultFiles():" << endl << args << endl;
  proc << "tar";
  proc << args;
  proc.start(KProcess::Block,KProcess::AllOutput);
  
  modifyDefaultFiles();
}
void ProjectSpace::modifyDefaultFiles(){
  
}
/*_____some get methods_____*/

/** returns the name of the projectspace*/
QString ProjectSpace::getName(){
  return "";
}

/** Fetch the name of the version control system */
QString ProjectSpace::getVCSystem(){
  return "";
}

/** Fetch the authors name. stored in the *_user files*/
QString ProjectSpace::getAuthor(){
  return m_author;
}

/** Fetch the authors eMail-address,  stored in the *_user files */
QString ProjectSpace::getEmail(){
  return m_email;
}
QString ProjectSpace::getCompany(){
  return m_company;
}

/** set the projectspace name*/
void ProjectSpace::setName(QString name){
  m_name = name;
}
void ProjectSpace::setAbsolutePath(QString path){
  m_path = path;
}

/** Store the name of version control system */
void ProjectSpace::setVCSystem(QString vcsystem){
}

/** stored in the *_user files*/
void ProjectSpace::setAuthor(QString name){
  m_author = name;
}

/** set the email, stored in the *_user file */
void ProjectSpace::setEmail(QString email){
  m_email = email;
}
void ProjectSpace::setCompany(QString company){
  m_company = company;
}
void ProjectSpace::setVersion(QString version){
  m_version = version;
}

/** method to fill up a string template with actual projectspace info
 */
void ProjectSpace::setInfosInString(QString& text){
  QDate date;
  text.replace(QRegExp("|EMAIL|"),m_email);
  text.replace(QRegExp("|YEAR|"),QString::number(date.year()));
  text.replace(QRegExp("|AUTHOR|"),m_author);
  text.replace(QRegExp("|VERSION|"),m_version);
  text.replace(QRegExp("|COMPANY|"),m_company);
  text.replace(QRegExp("|NAME|"),m_name);
  text.replace(QRegExp("|NAMELITTLE|"),m_name.lower());
  	      
}

bool ProjectSpace::readXMLConfig(QString absFilename){
  kdDebug(9000) << "enter ProjectSpace::readXMLConfig" << endl;
  QFileInfo fileInfo(absFilename);
  m_path = fileInfo.dirPath();
  m_projectspace_file = absFilename;

  QFile file(absFilename);
  if (!file.open(IO_ReadOnly)){
    KMessageBox::sorry(0, i18n("Can't open the file %1")
		       .arg(absFilename));
    return false;
  }
  
  // Parse the XML file.
  QDomDocument doc;
  // Read in file and check for a valid XML header.
  if (!doc.setContent(&file)){
    KMessageBox::sorry(0,
		       i18n("The file %1 does not contain valid XML").arg(absFilename));
      return false;
  }
  // Check for proper document type.
  if (doc.doctype().name() != "KDevProjectSpace"){
      KMessageBox::sorry(0,
			 i18n("The file %1 does not contain a valid work sheet\n"
			      "definition, which must have a document type\n"
			      "'KDevProjectSpace (General)'").arg(absFilename));
      return false;
    }
  QDomElement psElement = doc.documentElement();
  readGlobalConfig(doc,psElement);
  return true;
}



bool ProjectSpace::readGlobalConfig(QDomDocument& doc,QDomElement& psElement){
  m_name = psElement.attribute("name");
  //  m_path = psElement.attribut("path");
  m_version = psElement.attribute("version");
  QDomElement projectsElement = psElement.namedItem("Projects").toElement();
  if(projectsElement.isNull()){
    cerr << "\nProjectSpace::readGlobalConfig no \"Projects\" tag found!";
    return false;
  }
  Project* prj=0;
  QDomNodeList projectList = projectsElement.elementsByTagName("Project");
  unsigned int i;
  for (i = 0; i < projectList.count(); ++i){
    QDomElement projectElement = projectList.item(i).toElement();
    QString prjPluginName = projectElement.attribute("pluginName");
    prj = PluginLoader::getNewProject(prjPluginName);
    if(prj != 0){
      prj->readGlobalConfig(doc,projectElement);
      prj->setAbsolutePath(CToolClass::getAbsolutePath(m_path,prj->relativePath()));
      addProject(prj);
    }
    else {
      cerr << "\nProjectSpace::readGlobalConfig: prj not created!";
    }
  }
  return true;
 
}
bool ProjectSpace::readUserConfig(QDomDocument& doc,QDomElement& psElement){
  return true;
}


// xml stuff
bool ProjectSpace::writeXMLConfig(){
  kdDebug(9000) << "\nenter ProjectSpace::writeXMLConfig" << endl;

   // the "global" one
  QDomDocument doc("KDevProjectSpace");
  doc.appendChild(doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""));
  // save work projectspace information
  QDomElement ps = doc.appendChild(doc.createElement("ProjectSpace")).toElement();
  
  writeGlobalConfig(doc,ps);
  QString filename = m_path + "/" + m_name + ".kdevpsp";
  kdDebug(9000)  << "filename::" << filename << endl;
  QFile file(filename);
  if (!file.open(IO_WriteOnly)){
    KMessageBox::sorry(0, i18n("Can't save file %1")
		       .arg(filename));
    return false;
  }
  QTextStream s(&file);
  s << doc;
  file.close();
  return true;
}
// add the data to the psElement (Projectspace)
bool ProjectSpace::writeGlobalConfig(QDomDocument& doc, QDomElement& psElement){
  cerr << "\nenter ProjectSpace::writeGlobalConfig";
  // add Attributes
  psElement.setAttribute("name",m_name);
  //  psElement.setAttribute("path",m_path);
  psElement.setAttribute("pluginName", m_plugin_name); // the projectspacetype name
  psElement.setAttribute("version", m_version);
  //  psElement.setAttribute("programmingLanguage",m_language);

  // add <Projects>
  QDomElement projectsElement = psElement.appendChild(doc.createElement("Projects")).toElement();
  QStringList projectfiles;
  Project* prj;
  for(prj=m_projects->first();prj !=0;prj=m_projects->next()){
    QDomElement prjElement = projectsElement.appendChild(doc.createElement("Project")).toElement();
        prj->writeGlobalConfig(doc,prjElement);
  }
  return true;
}
bool ProjectSpace::writeUserConfig(QDomDocument& dom){
  cerr << "\nenter ProjectSpace::writeUserConfig";
  return true;
}


QString ProjectSpace::getProgrammingLanguage(){
  return m_language;
}

/** read a NAME.kdevpsp and .NAME.kdevpsp
    NAME.kdevpsp contains options for all users, like cvs system
    .NAME.kdevpsp contains options from the local user:
*/
bool ProjectSpace::readConfig(QString abs_filename){
  kdDebug(9000) << "enter ProjectSpace::readConfig" << endl;
  QFileInfo file_info(abs_filename);
  m_path = file_info.dirPath();
  m_projectspace_file = abs_filename;
  // the "global" one
  kdDebug(9000)  << "project_space_filename:" << m_projectspace_file << endl;
  KSimpleConfig* config = new KSimpleConfig(m_projectspace_file);
  readGeneralConfig(config); // maybe virtual overwritten
  config->sync();
  delete config;

  m_user_projectspace_file = m_path + "/." + m_name + ".kdevpsp";
  // the "local/user" on
  kdDebug(9000) << "user_projectspace_filename:" << m_user_projectspace_file << endl;
  config = new KSimpleConfig(m_user_projectspace_file);
  writeUserConfig(config); // maybe virtual overwritten
  config->sync();
  delete config;
  return true;
  
}

bool ProjectSpace::readGeneralConfig(KSimpleConfig* config){
  config->setGroup("General");
  m_name = config->readEntry("name");
  m_path = config->readEntry("path");
  m_plugin_name = config->readEntry("plugin_name");
  m_version = config->readEntry("version","0.1");
  m_language = config->readEntry("programming_language");

  // read all projects
  QStringList files = config->readListEntry("projectfiles");
  QStringList::Iterator it;
  QString filename;
  QFileInfo fileinfo;
  QString abs_filename;
  Project* prj=0;
  for(it = files.begin(); it != files.end(); ++it){
    fileinfo.setFile(*it);
    filename = fileinfo.fileName();
    abs_filename = CToolClass::getAbsolutePath(m_path,fileinfo.dirPath()) + filename;
    // bootstrapping
    KConfig prj_config(abs_filename);
    prj_config.setGroup("General");
    prj = PluginLoader::getNewProject(prj_config.readEntry("plugin_name"));
    if(prj != 0){
      prj->readConfig(abs_filename);
      addProject(prj);
    }
    else {
      cerr << "\nProjectSpace::readGeneralConfig: prj not created!";
    }
  }
  return true;
}

bool ProjectSpace::readUserConfig(KSimpleConfig* config){
  config->setGroup("General");
  m_email = config->readEntry("email");
  m_company = config->readEntry("company");
  m_author = config->readEntry("author_name");
  return true;
}


bool ProjectSpace::writeConfig(){
  kdDebug(9000) << "enter ProjectSpace::writeConfig" << endl;
  // the "global" one
  QString filename = m_path + "/" + m_name + ".kdevpsp";
  kdDebug(9000)  << "filename:" << filename << endl;
  KSimpleConfig* config = new KSimpleConfig(filename);
  writeGeneralConfig(config); // maybe virtual overwritten
  config->sync();
  delete config;

  // write all projectfiles
  Project* prj;
  for(prj=m_projects->first();prj !=0;prj=m_projects->next()){
    prj->writeConfig();
  }
  
  // the "local/user" on
  filename = m_path + "/." + m_name + ".kdevpsp";
  kdDebug(9000) << "filename:" << filename << endl;
  config = new KSimpleConfig(filename);
  writeUserConfig(config); // maybe virtual overwritten
  config->sync();
  delete config;
  return true;
}

bool ProjectSpace::writeGeneralConfig(KSimpleConfig* config){
  config->setGroup("General");
  config->writeEntry("name",m_name);
  config->writeEntry("path",m_path);
  config->writeEntry("plugin_name", m_plugin_name); // the projectspacetype name
  config->writeEntry("version", m_version);
  config->writeEntry("programming_language",m_language);
  QStringList projectfiles;
  Project* prj;
  for(prj=m_projects->first();prj !=0;prj=m_projects->next()){
    // add the relative path
    QString file = prj->getProjectFile();
    projectfiles.append(CToolClass::getRelativePath(m_path,file));
  }
  config->writeEntry("projectfiles",projectfiles);
  return true;
}

bool ProjectSpace::writeUserConfig(KSimpleConfig* config){
  config->setGroup("General");
  config->writeEntry("user", m_plugin_libraryname);
  config->writeEntry("email", m_email);
  config->writeEntry("company", m_company);
  config->writeEntry("author_name", m_author);
  return true;
}

void ProjectSpace::dump(){
  cerr << endl << "ProjectSpace Name: " << m_name;
  cerr << endl << "absolute Path: " << m_path;
  Project* prj;
  for(prj=m_projects->first();prj !=0;prj=m_projects->next()){
    prj->dump();
  }
}
#include "projectspace.moc"
