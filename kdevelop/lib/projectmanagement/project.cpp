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
#include "partloader.h"
#include "KDevCompiler.h"
#include "KDevLinker.h"

Project::Project(QObject * parent, const char* name) :  QObject(parent,name){
  m_pFiles = new QList<RegisteredFile>();
  m_compilers = new QList<KDevCompiler>;

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
QStringList Project::allAbsoluteFileNames()
{
  kdDebug(9030) << "Project::allFileNames()" << endl;
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
  kdDebug(9030) << "Project::fileProperties() return 0!:" << relFileName << endl;
  dump();
  return 0;
}
RegisteredFile* Project::fileAbsolute(QString absFileName){
  QString relFile = CToolClass::getRelativeFile(m_absPath,absFileName);
  RegisteredFile* pFile= file(relFile);
  if(pFile == 0){
    kdDebug(9030) << "Project::filePropertiesAbsolute() return 0!:" << absFileName << endl;
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


//////////////////////////////// some set methods //////////////////////

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
void Project::updateMakefile(QString directory,QTextStream& stream,QString target){
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
	kdDebug(9030) << "Project::removeFile file '" + relFile + "' found and removed" << endl;
      }
    }
  }
}
void Project::showAllFiles(){
  RegisteredFile* pFile;
  kdDebug(9030) << "show all registered Files for: " << m_name << endl;
  for(pFile = m_pFiles->first(); pFile != 0;pFile =  m_pFiles->next() ){
    kdDebug(9030) << "Filename:" << pFile->relativeFile() << endl;
  }
}

void Project::dump(){
  kdDebug(9030) << "Project Name: " << m_name << endl;
  kdDebug(9030) << "relative Path: " << m_relPath << endl;
  kdDebug(9030) << "absolute Path: " << m_absPath << endl;
  kdDebug(9030) << "Version: " << m_version << endl;
  showAllFiles();
}


bool Project::readUserConfig(QDomDocument& /*doc*/,QDomElement& projectElement){
  QDomElement filesElement = projectElement.namedItem("projecttest").toElement();
  if(filesElement.isNull()){
    kdDebug(9030) << "Project::readUserConfig no \"projecttest\" tag found!" << endl;
    return false;
  }
  return true;
}
void Project::updateMakefileEntries(QTextStream& stream){
  stream << "testtarget:";
}



bool Project::writeGlobalConfig(QDomDocument& doc,QDomElement& projectElement)
{
  kdDebug(9030) << "enter Project::writeGlobalConfig" << endl;
  KAboutData* pData = aboutPlugin();
  QString pluginName;
  if(pData !=0){
    pluginName = pData->appName();
  }
  else {
    kdDebug(9030) << "Project::writeGlobalConfig() no aboutPlugin() found :-(";
    return false;
  }
  projectElement.setAttribute("pluginName",pluginName);
  projectElement.setAttribute("name",m_name);
  projectElement.setAttribute("relativePath",m_relPath);
  projectElement.setAttribute("bugfile","bugs.xml");


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
    kdDebug(9030) << "ProjectSpace::readGlobalConfig no \"Files\" tag found!" << endl;
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
  kdDebug(9030) << "enter PluginLoader::getNewProject";
  QString constraint = QString("[Name] == '%1'").arg(projecttypeName);
  KTrader::OfferList offers = KTrader::self()->query("KDevelop/Project", constraint);
  KService *service = *offers.begin();
  kdDebug(9030) << "Found Project Component " << service->name() << endl;

  KLibFactory *factory = KLibLoader::self()->factory(service->library());
  if (!factory){
    kdDebug(9030) << "Factory not available " << service->library()  << endl;
  }
  
  Project* prj  = (Project*)factory->create(parent,service->name().latin1(),
					    "Project");
  if(!prj){
    kdDebug(9030) << "couldn't create the project "<<  service->library()  << endl;
  }else{
  	kdDebug(9030) << "Project::createNewProject: Name: "<<  service->property("Name").toString() << endl;
  	prj->loadCompilers(service);
  	prj->loadLinker(service);
  }
  return prj;
}

//++++++++++ Build-Objects Methods ++++++++++++++++++++++++++++++++++++++

void Project::loadCompilers(KService* ks){
	kdDebug(9030) << "Project::loadCompilers: Compilers needed: "<<  ks->property("X-KDevelop-Compilers").toString() << endl;
	QStringList compilers = QStringList::split(" ", ks->property("X-KDevelop-Compilers").toString());
  QStringList::ConstIterator cit;
  for (cit = compilers.begin(); cit != compilers.end(); ++cit){
  	kdDebug(9030) << "Project::loadCompilers: Expected compiler: " << *cit << endl;
    QString constraint = QString("[Name] == '%1'").arg(*cit);
		KDevCompiler* comp = static_cast<KDevCompiler*>(PartLoader::loadByName(0, *cit, "KDevCompiler"));
  	kdDebug(9030) << "Project::loadCompilers: Loaded compiler: " << *(comp->name()) << endl;
		addCompiler(comp);
  }
}

void Project::loadLinker(KService* ks){
	QString linkerName = ks->property("X-KDevelop-Linker").toString();
	kdDebug(9030) << "Project::loadLinker: Linker needed: "<<  linkerName << endl;
  QString constraint = QString("[Name] == '%1'").arg(linkerName);
  KDevLinker* linker = static_cast<KDevLinker*>(PartLoader::loadByName(0, linkerName, "KDevLinker"));
  kdDebug(9030) << "Project::loadLinker: Loaded Linker: " << *(linker->name()) << endl;
	registerLinker(linker);
}

void Project::addCompiler(KDevCompiler* comp){
	m_compilers->append(comp);
}


QList<KDevCompiler>* Project::compilers(){
	return m_compilers;
}

KDevCompiler* Project::compilerByName(const QString &name){
	QListIterator<KDevCompiler> it(*m_compilers);
  for ( ; it.current(); ++it ) {
    KDevCompiler *comp = it.current();
    if (*(comp->name()) == name){
    	return comp;
    }
  }
  // nothing found
  return 0;
}

void Project::registerLinker(KDevLinker* linker){
	m_linker = linker;
}

KDevLinker* Project::linker(){
	return m_linker;
}

/*----------------------------------------------- CProject::getType()
 * getType()
 *   Return the type of file based on the extension.
 *
 * Parameters:
 *   aFile           The file to check.
 *
 * Returns:
 *   ProjectFileType The filetype.
 *   DATA            If unknown.
 *-----------------------------------------------------------------*/
QString Project::getType(QString absFileName){
  QString retVal="DATA";
  
  QString ext(absFileName);
  int pos = ext.findRev('.');

  if (pos == -1 ){ return retVal;} // not found, so DATA
  ext = ext.right(ext.length()-pos);
  
  //ext = rindex( aFile, '.' );
  if( !ext.isEmpty() ){
    // Convert to lowercase.
    ext = ext.lower();
    
    // Check for a known extension.
    if( ext == ".cpp" || ext == ".c" || ext == ".cc" ||
	ext == ".ec" || ext == ".ecpp" || ext == ".C" || ext == ".cxx" || ext == ".ui" )
      retVal = "SOURCE";
    else if( ext == ".h" || ext == ".hxx" || ext == ".hpp" || ext == ".H" || ext == ".hh" )
      retVal = "HEADER";
    else if( ext == ".l++" || ext == ".lxx" || ext == ".ll" || ext == ".l")
      retVal = "LEXICAL";
  }
  return retVal;
}

#include "project.moc"
