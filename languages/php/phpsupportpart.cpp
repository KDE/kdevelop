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

#include "phpsupportpart.h"

#include <iostream>

#include <qdir.h>
#include <qfileinfo.h>
#include <qpopupmenu.h>
#include <qprogressbar.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qvbox.h>
#include <qwhatsthis.h>

#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <khtmlview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kregexp.h>
#include <kstatusbar.h>
#include <kparts/browserextension.h>

#include <kdevcore.h>
#include <kdevproject.h>
#include <kdevmainwindow.h>
#include <kdevpartcontroller.h>
#include <codemodel.h>
#include <domutil.h>

#include "phpconfigdata.h"
#include "phpconfigwidget.h"
#include "phpconfigparserwidget.h"
#include "phpcodecompletion.h"
#include "phpparser.h"
#include "phpnewclassdlg.h"


#include "phphtmlview.h"
#include "phperrorview.h"


using namespace std;

static const KAboutData data("kdevphpsupport", I18N_NOOP("Language"), "1.0");
K_EXPORT_COMPONENT_FACTORY( libkdevphpsupport, PHPSupportFactory( &data ) )

PHPSupportPart::PHPSupportPart(QObject *parent, const char *name, const QStringList &)
    : KDevLanguageSupport("PHPSupport", "php", parent, name ? name : "PHPSupportPart")
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
  action->setToolTip(i18n("Run"));
  action->setWhatsThis(i18n("<b>Run</b><p>Executes script on a terminal or a webserver."));

  action = new KAction( i18n("&New Class..."),0,
			this, SLOT(slotNewClass()),
			actionCollection(), "project_new_class" );
  action->setToolTip(i18n("New class"));
  action->setWhatsThis(i18n("<b>New class</b><p>Runs New Class wizard."));

  m_phpErrorView = new PHPErrorView(this);
  QWhatsThis::add(m_phpErrorView, i18n("<b>PHP problems</b><p>This view shows PHP parser warnings, errors, and fatal errors."));
  mainWindow()->embedOutputView(m_phpErrorView, i18n("PHP Problems"), i18n("PHP Problems"));
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
  mainWindow()->embedPartView(m_htmlView->view(), i18n("PHP"), "PHP");	// @fixme after stringfreeze - last argument should be i18n() 
  connect(m_htmlView,  SIGNAL(started(KIO::Job*)),
	  this, SLOT(slotWebJobStarted(KIO::Job*)));

  configData = new PHPConfigData(projectDom());
  connect(configData,  SIGNAL(configStored()),
	  this, SLOT(slotConfigStored()));

  m_parser = new  PHPParser(core(),codeModel());
  m_codeCompletion = new  PHPCodeCompletion(configData, core(),codeModel());

  connect(partController(), SIGNAL(activePartChanged(KParts::Part*)),
	  this, SLOT(slotActivePartChanged(KParts::Part *)));
}


PHPSupportPart::~PHPSupportPart()
{
    delete( m_parser );
    delete( m_codeCompletion );
    delete( configData );

    if( m_htmlView ){
	mainWindow()->removeView( m_htmlView->view() );
	delete( m_htmlView );
	m_htmlView = 0;
    }

    delete( phpExeProc );

    if(m_phpErrorView){
      mainWindow()->removeView( m_phpErrorView );
      delete( m_phpErrorView );
      m_phpErrorView = 0;
    }
}

void PHPSupportPart::slotActivePartChanged(KParts::Part *part){
  kdDebug(9018) << "enter slotActivePartChanged" << endl;
  if (!part || !part->widget())
    return;
  m_editInterface = dynamic_cast<KTextEditor::EditInterface*>(part);
  if(m_editInterface){ // connect to the editor
    disconnect(part, 0, this, 0 ); // to make sure that it is't connected twice
    if(configData->getRealtimeParsing()){
      connect(part,SIGNAL(textChanged()),this,SLOT(slotTextChanged()));
    }
    m_codeCompletion->setActiveEditorPart(part);
  }
  kdDebug(9018) << "exit slotActivePartChanged" << endl;
}

