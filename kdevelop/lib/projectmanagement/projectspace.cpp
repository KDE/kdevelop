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
    readXMLConfig(file);
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
			 i18n("The file %1 does not contain a valid ProjectSpace\n"
			      "definition, which must have a document type\n"
			      "'KDevProjectSpace'").arg(absFilename));
      return false;
    }
  QDomElement psElement = doc.documentElement();
  m_name = psElement.attribute("name");
  //  m_path = psElement.attribut("path");
  m_version = psElement.attribute("version");
  readGlobalConfig(doc,psElement);
  file.close();

  // the "user" one
  m_user_projectspace_file = m_path + "/." + m_name + ".kdevpsp";
  file.setName(m_user_projectspace_file);
  if (!file.open(IO_ReadOnly)){
    KMessageBox::sorry(0, i18n("Can't open the file %1")
		       .arg(absFilename));
    return false;
  }
  
  // Parse the XML file.
  QDomDocument userDoc;
  // Read in file and check for a valid XML header.
  if (!userDoc.setContent(&file)){
    KMessageBox::sorry(0,
		       i18n("The file %1 does not contain valid XML").arg(m_user_projectspace_file));
      return false;
  }
  // Check for proper document type.
  if (userDoc.doctype().name() != "KDevProjectSpace_User"){
      KMessageBox::sorry(0,
			 i18n("The file %1 does not contain a valid ProjectSpace\n"
			      "definition, which must have a document type\n"
			      "'KDevProjectSpace_User'").arg(m_user_projectspace_file));
      return false;
  }
  QDomElement userPsElement = userDoc.documentElement();
  readUserConfig(doc,userPsElement);
  return true;
}



bool ProjectSpace::readGlobalConfig(QDomDocument& doc,QDomElement& psElement){
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
  
  QDomElement testElement = psElement.namedItem("test").toElement();
  QString testValue = testElement.attribute("name");
  cerr << "\nProjectSpace::readUserConfig test element conaints value: " << testValue;
  // projects
  QDomElement projectsElement = psElement.namedItem("Projects").toElement();
  if(projectsElement.isNull()){
    cerr << "\nProjectSpace::readUserConfig no \"Projects\" tag found!";
    return false;
  }
  Project* prj=0;
  QDomNodeList projectList = projectsElement.elementsByTagName("Project");
  unsigned int i;
  for (i = 0; i < projectList.count(); ++i){
    QDomElement projectElement = projectList.item(i).toElement();
    // not a good solution, the projectspace know to much from the project document structure :-(
    QString prjName = projectElement.attribute("name"); 
    for(prj=m_projects->first();prj !=0;prj=m_projects->next()){
      if(prj->getName() == prjName){
	prj->readGlobalConfig(doc,projectElement);
      }
    }
  }
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
  // add Attributes
  ps.setAttribute("name",m_name);
  //  psElement.setAttribute("path",m_path);
  ps.setAttribute("pluginName", m_plugin_name); // the projectspacetype name
  ps.setAttribute("version", m_version);

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

  // the "user" one
  QDomDocument userDoc("KDevProjectSpace_User");
  userDoc.appendChild(doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""));
  // save work projectspace information
  QDomElement userPs = userDoc.appendChild(userDoc.createElement("ProjectSpace")).toElement();
  writeUserConfig(userDoc,userPs);
  filename = m_path + "/." + m_name + ".kdevpsp";
  kdDebug(9000)  << "filename::" << filename << endl;
  file.setName(filename);
  if (!file.open(IO_WriteOnly)){
    KMessageBox::sorry(0, i18n("Can't save file %1")
		       .arg(filename));
    return false;
  }
  QTextStream userStream(&file);
  userStream << userDoc;
  file.close();
  return true;
}
// add the data to the psElement (Projectspace)
bool ProjectSpace::writeGlobalConfig(QDomDocument& doc, QDomElement& psElement){
  cerr << "\nenter ProjectSpace::writeGlobalConfig";
  
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
bool ProjectSpace::writeUserConfig(QDomDocument& doc,QDomElement& psElement){
  cerr << "\nenter ProjectSpace::writeUserConfig";
  QDomElement testElement = psElement.appendChild(doc.createElement("Test")).toElement();
  testElement.setAttribute("name","noname");

  // add <Projects>
  QDomElement projectsElement = psElement.appendChild(doc.createElement("Projects")).toElement();
  QStringList projectfiles;
  Project* prj;
  for(prj=m_projects->first();prj !=0;prj=m_projects->next()){
    QDomElement prjElement = projectsElement.appendChild(doc.createElement("Project")).toElement();
        prj->writeUserConfig(doc,prjElement);
  }
  return true;
}


QString ProjectSpace::getProgrammingLanguage(){
  return m_language;
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
