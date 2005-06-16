/*
   Copyright (C) 2005 by Nicolas Escuder <n.escuder@intra-links.com>
   Copyright (C) 2001 by smeier@kdevelop.org
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   version 2, License as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

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
#include <kdevplugininfo.h>
#include <kiconloader.h>

#include "phpconfigdata.h"
#include "phpconfigwidget.h"
#include "phpcodecompletion.h"
#include "phpparser.h"
#include "phpnewclassdlg.h"


#include "phphtmlview.h"
#include "phperrorview.h"

#include "phpsupport_event.h"

using namespace std;

static const KDevPluginInfo data("kdevphpsupport");
K_EXPORT_COMPONENT_FACTORY( libkdevphpsupport, PHPSupportFactory( data ) )

PHPSupportPart::PHPSupportPart(QObject *parent, const char *name, const QStringList &)
    : KDevLanguageSupport(&data, parent, name ? name : "PHPSupportPart")
{
  m_htmlView=0;
  m_parser=0;
  phpExeProc=0;
  setInstance(PHPSupportFactory::instance());
  
  setXMLFile("kdevphpsupport.rc");

  connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
  connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
  connect( partController(), SIGNAL(savedFile(const KURL&)),
             this, SLOT(savedFile(const KURL&)) );
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
  m_phpErrorView->setIcon( SmallIcon("info") );

  QWhatsThis::add(m_phpErrorView, i18n("<b>PHP problems</b><p>This view shows PHP parser warnings, errors, and fatal errors."));
  mainWindow()->embedOutputView(m_phpErrorView, i18n("Problems"), i18n("Problems"));

  phpExeProc = new KShellProcess("/bin/sh");
  connect(phpExeProc, SIGNAL(receivedStdout (KProcess*, char*, int)),
	  this, SLOT(slotReceivedPHPExeStdout (KProcess*, char*, int)));
  connect(phpExeProc, SIGNAL(receivedStderr (KProcess*, char*, int)),
	  this, SLOT(slotReceivedPHPExeStderr (KProcess*, char*, int)));
  connect(phpExeProc, SIGNAL(processExited(KProcess*)),
	  this, SLOT(slotPHPExeExited(KProcess*)));
  
  m_htmlView = new PHPHTMLView(this);
  mainWindow()->embedPartView(m_htmlView->view(), i18n("PHP"), i18n("PHP"));
  connect(m_htmlView,  SIGNAL(started(KIO::Job*)),
	  this, SLOT(slotWebJobStarted(KIO::Job*)));

  configData = new PHPConfigData(projectDom());
  connect(configData,  SIGNAL(configStored()),
	  this, SLOT(slotConfigStored()));

  m_codeCompletion = new  PHPCodeCompletion(this, configData);

  new KAction(i18n("Complete Text"), CTRL+Key_Space, m_codeCompletion, SLOT(cursorPositionChanged()),
	actionCollection(), "edit_complete_text");
  
  connect(partController(), SIGNAL(activePartChanged(KParts::Part*)),
	  this, SLOT(slotActivePartChanged(KParts::Part *)));
   
  connect(this, SIGNAL(fileParsed( PHPFile* )), this, SLOT(slotfileParsed( PHPFile* )));
}

PHPSupportPart::~PHPSupportPart()
{
   if (m_parser) {
      m_parser->terminate() ;
      delete( m_parser );
   }    

    if(m_phpErrorView){
      mainWindow()->removeView( m_phpErrorView );
      delete( m_phpErrorView );
      m_phpErrorView = 0;
    }

    delete( m_codeCompletion );
    delete( configData );

    if( m_htmlView ){
	mainWindow()->removeView( m_htmlView->view() );
	delete( m_htmlView );
	m_htmlView = 0;
    }

    delete( phpExeProc );
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

   if (m_parser) {
      if (m_parser->hasFile( fileName ))
         m_parser->reparseFile( fileName );
   }

}

void PHPSupportPart::slotConfigStored(){
  // fake a changing, this will read the configuration again and install the connects
  slotActivePartChanged(partController()->activePart());
}

void PHPSupportPart::projectConfigWidget(KDialogBase *dlg){
  QVBox *vbox = dlg->addVBoxPage(i18n( "PHP Specific" ), i18n("PHP Settings"), BarIcon( "source", KIcon::SizeMedium ));
  PHPConfigWidget* w = new PHPConfigWidget(configData,vbox, "php config widget");
  connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
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
  if (partController()->saveAllFiles()==false)
       return; //user cancelled
  
  // Figure out the name of the remote file
  QString weburl = configData->getWebURL();
  QString file = getExecuteFile();
  
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

QString PHPSupportPart::getExecuteFile() {
   QString file;
   PHPConfigData::StartupFileMode mode = configData->getStartupFileMode();
  
   QString weburl = configData->getWebURL();
   if (mode == PHPConfigData::Current) {
      KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(partController()->activePart());
      if (ro_part) {
         file = QFileInfo(ro_part->url().url()).fileName();
      }
   }
   if (mode == PHPConfigData::Default) {
      file = configData->getStartupFile();
   }
   return file;
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
   QString file = getExecuteFile();
   PHPFile *pfile = new PHPFile(this, file);
   pfile->ParseStdout(m_phpExeOutput);
   KApplication::postEvent( this, new FileParsedEvent(file, pfile->getActions()) );
   delete pfile;
}

void PHPSupportPart::executeInTerminal(){
  kdDebug(9018) << "slotExecuteInTerminal()" << endl;

  // Save all files once
  if (partController()->saveAllFiles()==false)
       return; //user cancelled
      
  QString file = getExecuteFile();
  
  if(m_htmlView == 0) {
    m_htmlView = new PHPHTMLView(this);
    mainWindow()->embedPartView(m_htmlView->view(), i18n("PHP"));
  }
  
  m_htmlView->show();
  m_htmlView->begin();

  m_phpExeOutput="";
  phpExeProc->clearArguments();
  *phpExeProc << configData->getPHPExecPath();
  *phpExeProc << "-f";

  *phpExeProc << KShellProcess::quote(file);
  kdDebug(9018) << "" << file.latin1() << endl;
  phpExeProc->start(KProcess::NotifyOnExit,KProcess::All);

  //    core()->gotoDocumentationFile(KURL("http://www.php.net"));
}

void PHPSupportPart::slotReceivedPHPExeStdout (KProcess* /*proc*/, char* buffer, int buflen){
  kdDebug(9018) << "slotPHPExeStdout()" << endl;
  m_phpExeOutput += QString::fromLocal8Bit(buffer,buflen+1);

   QString buf = buffer;
   if(configData->getInvocationMode() == PHPConfigData::Shell)
     buf.replace("\n", "<br>");
    m_htmlView->write(buf);
}

