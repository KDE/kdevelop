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
#include "addexistingfilesdlg.h"
#include "kdevlanguagesupport.h"
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
#include <kdevactions.h>
#include <kpopupmenu.h>
#include <kaboutdata.h>
#include <ktrader.h>
#include <klibloader.h>
#include <qlineedit.h>
#include "renamefiledlg.h"
#include <kio/job.h>
#include <kfiledialog.h>
#include "kdevnodes.h"

void FileGroup::setName(QString name) {
  m_name = name;
}
QString FileGroup::name(){
  return m_name;
}
void FileGroup::setFilter(QString filter){
  m_filter = filter;
}
QString FileGroup::filter(){
  return m_filter;
}

ProjectSpace::ProjectSpace(QObject* parent,const char* name)
    : KDevComponent(parent,name){
  m_pProjects = new QList<Project>;
  m_pUserDoc = 0;
  m_pGlobalDoc = 0;
  m_pCurrentProject =0;
}

ProjectSpace::~ProjectSpace(){
}


void ProjectSpace::setupGUI()
{
  kdDebug(9030) << "enter ProjectSpace::setupGUI" << endl;
  KActionMenu* pActionMenu = new KActionMenu(i18n("Set active Project"),actionCollection(),"project_set_active");
  connect( pActionMenu->popupMenu(), SIGNAL( activated( int ) ),
	   this, SLOT( slotProjectSetActivate( int ) ) );

   KAction* pAction = new KAction(i18n("Add existing File(s)..."), "file", 0, 
			 this, SLOT( slotProjectAddExistingFiles() ),
			 actionCollection(), "project_add_existing_files");
  pAction->setEnabled(true);
  pAction->setStatusText( i18n("Adds existing file(s) to the active project") );
  
}


void ProjectSpace::slotProjectSetActivate( int id)
{
  kdDebug(9030) << "ProjectSpace::slotProjectSetActivate" << endl;
  KActionCollection *pAC = actionCollection();
  QPopupMenu* pMenu = ((KActionMenu*)pAC->action("project_set_active"))->popupMenu();
  QString name = pMenu->text(id);
  setCurrentProject(name);
  fillActiveProjectPopupMenu();
}


