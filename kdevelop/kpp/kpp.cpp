/***************************************************************************
                          kpp.cpp  -  description
                             -------------------
    begin                : Sun Oct 10 16:16:40 CDT 1999
    copyright            : (C) 1999 by ian geiser
    email                : geiseri@msoe.edu
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <kapp.h>
#include <stdlib.h>
#include <kfiledialog.h>
#include <qstring.h>
#include <qlistbox.h>
#include <qmultilineedit.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qtextstream.h>
#include <kapp.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kprocess.h>
#include "kpp.h"
#include "kpp.moc"
#include <kmessagebox.h>
#include <qpushbutton.h>

Kpp::Kpp(QWidget*parent,const char* name, bool modal):projectview(parent,name,modal){

  connect ((const QObject*)QPushButton_1, SIGNAL(clicked()), SLOT(notYet()));
  connect ((const QObject*)QPushButton_2, SIGNAL(clicked()), SLOT(exitApp()));
  connect ((const QObject*)QPushButton_3, SIGNAL(clicked()), SLOT(saveFile()));
  connect ((const QObject*)QPushButton_4, SIGNAL(clicked()), SLOT(openFile()));
  //connect (QPushButton_5, SIGNAL(clicked()), SLOT(helpMe()));
  connect ((const QObject*)QPushButton_6, SIGNAL(clicked()), SLOT(changeSpec()));
  connect ((const QObject*)QPushButton_7, SIGNAL(clicked()), SLOT(rpmBuildSlot()));
  currentProject = new ckdevelProject();
  kcConfig = kapp->config();
  loadPrefs();
}

Kpp::~Kpp(){
  savePrefs();
  kdDebug() << "Kpp is cleaning up..." << endl;
}

/** This will allow the user to save the variables for the current project. */
void Kpp::saveFile(){
  kdDebug() << "Save File" << endl;

  currentSpecPath=KFileDialog::getSaveFileName(qsRPMBaseDir, "*.spec");

  if(!currentSpecPath.isEmpty())
  {
    // read the file
    kdDebug() << "Saveing File" << endl;

    updateSpec();
    generateSpec(currentSpecPath);
    emit newSpec(currentSpecPath);
  }
}

/** Opens a KPP project file. */
void Kpp::openFile(){
  kdDebug() << "Open File" << endl;

  QString open_filename;
  open_filename=KFileDialog::getOpenFileName(qsRPMBaseDir, "*.kdevprj");

  if(!open_filename.isEmpty())
  {
    // read the file
    kdDebug() << "Opening File" << endl;
    if(currentProject->loadProject(open_filename))
    {
      //kdDebug() << "Project Name: " << currentProject->getProjectName() << endl;
      //kdDebug() << "Author: " << currentProject->getAuthor() << endl;
      //kdDebug() << "Version: " << currentProject->getVersion() << endl;
      //kdDebug() << "Info: " << currentProject->getInfo() << endl;
      //kdDebug() << "Configure Options:" << currentProject->getConfig() << endl;
      QMultiLineEdit_1->setText(currentProject->getInfo());
      QLineEdit_1->setText(currentProject->getProjectName());
      QLineEdit_4->setText(currentProject->getAuthor());
      QLineEdit_9->setText(currentProject->getAuthor());
      QLineEdit_2->setText(currentProject->getVersion());
      QLineEdit_5->setText(currentProject->getConfig());
      updateSpec();
    }
    else
      kdDebug() << "File failed.... " << endl;
  }
}

/** Opens the online help */
void Kpp::helpMe(){
  kdDebug() << "Help Me" << endl;
  kapp->invokeHTMLHelp("/kpp/index-3.html", "");
}

/** Exit's the application. */
void Kpp::exitApp(){
  savePrefs();
  kdDebug() << "Exit App" << endl;
  emit finished();
}


/** Edit the current preferences */
void Kpp::editPrefs(){
  kdDebug() << "Editing the prefs..." << endl;
}

/** Update the spec object to the current data in the dialog */
void Kpp::updateSpec(){
  currentSpec.setAuthor(QLineEdit_4->text());
  currentSpec.setVersion(QLineEdit_2->text());
  currentSpec.setName(QLineEdit_1->text());
  currentSpec.setShortInfo(QLineEdit_10->text());
  currentSpec.setBuildRoot(QLineEdit_11->text());
  currentSpec.setRelease(QLineEdit_3->text());
  currentSpec.setConfigOpts(QLineEdit_5->text());
  currentSpec.setDesc(QMultiLineEdit_1->text());
  currentSpec.setLic(QComboBox_1->currentText());
  currentSpec.setVendor(QLineEdit_9->text());
  currentSpec.setURL(QLineEdit_8->text());
  currentSpec.setCatagory(QLineEdit_6->text());
  if (QCheckBox_2->isChecked ())
    currentSpec.setIcon();
}