void PHPSupportPart::slotReceivedPHPExeStderr (KProcess* /*proc*/, char* buffer, int buflen){
  kdDebug(9018) << "slotPHPExeStderr()" << endl;
  m_phpExeOutput += QString::fromLocal8Bit(buffer,buflen+1);

   QString buf = buffer;
   if(configData->getInvocationMode() == PHPConfigData::Shell)
     buf.replace("\n", "<br>");
    m_htmlView->write(buf);
}

void PHPSupportPart::slotPHPExeExited (KProcess* /*proc*/){
   kdDebug(9018) << "slotPHPExeExited()" << endl;
   m_htmlView->end();
   QString file = getExecuteFile();
   PHPFile *pfile = new PHPFile(this, file);
   pfile->ParseStdout(m_phpExeOutput);
   KApplication::postEvent( this, new FileParsedEvent(file, pfile->getActions()) );
   delete pfile;
}

void PHPSupportPart::projectOpened()
{
    kdDebug(9018) << "projectOpened()" << endl;

    connect( project(), SIGNAL(addedFilesToProject(const QStringList &)),
             this, SLOT(addedFilesToProject(const QStringList &)) );
    connect( project(), SIGNAL(removedFilesFromProject(const QStringList &)),
             this, SLOT(removedFilesFromProject(const QStringList &)) );

    if (!m_parser) {
       m_parser = new  PHPParser( this );
       m_parser->start();
    }

    // We want to parse only after all components have been
    // properly initialized
    QTimer::singleShot(0, this, SLOT(initialParse()));
}


