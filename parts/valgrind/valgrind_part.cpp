#include <qregexp.h>
#include <qfile.h>
//Added by qt3to4:
#include <QTextStream>

#include <kiconloader.h>
#include <klocale.h>
#include <kdevgenericfactory.h>
#include <kaction.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kdebug.h>

#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevproject.h"
#include "kdevplugininfo.h"

#include "valgrind_widget.h"
#include "valgrind_part.h"
#include "valgrind_dialog.h"
#include "valgrinditem.h"

typedef KDevGenericFactory<ValgrindPart> ValgrindFactory;
static const KDevPluginInfo data("kdevvalgrind");
K_EXPORT_COMPONENT_FACTORY( libkdevvalgrind, ValgrindFactory( data ) )

ValgrindPart::ValgrindPart( QObject *parent, const char *name, const QStringList& )
  : KDevPlugin( &data, parent)
{
  setObjectName(QString::fromUtf8(name));
  setInstance( ValgrindFactory::instance() );
  setXMLFile( "kdevpart_valgrind.rc" );

  proc = new KShellProcess();
  connect( proc, SIGNAL(receivedStdout( KProcess*, char*, int )),
           this, SLOT(receivedStdout( KProcess*, char*, int )) );
  connect( proc, SIGNAL(receivedStderr( KProcess*, char*, int )),
           this, SLOT(receivedStderr( KProcess*, char*, int )) );
  connect( proc, SIGNAL(processExited( KProcess* )),
           this, SLOT(processExited( KProcess* )) );
  connect( core(), SIGNAL(stopButtonClicked(KDevPlugin*)),
           this, SLOT(slotStopButtonClicked(KDevPlugin*)) );
  connect( core(), SIGNAL(projectOpened()),
           this, SLOT(projectOpened()) );

  m_widget = new ValgrindWidget( this );
  m_widget->setIcon( SmallIcon("fork") );
  m_widget->setCaption( i18n("Valgrind Output"));

  m_widget->setWhatsThis( i18n( "<b>Valgrind</b><p>Shows the output of the valgrind. Valgrind detects<br>"
    "use of uninitialized memory<br>"
    "reading/writing memory after it has been free'd<br>"
    "reading/writing off the end of malloc'd blocks<br>"
    "reading/writing inappropriate areas on the stack<br>"
    "memory leaks -- where pointers to malloc'd blocks are lost forever<br>"
    "passing of uninitialised and/or unaddressable memory to system calls<br>"
    "mismatched use of malloc/new/new [] vs free/delete/delete []<br>"
    "some abuses of the POSIX pthread API." ) );

  KAction* action = new KAction( i18n("&Valgrind Memory Leak Check"), 0, this,
           SLOT(slotExecValgrind()), actionCollection(), "tools_valgrind" );
  action->setToolTip(i18n("Valgrind memory leak check"));
  action->setWhatsThis(i18n("<b>Valgrind memory leak check</b><p>Runs Valgrind - a tool to help you find memory-management problems in your programs."));

  action = new KAction( i18n("P&rofile with KCachegrind"), 0, this,
           SLOT(slotExecCalltree()), actionCollection(), "tools_calltree" );
  action->setToolTip(i18n("Profile with KCachegrind"));
  action->setWhatsThis(i18n("<b>Profile with KCachegrind</b><p>Runs your program in calltree and then displays profiler information in KCachegrind."));

  mainWindow()->embedOutputView( m_widget, "Valgrind", i18n("Valgrind memory leak check") );
}


ValgrindPart::~ValgrindPart()
{
  if ( m_widget )
    mainWindow()->removeView( m_widget );
  delete m_widget;
  delete proc;
}

void ValgrindPart::projectOpened()
{
  _lastExec.truncate( 0 );
}

void ValgrindPart::loadOutput()
{
  QString fName = KFileDialog::getOpenFileName(QString(), "*", 0, i18n("Open Valgrind Output"));
  if ( fName.isEmpty() )
    return;

  QFile f( fName );
  if ( !f.open( QIODevice::ReadOnly ) ) {
    KMessageBox::sorry( 0, i18n("Could not open valgrind output: %1").arg(fName) );
    return;
  }

  clear();
  getActiveFiles();

  QTextStream stream( &f );
  while ( !stream.atEnd() ) {
    receivedString( stream.readLine() + "\n" );
  }
  f.close();
}

