/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann,Sandy Meier                      *
 *   bernd@kdevelop.org,smeier@kdevelop.org                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qfileinfo.h>
#include <qpopupmenu.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qvbox.h>
#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kregexp.h>
#include <iostream.h>
#include <khtmlview.h>
#include <kprocess.h>
#include <qstringlist.h>
#include <kmessagebox.h>
#include "keditor/editor.h"

#include "kdevcore.h"
#include "kdevproject.h"
#include "classstore.h"

#include "phpsupportpart.h"
#include "phpsupportfactory.h"
#include "phpconfigwidget.h"
#include "phpconfigdata.h"

#include "phphtmlview.h"
#include "phperrorview.h"

#include "parsedclass.h"
#include "parsedmethod.h"
#include "domutil.h"


PHPSupportPart::PHPSupportPart(KDevApi *api, QObject *parent, const char *name)
    : KDevLanguageSupport(api, parent, name)
{
  m_htmlView=0;
  phpExeProc=0;
  setInstance(PHPSupportFactory::instance());
  
  setXMLFile("kdevphpsupport.rc");
  
  connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
  connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
  connect( core(), SIGNAL(savedFile(const QString&)),
             this, SLOT(savedFile(const QString&)) );
  connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
	   this, SLOT(projectConfigWidget(KDialogBase*)) );
  
  KAction *action;
  action = new KAction( i18n("&Run"), Key_F9,
			this, SLOT(slotRun()),
			actionCollection(), "build_run" );

  m_phpErrorView = new PHPErrorView(this);
  core()->embedWidget(m_phpErrorView, KDevCore::OutputView, i18n("PHP"));
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
  core()->embedWidget(m_htmlView->view(), KDevCore::DocumentView, i18n("PHP"));
  connect(m_htmlView,  SIGNAL(started(KIO::Job*)), 
	  this, SLOT(slotWebJobStarted(KIO::Job*)));

  configData = new PHPConfigData(document());
}


PHPSupportPart::~PHPSupportPart()
{}

void PHPSupportPart::slotErrorMessageSelected(const QString& filename,int line){
  cerr << endl << "kdevelop (phpsupport): slotWebResult()" << filename << line;
  core()->gotoSourceFile(filename,line);
}
void PHPSupportPart::projectConfigWidget(KDialogBase *dlg){
  QVBox *vbox = dlg->addVBoxPage(i18n("PHP Settings"));
  PHPConfigWidget* w = new PHPConfigWidget(configData,vbox, "php config widget");
  connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}

