/***************************************************************************
                          project.cpp  -  description
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

#include "project.h"
#include <iostream.h>
#include "registeredfile.h"
#include <kdebug.h>
#include <ctoolclass.h>
#include <qfileinfo.h>
#include <kaboutdata.h>
#include <kservice.h>
#include <ktrader.h>
#include <klibloader.h>

Project::Project(QObject * parent, const char* name) :  QObject(parent,name){
  m_pFiles = new QList<RegisteredFile>();
}
Project::~Project(){
}
/*____some get methods_____ */

QString Project::projectFile(){
  return m_projectFile;
}
QString Project::userProjectFile(){
  return m_userProjectFile;
}
/** */
QStringList Project::allSources(){
  QStringList list;
  return list;
}

/** returns all files*/
QStringList Project::allAbsoluteFileNames(){
  cerr  << endl << "kdevelop (project): Project::allFileNames() ";
  QStringList list;
  RegisteredFile* pFile;
  for(pFile = m_pFiles->first(); pFile != 0;pFile =  m_pFiles->next() ){
    list.append(CToolClass::getAbsoluteFile(m_absPath,pFile->relativeFile()));
  }
  return list;
}
RegisteredFile* Project::file(QString relFileName){
  RegisteredFile* pFile;
  for(pFile = m_pFiles->first(); pFile != 0;pFile =  m_pFiles->next() ){
    if(relFileName == pFile->relativeFile()){
      return pFile;
    }
  }
  cerr << endl << "kdevelop (project): Project::fileProperties() return 0!:" << relFileName;
  dump();
  return 0;
}
RegisteredFile* Project::fileAbsolute(QString absFileName){
  QString relFile = CToolClass::getRelativeFile(m_absPath,absFileName);
  RegisteredFile* pFile= file(relFile);
  if(pFile == 0){
    cerr << endl << "kdevelop (project): Project::filePropertiesAbsolute() return 0!:" << absFileName;
    dump();
  }
  return pFile;
}
QString Project::version(){
  return m_version;
}
QString Project::name(){
  return m_name;
}
QString Project::absolutePath(){
  return m_absPath;
}
QString Project::relativePath(){
  return m_relPath;
}
void Project::setRelativePath(QString path){
  m_relPath = path;
}
//////////////////////////////// some set methods//////////////////////

/** store the project version. */
void Project::setVersion(QString version){
  m_version = version;
}
void Project::setName(QString name){
  m_name = name;
}
void Project::setAbsolutePath(QString path){
  m_absPath = path;
  m_projectFile = m_absPath + "/" + m_name + ".kdevprj2";
  m_userProjectFile = m_absPath + "/." + m_name + ".kdevprj2";
}
/** generate/modifiy the Makefile*/
void Project::updateMakefile(){
}
void Project::addFile(RegisteredFile* pFile){
  m_pFiles->append(pFile);
}

void Project::addFile(QString absFilename){
  QString relFile = CToolClass::getRelativeFile(m_absPath,absFilename);
  RegisteredFile* pFile = new RegisteredFile(relFile);
  m_pFiles->append(pFile);
}


void Project::removeFile(RegisteredFile* /*pFile*/){
  //	  m_files->remove(file);
}
void Project::removeFile(QString absFilename){
  QString relFile = CToolClass::getRelativeFile(m_absPath,absFilename);
  RegisteredFile* pFile;
  for(pFile = m_pFiles->first(); pFile != 0;pFile =  m_pFiles->next() ){
    if( pFile->relativeFile() == relFile){
      if(m_pFiles->remove(pFile)){
	cerr << endl << " Project::removeFile file '" + relFile + "' found and removed";
      }
    }
  }
}
void Project::showAllFiles(){
  RegisteredFile* pFile;
  cerr << endl << "show all registered Files for: " << m_name;
  for(pFile = m_pFiles->first(); pFile != 0;pFile =  m_pFiles->next() ){
    cerr << "\nFilename:" << pFile->relativeFile();
  }
}

void Project::dump(){
  cerr << endl << "Project Name: " << m_name;
  cerr << endl << "relative Path: " << m_relPath;
  cerr << endl << "absolute Path: " << m_absPath;
  cerr << endl << "Version: " << m_version;
  showAllFiles();
}


bool Project::readUserConfig(QDomDocument& /*doc*/,QDomElement& projectElement){
  QDomElement filesElement = projectElement.namedItem("projecttest").toElement();
  if(filesElement.isNull()){
    cerr << "\nProject::readUserConfig no \"projecttest\" tag found!";
    return false;
  }
  return true;
}



bool Project::writeGlobalConfig(QDomDocument& doc,QDomElement& projectElement){
  cerr << "\nenter Project::writeGlobalConfig";
  KAboutData* pData = aboutPlugin();
  QString pluginName;
  if(pData !=0){
    pluginName = pData->appName();
  }
  else {
    kdDebug(9000) << "Project::writeGlobalConfig() no aboutPlugin() found :-(";
    return false;
  }
  projectElement.setAttribute("pluginName",pluginName);
  projectElement.setAttribute("name",m_name);
  projectElement.setAttribute("relativePath",m_relPath);
  

  // Files tag
  QDomElement filesElement = projectElement.appendChild(doc.createElement("Files")).toElement();
  QStringList fileList;
  RegisteredFile* pFile=0;
  for(pFile = m_pFiles->first(); pFile != 0; pFile= m_pFiles->next() ){
    QDomElement fileElement = filesElement.appendChild(doc.createElement("File")).toElement();
    pFile->writeConfig(doc,fileElement);
  }
  return true;
  
}
bool Project::writeUserConfig(QDomDocument& doc,QDomElement& projectElement){
  QDomElement openFilesElement = projectElement.appendChild(doc.createElement("projecttest")).toElement();
  return true;
}

bool Project::readGlobalConfig(QDomDocument&/*doc*/,QDomElement& projectElement){
  m_name = projectElement.attribute("name");
  m_relPath =  projectElement.attribute("relativePath");

  QDomElement filesElement = projectElement.namedItem("Files").toElement();
  if(filesElement.isNull()){
    cerr << "\nProjectSpace::readGlobalConfig no \"Files\" tag found!";
    return false;
  }

  RegisteredFile* file;
  QDomNodeList fileList = filesElement.elementsByTagName("File");
  unsigned int i;
  for (i = 0; i < fileList.count(); ++i){
    QDomElement fileElement = fileList.item(i).toElement();
    file = new RegisteredFile();
    file->readConfig(fileElement);
    m_pFiles->append(file);
  }
  //  showAllFiles();
  return true;
}
KAboutData* Project::aboutPlugin(){
  return 0;
}
// a Factory ?

Project* Project::createNewProject(QString projecttypeName,QObject* parent){
  kdDebug(9000) << "enter PluginLoader::getNewProject";
  QString constraint = QString("[Name] == '%1'").arg(projecttypeName);
  KTrader::OfferList offers = KTrader::self()->query("KDevelop/Project", constraint);
  KService *service = *offers.begin();
  kdDebug(9000) << "Found Project Component " << service->name() << endl;

  KLibFactory *factory = KLibLoader::self()->factory(service->library());
  if (!factory){
    kdDebug(9000) << "Factory not available " << service->library()  << endl;
  }
  
  Project* prj  = (Project*)factory->create(parent,service->name().latin1(),
					    "Project");
  if(!prj){
    kdDebug(9000) << "couldn't create the project "<<  service->library()  << endl;
  }
  return prj;
}

#include "project.moc"