void ProjectSpace::slotProjectAddExistingFiles()
{
   kdDebug(9030) << "ProjectSpace::slotProjectAddExistingFiles" << endl;
   QStringList fileFilters;
   if(languageSupport() != 0){
     fileFilters = languageSupport()->fileFilters();
   }
   else {
     fileFilters << "*"; // default
   }
   //AddExistingFilesDlg dlg(0,"addDlg",m_path,fileFilters);
   AddExistingFilesDlg dlg(0,"addDlg",QDir::homeDirPath(),fileFilters);
   

   if(dlg.exec()){
     QStringList files = dlg.addedFiles();

     if (m_pCurrentProject != 0){
       for ( QStringList::Iterator it = files.begin(); it != files.end(); ++it ) { 
	 m_pCurrentProject->addFile(*it);
	 KDevFileNode* pNode = new KDevFileNode(*it,m_name,m_pCurrentProject->name());
	 emit sigAddedFileToProject(pNode); // inform the other components
       }
     }
     else{
       kdDebug(9030) << "Error! no current project found!!" << endl;
     }
     
   }
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
void ProjectSpace::addProject(Project* pProject)
{
  kdDebug(9030) << "enter ProjectSpace::addProject" << endl;
  m_pProjects->append (pProject);
  setCurrentProject(pProject);
  KDevNode* pNode = new KDevNode(m_name,pProject->name());
  emit sigAddedProject(pNode);
}


void ProjectSpace::setCurrentProject(Project* prj){
    // Change to the new project.
    m_pCurrentProject = prj;

    // Notify all components that we are switching current project.
    TheCore()->changeProjectSpace ();

    // Update the popup menu.
    fillActiveProjectPopupMenu();
}


void ProjectSpace::setCurrentProject(QString name)
{
  Project* pProject;

  for(pProject=m_pProjects->first();pProject !=0;pProject=m_pProjects->next())
  {
      if(pProject->name() == name)
      {
	    setCurrentProject(pProject);
      }
  }  
}


Project* ProjectSpace::currentProject(){
  return m_pCurrentProject;
}


void ProjectSpace::removeProject(QString /*name*/){
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
  kdDebug(9030) << "ProjectSpace::generateDefaultFiles():" << endl;
  kdDebug(9030) << args << endl;
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

/** Fetch the author's initials. stored in the *_user files*/
QString ProjectSpace::initials(){
  return m_initials;
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
QString ProjectSpace::projectSpaceFile(){
  return m_projectspaceFile;
}

QString ProjectSpace::absolutePath(){
  return m_path;
}
/** Store the name of version control system */
void ProjectSpace::setVCSystem(QString /*vcsystem*/){
}

/** stored in the *_user files*/
void ProjectSpace::setAuthor(QString name){
  m_author = name;
}


/** stored in the *_user files*/
void ProjectSpace::setInitials(QString initials)
{
  m_initials = initials;
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
  kdDebug(9030) << "enter ProjectSpace::readConfig" << endl;
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
    kdDebug(9030) << "ProjectSpace::readGlobalConfig no \"Projects\" tag found!" << endl;
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
      m_pProjects->append (pProject);
    }
    else {
      kdDebug(9030) << "ProjectSpace::readGlobalConfig:  pProject not created!" << endl;
    }
  }
  return true;
 
}


bool ProjectSpace::readUserConfig(QDomDocument& doc,QDomElement& psElement){
  
  QDomElement testElement = psElement.namedItem("test").toElement();
  QString testValue = testElement.attribute("name");
  kdDebug(9030) << "ProjectSpace::readUserConfig test element contains value: " << testValue << endl;
  // projects
  QDomElement projectsElement = psElement.namedItem("Projects").toElement();
  if(projectsElement.isNull()){
    kdDebug(9030) << "ProjectSpace::readUserConfig no \"Projects\" tag found!" << endl;
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
  kdDebug(9030) << "\nenter ProjectSpace::writeXMLConfig" << endl;
  
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
    kdDebug(9030) << "ProjectSpace::writeXMLConfig() no aboutPlugin() found :-(";
    return false;
  }
  ps.setAttribute("pluginName", pluginName); // the projectspacetype name
  ps.setAttribute("version", m_version);
  ps.setAttribute("lastActiveProject",m_pCurrentProject->name());

  writeGlobalConfig(*m_pGlobalDoc,ps);

  QString filename = m_path + "/" + m_name + ".kdevpsp";
  kdDebug(9030)  << "filename::" << filename << endl;
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
  kdDebug(9030)  << "filename::" << filename << endl;
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
bool ProjectSpace::writeGlobalConfig(QDomDocument& doc, QDomElement& psElement)
{
  kdDebug(9030) << "enter ProjectSpace::writeGlobalConfig" << endl;
  
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


bool ProjectSpace::writeUserConfig(QDomDocument& doc,QDomElement& psElement)
{
  kdDebug(9030) << "enter ProjectSpace::writeUserConfig" << endl;
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
  KActionMenu* pAction = (KActionMenu*)pAC->action("project_set_active");
  if(pAction == 0){ // no action available
    return;
  }
  //  cerr << endl << "fillActiveProjectPopupMenu()";
  QPopupMenu* pMenu = pAction->popupMenu();
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

QDomDocument* ProjectSpace::writeGlobalDocument()
{
  kdDebug(9030) << "enter ProjectSpace::writeXMLConfig" << endl;

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
    kdDebug(9030) << "ProjectSpace::writeXMLConfig() no aboutPlugin() found :-(";
    return 0L;
  }
  ps.setAttribute("pluginName", pluginName); // the projectspacetype name
  ps.setAttribute("version", m_version);
  ps.setAttribute("lastActiveProject",m_pCurrentProject->name());
  ps.setAttribute("bugfile","bugs.xml");    //HACK: Add properly into project later.

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
  m_projectspaceFile = m_path + "/" + m_name + ".kdevpsp";
  
  kdDebug(9030)  << "filename::" << m_projectspaceFile << endl;
  QFile file(m_projectspaceFile);
  if (!file.open(IO_WriteOnly)){
    KMessageBox::sorry(0, i18n("Can't save file %1")
		       .arg(m_projectspaceFile));
    return false;
  }
  QTextStream s(&file);
  m_pGlobalDoc->save(s,4);
  file.close();

  QString filename = m_path + "/." + m_name + ".kdevpsp";
  kdDebug(9030)  << "filename::" << filename << endl;
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


QList<FileGroup> ProjectSpace::defaultFileGroups(){
  QList<FileGroup> list;
  FileGroup* pGroup = new FileGroup();
  pGroup->setName(i18n("Source Files"));
  pGroup->setFilter("*.cpp;*.c");

  list.append(pGroup);
  pGroup = new FileGroup();
  pGroup->setName(i18n("Header Files"));
  pGroup->setFilter("*.h");
  list.append(pGroup);
  return list;
}
QList<Project>* ProjectSpace::allProjects(){
  return m_pProjects;
}
Project* ProjectSpace::project(QString projectName){
  Project* pProject=0;
  for(pProject=m_pProjects->first();pProject !=0;pProject=m_pProjects->next()){
    if (pProject->name() == projectName){
      return pProject;
    }
  }
  kdDebug(9030) << "ProjectSpace::project()  return 0!" << endl;
  return 0;
}

QList<KAction> ProjectSpace::kdevNodeActions(KDevNode* pNode){
  QList<KAction> pList;

  KDevNodeAction* pAction=0;
  if(pNode->inherits("KDevFileNode")){
    pAction = new KDevNodeAction(pNode,"Move to...");
    connect(pAction,SIGNAL(activated(KDevNode*)),this,
	    SLOT(slotMoveFileTo(KDevNode*)));
    pList.append(pAction);
    
    pAction = new KDevNodeAction(pNode,"Copy to...","editcopy");
    connect(pAction,SIGNAL(activated(KDevNode*)),this,
	    SLOT(slotCopyFileTo(KDevNode*)));
    pList.append(pAction);
    
    pAction = new KDevNodeAction(pNode,"Rename...");
    connect(pAction,SIGNAL(activated(KDevNode*)),
	    this,SLOT(slotRenameFile(KDevNode*)));
    pList.append(pAction);
    
    pList.append(new KActionSeparator());
    pAction = new KDevNodeAction(pNode,"Remove from Project...");
    connect(pAction,SIGNAL(activated(KDevNode*)),
	    this,SLOT(slotRemoveFileFromProject(KDevNode*)));
    pList.append(pAction);
    
    pAction = new KDevNodeAction(pNode,"Delete File...","edittrash");
    connect(pAction,SIGNAL(activated(KDevNode*)),
	    this,SLOT(slotDeleteFile(KDevNode*)));
    pList.append(pAction);
  }
  return pList;
}

void ProjectSpace::slotRenameFile(KDevNode* pNode){
  kdDebug(9030) << "renameFile called: ";
  pNode->show();
  if (!pNode->inherits("KDevFileNode")){
    kdDebug(9030) << "ProjectSpace::slotRenameFile() pNode doesn't inherit KDevFileNode" << endl;
    return;
  }
  KDevFileNode* pFileNode = static_cast<KDevFileNode*> (pNode);
  KDevFileNode* pNewFileNode=0;
  QString absFileName = pFileNode->absoluteFileName();
  QString projectName = pFileNode->projectName();

  
  RenameFileDlg dlg(0,"renamedlg",true);
  QFileInfo info(absFileName);
  dlg.m_oldNameLineEdit->setText(info.fileName());
  dlg.m_newNameLineEdit->setText(info.fileName());
  dlg.m_newNameLineEdit->end(true);
  dlg.m_newNameLineEdit->setFocus();
  if(dlg.exec()){
    QString newName = dlg.m_newNameLineEdit->text();
    if(newName != ""){
      Project* pProject = project(projectName);
      if(pProject != 0){
	kdDebug(9030) << "m_path:" << m_path << endl;
	kdDebug(9030) << "relative Path:" << pProject->relativePath() << endl;
	QString absProjectPath = CToolClass::getAbsolutePath(m_path,pProject->relativePath());
	QString filePath = CToolClass::getRelativePath(absProjectPath,info.dirPath() +"/");
	kdDebug(9030) << "filepath:" << filePath << endl;
	kdDebug(9030) << "absProjectPath:" << absProjectPath << endl;
	kdDebug(9030) << "dirPath:" << info.dirPath() << endl;
	RegisteredFile* pRegFile = pProject->file(filePath + info.fileName());
	pRegFile->setRelativeFile(filePath + newName);
	pNewFileNode = new KDevFileNode(info.dirPath() + "/" +newName,m_name,pProject->name());
      }
      else {
	kdDebug(9030) << "slotRenameFile() No Project found:" << projectName << endl;	
      }
      // start rename
      KURL srcURL(absFileName);
      KURL destURL(info.dirPath() +"/" + newName);
      KIO::SimpleJob* pJob = KIO::rename(srcURL,destURL,false);
      emit sigRemovedFileFromProject(pFileNode);
      emit sigAddedFileToProject(pNewFileNode); // inform the other components
    }
  }
}
void ProjectSpace::slotDeleteFile(KDevNode* pNode){
  kdDebug(9030) << "deleteFile called: ";
  pNode->show();
  if (!pNode->inherits("KDevFileNode")){
    kdDebug(9030) << "ProjectSpace::slotDeleteFile() pNode doesn't inherit KDevFileNode" << endl;
    return;
  }
  KDevFileNode* pFileNode = static_cast<KDevFileNode*> (pNode);
  QString absFileName = pFileNode->absoluteFileName();
  QString projectName = pFileNode->projectName();

  QFileInfo info(absFileName);
  if (KMessageBox::warningYesNo(0, i18n("Do you really want to delete the file \"%1\"?\nThere is no way to restore it!").arg(info.fileName())) == KMessageBox::No){
    return;
  }
  
  Project* pProject = project(projectName);
  if(pProject != 0){
    pProject->removeFile(absFileName);
  }
  else {
    kdDebug(9030) << "slotDeleteFile() No Project found:" << projectName << endl;	
  }
  KURL srcURL(absFileName);
  KIO::DeleteJob* pJob = KIO::del(srcURL);
  emit sigRemovedFileFromProject(pFileNode);
  
}
void ProjectSpace::slotRemoveFileFromProject(KDevNode* pNode){
  kdDebug(9030) << "removeFileFromProject called:";
  pNode->show();
  if (!pNode->inherits("KDevFileNode")){
    kdDebug(9030) << "ProjectSpace::slotRemoveFileFromProject() pNode doesn't inherit KDevFileNode" << endl;
    return;
  }
  KDevFileNode* pFileNode = static_cast<KDevFileNode*> (pNode);
  QString absFileName = pFileNode->absoluteFileName();
  QString projectName = pFileNode->projectName();

  QFileInfo info(absFileName);
  if (KMessageBox::warningYesNo(0, i18n("Do you really want to remove the file \"%1\"\nfrom project? It will remain on disk.").arg(info.fileName())) == KMessageBox::No){
    return;
  }

  Project* pProject = project(projectName);
  if(pProject != 0){
    pProject->removeFile(absFileName);
    emit sigRemovedFileFromProject(pFileNode);
  }
  else {
    kdDebug(9030) << "slotDeleteFile() No Project found:" << projectName << endl;	
  }
  
}
void ProjectSpace::slotMoveFileTo(KDevNode* pNode){
  kdDebug(9030) << "slotMoveFileTo called:";
  pNode->show();
  if (!pNode->inherits("KDevNode")){
    kdDebug(9030) << "ProjectSpace::slotMoveFileTo() pNode doesn't inherit KDevFileNode" << endl;
    return;
  }
  KDevFileNode* pFileNode = static_cast<KDevFileNode*> (pNode);
  KDevFileNode* pNewFileNode=0;
  QString absFileName = pFileNode->absoluteFileName();
  QString projectName = pFileNode->projectName();

  QString dir = KFileDialog::getExistingDirectory(QString::null,0,i18n("Destination directory..."));
  if(dir.isEmpty()){
    return; // cancel or no directory
  }
  QFileInfo info(absFileName);

  Project* pProject = project(projectName);
  if(pProject != 0){
    RegisteredFile* pFile = pProject->fileAbsolute(absFileName);
    RegisteredFile* pNewFile = new RegisteredFile(*pFile); // copy all properties
    // change the filelocation
    QString relFile = CToolClass::getRelativeFile(pProject->absolutePath(),dir +"/" + info.fileName());
    kdDebug(9030) << "relFile" <<  relFile << endl;
    pNewFile->setRelativeFile(relFile);
    pProject->removeFile(pFile);
    pProject->addFile(pNewFile);
    pNewFileNode = new KDevFileNode(dir + "/" +info.fileName(),m_name,pProject->name());
    
  }
  
  KURL srcURL(absFileName);
  KURL destURL(dir + "/" + info.fileName());
  KIO::FileCopyJob* pJob = KIO::file_move(srcURL,destURL);
  emit sigRemovedFileFromProject(pFileNode);
  emit sigAddedFileToProject(pNewFileNode); // inform the other components
}
void ProjectSpace::slotCopyFileTo(KDevNode* pNode){
  kdDebug(9030) << "slotCopyFileTo called:";
  pNode->show();
  if (!pNode->inherits("KDevFileNode")){
    kdDebug(9030) << "ProjectSpace::slotCopyFileTo() pNode doesn't inherit KDevFileNode" << endl;
    return;
  }
  KDevFileNode* pFileNode = static_cast<KDevFileNode*> (pNode);
  KDevFileNode* pNewFileNode=0;
  QString absFileName = pFileNode->absoluteFileName();
  QString projectName = pFileNode->projectName();

  QString dir = KFileDialog::getExistingDirectory(QString::null,0,i18n("Destination directory..."));
  if(dir.isEmpty()){
    return; // cancel or no directory
  }
  QFileInfo info(absFileName);
  
  Project* pProject = project(projectName);
  if(pProject != 0){
    RegisteredFile* pFile = pProject->fileAbsolute(absFileName);
    RegisteredFile* pNewFile = new RegisteredFile(*pFile); // copy all properties
    // change the filelocation
    QString relFile = CToolClass::getRelativeFile(pProject->absolutePath(),dir +"/" + info.fileName());
    kdDebug(9030) << "relFile" <<  relFile << endl;
    pNewFile->setRelativeFile(relFile);
    pProject->addFile(pNewFile);
    pNewFileNode = new KDevFileNode(dir + "/" + info.fileName(),m_name,pProject->name());
    kdDebug(9030) << "emit sigAddedFileToProject";
  }
  
  KURL srcURL(absFileName);
  KURL destURL(dir + "/" + info.fileName());
  KIO::FileCopyJob* pJob = KIO::file_copy(srcURL,destURL);
  emit sigAddedFileToProject(pNewFileNode); // inform the other components
}
#include "projectspace.moc"