void PHPSupportPart::slotTextChanged(){
  kdDebug(9018) << "enter text changed" << endl;

  KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(partController()->activePart());
  if (!ro_part)
    return;

  QString fileName = ro_part->url().directory() + "/" + ro_part->url().fileName();
  kdDebug(9018) << "filename:" << fileName << endl;
  int numLines = m_editInterface->numLines();

  //Abort if the file which was changed is not part of the project
  if (!project()->allFiles().contains(fileName.mid ( project()->projectDirectory().length() + 1 ))) {
    kdDebug(9018) << "Not Parsing file " << fileName << ", file is not part of the project" << endl;
    return;
  }

  QStringList lines;
  for(int i=0;i<numLines;i++){
    lines.append(m_editInterface->textLine(i));
  }

  if( codeModel()->hasFile(fileName) ){
      emit aboutToRemoveSourceInfo( fileName );
      codeModel()->removeFile( codeModel()->fileByName(fileName) );
  }
  m_parser->parseLines(&lines,fileName);

  emit addedSourceInfo( fileName );
  kdDebug(9018) << "exit text changed" << endl;
}

void PHPSupportPart::slotConfigStored(){
  // fake a changing, this will read the configuration again and install the connects
  slotActivePartChanged(partController()->activePart());
}


void PHPSupportPart::slotErrorMessageSelected(const QString& filename,int line){
  kdDebug(9018) << endl << "slotWebResult()" << filename.latin1() << line;
  partController()->editDocument(KURL( filename ),line);
}
void PHPSupportPart::projectConfigWidget(KDialogBase *dlg){
  QVBox *vbox = dlg->addVBoxPage(i18n("PHP Settings"));
  PHPConfigWidget* w = new PHPConfigWidget(configData,vbox, "php config widget");
  connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );

  vbox = dlg->addVBoxPage(i18n("PHP Parser"));
  PHPConfigParserWidget* wp = new PHPConfigParserWidget(configData,vbox, "php parser config widget");
  connect( dlg, SIGNAL(okClicked()), wp, SLOT(accept()) );
}

void PHPSupportPart::slotNewClass(){
  QStringList classNames = sortedNameList( codeModel()->globalNamespace()->classList() );
  PHPNewClassDlg dlg(classNames,project()->projectDirectory());
  dlg.exec();
 }