/** Generates a spec file from a template. */
bool Kpp::generateSpec(QString fileName){
  qsCurrentSpec = fileName;
  // Load SPEC Template
  bool isGood = true;
  QString newLine;
  //QFile specTemplate( "spec.template" );
  QFile specTemplate( qsSpecTemplate );

  QFile specFile( fileName );
  QTextStream specOut( &specFile );        // use a text stream

  if (specTemplate.open( IO_ReadOnly ) && specFile.open (IO_WriteOnly))
  {
    QTextStream t( &specTemplate);
    QString newLine;
    while(!t.eof())
    {
    newLine = t.readLine();
    //bldView->addLine(currentSpec.generateSpec(newLine));
    specOut << currentSpec.generateSpec(newLine) << endl;
    kdDebug() << newLine << endl << currentSpec.generateSpec(newLine) << endl;
    }
  }
  else
    isGood = false;

  specTemplate.close();
  return isGood;
}

/** This is a default dialog for things not yet implemented. */
void Kpp::notYet(){
}

/** Change the template that we will use to generate the spec file. */
void Kpp::changeSpec(){
  QString qsTempFileName = "";
  QString qsPath = locate("appdata", "kdevelop");
  kdDebug() << "Path: " << qsPath << endl;
  qsTempFileName = KFileDialog::getOpenFileName(qsPath, "*.spec");
  if (qsTempFileName)
     qsSpecTemplate = qsTempFileName;
}
/** Load the user spacific prefs for the application. */
bool Kpp::loadPrefs(){
// Here we want to load the default template for the application
// The default packager
// The default URL
  QString tempqsSpecTemplate = locate("appdata","template.spec");
  kdDebug() << "I got spec template from " << tempqsSpecTemplate << endl;
  qsSpecTemplate = kcConfig->readEntry( "specTemplate", tempqsSpecTemplate);
  kdDebug() << "I got spec template from " << qsSpecTemplate << endl;
  if (qsSpecTemplate == "")
	   KMessageBox::sorry(0, i18n("Currently there is no default template for your SPEC file. \n You will have select one before you can create an RPM." ), i18n("Missing SPEC Template"), true);

  //QLineEdit_8->setText(kcConfig->readEntry("url", "none"));
  //QLineEdit_9->setText(kcConfig->readEntry("vendor", "none"));
  //QComboBox_1->setCurrentItem(kcConfig->readNumEntry("license", 0));
  //QLineEdit_11->setText(kcConfig->readEntry("baseDir", "none"));

  return true;
}
/** Save the current enviroment information for the application. */
bool Kpp::saveClass(){
  return true;
}
/** Save all of the current application preferences */
void Kpp::savePrefs(){
     kdDebug() << "Saving Prefs..." << endl;
     kcConfig->writeEntry( "specTemplate", qsSpecTemplate);
//     kdDebug() << kcConfig->writeEntry( "url", QLineEdit_8->text()) << endl;
//     kdDebug() << kcConfig->writeEntry( "vendor", QLineEdit_9->text()) << endl;
//     kdDebug() << kcConfig->writeEntry( "license", QComboBox_1->currentItem()) << endl;
//     kdDebug() << kcConfig->writeEntry( "baseDir", QLineEdit_11->text()) << endl;
     kdDebug() << "Done Saving Prefs...." << endl;
     kcConfig->sync();
}
/** Set the current config path */
void Kpp::setConfig( KConfig *theConfig){
	kcConfig = theConfig;
}

