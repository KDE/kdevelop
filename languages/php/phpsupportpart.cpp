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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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
#include <qthread.h>

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
   m_htmlView = 0;
   m_parser = 0;
   phpExeProc = 0;
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
   connect( phpExeProc, SIGNAL(receivedStdout (KProcess*, char*, int)),
            this, SLOT(slotReceivedPHPExeStdout (KProcess*, char*, int)));
   connect( phpExeProc, SIGNAL(receivedStderr (KProcess*, char*, int)),
            this, SLOT(slotReceivedPHPExeStderr (KProcess*, char*, int)));
   connect( phpExeProc, SIGNAL(processExited(KProcess*)),
            this, SLOT(slotPHPExeExited(KProcess*)));

   m_htmlView = new PHPHTMLView(this);
   mainWindow()->embedPartView(m_htmlView->view(), i18n("PHP"), i18n("PHP"));
   connect( m_htmlView,  SIGNAL(started(KIO::Job*)),
            this, SLOT(slotWebJobStarted(KIO::Job*)));

   configData = new PHPConfigData(projectDom());
   connect( configData,  SIGNAL(configStored()),
            this, SLOT(slotConfigStored()));

   m_codeCompletion = new  PHPCodeCompletion(this, configData);

   new KAction(i18n("Complete Text"), CTRL+Key_Space, m_codeCompletion, SLOT(cursorPositionChanged()), actionCollection(), "edit_complete_text");

   connect( partController(), SIGNAL(activePartChanged(KParts::Part*)),
            this, SLOT(slotActivePartChanged(KParts::Part *)));
   connect( this, SIGNAL(fileParsed( PHPFile* )), this, SLOT(slotfileParsed( PHPFile* )));
}

PHPSupportPart::~PHPSupportPart()
{
   LastClass = NULL;
   LastMethod = NULL;
   LastVariable = NULL;

   if ( m_parser ) {
      m_parser->close() ;
      delete( m_parser );
      m_parser = NULL;
   }

   if ( m_phpErrorView ) {
      mainWindow()->removeView( m_phpErrorView );
      delete( m_phpErrorView );
      m_phpErrorView = NULL;
   }

   kdDebug(9018) << "remove codeCompletition" << endl;
   if ( m_codeCompletion )
      delete( m_codeCompletion );

   kdDebug(9018) << "remove configData" << endl;
   if ( configData )
      delete( configData );

   if ( m_htmlView ) {
      kdDebug(9018) << "remove htmlView" << endl;
      mainWindow()->removeView( m_htmlView->view() );
      delete( m_htmlView );
      m_htmlView = NULL;
   }

   kdDebug(9018) << "remove phpExec" << endl;
   if ( phpExeProc )
      delete( phpExeProc );

   kdDebug(9018) << "finish" << endl;
}

void PHPSupportPart::slotActivePartChanged(KParts::Part *part) {
   kdDebug(9018) << "enter slotActivePartChanged" << endl;
   if (!part || !part->widget())
      return;
   m_editInterface = dynamic_cast<KTextEditor::EditInterface*>(part);
   if (m_editInterface) { // connect to the editor
      disconnect(part, 0, this, 0 ); // to make sure that it is't connected twice
      if (configData->getRealtimeParsing()) {
         connect(part,SIGNAL(textChanged()),this,SLOT(slotTextChanged()));
      }
      m_codeCompletion->setActiveEditorPart(part);
   }
   kdDebug(9018) << "exit slotActivePartChanged" << endl;
}

