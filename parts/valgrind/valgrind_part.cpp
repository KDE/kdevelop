#include <qwhatsthis.h>
#include <qregexp.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kaction.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include "kdevcore.h"
#include "kdevtoplevel.h"
#include "kdevproject.h"

#include "valgrind_widget.h"
#include "valgrind_part.h"
#include "valgrind_dialog.h"
#include "valgrinditem.h"

typedef KGenericFactory<ValgrindPart> ValgrindFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevvalgrind, ValgrindFactory( "kdevvalgrind" ) );

ValgrindPart::ValgrindPart( QObject *parent, const char *name, const QStringList& )
  : KDevPlugin( parent, name )
{
  setInstance( ValgrindFactory::instance() );
  setXMLFile( "kdevpart_valgrind.rc" );

  proc = new KShellProcess();
  connect( proc, SIGNAL(receivedStdout( KProcess*, char*, int )),
           this, SLOT(receivedStdout( KProcess*, char*, int )) );
  connect( proc, SIGNAL(receivedStderr( KProcess*, char*, int )),
           this, SLOT(receivedStderr( KProcess*, char*, int )) );
  connect( proc, SIGNAL(processExited( KProcess* )),
           this, SLOT(processExited( KProcess* )) );
  connect( core(), SIGNAL(stopButtonClicked()),
           this, SLOT(slotKillValgrind()) );
  
  m_widget = new ValgrindWidget( this );
  
  QWhatsThis::add( m_widget, i18n( "Valgrind memory leak check" ) );

  (void) new KAction( i18n("&Valgrind Memory Leak Check"), 0, this,
		      SLOT(slotExecValgrind()), actionCollection(), "tools_valgrind" );
  
  topLevel()->embedOutputView( m_widget, "Valgrind", "Valgrind memory leak check" );
}


ValgrindPart::~ValgrindPart()
{
  delete m_widget;
  delete proc;
}

void ValgrindPart::getActiveFiles()
{
  activeFiles.clear();
  if ( project() ) {
    QStringList projectFiles = project()->allFiles();
    QString projectDirectory = project()->projectDirectory();
    KURL url;
    for ( QStringList::Iterator it = projectFiles.begin(); it != projectFiles.end(); ++it ) {
      KURL url( projectDirectory + "/" + (*it) );
      url.cleanPath( true );
      activeFiles += url.path();
      qDebug( "set project file: " + url.path() );
    }
  }
}

static void guessActiveItem( ValgrindItem& item, const QStringList activeFiles )
{
  if ( activeFiles.isEmpty() && item.backtrace().isEmpty() )
    return;
  for ( ValgrindItem::BacktraceList::Iterator it = item.backtrace().begin(); it != item.backtrace().end(); ++it ) {
    // active: first line of backtrace that lies in project source file
    for ( QStringList::ConstIterator it2 = activeFiles.begin(); it2 != activeFiles.end(); ++it2 ) {
      if ( (*it).url() == (*it2) ) {
        (*it).setHighlighted( true );
        return;
      }
    }
  }
}

void ValgrindPart::appendMessage( const QString& message )
{
  if ( message.isEmpty() )
    return;

  ValgrindItem item( message );
  guessActiveItem( item, activeFiles );
  m_widget->addMessage( item );
}

void ValgrindPart::slotExecValgrind()
{
  ValgrindDialog* dlg = new ValgrindDialog();
  if ( project() ) {
    QString pExec = project()->projectDirectory();
    if ( !pExec.endsWith( "/" ) && !project()->mainProgram().startsWith( "/" ) )
      pExec += "/";
    pExec += project()->mainProgram();
    dlg->setExecutable( pExec );
  }
  if ( dlg->exec() == QDialog::Accepted ) {
    runValgrind( dlg->executableName(), dlg->parameters(), dlg->valExecutable(), dlg->valParams() );
  }
}

void ValgrindPart::slotKillValgrind()
{
  if ( proc )
    proc->kill();
}

void ValgrindPart::runValgrind( const QString& exec, const QString& params, const QString& valExec, const QString& valParams )
{
  if ( proc->isRunning() ) {
    KMessageBox::sorry( 0, i18n( "There is already an instance of valgrind running." ) );
    return;
    // todo - ask for forced kill
  }
  
  m_widget->clear();
  currentMessage = QString::null;
  currentPid = -1;
  getActiveFiles();
  
  proc->clearArguments();  
  *proc << valExec << valParams << exec << params;
  proc->start( KProcess::NotifyOnExit, KProcess::AllOutput );
  topLevel()->raiseView( m_widget );
  core()->running( this, true );
}

void ValgrindPart::receivedStdout( KProcess*, char* msg, int len )
{
  kdDebug() << "got StdOut: " <<QString::fromLocal8Bit( msg, len ) << endl;
}

void ValgrindPart::receivedStderr( KProcess*, char* msg, int len )
{
  QRegExp valRe( "==(\\d+)== (.*)" );
  QString rmsg = lastPiece + QString::fromLocal8Bit( msg, len );
  QStringList lines = QStringList::split( "\n", rmsg );

//  kdDebug() << "got: " << QString::fromLocal8Bit( msg, len ) << endl;

  if ( !rmsg.endsWith( "\n" ) ) {
    // the last message is trucated, we'll receive
    // the rest in the next call
    lastPiece = lines.back();
    lines.pop_back();
  } else {
    lastPiece = QString::null;
  }
  
  for ( QStringList::ConstIterator it = lines.begin(); it != lines.end(); ++it ) {
    if ( valRe.search( *it ) < 0 )
      continue;
    
    int cPid = valRe.cap( 1 ).toInt();
      
    if ( valRe.cap( 2 ).isEmpty() ) {
      appendMessage( currentMessage );
      currentMessage = QString::null;
    } else if ( cPid != currentPid ) {
      appendMessage( currentMessage );
      currentMessage = *it;
      currentPid = cPid;
    } else {
      if ( !currentMessage.isEmpty() )
        currentMessage += "\n";
      currentMessage += *it;
    }
  }
}

void ValgrindPart::processExited( KProcess* p )
{
  if ( p == proc ) {
    appendMessage( currentMessage + lastPiece );
    currentMessage = QString::null;
    lastPiece = QString::null;
    core()->running( this, false );
  }
}

#include "valgrind_part.moc"
