/***************************************************************************
                          appwizard.cpp  -  description
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

#include "appwizard.h"
#include <qfiledialog.h>
#include <iostream.h>
#include <kprocess.h>
#include <kstddirs.h>
#include <kdebug.h>
#include <qdatetime.h>
#include <pwd.h>
#include <unistd.h>
#include <qfile.h>
#include <qtextstream.h>
#include <ctoolclass.h>
#include <qregexp.h>

/************************************************** 
 * Will return the capital letters of name
 * i.e John Q. Customer will return JQC
 * and if it's John Customer it will return JC
 * CAVEAT it will return ALL capital letters
 **************************************************/
QString Initials(const QString name)
{
 QChar c;
 QString ret;
 for(uint i=0;i<name.length();i++)
 {
   c=name.at(i);
   if(c.category() == QChar::Letter_Uppercase) ret+=c;
 }
 return ret; 
}

AppWizard::AppWizard(QWidget* parent, const char* obj_name) : AppWizardBase(parent,obj_name,true){
  // some code from kbugreport by David Faure
  KConfig emailConf( QString::fromLatin1("emaildefaults") );
  emailConf.setGroup( QString::fromLatin1("UserInfo") );
  QString fromaddr = emailConf.readEntry( QString::fromLatin1("EmailAddress") );
  if (fromaddr.isEmpty()) {
     struct passwd *p;
     p = getpwuid(getuid());  
     fromaddr = QString::fromLatin1(p->pw_name) + "@";
  }
  QString name = emailConf.readEntry( QString::fromLatin1("FullName"));
  QString company = emailConf.readEntry( QString::fromLatin1("Organization"));
  QString initials = emailConf.readEntry(QString::fromLatin1("Initials"),"Default");
 
#warning FIXME is it safe to write to emailConf?
  if(initials == QString("Default")) { 
    initials=Initials(name);
    emailConf.writeEntry("Initials",initials);
  }
  email_edit->setText( fromaddr );
  author_edit->setText( name );
  initial_edit->setText(initials);
 // initial_edit->setText( "JD" );    // Get from config when available.
  company_edit->setText( company );
  
  // conntects
  m_pProject=0;
  
}
AppWizard::~AppWizard(){
}

void AppWizard::init(bool newProjectspace,ProjectSpace* pProjectspace,QString projectName,QString absProjectLocation){
  kdDebug(9030) << "enter AppWizard::init" << endl;
  m_newProjectspace = newProjectspace;
  m_pProjectspace = pProjectspace;
  m_projectName = projectName;
  m_absProjectLocation = absProjectLocation;

  // create the project
  m_pProject = Project::createNewProject(m_projecttypeName);
  m_pProject->setName(projectName);
  m_pProject->setAbsolutePath(absProjectLocation);
  QString relProjectPath = CToolClass::getRelativePath(pProjectspace->absolutePath(),
						       absProjectLocation);
  m_pProject->setRelativePath(relProjectPath);
 
  if(!m_newProjectspace){
    //    removePage(page(0)); // remove the name,license page
    //    removePage(page(0)); // remove the headerpage
  }
  
}

void AppWizard::accept(){
  kdDebug(9030) << "start generation" << endl;
  if(m_newProjectspace){ // only if a new one was selected
    m_pProjectspace->setAuthor(author_edit->text());
    m_pProjectspace->setInitials(initial_edit->text());
    m_pProjectspace->setEmail(email_edit->text());
    m_pProjectspace->setCompany(company_edit->text());
    m_pProjectspace->setVersion(version_edit->text());
    m_pProjectspace->generateDefaultFiles();
  }
  generateDefaultFiles(); // generate and add files to project
  m_pProjectspace->addProject(m_pProject);
  m_pProjectspace->writeGlobalDocument();
  m_pProjectspace->writeUserDocument();
  if(!m_pProjectspace->saveConfig()){
    kdDebug(9030) << "error in writing ConfigFile" << endl;
  }
  else{
    kdDebug(9030) << "ConfigFile successfully written" << endl;
  }
  m_pProjectspace->updateAdminFiles(); // generte configure.in.in, Makefileam's or similar
  QWizard::accept();
}

QString  AppWizard::projectSpaceName(){
  return m_projectspaceName;
}
QString  AppWizard::previewPicture(){
  return m_applicationPicture;
}

void AppWizard::generateDefaultFiles(){
  KShellProcess proc("/bin/sh");
  
  QString absProjectPath = m_pProject->absolutePath();
  kdDebug(9030) << "creating directory: " << absProjectPath << endl;
  // create the directories
  proc << "mkdirhier";
  proc << absProjectPath;
  proc.start(KProcess::Block,KProcess::AllOutput);
  
  // untar/unzip the project
  proc.clearArguments();
  QString args = "xzvf " + m_projectTemplate + " -C " + absProjectPath;
  kdDebug(9030) << "AppWizard::generateDefaultFiles():" << args << endl;
  proc << "tar";
  proc << args;
  proc.start(KProcess::Block,KProcess::AllOutput);
}
void AppWizard::slotNewHeader(){
  header_multiedit->clear();
}
/*
 * Made it do something whether
 * it is correct or not is another thing
 * SDM 
 */

void AppWizard::slotLoadHeader()
{
   QFile headIODev(locate("appdata", "templates/header_template"));
  if (headIODev.open(IO_ReadOnly))
  {
   QTextStream textStream(&headIODev);
   header_multiedit->setText(textStream.read());
  }
}
/* 
 * Added date = QDate::currentDate() as 
 * date() is invalid
 * SDM
 */

void AppWizard::setInfosInString(QString& text){
  QDate date = QDate::currentDate();
  text.replace(QRegExp("|NAME|"),m_pProject->name());
  text.replace(QRegExp("|NAMELITTLE|"),m_pProject->name().lower());
  text.replace(QRegExp("|YEAR|"),QString::number(date.year()));
  text.replace(QRegExp("|EMAIL|"),email_edit->text());
  text.replace(QRegExp("|AUTHOR|"),author_edit->text());
  text.replace(QRegExp("|VERSION|"),version_edit->text());
}

void AppWizard::generateFile(QString abs_oldpos,QString abs_newpos){
  QFile file(abs_oldpos);
  if ( file.open(IO_ReadOnly) ) {    // file opened successfully
    QTextStream t( &file );        // use a text stream
    QString text = t.read();
    file.close();
    setInfosInString(text);
    // save
    file.setName(abs_newpos);
    if ( file.open(IO_WriteOnly) ){
      QTextStream tw( &file );
      tw << text;
      file.close();
    }
    else {
      kdDebug(9030) << "ERROR! couldn't open file to write: " << abs_newpos << endl;
    }
  }
  else {
    kdDebug(9030)  << "ERROR! couldn't open file to read: " << abs_oldpos << endl;
  }
}
KAboutData* AppWizard::aboutPlugin(){
  return 0;
}
QString AppWizard::applicationDescription(){
  return m_applicationDescription;
}

#include "appwizard.moc"