void ValgrindPart::getActiveFiles()
{
  activeFiles.clear();
  if ( project() ) {
    QStringList projectFiles = project()->allFiles();
    QString projectDirectory = project()->projectDirectory();
    KUrl url;
    for ( QStringList::Iterator it = projectFiles.begin(); it != projectFiles.end(); ++it ) {
      KUrl url( projectDirectory + "/" + (*it) );
      url.cleanPath( true );
      activeFiles += url.path();
      kdDebug() << "set project file: " << url.path().latin1() << endl;
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
  ValgrindDialog* dlg = new ValgrindDialog(ValgrindDialog::Memcheck);
  if ( project() && _lastExec.isEmpty() ) {
    dlg->setExecutable( project()->mainProgram() );
  } else {
    dlg->setExecutable( _lastExec );
  }
  dlg->setParameters( _lastParams );
  dlg->setValExecutable( _lastValExec );
  dlg->setValParams( _lastValParams );
  kcInfo.runKc = false;
  _lastValExec = dlg->valExecutable();
  _lastValParams = dlg->valParams();
  if ( dlg->exec() == QDialog::Accepted ) {
    runValgrind( dlg->executableName(), dlg->parameters(), dlg->valExecutable(), dlg->valParams() );
  }
}

void ValgrindPart::slotExecCalltree()
{
  ValgrindDialog* dlg = new ValgrindDialog(ValgrindDialog::Calltree);
  if ( project() && _lastExec.isEmpty() ) {
    dlg->setExecutable( project()->mainProgram() );
  } else {
    dlg->setExecutable( _lastExec );
  }
  dlg->setParameters( _lastParams );
  dlg->setCtExecutable( _lastCtExec );
  dlg->setKcExecutable( _lastKcExec );
  dlg->setCtParams( _lastCtParams );
  kcInfo.runKc = true;
  kcInfo.kcPath = dlg->kcExecutable();
//  kcInfo.kcWorkDir = KUrl(dlg->executableName()).directory();
  if ( dlg->exec() == QDialog::Accepted ) {
    runValgrind( dlg->executableName(), dlg->parameters(), dlg->ctExecutable(), dlg->ctParams() );
  }
  _lastKcExec = dlg->kcExecutable();
  _lastCtExec = dlg->ctExecutable();
  _lastCtParams = dlg->ctParams();
}

void ValgrindPart::slotKillValgrind()
{
  if ( proc )
    proc->kill();
}

void ValgrindPart::slotStopButtonClicked( KDevPlugin* which )
{
  if ( which != 0 && which != this )
    return;
  slotKillValgrind();
}

void ValgrindPart::clear()
{
  m_widget->clear();
  currentMessage = QString();
  currentPid = -1;
  lastPiece = QString();
}

void ValgrindPart::runValgrind( const QString& exec, const QString& params, const QString& valExec, const QString& valParams )
{
  if ( proc->isRunning() ) {
    KMessageBox::sorry( 0, i18n( "There is already an instance of valgrind running." ) );
    return;
    /// @todo - ask for forced kill
  }

  clear();

  getActiveFiles();

//  proc->setWorkingDirectory(KUrl(exec).directory());
  proc->clearArguments();
  *proc << valExec << /*"--tool=memcheck" << */valParams << exec << params;
  proc->start( KProcess::NotifyOnExit, KProcess::AllOutput );
  mainWindow()->raiseView( m_widget );
  core()->running( this, true );

  _lastExec = exec;
  _lastParams = params;
}

void ValgrindPart::receivedStdout( KProcess*, char* /* msg */, int /* len */ )
{
  //kdDebug() << "got StdOut: " <<QString::fromLocal8Bit( msg, len ) << endl;
}

void ValgrindPart::receivedStderr( KProcess*, char* msg, int len )
{
  receivedString( QString::fromLocal8Bit( msg, len ) );
}

void ValgrindPart::receivedString( const QString& str )
{
  QString rmsg = lastPiece + str;
  QStringList lines = QStringList::split( "\n", rmsg );

//  kdDebug() << "got: " << QString::fromLocal8Bit( msg, len ) << endl;

  if ( !rmsg.endsWith( "\n" ) ) {
    // the last message is trucated, we'll receive
    // the rest in the next call
    lastPiece = lines.back();
    lines.pop_back();
  } else {
    lastPiece = QString();
  }
  appendMessages( lines );
}

void ValgrindPart::appendMessages( const QStringList& lines )
{
  QRegExp valRe( "==(\\d+)== (.*)" );

  for ( QStringList::ConstIterator it = lines.begin(); it != lines.end(); ++it ) {
    if ( valRe.search( *it ) < 0 )
      continue;

    int cPid = valRe.cap( 1 ).toInt();

    if ( valRe.cap( 2 ).isEmpty() ) {
      appendMessage( currentMessage );
      currentMessage = QString();
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
    currentMessage = QString();
    lastPiece = QString();
    core()->running( this, false );

    if (kcInfo.runKc)
    {
        KProcess *kcProc = new KProcess;
//        kcProc->setWorkingDirectory(kcInfo.kcWorkDir);
        *kcProc << kcInfo.kcPath;
        *kcProc << QString("cachegrind.out.%1").arg(p->pid());
        kcProc->start(KProcess::DontCare);
    }
  }
}

void ValgrindPart::restorePartialProjectSession( const QDomElement* el )
{
  QDomElement execElem = el->namedItem( "executable" ).toElement();
  _lastExec = execElem.attribute( "path", "" );
  _lastParams = execElem.attribute( "params", "" );

  QDomElement valElem = el->namedItem( "valgrind" ).toElement();
  _lastValExec = valElem.attribute( "path", "" );
  _lastValParams = valElem.attribute( "params", "" );

  QDomElement ctElem = el->namedItem( "calltree" ).toElement();
  _lastCtExec = ctElem.attribute( "path", "" );
  _lastCtParams = ctElem.attribute( "params", "" );

  QDomElement kcElem = el->namedItem( "kcachegrind" ).toElement();
  _lastKcExec = kcElem.attribute( "path", "" );
}

void ValgrindPart::savePartialProjectSession( QDomElement* el )
{
  QDomDocument domDoc = el->ownerDocument();
  if ( domDoc.isNull() )
    return;

  QDomElement execElem = domDoc.createElement( "executable" );
  execElem.setAttribute( "path", _lastExec );
  execElem.setAttribute( "params", _lastParams );

  QDomElement valElem = domDoc.createElement( "valgrind" );
  valElem.setAttribute( "path", _lastValExec );
  valElem.setAttribute( "params", _lastValParams );

  QDomElement ctElem = domDoc.createElement( "calltree" );
  ctElem.setAttribute( "path", _lastCtExec );
  ctElem.setAttribute( "params", _lastCtParams );

  QDomElement kcElem = domDoc.createElement( "kcachegrind" );
  kcElem.setAttribute( "path", _lastKcExec );

  el->appendChild( execElem );
  el->appendChild( valElem );
  el->appendChild( ctElem );
  el->appendChild( kcElem );
}

#include "valgrind_part.moc"
