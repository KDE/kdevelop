/***************************************************************************
 *   Copyright (C) 2001 by Sandy Meier                                     *
 *   smeier@kdevelop.org                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qfileinfo.h>
#include <qpopupmenu.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qvbox.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kregexp.h>
#include <iostream>
#include <khtmlview.h>
#include <kprocess.h>
#include <qstringlist.h>
#include <kmessagebox.h>
#include <kstatusbar.h>
#include <qprogressbar.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevtoplevel.h"
#include "kdevpartcontroller.h"
#include "classstore.h"
#include <kdevpartcontroller.h>

#include "phpsupportpart.h"
#include "phpsupportfactory.h"
#include "phpconfigdata.h"
#include "phpconfigwidget.h"
#include "phpcodecompletion.h"
#include "phpparser.h"
#include "phpnewclassdlg.h"


#include "phphtmlview.h"
#include "phperrorview.h"

#include "parsedclass.h"
#include "parsedmethod.h"
#include "domutil.h"

using namespace std;

PHPSupportPart::PHPSupportPart(KDevApi *api, QObject *parent, const char *name)
    : KDevLanguageSupport(api, parent, name)
{
  m_htmlView=0;
  phpExeProc=0;
  setInstance(PHPSupportFactory::instance());

  setXMLFile("kdevphpsupport.rc");

  connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
  connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
  connect( partController(), SIGNAL(savedFile(const QString&)),
             this, SLOT(savedFile(const QString&)) );
  connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
	   this, SLOT(projectConfigWidget(KDialogBase*)) );

  KAction *action;

  action = new KAction( i18n("&Run"), "exec",Key_F9,
			this, SLOT(slotRun()),
			actionCollection(), "build_execute" );
  action = new KAction( i18n("&New Class..."),0,
			this, SLOT(slotNewClass()),
			actionCollection(), "project_new_class" );

  m_phpErrorView = new PHPErrorView(this);
  topLevel()->embedOutputView(m_phpErrorView, i18n("PHP"));
  connect(m_phpErrorView,SIGNAL(fileSelected(const QString&,int)),
	  this,SLOT(slotErrorMessageSelected(const QString&,int)));

  phpExeProc = new KShellProcess("/bin/sh");
  connect(phpExeProc, SIGNAL(receivedStdout (KProcess*, char*, int)),
	  this, SLOT(slotReceivedPHPExeStdout (KProcess*, char*, int)));
  connect(phpExeProc, SIGNAL(receivedStderr (KProcess*, char*, int)),
	  this, SLOT(slotReceivedPHPExeStderr (KProcess*, char*, int)));
  connect(phpExeProc, SIGNAL(processExited(KProcess*)),
	  this, SLOT(slotPHPExeExited(KProcess*)));

  m_htmlView = new PHPHTMLView();
  topLevel()->embedPartView(m_htmlView->view(), i18n("PHP"));
  connect(m_htmlView,  SIGNAL(started(KIO::Job*)),
	  this, SLOT(slotWebJobStarted(KIO::Job*)));

  configData = new PHPConfigData(projectDom());
  m_parser = new  PHPParser(core(),classStore());
  m_codeCompletion = new  PHPCodeCompletion(this, core(),classStore());

  connect(partController(), SIGNAL(activePartChanged(KParts::Part*)),
	  this, SLOT(slotActivePartChanged(KParts::Part *)));
}


PHPSupportPart::~PHPSupportPart()
{}

void PHPSupportPart::slotActivePartChanged(KParts::Part *part)
{
  if (!part || !part->widget())
    return;

  m_editInterface = dynamic_cast<KTextEditor::EditInterface*>(part);
  disconnect(part, 0, this, 0 ); // to make sure that it is't connected twice
  connect(part,SIGNAL(textChanged()),this,SLOT(slotTextChanged()));
}

void PHPSupportPart::slotTextChanged(){
  cerr << "text changed" << endl;

  KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(partController()->activePart());
  if (!ro_part)
    return;

  QString fileName = ro_part->url().directory() + "/" + ro_part->url().fileName();
  int numLines = m_editInterface->numLines();

  QStringList lines;
  for(int i=0;i<numLines;i++){
    lines.append(m_editInterface->textLine(i));
  }
  classStore()->removeWithReferences(fileName);
  m_parser->parseLines(&lines,fileName);
  emit updatedSourceInfo();
}




void PHPSupportPart::slotErrorMessageSelected(const QString& filename,int line){
  cerr << endl << "kdevelop (phpsupport): slotWebResult()" << filename.latin1() << line;
  partController()->editDocument(filename,line);
}
void PHPSupportPart::projectConfigWidget(KDialogBase *dlg){
  QVBox *vbox = dlg->addVBoxPage(i18n("PHP Settings"));
  PHPConfigWidget* w = new PHPConfigWidget(configData,vbox, "php config widget");
  connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}

void PHPSupportPart::slotNewClass(){
  QStringList classNames = classStore()->getSortedClassNameList();
  PHPNewClassDlg dlg(classNames,project()->projectDirectory());
  dlg.exec();
 }

void PHPSupportPart::slotRun(){
  configData = new PHPConfigData(projectDom());
  if(validateConfig()){
    topLevel()->raiseView(m_phpErrorView);
    PHPConfigData::InvocationMode mode = configData->getInvocationMode() ;
    if(mode == PHPConfigData::Web){
      executeOnWebserver();
    }
    else if(mode == PHPConfigData::Shell){
      executeInTerminal();
    }
  }
}

bool PHPSupportPart::validateConfig(){
  if(!configData->validateConfig()){
    KMessageBox::information(0,"There is no configuration for executing a PHP file.\nPlease set the correct values in the next dialog.");
    KDialogBase dlg(KDialogBase::TreeList, i18n("Customize PHP Mode"),
                    KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, 0,
                    "php config dialog");

    QVBox *vbox = dlg.addVBoxPage(i18n("PHP Settings"));
    PHPConfigWidget* w = new PHPConfigWidget(configData,vbox, "php config widget");
    connect( &dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
    dlg.exec();
  }
  if(configData->validateConfig()){
    return true;
  }
  return false;
}

void PHPSupportPart::executeOnWebserver(){
  QString file;
  PHPConfigData::WebFileMode mode = configData->getWebFileMode();
  QString weburl = configData->getWebURL();
  if(mode == PHPConfigData::Current){
    KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(partController()->activePart());
    if(ro_part){
      file = QFileInfo(ro_part->url().url()).fileName();
    }
  }
  if(mode == PHPConfigData::Default){
    file = configData->getWebDefaultFile();
  }
  m_phpExeOutput="";
  m_htmlView->openURL(KURL(weburl + file));
  m_htmlView->show();
}

void PHPSupportPart::slotWebJobStarted(KIO::Job* job){
  cerr << endl << "job started" << job->progressId();
  if (job->className() == QString("KIO::TransferJob")){
    KIO::TransferJob *tjob = static_cast<KIO::TransferJob*>(job);
    connect(tjob,  SIGNAL(data(KIO::Job*, const QByteArray&)),
	    this, SLOT(slotWebData(KIO::Job*, const QByteArray&)));
    connect(tjob,  SIGNAL(result(KIO::Job*)),
	    this, SLOT(slotWebResult(KIO::Job*)));
  }
}

void PHPSupportPart::slotWebData(KIO::Job* job,const QByteArray& data){
  cerr << "kdevelop (phpsupport): slotWebData()" << endl;
  QString strData(data);
  m_phpExeOutput += strData;
}

void PHPSupportPart::slotWebResult(KIO::Job* job){
  cerr << "kdevelop (phpsupport): slotWebResult()" << endl;
  m_phpErrorView->parse(m_phpExeOutput);
}

void PHPSupportPart::executeInTerminal(){
  cerr << "kdevelop (phpsupport): slotExecuteInTerminal()" << endl;
  QString file;
  if(m_htmlView==0){
    m_htmlView = new PHPHTMLView();
    topLevel()->embedPartView(m_htmlView->view(), i18n("PHP"));
  }
  m_htmlView->show();
  m_htmlView->begin();

  m_phpExeOutput="";
  phpExeProc->clearArguments();
  *phpExeProc << configData->getPHPExecPath();
  *phpExeProc << "-f";


  KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(partController()->activePart());
  if(ro_part){
    file = ro_part->url().path();
  }

  *phpExeProc << file;
  cerr << "kdevelop (phpsupport): " << file.latin1() << endl;
  phpExeProc->start(KProcess::NotifyOnExit,KProcess::All);



  //    core()->gotoDocumentationFile(KURL("http://www.php.net"));
}
void PHPSupportPart::slotReceivedPHPExeStdout (KProcess* proc, char* buffer, int buflen){
  cerr << "kdevelop (phpsupport): slotPHPExeStdout()" << endl;
  m_htmlView->write(buffer,buflen+1);
  m_phpExeOutput += QCString(buffer,buflen+1);
}

void PHPSupportPart::slotReceivedPHPExeStderr (KProcess* proc, char* buffer, int buflen){
  cerr << "kdevelop (phpsupport): slotPHPExeStderr()" << endl;
  m_htmlView->write(buffer,buflen+1);
  m_phpExeOutput += QCString(buffer,buflen+1);
}

void PHPSupportPart::slotPHPExeExited (KProcess* proc){
  cerr << "kdevelop (phpsupport): slotPHPExeExited()" << endl;
  m_htmlView->end();
  m_phpErrorView->parse(m_phpExeOutput);
}

void PHPSupportPart::projectOpened()
{
    cerr << "kdevelop (phpsupport): projectOpened()" << endl;

    connect( project(), SIGNAL(addedFileToProject(const QString &)),
             this, SLOT(addedFileToProject(const QString &)) );
    connect( project(), SIGNAL(removedFileFromProject(const QString &)),
             this, SLOT(removedFileFromProject(const QString &)) );

    // We want to parse only after all components have been
    // properly initialized
    QTimer::singleShot(0, this, SLOT(initialParse()));
}


void PHPSupportPart::projectClosed()
{
}


void PHPSupportPart::maybeParse(const QString fileName)
{
  //    kdDebug(9007) << "maybeParse()" << endl;
    QFileInfo fi(fileName);
    QString path = fi.filePath();
    if ((fi.extension().contains("inc") || fi.extension().contains("php")
	|| fi.extension().contains("html")
	|| fi.extension().contains("php3")) && !fi.extension().contains("~")) {
      cerr << "remove and parse" << fileName.latin1() << endl;
        classStore()->removeWithReferences(fileName);
        m_parser->parseFile(fileName);
    }
}


void PHPSupportPart::initialParse()
{
    cerr << "kdevelop (phpsupport): initialParse()" << endl;

    if (project()) {
      //  kdDebug(9016) << "project" << endl;
        kapp->setOverrideCursor(waitCursor);
        QStringList files = project()->allSourceFiles();
	int n = 0;
        QProgressBar *bar = new QProgressBar(files.count(), topLevel()->statusBar());
        bar->setMinimumWidth(120);
        bar->setCenterIndicator(true);
        topLevel()->statusBar()->addWidget(bar);
        bar->show();

        for (QStringList::Iterator it = files.begin(); it != files.end() ;++it) {
	  //kdDebug(9016) << "maybe parse " << (*it) << endl;
	  bar->setProgress(n);
	  kapp->processEvents();
	  maybeParse(*it);
	  ++n;
        }
        topLevel()->statusBar()->removeWidget(bar);
        delete bar;
        emit updatedSourceInfo();
        kapp->restoreOverrideCursor();
    } else {
        cerr << "kdevelop (phpsupport): No project" << endl;
    }
}


void PHPSupportPart::addedFileToProject(const QString &fileName)
{
    cerr << "kdevelop (phpsupport): addedFileToProject()" << endl;
    maybeParse(fileName);
    emit updatedSourceInfo();
}


void PHPSupportPart::removedFileFromProject(const QString &fileName)
{
    cerr << "kdevelop (phpsupport): removedFileFromProject()" << endl;
    classStore()->removeWithReferences(fileName);
    emit updatedSourceInfo();
}


void PHPSupportPart::savedFile(const QString &fileName)
{
    cerr << "kdevelop (phpsupport): savedFile()" << endl;

    if (project()->allSourceFiles().contains(fileName)) {
        maybeParse(fileName);
        emit updatedSourceInfo();
    }
}


KDevLanguageSupport::Features PHPSupportPart::features()
{
        return Features(Classes | Functions);
}



#include "phpsupportpart.moc"