/** start the rpm build process */
bool Kpp::startBuild(){
  kdDebug() << "Starting build" << endl;
  // do some sanity checks here...
  // 1) make sure source-version.tar.gz is there...
  QString tgzFile = qsRPMBaseDir + "/" + (QLineEdit_1->text().lower()) + "-" + (QLineEdit_2->text()) + ".tar.gz";
  if(!QFile::exists(tgzFile))
  {
    // the source is not there, display a dialog and exit this dialog.
    kdDebug() << "need source file" << endl;
    KMessageBox::sorry(0, i18n("You need to generate a source dist first..."), i18n("Missing tar.gz File"), true);
    exitApp();
  }
  else
  {
    // 2) make sure the spec entries are all filled out...
    QString tmpSpec =qsRPMBaseDir + "/" + (QLineEdit_1->text().lower()) + ".spec";
    // we may already have a spec there, if so dont bother redoing it unless they want to...
    kdDebug() << "Looking for " << tmpSpec << endl;
    if(QFile::exists(tmpSpec))
    {
      kdDebug() << "spec file is there" << endl;
      // The build env looks good so we will start
      rpmBuild = new KShellProcess();
      QString buildProc = "sh $KDEDIR/share/apps/kdevelop/tools/buildrpm.sh ";
      // the build script take a few args
      // 1) base directory  - qaRPMBaseDir
      // 2) project name    - qlineEdit_1->text()
      // 3) the applications version  qlineEdit_->text()
      // 4) the path to the source file qsRPMBaseDir + name + version + tar.gz
      // 5) the path to the spec file  currentSpecPath
      buildProc += QLineEdit_1->text().lower() + " ";
      buildProc += QLineEdit_2->text().lower() + " ";
      buildProc += tgzFile + " ";
      buildProc += qsRPMBaseDir;
      kdDebug() << "Running " << buildProc << endl;
      *rpmBuild << buildProc;
      rpmBuild->start(KShellProcess::NotifyOnExit,KShellProcess::All);
      connect(rpmBuild, SIGNAL(receivedStdout(KProcess *, char *, int)), SLOT(readStdOut(KProcess*, char *, int)));
      connect(rpmBuild, SIGNAL(receivedStdout(KProcess *, char *, int)), SLOT(readStdErr(KProcess*, char *, int)));
      connect(rpmBuild, SIGNAL(processExited(KProcess *)), SLOT(buildDone(KProcess*)));
      emit building();
      return true;
    }
    else
    {
      // the spec is not filled out correctly do no clobber the dialog
      // but do not start the build.
      kdDebug() << "spec form not filled out completely" << endl;
      KMessageBox::sorry(0,i18n("You need to generate a spec file first..."), i18n("Missing spec File"), true);

      return false;
    }
  }
  return false;
}

/** read the standard out */
void Kpp::readStdOut(KProcess */*proc*/, char *buffer, int buflen){
  QString temp = QString::fromLocal8Bit(buffer, buflen);
  emit stdERR(temp);
}

/** read teh standard error */
void Kpp::readStdErr(KProcess */*proc*/, char *buffer, int buflen){
  QString temp = QString::fromLocal8Bit(buffer, buflen);
  emit stdOUT(temp);
}

/** done building... */
void Kpp::buildDone(KProcess *proc){
  if( proc->normalExit())
    kdDebug() << "done" << endl;
  else
    kdDebug() << "there was an error" << endl;
  delete proc;
  emit finished();
}

/**  */
void Kpp::rpmBuildSlot(){
	startBuild();
}

void Kpp::setProjectData(QString appName, QString appVer, QString appAuth, QString appEmail, QString configOpts, QString appDesc, QString rev, QString license, QString url, QString appGroup, QString bldPfx, QString info, QString /*icon*/){
  currentProject->importProject(appName, appVer, appAuth, appEmail, configOpts,appDesc);
  kdDebug() << "Project Name: " << currentProject->getProjectName() << endl;
  kdDebug() << "Author: " << currentProject->getAuthor() << endl;
  kdDebug() << "Version: " << currentProject->getVersion() << endl;
  kdDebug() << "Info: " << currentProject->getInfo() << endl;
  kdDebug() << "Configure Options:" << currentProject->getConfig() << endl;
  QMultiLineEdit_1->setText(currentProject->getInfo());
  QLineEdit_1->setText(currentProject->getProjectName());
  QLineEdit_4->setText(currentProject->getAuthor());
  QLineEdit_9->setText(currentProject->getAuthor());
  QLineEdit_2->setText(currentProject->getVersion());
  QLineEdit_5->setText(currentProject->getConfig());
  QLineEdit_3->setText(rev);
  QLineEdit_8->setText(url);
  QLineEdit_6->setText(appGroup);
  QLineEdit_11->setText(bldPfx);
  QLineEdit_10->setText(info);
  QComboBox_1->setCurrentItem(license.toInt());
  updateSpec();
}

void Kpp::setProjectRoot(QString path)
{
  qsRPMBaseDir = path;
  kdDebug() << "we are going to try " << path << endl;
}

QString Kpp::getAppGroup(){
  return  QLineEdit_6->text();
}
QString Kpp::getBuildRoot(){
  return  QLineEdit_11->text();
}
QString Kpp::getIcon(){
  return "0";
}
QString Kpp::getLicense(){
  return QString::number(QComboBox_1->currentItem());
}
QString Kpp::getVersion(){
	return QLineEdit_2->text();
}
QString Kpp::getSummary(){
  return  QLineEdit_10->text();
}
QString Kpp::getURL(){
  return QLineEdit_8->text();
}