void PHPSupportPart::projectClosed()
{
   kdDebug(9018) << "projectClosed()" << endl;
   if (m_parser) {
      m_parser->close() ;
      delete( m_parser );
      m_parser = 0;
   }   
}

void PHPSupportPart::initialParse()
{
  kdDebug(9018) << "initialParse()" << endl;

  if (project()) {
    kapp->setOverrideCursor(waitCursor);
    QStringList files = project()->allFiles();
    int n = 0;
    QProgressBar *bar = new QProgressBar(files.count(), mainWindow()->statusBar());
    bar->setMinimumWidth(120);
    bar->setCenterIndicator(true);
    mainWindow()->statusBar()->addWidget(bar);
    bar->show();
   for ( QStringList::ConstIterator it = files.begin(); it != files.end(); ++it ) {
      QFileInfo fileInfo( project()->projectDirectory(), *it );
      bar->setProgress(n);
      kapp->processEvents();
      if (m_parser)
         m_parser->addFile( fileInfo.filePath() );
      ++n;
    }
    mainWindow()->statusBar()->removeWidget(bar);
    delete bar;
    emit updatedSourceInfo();
    kapp->restoreOverrideCursor();
  } else {
    kdDebug(9018) << "No project" << endl;
  }
  if (m_parser)
     m_parser->startParse();
}

void PHPSupportPart::addedFilesToProject(const QStringList &fileList)
{
    kdDebug(9018) << "addedFilesToProject()" << endl;

	QStringList::ConstIterator it;

	for ( it = fileList.begin(); it != fileList.end(); ++it )
	{
		QFileInfo fileInfo( project()->projectDirectory(), *it );
      if (m_parser) {
         m_parser->addFile( fileInfo.absFilePath() );
         emit addedSourceInfo( fileInfo.absFilePath() );
      }
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

void PHPSupportPart::savedFile(const KURL &fileName)
{
    kdDebug(9018) << "savedFile()" << fileName.fileName() << endl;

    if (m_parser) {
       if (m_parser->hasFile( fileName.path() )) {
          m_parser->reparseFile( fileName.path() );
       }
    }
}

QString PHPSupportPart::getIncludePath()
{
  return configData->getPHPIncludePath();
}

QString PHPSupportPart::getExePath()
{
  return configData->getPHPExecPath();
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

void PHPSupportPart::customEvent( QCustomEvent* ev )
{
//   kdDebug(9018) << "phpSupportPart::customEvent(" << ev->type() << ")" << endl;

   if ( ev->type() == int(Event_FileParsed) ){
      FileParsedEvent* event = (FileParsedEvent*) ev;
      
      QString fileName = event->fileName();

      // ClassView work with absolute path not links
      QString abso = URLUtil::canonicalPath(fileName);
      if (codeModel()->hasFile( abso )) {
         emit aboutToRemoveSourceInfo( abso );
         codeModel()->removeFile( codeModel()->fileByName(abso) );
         emit removedSourceInfo( abso );
      }

      m_phpErrorView->removeAllProblems( abso );

      PHPFile m_phpfile(this, abso);
           
      QValueList<Action *> actions = event->actions();
      QValueList<Action *>::ConstIterator it = actions.begin();
      while( it != actions.end() ){
         Action *p = *it++;
         switch (p->quoi()) {
            case Add_Include:
            m_parser->addFile( p->name() );
            break;
            
            case Add_ErrorNoSuchFunction:
            case Add_ErrorParse:
            case Add_Error:
            m_phpErrorView->reportProblem( fileName, p->start(), p->quoi(), p->args() );
            break;
            
            case Add_Fixme:
            case Add_Todo:
            m_phpErrorView->reportProblem( fileName, p->start(), p->quoi(), p->name() );
            break;

            default:
            m_phpfile.doAction(fileName, p);
            break;
            
         }
         
         emit addedSourceInfo( abso );

      }
     
    }
}

#include "phpsupportpart.moc"