void PHPSupportPart::slotTextChanged() {
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

void PHPSupportPart::slotConfigStored() {
   // fake a changing, this will read the configuration again and install the connects
   slotActivePartChanged(partController()->activePart());
}

void PHPSupportPart::projectConfigWidget(KDialogBase *dlg) {
   QVBox *vbox = dlg->addVBoxPage(i18n( "PHP Specific" ), i18n("PHP Settings"), BarIcon( "source", KIcon::SizeMedium ));
   PHPConfigWidget* w = new PHPConfigWidget(configData,vbox, "php config widget");
   connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}

void PHPSupportPart::slotNewClass() {
   QStringList classNames = sortedNameList( codeModel()->globalNamespace()->classList() );
   PHPNewClassDlg dlg(classNames,project()->projectDirectory());
   dlg.exec();
 }

void PHPSupportPart::slotRun() {
   configData = new PHPConfigData(projectDom());
   if (validateConfig()) {
      mainWindow()->raiseView(m_phpErrorView);
      mainWindow()->raiseView(m_htmlView->view());
      PHPConfigData::InvocationMode mode = configData->getInvocationMode() ;
      if (mode == PHPConfigData::Web) {
         executeOnWebserver();
      }
      else if (mode == PHPConfigData::Shell) {
         executeInTerminal();
      }
   }
}

bool PHPSupportPart::validateConfig() {
   if (!configData->validateConfig()) {
      KMessageBox::information(0,i18n("There is no configuration for executing a PHP file.\nPlease set the correct values in the next dialog."));
      KDialogBase dlg(KDialogBase::TreeList, i18n("Customize PHP Mode"),
                     KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, 0,
                     "php config dialog");

      QVBox *vbox = dlg.addVBoxPage(i18n("PHP Settings"));
      PHPConfigWidget* w = new PHPConfigWidget(configData,vbox, "php config widget");
      connect( &dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
      dlg.exec();
   }
   if (configData->validateConfig()) {
      return true;
   }
   return false;
}

void PHPSupportPart::executeOnWebserver() {
   // Save all files once
   if (partController()->saveAllFiles()==false)
      return; //user cancelled

   // Figure out the name of the remote file
   QString weburl = configData->getWebURL();
   QString file = getExecuteFile();

   // Force KHTMLPart to reload the page
   KParts::BrowserExtension* be = m_htmlView->browserExtension();
   if (be) {
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

void PHPSupportPart::slotWebJobStarted(KIO::Job* job) {
   if (job && job->className() == QString("KIO::TransferJob")) {
      kdDebug(9018) << endl << "job started" << job->progressId();
      KIO::TransferJob *tjob = static_cast<KIO::TransferJob*>(job);
      connect( tjob,  SIGNAL(data(KIO::Job*, const QByteArray&)),
               this, SLOT(slotWebData(KIO::Job*, const QByteArray&)));
      connect( tjob,  SIGNAL(result(KIO::Job*)),
               this, SLOT(slotWebResult(KIO::Job*)));
   }
}

void PHPSupportPart::slotWebData(KIO::Job* /*job*/,const QByteArray& data) {
   kdDebug(9018) << "slotWebData()" << endl;
   QString strData(data);
   m_phpExeOutput += strData;
}

void PHPSupportPart::slotWebResult(KIO::Job* /*job*/) {
   kdDebug(9018) << "slotWebResult()" << endl;
   QString file = getExecuteFile();
   PHPFile *pfile = new PHPFile(this, file);
   pfile->ParseStdout(m_phpExeOutput);
   delete pfile;
}

void PHPSupportPart::executeInTerminal() {
   kdDebug(9018) << "slotExecuteInTerminal()" << endl;

   // Save all files once
   if (partController()->saveAllFiles()==false)
       return; //user cancelled

   QString file = getExecuteFile();

   if (m_htmlView == 0) {
      m_htmlView = new PHPHTMLView(this);
      mainWindow()->embedPartView(m_htmlView->view(), i18n("PHP"));
   }

   m_htmlView->show();
   m_htmlView->begin();

   m_phpExeOutput = "";
   phpExeProc->clearArguments();
   *phpExeProc << configData->getPHPExecPath();
   *phpExeProc << "-f";

   *phpExeProc << KShellProcess::quote(file);
   kdDebug(9018) << "" << file.latin1() << endl;
   phpExeProc->start(KProcess::NotifyOnExit,KProcess::All);

   //    core()->gotoDocumentationFile(KURL("http://www.php.net"));
}

void PHPSupportPart::slotReceivedPHPExeStdout (KProcess* /*proc*/, char* buffer, int buflen) {
   kdDebug(9018) << "slotPHPExeStdout()" << endl;
   m_phpExeOutput += QString::fromLocal8Bit(buffer,buflen+1);

   QString buf = buffer;
   if (configData->getInvocationMode() == PHPConfigData::Shell)
     buf.replace("\n", "<br>");
    m_htmlView->write(buf);
}

void PHPSupportPart::slotReceivedPHPExeStderr (KProcess* /*proc*/, char* buffer, int buflen) {
   kdDebug(9018) << "slotPHPExeStderr()" << endl;
   m_phpExeOutput += QString::fromLocal8Bit(buffer,buflen+1);

   QString buf = buffer;
   if (configData->getInvocationMode() == PHPConfigData::Shell)
      buf.replace("\n", "<br>");
   m_htmlView->write(buf);
}

void PHPSupportPart::slotPHPExeExited (KProcess* /*proc*/) {
   kdDebug(9018) << "slotPHPExeExited()" << endl;
   m_htmlView->end();
   QString file = getExecuteFile();
   PHPFile *pfile = new PHPFile(this, file);
   pfile->ParseStdout(m_phpExeOutput);
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
    QTimer::singleShot(500, this, SLOT( initialParse() ) );
}

void PHPSupportPart::initialParse( )
{
        // For debugging
        if ( !project( ) )
        {
                // messagebox ?
                kdDebug( 9018 ) << "No project" << endl;
                return ;
        }

        parseProject( );
        return ;
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

bool PHPSupportPart::parseProject()
{
   kdDebug(9018) << "parseProject()" << endl;
   mainWindow() ->statusBar() ->message( i18n( "Updating..." ) );

   kapp->setOverrideCursor( waitCursor );

   _jd = new JobData;

   _jd->files = project()->allFiles();

   QProgressBar* bar = new QProgressBar( _jd->files.count( ), mainWindow( ) ->statusBar( ) );
   bar->setMinimumWidth( 120 );
   bar->setCenterIndicator( true );
   mainWindow()->statusBar()->addWidget( bar );
   bar->show();

   _jd->progressBar = bar;
   _jd->it = _jd->files.begin();
   _jd->dir.setPath( project()->projectDirectory() );

   QTimer::singleShot( 0, this, SLOT( slotParseFiles() ) );
   return TRUE;
}

void PHPSupportPart::slotParseFiles()
{
   kdDebug(9018) << "slotParseFiles()" << endl;

   if ( _jd->it != _jd->files.end() )
   {
      _jd->progressBar->setProgress( _jd->progressBar->progress() + 1 );

      QFileInfo fileInfo( _jd->dir, *( _jd->it ) );

      if ( fileInfo.exists() && fileInfo.isFile() && fileInfo.isReadable() )
      {
         QString absFilePath = URLUtil::canonicalPath( fileInfo.absFilePath() );

//         if ( isValidSource( absFilePath ) )
         {
            if (m_parser)
               m_parser->addFile( absFilePath );
         }

         ++( _jd->it );
      }
      QTimer::singleShot( 0, this, SLOT( slotParseFiles() ) );
   }
   else // finished or interrupted
   {
      kapp->restoreOverrideCursor();
      mainWindow()->statusBar()->removeWidget( _jd->progressBar );
      mainWindow()->statusBar()->message( i18n( "Done" ), 2000 );

      emit updatedSourceInfo();
      if (m_parser)
         m_parser->startParse();

      delete _jd;
      _jd = 0;
   }
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
}

void PHPSupportPart::removedFilesFromProject(const QStringList &fileList)
{
   kdDebug(9018) << "removedFilesFromProject()" << endl;

   QStringList::ConstIterator it;

   for ( it = fileList.begin(); it != fileList.end(); ++it )
   {
      QFileInfo fileInfo( project()->projectDirectory(), *it );
      QString path = fileInfo.absFilePath();
      if ( codeModel()->hasFile(path) ) {
         emit aboutToRemoveSourceInfo( path );
         codeModel()->removeFile( codeModel()->fileByName(path) );
      }
   }
}

void PHPSupportPart::savedFile(const KURL &fileName)
{
   kdDebug(9018) << "savedFile()" << fileName.fileName() << endl;
/// @fixme when activated could cause stop
/*
   if (m_parser) {
      if (m_parser->hasFile( fileName.path() )) {
         m_parser->reparseFile( fileName.path() );
      }
   }
*/
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
   if ( mime )
      list << mime;

   mime = KMimeType::mimeType( "text/plain" );
   if ( mime )
      list << mime;
   return list;
}

void PHPSupportPart::customEvent( QCustomEvent* ev )
{
//   kdDebug(9018) << "phpSupportPart::customEvent(" << ev->type() << ") " << endl;

   if ( ev->type() < Event_AddFile || ev->type() > Event_AddFixme )
      return;

  // kapp->lock();

   FileParseEvent* event = (FileParseEvent*) ev;
   NamespaceDom ns = codeModel()->globalNamespace();
   FileDom m_file = codeModel()->fileByName( event->fileName() );

   if (!m_file) {
      m_file = codeModel()->create<FileModel>();
      m_file->setName( event->fileName() );
      codeModel()->addFile( m_file );
   }

   switch (int(ev->type())) {
      case Event_AddFile:
         m_parser->addFile( event->fileName() );
      break;

      case Event_StartParse:
         kdDebug(9018) << "StartParse " << event->fileName() << endl;
         LastClass = NULL;
         LastMethod = NULL;
         LastVariable = NULL;
         if ( codeModel()->hasFile( event->fileName() ) ) {
            emit aboutToRemoveSourceInfo( event->fileName() );
            codeModel()->removeFile( codeModel()->fileByName( event->fileName() ) );
            emit removedSourceInfo( event->fileName() );
         }
         ErrorView()->removeAllProblems( event->fileName() );
      break;

      case Event_AddClass:
      {
         kdDebug(9018) << "AddClass " << event->name() << endl;
         ClassDom nClass = codeModel()->create<ClassModel>();
         nClass->setFileName( event->fileName() );
         nClass->setName( event->name() );
         nClass->setStartPosition( event->posititon(), 0);

         m_file->addClass( nClass );
         if ( event->arguments().isEmpty() != TRUE )
            nClass->addBaseClass( event->arguments() );

         ns->addClass( nClass );
         LastClass = nClass;
      }
      break;

      case Event_CloseClass:
         if ( LastClass != NULL ) {
            kdDebug(9018) << "CloseClass " << LastClass->name() << endl;
            LastClass->setEndPosition( event->posititon(), 0 );
            LastClass = NULL;
            LastMethod = NULL;
            LastVariable = NULL;
         }
      break;

      case Event_AddFunction:
      {
         kdDebug(9018) << "AddFunction " << event->name() << endl;
         FunctionDom nMethod = codeModel()->create<FunctionModel>();
         nMethod->setFileName( event->fileName() );
         nMethod->setName( event->name() );
         nMethod->setStartPosition( event->posititon(), 0 );

         ArgumentDom nArgument;
         nArgument = codeModel()->create<ArgumentModel>();
         nArgument->setType(event->arguments().stripWhiteSpace().local8Bit());
         nMethod->addArgument( nArgument );

         if (LastClass != NULL) {
            LastClass->addFunction(nMethod);
         } else {
            ns->addFunction(nMethod);
         }
         LastMethod = nMethod;
      }
      break;

      case Event_SetFunction:
         if ( LastMethod != NULL ) {
            kdDebug(9018) << "SetFunction " << LastMethod->name() << " " << event->name() << endl;
            if ( event->name() == "static" )
               LastMethod->setStatic(true);
            else if ( event->name() == "abstract" )
               LastMethod->setAbstract(true);
            else if ( event->name() == "private" )
               LastMethod->setAccess(FunctionModel::Private);
            else if ( event->name() == "public" )
               LastMethod->setAccess(FunctionModel::Public);
            else if ( event->name() == "protected" )
               LastMethod->setAccess(FunctionModel::Protected);
            else if ( event->name() == "result" ) {
               QString ret = "";
               if (event->arguments().lower() == "$this") {
                  ret = LastClass->name();
               }
               LastMethod->setResultType(ret);
            }
         }
      break;

      case Event_CloseFunction:
         if ( LastMethod != NULL ) {
            kdDebug(9018) << "CloseFunction " << LastMethod->name() << endl;
            LastMethod->setEndPosition( event->posititon(), 0 );
            LastMethod = NULL;
            LastVariable = NULL;
         }
      break;

      case Event_AddVariable:
      {
         VariableDom nVariable = codeModel()->create<VariableModel>();

         nVariable->setFileName( event->fileName() );
         nVariable->setName( event->name() );
         nVariable->setStartPosition( event->posititon(), 0 );
         nVariable->setAccess(VariableModel::Public);

         if ( event->arguments().isEmpty() != TRUE )
            nVariable->setType( event->arguments() );

         if ( LastClass != NULL && ( LastMethod == NULL || event->global() == TRUE ) ) {
               kdDebug(9018) << "AddVariable To Class " << LastClass->name() << " " << nVariable->name() << endl;
            LastClass->addVariable(nVariable);
         } else {
            if ( LastMethod != NULL ) {
               kdDebug(9018) << "AddVariable " << LastMethod->name() << " " << nVariable->name() << endl;
            } else {
               ns->addVariable(nVariable);
            }
         }
         LastVariable = nVariable;
      }
      break;

      case Event_SetVariable:
         if ( LastVariable != NULL ) {
            kdDebug(9018) << "SetVariable " << LastVariable->name() << " " << event->arguments() << endl;
            if ( event->arguments() == "static" )
               LastVariable->setStatic(true);
            else if ( event->arguments() == "private" )
               LastVariable->setAccess(FunctionModel::Private);
            else if ( event->arguments() == "public" )
               LastVariable->setAccess(FunctionModel::Public);
            else if ( event->arguments() == "protected" )
               LastVariable->setAccess(FunctionModel::Protected);
         }
      break;

      case Event_AddTodo:
         ErrorView()->reportProblem(Todo, event->fileName(), event->posititon(), event->arguments());
      break;

      case Event_AddFixme:
         ErrorView()->reportProblem(Fixme, event->fileName(), event->posititon(), event->arguments());
      break;

      case Event_EndParse:
         kdDebug(9018) << "EndParse " << event->fileName() << endl;
         emit addedSourceInfo( event->fileName() );
      break;

   }
//   kapp->unlock();
}

PHPErrorView *PHPSupportPart::ErrorView( ) {
   return m_phpErrorView;
}

PHPParser *PHPSupportPart::Parser( ) {
   return m_parser;
}

#include "phpsupportpart.moc"
