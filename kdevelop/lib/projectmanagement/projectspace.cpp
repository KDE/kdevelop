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
#include <kprocess.h>
#include <kstddirs.h>
#include <iostream.h>
#include <ksimpleconfig.h>
#include <kdebug.h>
#include <qdir.h>
#include <ctoolclass.h>
#include <qfileinfo.h>
#include <qdatetime.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <qtextstream.h>
#include <kaction.h>
#include <kpopupmenu.h>
#include <kaboutdata.h>


ProjectSpace::ProjectSpace(QObject* parent,const char* name) : KDevComponent(parent,name){
  m_pProjects = new QList<Project>;
  m_pUserDoc = 0;
  m_pGlobalDoc = 0;
}
ProjectSpace::~ProjectSpace(){
}
void ProjectSpace::setupGUI(){
  cerr << endl << "enter ProjectSpace::setupGUI";
  KActionMenu* pActionMenu = new KActionMenu(i18n("Set active Project"),actionCollection(),"project_set_active");
  connect( pActionMenu->popupMenu(), SIGNAL( activated( int ) ),
	   this, SLOT( slotProjectSetActivate( int ) ) );
}
void ProjectSpace::slotProjectSetActivate( int id){
  kdDebug(9000) << "KDevelopCore::slotProjectSetActivate";
  KActionCollection *pAC = actionCollection();
  QPopupMenu* pMenu = ((KActionMenu*)pAC->action("project_set_active"))->popupMenu();
  QString name = pMenu->text(id);
  setCurrentProject(name);
  fillActiveProjectPopupMenu();
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
void ProjectSpace::addProject(Project* prj){
  cerr << endl << "enter ProjectSpace::addProject";
  m_pProjects->append (prj);
  m_pCurrentProject = prj;
}
void ProjectSpace::setCurrentProject(Project* prj){
  m_pCurrentProject = prj;
}

void ProjectSpace::setCurrentProject(QString name){
  Project* pProject;
  for(pProject=m_pProjects->first();pProject !=0;pProject=m_pProjects->next()){
      if(pProject->name() == name){
	m_pCurrentProject = pProject;
      }
  }  
}
Project* ProjectSpace::currentProject(){
  return m_pCurrentProject;
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
  QString args = "xzvf " + m_projectspaceTemplate + " -C " + m_path;
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
QString ProjectSpace::name(){
  return m_name;
}

/** Fetch the name of the version control system */
QString ProjectSpace::VCSystem(){
  return "";
}

/** Fetch the authors name. stored in the *_user files*/
QString ProjectSpace::author(){
  return m_author;
}

/** Fetch the authors eMail-address,  stored in the *_user files */
QString ProjectSpace::email(){
  return m_email;
}
QString ProjectSpace::company(){
  return m_company;
}

/** set the projectspace name*/
void ProjectSpace::setName(QString name){
  m_name = name;
}
void ProjectSpace::setAbsolutePath(QString path){
  m_path = path;
}

QString ProjectSpace::absolutePath(){
  return m_path;
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

bool ProjectSpace::readConfig(QString absFilename){
  kdDebug(9000) << "enter ProjectSpace::readXMLConfig" << endl;
  QFileInfo fileInfo(absFilename);
  m_path = fileInfo.dirPath();
  m_projectspaceFile = absFilename;

  QFile file(absFilename);
  if (!file.open(IO_ReadOnly)){
    KMessageBox::sorry(0, i18n("Can't open the file %1")
		       .arg(absFilename));
    return false;
  }
  
  if(m_pGlobalDoc != 0){
    delete m_pGlobalDoc;
  }
  m_pGlobalDoc = new QDomDocument("KDevProjectSpace");
  
  // Parse the XML file.
  // Read in file and check for a valid XML header.
  if (!m_pGlobalDoc->setContent(&file)){
    KMessageBox::sorry(0,
		       i18n("The file %1 does not contain valid XML").arg(absFilename));
    return false;
  }
  // Check for proper document type.
  if (m_pGlobalDoc->doctype().name() != "KDevProjectSpace"){
    KMessageBox::sorry(0,
		       i18n("The file %1 does not contain a valid ProjectSpace\n"
			    "definition, which must have a document type\n"
			    "'KDevProjectSpace'").arg(absFilename));
    return false;
  }
  QDomElement psElement = m_pGlobalDoc->documentElement();
  m_name = psElement.attribute("name");
  //  m_path = psElement.attribut("path");
  m_version = psElement.attribute("version");
  QString lastActive = psElement.attribute("lastActiveProject");
  readGlobalConfig(*m_pGlobalDoc,psElement);
  setCurrentProject(lastActive);
  file.close();

  // the "user" one
  m_userProjectspaceFile = m_path + "/." + m_name + ".kdevpsp";
  file.setName(m_userProjectspaceFile);
  if (!file.open(IO_ReadOnly)){
    KMessageBox::sorry(0, i18n("Can't open the file %1")
		       .arg(absFilename));
    return false;
  }
  
  if(m_pUserDoc != 0){
    delete m_pUserDoc;
  }
  m_pUserDoc = new QDomDocument("KDevProjectSpace_User");
  // Parse the XML file.
  // Read in file and check for a valid XML header.
  if (!m_pUserDoc->setContent(&file)){
    KMessageBox::sorry(0,
		       i18n("The file %1 does not contain valid XML").arg(m_userProjectspaceFile));
    return false;
  }
  // Check for proper document type.
  if (m_pUserDoc->doctype().name() != "KDevProjectSpace_User"){
    KMessageBox::sorry(0,
		       i18n("The file %1 does not contain a valid ProjectSpace\n"
			    "definition, which must have a document type\n"
			    "'KDevProjectSpace_User'").arg(m_userProjectspaceFile));
    return false;
  }
  QDomElement userPsElement = m_pUserDoc->documentElement();
  readUserConfig((*m_pUserDoc),userPsElement);
  fillActiveProjectPopupMenu();
  return true;
}


bool ProjectSpace::readGlobalConfig(QDomDocument& doc,QDomElement& psElement){
  QDomElement projectsElement = psElement.namedItem("Projects").toElement();
  if(projectsElement.isNull()){
    cerr << "\nProjectSpace::readGlobalConfig no \"Projects\" tag found!";
    return false;
  }
  Project* pProject=0;
  QDomNodeList projectList = projectsElement.elementsByTagName("Project");
  unsigned int i;
  for (i = 0; i < projectList.count(); ++i){
    QDomElement projectElement = projectList.item(i).toElement();
    QString prjPluginName = projectElement.attribute("pluginName");
    pProject = Project::createNewProject(prjPluginName);
    if(pProject != 0){
      pProject->readGlobalConfig(doc,projectElement);
      pProject->setAbsolutePath(CToolClass::getAbsolutePath(m_path,pProject->relativePath()));
      addProject(pProject);
    }
    else {
      cerr << "\nProjectSpace::readGlobalConfig:  pProjectnot created!";
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
  Project* pProject=0;
  QDomNodeList projectList = projectsElement.elementsByTagName("Project");
  unsigned int i;
  for (i = 0; i < projectList.count(); ++i){
    QDomElement projectElement = projectList.item(i).toElement();
    // not a good solution, the projectspace know to much from the project document structure :-(
    QString prjName = projectElement.attribute("name"); 
    for(pProject=m_pProjects->first();pProject !=0;pProject=m_pProjects->next()){
      if(pProject->name() == prjName){
	pProject->readGlobalConfig(doc,projectElement);
      }
    }
  }
  return true;
}
/*
  // xml stuff
  bool ProjectSpace::writeXMLConfig(){
  kdDebug(9000) << "\nenter ProjectSpace::writeXMLConfig" << endl;
  
  // the "global" one
  if(m_pGlobalDoc != 0){
  delete m_pGlobalDoc;
  }
  m_pGlobalDoc = new QDomDocument("KDevProjectSpace");
  m_pGlobalDoc->appendChild(m_pGlobalDoc->createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\""));
  // save work projectspace information
  QDomElement ps = m_pGlobalDoc->appendChild(m_pGlobalDoc->createElement("ProjectSpace")).toElement();
  // add Attributes
  ps.setAttribute("name",m_name);
  //  psElement.setAttribute("path",m_path);
  KAboutData* pData = aboutPlugin();
  QString pluginName;
  if(pData !=0){
    pluginName = pData->appName();
  }
  else {
    kdDebug(9000) << "ProjectSpace::writeXMLConfig() no aboutPlugin() found :-(";
    return false;
  }
  ps.setAttribute("pluginName", pluginName); // the projectspacetype name
  ps.setAttribute("version", m_version);
  ps.setAttribute("lastActiveProject",m_pCurrentProject->name());

  writeGlobalConfig(*m_pGlobalDoc,ps);

  QString filename = m_path + "/" + m_name + ".kdevpsp";
  kdDebug(9000)  << "filename::" << filename << endl;
  QFile file(filename);
  if (!file.open(IO_WriteOnly)){
    KMessageBox::sorry(0, i18n("Can't save file %1")
		       .arg(filename));
    return false;
  }
  QTextStream s(&file);
  m_pGlobalDoc->save(s,4);
  file.close();
  
  // the "user" one
  if(m_pUserDoc != 0){
    delete m_pUserDoc;
  }
  m_pUserDoc = new QDomDocument("KDevProjectSpace_User");
  m_pUserDoc->appendChild(m_pUserDoc->createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""));
  // save work projectspace information
  QDomElement userPs = m_pUserDoc->appendChild(m_pUserDoc->createElement("ProjectSpace")).toElement();
  writeUserConfig(*m_pUserDoc,userPs);
  filename = m_path + "/." + m_name + ".kdevpsp";
  kdDebug(9000)  << "filename::" << filename << endl;
  file.setName(filename);
  if (!file.open(IO_WriteOnly)){
    KMessageBox::sorry(0, i18n("Can't save file %1")
		       .arg(filename));
    return false;
  }
  QTextStream userStream(&file);
  m_pUserDoc->save(userStream,4);
  file.close();
  return true;
  }
*/
// add the data to the psElement (Projectspace)
bool ProjectSpace::writeGlobalConfig(QDomDocument& doc, QDomElement& psElement){
  cerr << "\nenter ProjectSpace::writeGlobalConfig";
  
  // add <Projects>
  QDomElement projectsElement = psElement.appendChild(doc.createElement("Projects")).toElement();
  QStringList projectfiles;
  Project* pProject=0;
  for(pProject=m_pProjects->first();pProject !=0;pProject=m_pProjects->next()){
    QDomElement prjElement = projectsElement.appendChild(doc.createElement("Project")).toElement();
        pProject->writeGlobalConfig(doc,prjElement);
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
  Project* pProject=0;
  for(pProject=m_pProjects->first();pProject !=0;pProject=m_pProjects->next()){
    QDomElement prjElement = projectsElement.appendChild(doc.createElement("Project")).toElement();
        pProject->writeUserConfig(doc,prjElement);
  }
  return true;
}


QString ProjectSpace::programmingLanguage(){
  return m_language;
}
QStringList ProjectSpace::allProjectNames(){
  QStringList list;
  Project* pProject=0;
  for(pProject=m_pProjects->first();pProject !=0;pProject=m_pProjects->next()){
    list.append(pProject->name());
  }
  return list;
}

void ProjectSpace::dump(){
  cerr << endl << "ProjectSpace Name: " << m_name;
  cerr << endl << "absolute Path: " << m_path;
  Project* pProject=0;
  for(pProject=m_pProjects->first();pProject !=0;pProject=m_pProjects->next()){
    pProject->dump();
  }
}

void ProjectSpace::fillActiveProjectPopupMenu(){
  KActionCollection *pAC = actionCollection();
  QPopupMenu* pMenu = ((KActionMenu*)pAC->action("project_set_active"))->popupMenu();
  pMenu->clear();
  Project* pProject=0;
  int id=0;
  for(pProject=m_pProjects->first();pProject !=0;pProject=m_pProjects->next()){
    id = pMenu->insertItem(pProject->name());
    if(m_pCurrentProject->name() == pProject->name()){
      pMenu->setItemChecked(id,true); // set the current Project
    }
  }
}

QDomDocument* ProjectSpace::writeGlobalDocument(){
  kdDebug(9000) << "\nenter ProjectSpace::writeXMLConfig" << endl;

   // the "global" one
  if(m_pGlobalDoc != 0){
    delete m_pGlobalDoc;
  }
  m_pGlobalDoc = new QDomDocument("KDevProjectSpace");
  m_pGlobalDoc->appendChild(m_pGlobalDoc->createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\""));
  // save work projectspace information
  QDomElement ps = m_pGlobalDoc->appendChild(m_pGlobalDoc->createElement("ProjectSpace")).toElement();
  // add Attributes
  ps.setAttribute("name",m_name);
  //  psElement.setAttribute("path",m_path);
  KAboutData* pData = aboutPlugin();
  QString pluginName;
  if(pData !=0){
    pluginName = pData->appName();
  }
  else {
    kdDebug(9000) << "ProjectSpace::writeXMLConfig() no aboutPlugin() found :-(";
    return false;
  }
  ps.setAttribute("pluginName", pluginName); // the projectspacetype name
  ps.setAttribute("version", m_version);
  ps.setAttribute("lastActiveProject",m_pCurrentProject->name());

  writeGlobalConfig(*m_pGlobalDoc,ps);
  return m_pGlobalDoc;
}

QDomDocument* ProjectSpace::writeUserDocument(){
  if(m_pUserDoc != 0){
    delete m_pUserDoc;
  }
  m_pUserDoc = new QDomDocument("KDevProjectSpace_User");
  m_pUserDoc->appendChild(m_pUserDoc->createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""));
  // save work projectspace information
  QDomElement userPs = m_pUserDoc->appendChild(m_pUserDoc->createElement("ProjectSpace")).toElement();
  writeUserConfig(*m_pUserDoc,userPs);
  return m_pUserDoc;
}
QDomDocument* ProjectSpace::readGlobalDocument(){
  return m_pGlobalDoc;
}
QDomDocument* ProjectSpace::readUserDocument(){
  return m_pUserDoc;
}

bool ProjectSpace::saveConfig(){
  QString filename = m_path + "/" + m_name + ".kdevpsp";
  kdDebug(9000)  << "filename::" << filename << endl;
  QFile file(filename);
  if (!file.open(IO_WriteOnly)){
    KMessageBox::sorry(0, i18n("Can't save file %1")
		       .arg(filename));
    return false;
  }
  QTextStream s(&file);
  m_pGlobalDoc->save(s,4);
  file.close();

  filename = m_path + "/." + m_name + ".kdevpsp";
  kdDebug(9000)  << "filename::" << filename << endl;
  file.setName(filename);
  if (!file.open(IO_WriteOnly)){
    KMessageBox::sorry(0, i18n("Can't save file %1")
		       .arg(filename));
    return false;
  }
  QTextStream userStream(&file);
  m_pUserDoc->save(userStream,4);
  file.close();
  return true;
}


#include "projectspace.moc"