void PHPSupportPart::slotRun(){
  configData = new PHPConfigData(document());
  if(validateConfig()){
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
    KEditor::Editor* editor = core()->editor();
    if(editor){
      file = QFileInfo(editor->currentDocument()->url().url()).fileName();
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
  if(m_htmlView==0){
    m_htmlView = new PHPHTMLView();
    core()->embedWidget(m_htmlView->view(), KDevCore::DocumentView, i18n("PHP"));
  }
  m_htmlView->show();
  m_htmlView->begin();

  m_phpExeOutput="";
  phpExeProc->clearArguments();
  *phpExeProc << "php";
  *phpExeProc << "-f";
  *phpExeProc << "/home/smeier/phpHello/app.php"; 
  phpExeProc->start(KProcess::Block,KProcess::Stdout);
  
  cerr << "kdevelop (phpsupport): slotExecuteInTerminal()" << endl;
  //    core()->gotoDocumentationFile(KURL("http://www.php.net"));
}
void PHPSupportPart::slotReceivedPHPExeStdout (KProcess* proc, char* buffer, int buflen){
  cerr << "kdevelop (phpsupport): slotPHPExeStderr()" << endl;
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
        classStore()->removeWithReferences(fileName);
        parse(fileName);
    }
}


void PHPSupportPart::initialParse()
{
    cerr << "kdevelop (phpsupport): initialParse()" << endl;
    
    if (project()) {
      //  kdDebug(9016) << "project" << endl;
        kapp->setOverrideCursor(waitCursor);
        QStringList files = project()->allSourceFiles();
        for (QStringList::Iterator it = files.begin(); it != files.end() ;++it) {
	  //kdDebug(9016) << "maybe parse " << (*it) << endl;
	  maybeParse(*it);
        }
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


void PHPSupportPart::parse(const QString &fileName)
{
    QFile f(QFile::encodeName(fileName));
    if (!f.open(IO_ReadOnly))
        return;
    QTextStream stream(&f);

    KRegExp classre("^[ \t]*class[ \t]+([A-Za-z_]+)[ \t]*(extends[ \t]*([A-Za-z_]+))?.*$");
    KRegExp methodre("^[ \t]*function[ \t]*([0-9A-Za-z_]*)[ \t]*\\(([0-9A-Za-z_\\$\\, \t=&\\'\\\"]*)\\).*$");
    KRegExp varre("^[ \t]*var[ \t]*([0-9A-Za-z_\\$]+)[ \t;=].*$");

    ParsedClass *lastClass = 0;
    QString rawline;
    QCString line;
    int lineNo = 0;
    int bracketOpen = 0;
    int bracketClose = 0;
    bool inClass = false;
    
    while (!stream.eof()) {
        rawline = stream.readLine();
        line = rawline.stripWhiteSpace().latin1();
	bracketOpen += line.contains("{");
	bracketClose += line.contains("}");

	//cerr << "kdevelop (phpsupport): try match line: " << line << endl;
        if (classre.match(line)) {
	  //  cerr << "kdevelop (phpsupport): regex match line: " << line << endl;
	  inClass= true;
	  bracketOpen = line.contains("{");
	  bracketClose = line.contains("}");
	  lastClass = new ParsedClass;
	  lastClass->setName(classre.group(1));
	  lastClass->setDefinedInFile(fileName);
	  lastClass->setDefinedOnLine(lineNo);
	  
	  QString parentStr = classre.group(3);
	  if(parentStr !=""){
	    ParsedParent *parent = new ParsedParent;
	    parent->setName(parentStr);
	    parent->setAccess(PIE_PUBLIC);
	    lastClass->addParent(parent);
	  }
	  
	  if (classStore()->hasClass(lastClass->name)) {
	    ParsedClass *old = classStore()->getClassByName(lastClass->name);
	    old->setDeclaredOnLine(lastClass->declaredOnLine);
	    old->setDeclaredInFile(lastClass->declaredInFile);
	    delete lastClass;
	    lastClass=0;
	  } else {
	    classStore()->addClass(lastClass);
	  }
	  
	  if(bracketOpen == bracketClose && bracketOpen !=0 && bracketClose !=0){
	    inClass = false; // ok we are out ouf class
	  }

	     
        } else if (methodre.match(line)) {
	  //	  cerr << "kdevelop (phpsupport): regex match line ( method ): " << line << endl;
	  ParsedMethod *method = new ParsedMethod;
	  method->setName(methodre.group(1));
	  ParsedArgument* anArg = new ParsedArgument();
	  QString arguments = methodre.group(2);
	  anArg->setType(arguments.stripWhiteSpace().latin1());
	  method->addArgument( anArg );

	  method->setDefinedInFile(fileName);
	  method->setDefinedOnLine(lineNo);
            
	  if (lastClass && inClass) {
	    //	    kdDebug(9018) << "in Class: " << line << endl;
	    ParsedMethod *old = lastClass->getMethod(method);
	    if (!old)
	      lastClass->addMethod(method);
	  } else {
	    ParsedMethod *old = classStore()->globalContainer.getMethod(method);
	    if (!old)
	      classStore()->globalContainer.addMethod(method);
	  }
        }
	else if (varre.match(line)) {
	  //	  kdDebug(9018) << "###########regex match line ( var ): " << varre.group(1) << endl;
	  if (lastClass && inClass) {
	    ParsedAttribute* anAttr = new ParsedAttribute();
	    anAttr->setName(varre.group(1));
	    anAttr->setDefinedInFile(fileName);
	    anAttr->setDefinedOnLine(lineNo);
	    lastClass->addAttribute( anAttr );
	    
	  }
	  
	}
        ++lineNo;
    }
    f.close();
}

#include "phpsupportpart.moc"
