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
#include <iostream.h>
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
#include <kglobal.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kprocess.h>
#include "kpp.h"
#include <kmessagebox.h>

Kpp::Kpp(QWidget*parent,const char* name, bool modal):projectview(parent,name,modal){

connect (QPushButton_1, SIGNAL(clicked()), SLOT(notYet()));
connect (QPushButton_2, SIGNAL(clicked()), SLOT(exitApp()));
connect (QPushButton_3, SIGNAL(clicked()), SLOT(saveFile()));
connect (QPushButton_4, SIGNAL(clicked()), SLOT(openFile()));
//connect (QPushButton_5, SIGNAL(clicked()), SLOT(helpMe()));
connect (QPushButton_6, SIGNAL(clicked()), SLOT(changeSpec()));
connect (QPushButton_7, SIGNAL(clicked()), SLOT(rpmBuildSlot()));
currentProject = new ckdevelProject();
kcConfig = kapp->config();
loadPrefs();

}
Kpp::~Kpp(){
        cerr << "Kpp is cleaning up..." << endl;
}

/** This will allow the user to save the variables for the current project. */
void Kpp::saveFile(){
        cout << "Save File" << endl;


        currentSpecPath=KFileDialog::getSaveFileName(qsRPMBaseDir, "*.spec");

        if(!currentSpecPath.isEmpty())
        {
                // read the file
                cout << "Saveing File" << endl;

                updateSpec();
                generateSpec(currentSpecPath);
                emit newSpec(currentSpecPath);
        }
}

/** Opens a KPP project file. */
void Kpp::openFile(){
  cout << "Open File" << endl;

  QString open_filename;
  open_filename=KFileDialog::getOpenFileName(qsRPMBaseDir, "*.kdevprj");

    if(!open_filename.isEmpty())
    {
      // read the file
      cout << "Opening File" << endl;
      if(currentProject->loadProject(open_filename))
      {
        //cout << "Project Name: " << currentProject->getProjectName() << endl;
        //cout << "Author: " << currentProject->getAuthor() << endl;
        //cout << "Version: " << currentProject->getVersion() << endl;
        //cout << "Info: " << currentProject->getInfo() << endl;
        //cout << "Configure Options:" << currentProject->getConfig() << endl;
        QMultiLineEdit_1->setText(currentProject->getInfo());
        QLineEdit_1->setText(currentProject->getProjectName());
        QLineEdit_4->setText(currentProject->getAuthor());
        QLineEdit_9->setText(currentProject->getAuthor());
        QLineEdit_2->setText(currentProject->getVersion());
        QLineEdit_5->setText(currentProject->getConfig());
        updateSpec();
      }
      else
        cout << "File failed.... " << endl;
    }

}

/** Opens the online help */
void Kpp::helpMe(){
cout << "Help Me" << endl;
kapp->invokeHTMLHelp("/kpp/index-3.html", "");
}

/** Exit's the application. */
void Kpp::exitApp(){
savePrefs();
cout << "Exit App" << endl;
emit finished();
}


/** Edit the current preferences */
void Kpp::editPrefs(){
cout << "Editing the prefs..." << endl;
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
                cout << newLine << endl << currentSpec.generateSpec(newLine) << endl;
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
  cerr << "Path: " << qsPath << endl;
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
qsSpecTemplate = kcConfig->readEntry( "specTemplate", tempqsSpecTemplate);
QLineEdit_8->setText(kcConfig->readEntry("url", "none"));
QLineEdit_9->setText(kcConfig->readEntry("vendor", "none"));
QComboBox_1->setCurrentItem(kcConfig->readNumEntry("license", 0));
QLineEdit_11->setText(kcConfig->readEntry("baseDir", "none"));
}
/** Save the current enviroment information for the application. */
bool Kpp::saveClass(){


}
/** Save all of the current application preferences */
void Kpp::savePrefs(){
     cout << "Saving Prefs..." << endl;
     cout << kcConfig->writeEntry( "specTemplate", qsSpecTemplate) << endl;
     cout << kcConfig->writeEntry( "url", QLineEdit_8->text()) << endl;
     cout << kcConfig->writeEntry( "vendor", QLineEdit_9->text()) << endl;
     cout << kcConfig->writeEntry( "license", QComboBox_1->currentItem()) << endl;
     cout << kcConfig->writeEntry( "baseDir", QLineEdit_11->text()) << endl;
     cout << "Done Saving Prefs...." << endl;
     kcConfig->sync();
}
/** Set the current config path */
void Kpp::setConfig( KConfig *theConfig){
	kcConfig = theConfig;
}

/** start the rpm build process */
bool Kpp::startBuild(){
	cerr << "Starting build" << endl;
        // do some sanity checks here...
        // 1) make sure source-version.tar.gz is there...
        QString tgzFile = qsRPMBaseDir + "/" + (QLineEdit_1->text().lower()) + "-" + (QLineEdit_2->text()) + ".tar.gz";
        if(!QFile::exists(tgzFile))
        {
         // the source is not there, display a dialog and exit this dialog.
         cerr << "need source file" << endl;
         KMessageBox::sorry(0,"You need to generate a source dist first...", "Missing tar.gz file", true);
         exitApp();
        }
        else
        {
                // 2) make sure the spec entries are all filled out...
                QString tmpSpec =qsRPMBaseDir + "/" + (QLineEdit_1->text().lower()) + "-spec";
                // we may already have a spec there, if so dont bother redoing it unless they want to...
               if(QFile::exists(currentSpecPath) || QFile::exists(tmpSpec))
               {
                        cerr << "spec file is there" << endl;
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
                        cerr << "Running " << buildProc << endl;
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
                        cerr << "spec form not filled out completely" << endl;
                        KMessageBox::sorry(0,"You need to generate a spec file first...", "Missing spec file", true);

                        return false;
               }
        }
}
/** read the standard out */
void Kpp::readStdOut(KProcess *proc, char *buffer, int buflen){
  QString temp = QString::fromLatin1(buffer, buflen);
  emit stdERR(temp);
}
/** read teh standard error */
void Kpp::readStdErr(KProcess *proc, char *buffer, int buflen){
  QString temp = QString::fromLatin1(buffer, buflen);
  emit stdOUT(temp);
}
/** done building... */
void Kpp::buildDone(KProcess *proc){
	if( proc->normalExit())
		cerr << "done" << endl;
	else
		cerr << "there was an error" << endl;
	delete proc;
        emit finished();
}
/**  */
void Kpp::rpmBuildSlot(){
	startBuild();
}

void Kpp::setProjectData(QString appName, QString appVer, QString appAuth, QString appEmail, QString configOpts, QString appDesc){
 currentProject->importProject(appName, appVer, appAuth, appEmail, configOpts,appDesc);
        cout << "Project Name: " << currentProject->getProjectName() << endl;
        cout << "Author: " << currentProject->getAuthor() << endl;
        cout << "Version: " << currentProject->getVersion() << endl;
        cout << "Info: " << currentProject->getInfo() << endl;
        cout << "Configure Options:" << currentProject->getConfig() << endl;
        QMultiLineEdit_1->setText(currentProject->getInfo());
        QLineEdit_1->setText(currentProject->getProjectName());
        QLineEdit_4->setText(currentProject->getAuthor());
        QLineEdit_9->setText(currentProject->getAuthor());
        QLineEdit_2->setText(currentProject->getVersion());
        QLineEdit_5->setText(currentProject->getConfig());
        updateSpec();
}

void Kpp::setProjectRoot(QString path)
{
        qsRPMBaseDir = path;
        cerr << "we are going to try " << path << endl;
}
