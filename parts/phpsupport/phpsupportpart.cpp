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
#include <iostream.h>
#include <khtmlview.h>
#include <kprocess.h>
#include <qstringlist.h>
#include <kmessagebox.h>
#include <qstatusbar.h>
#include <qprogressbar.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "classstore.h"

#include "phpsupportpart.h"
#include "phpsupportfactory.h"
#include "phpconfigdata.h"
#include "phpconfigwidget.h"
#include "phpbookconfig.h"
#include "phpcodecompletion.h"
#include "phpparser.h"
#include "phpnewclassdlg.h"


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
  action = new KAction( i18n("&New Class..."),0,
			this, SLOT(slotNewClass()),
			actionCollection(), "project_new_class" );

  action = new KAction( i18n("&PhpBook"), 0,
			this, SLOT(slotPhpBook(KDialogBase*)),
			actionCollection(), "phpbook_config" );

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

  configData = new PHPConfigData(projectDom());
  m_parser = new  PHPParser(core(),classStore());
  m_codeCompletion = new  PHPCodeCompletion(core(),classStore());

  connect(core()->editor(), SIGNAL(documentActivated(KEditor::Document*)),
	  this, SLOT(documentActivated(KEditor::Document*)));

}


PHPSupportPart::~PHPSupportPart()
{}

void PHPSupportPart::documentActivated(KEditor::Document* doc){
  //  return;
  m_cursorInterface = KEditor::CursorDocumentIface::interface(doc);
  if (!m_cursorInterface) { // no CursorDocument available
    cerr << endl << "editor doesn't support the CursorDocumentIface";
    return;
  }
  m_editInterface = KEditor::EditDocumentIface::interface(doc);
  disconnect(m_editInterface, 0, this, 0 ); // to make sure that it is't connected twice
  connect(m_editInterface,SIGNAL(textChanged()),this,SLOT(slotTextChanged()));
}

void PHPSupportPart::slotTextChanged(){
  cerr << "text changed" << endl;
  QString fileName = core()->editor()->currentDocument()->url().directory() + "/" +
    core()->editor()->currentDocument()->url().fileName();
  int numLines = m_cursorInterface->numberOfLines();

  QStringList lines;
  for(int i=0;i<numLines;i++){
    lines.append(m_editInterface->line(i));
  }
  classStore()->removeWithReferences(fileName);
  m_parser->parseLines(&lines,fileName);
  emit updatedSourceInfo();
}


void PHPSupportPart::slotPhpBook(KDialogBase *dlg){
        cerr << "slotPhPBook" << endl;
        QVBox *vbox = dlg->addVBoxPage(i18n("PHP Book Settings"));
          PHPBookConfig *w_PhpBook;
	  w_PhpBook =new PHPBookConfig(configData, vbox, "PHP Book Config");
//	  connect(w_PhpBook, SIGNAL(slotOK), this,
//		  SLOT(slotOK()));
}

void PHPSupportPart::slotErrorMessageSelected(const QString& filename,int line){
  cerr << endl << "kdevelop (phpsupport): slotWebResult()" << filename.latin1() << line;
  core()->gotoSourceFile(filename,line);
}
void PHPSupportPart::projectConfigWidget(KDialogBase *dlg){
  QVBox *vbox = dlg->addVBoxPage(i18n("PHP Settings"));
  PHPConfigWidget* w = new PHPConfigWidget(configData,vbox, "php config widget");
  connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}

void PHPSupportPart::slotNewClass(){
  QStringList classNames;
  QList<ParsedClass>* classList = classStore()->globalContainer.getSortedClassList();
  for ( ParsedClass *pclass = classList->first(); pclass != 0;pclass =classList->next() ) {
    classNames.append(pclass->name());
  }
  PHPNewClassDlg dlg(classNames);
  dlg.show();
 }
void PHPSupportPart::slotRun(){
  KEditor::EditDocumentIface *e_iface = KEditor::EditDocumentIface::interface(core()->editor()->currentDocument());
  configData = new PHPConfigData(projectDom());
  if(validateConfig()){
    core()->raiseWidget(m_phpErrorView);
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
  cerr << "kdevelop (phpsupport): slotExecuteInTerminal()" << endl;
  QString file;
  if(m_htmlView==0){
    m_htmlView = new PHPHTMLView();
    core()->embedWidget(m_htmlView->view(), KDevCore::DocumentView, i18n("PHP"));
  }
  m_htmlView->show();
  m_htmlView->begin();

  m_phpExeOutput="";
  phpExeProc->clearArguments();
  *phpExeProc << configData->getPHPExecPath();
  *phpExeProc << "-f";


  KEditor::Editor* editor = core()->editor();
  if(editor){
    file = editor->currentDocument()->url().path();
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
        QProgressBar *bar = new QProgressBar(files.count(), core()->statusBar());
        bar->setMinimumWidth(120);
        bar->setCenterIndicator(true);
        core()->statusBar()->addWidget(bar);
        bar->show();

        for (QStringList::Iterator it = files.begin(); it != files.end() ;++it) {
	  //kdDebug(9016) << "maybe parse " << (*it) << endl;
	  bar->setProgress(n);
	  kapp->processEvents();
	  maybeParse(*it);
	  ++n;
        }
        core()->statusBar()->removeWidget(bar);
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