void PHPSupportPart::slotRun(){
  configData = new PHPConfigData(projectDom());
  if(validateConfig()){
    mainWindow()->raiseView(m_phpErrorView);
    mainWindow()->raiseView(m_htmlView->view());
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
    KMessageBox::information(0,i18n("There is no configuration for executing a PHP file.\nPlease set the correct values in the next dialog."));
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

  // Save all files once
  partController()->saveAllFiles();
  
  // Figure out the name of the remote file
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
  
  // Force KHTMLPart to reload the page
  KParts::BrowserExtension* be = m_htmlView->browserExtension();
  if(be){
    KParts::URLArgs urlArgs( be->urlArgs() );
    urlArgs.reload = true;
    be->setURLArgs( urlArgs );
  }

  // Acutally do the request
  m_phpExeOutput="";
  m_htmlView->openURL(KURL(weburl + file));
  m_htmlView->show();
}

void PHPSupportPart::slotWebJobStarted(KIO::Job* job){
  if (job && job->className() == QString("KIO::TransferJob")){
    kdDebug(9018) << endl << "job started" << job->progressId();
    KIO::TransferJob *tjob = static_cast<KIO::TransferJob*>(job);
    connect(tjob,  SIGNAL(data(KIO::Job*, const QByteArray&)),
	    this, SLOT(slotWebData(KIO::Job*, const QByteArray&)));
    connect(tjob,  SIGNAL(result(KIO::Job*)),
	    this, SLOT(slotWebResult(KIO::Job*)));
  }
}

void PHPSupportPart::slotWebData(KIO::Job* /*job*/,const QByteArray& data){
  kdDebug(9018) << "slotWebData()" << endl;
  QString strData(data);
  m_phpExeOutput += strData;
}

void PHPSupportPart::slotWebResult(KIO::Job* /*job*/){
  kdDebug(9018) << "slotWebResult()" << endl;
  m_phpErrorView->parse(m_phpExeOutput);
}

void PHPSupportPart::executeInTerminal(){
  kdDebug(9018) << "slotExecuteInTerminal()" << endl;

  // Save all files once
  partController()->saveAllFiles();
      
  QString file;
  if(m_htmlView==0){
    m_htmlView = new PHPHTMLView();
    mainWindow()->embedPartView(m_htmlView->view(), i18n("PHP"));
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

  *phpExeProc << KShellProcess::quote(file);
  kdDebug(9018) << "" << file.latin1() << endl;
  phpExeProc->start(KProcess::NotifyOnExit,KProcess::All);



  //    core()->gotoDocumentationFile(KURL("http://www.php.net"));
}
void PHPSupportPart::slotReceivedPHPExeStdout (KProcess* /*proc*/, char* buffer, int buflen){
  kdDebug(9018) << "slotPHPExeStdout()" << endl;
  m_htmlView->write(buffer,buflen+1);
  m_phpExeOutput += QString::fromLocal8Bit(buffer,buflen+1);
}

void PHPSupportPart::slotReceivedPHPExeStderr (KProcess* /*proc*/, char* buffer, int buflen){
  kdDebug(9018) << "slotPHPExeStderr()" << endl;
  m_htmlView->write(buffer,buflen+1);
  m_phpExeOutput += QString::fromLocal8Bit(buffer,buflen+1);
}

void PHPSupportPart::slotPHPExeExited (KProcess* /*proc*/){
  kdDebug(9018) << "slotPHPExeExited()" << endl;
  m_htmlView->end();
  m_phpErrorView->parse(m_phpExeOutput);
}

void PHPSupportPart::projectOpened()
{
    kdDebug(9018) << "projectOpened()" << endl;

    connect( project(), SIGNAL(addedFilesToProject(const QStringList &)),
             this, SLOT(addedFilesToProject(const QStringList &)) );
    connect( project(), SIGNAL(removedFilesFromProject(const QStringList &)),
             this, SLOT(removedFilesFromProject(const QStringList &)) );

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
      kdDebug(9018) << "remove and parse" << fileName.latin1() << endl;
        if( codeModel()->hasFile(fileName) ){
	    emit aboutToRemoveSourceInfo( fileName );
	    codeModel()->removeFile( codeModel()->fileByName(fileName) );
	}
        m_parser->parseFile(fileName);
    }
}


void PHPSupportPart::initialParse(){
  kdDebug(9018) << "initialParse()" << endl;

  if (project()) {
    kdDebug(9018) << "project" << endl;
    kapp->setOverrideCursor(waitCursor);
    QStringList files = project()->allFiles();
    int n = 0;
    QProgressBar *bar = new QProgressBar(files.count(), mainWindow()->statusBar());
    bar->setMinimumWidth(120);
    bar->setCenterIndicator(true);
    mainWindow()->statusBar()->addWidget(bar);
    bar->show();

    for (QStringList::Iterator it = files.begin(); it != files.end() ;++it) {
      QFileInfo fileInfo( project()->projectDirectory(), *it );
      kdDebug(9018) << "maybe parse " << fileInfo.absFilePath() << endl;
      bar->setProgress(n);
      kapp->processEvents();
      maybeParse( fileInfo.absFilePath() );
      ++n;
    }
    mainWindow()->statusBar()->removeWidget(bar);
    delete bar;
    emit updatedSourceInfo();
    kapp->restoreOverrideCursor();
  } else {
    kdDebug(9018) << "No project" << endl;
  }
}


void PHPSupportPart::addedFilesToProject(const QStringList &fileList)
{
    kdDebug(9018) << "addedFilesToProject()" << endl;

	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		QFileInfo fileInfo( project()->projectDirectory(), *it );
		maybeParse( fileInfo.absFilePath() );
		emit addedSourceInfo( fileInfo.absFilePath() );
	}

    //emit updatedSourceInfo();
}


void PHPSupportPart::removedFilesFromProject(const QStringList &fileList)
{
    kdDebug(9018) << "removedFilesFromProject()" << endl;

	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		QFileInfo fileInfo( project()->projectDirectory(), *it );
		QString path = fileInfo.absFilePath();
		if( codeModel()->hasFile(path) ){
		    emit aboutToRemoveSourceInfo( path );
		    codeModel()->removeFile( codeModel()->fileByName(path) );
		}
	}

    //emit updatedSourceInfo();
}


void PHPSupportPart::savedFile(const QString &fileName)
{
    kdDebug(9018) << "savedFile()" << endl;

    if (project()->allFiles().contains(fileName.mid ( project()->projectDirectory().length() + 1 ))) {
        maybeParse(fileName);
        emit addedSourceInfo( fileName );
    }
}


KDevLanguageSupport::Features PHPSupportPart::features()
{
        return Features(Classes | Functions);
}

KMimeType::List PHPSupportPart::mimeTypes( )
{
    KMimeType::List list;
    KMimeType::Ptr mime = KMimeType::mimeType( "application/x-php" );
    if( mime )
	list << mime;

    mime = KMimeType::mimeType( "text/plain" );
    if( mime )
	list << mime;
    return list;
}

#include "phpsupportpart.moc"
