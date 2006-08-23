#include <unistd.h>

#include <QRegExp>
#include <QFile>
#include <QTreeView>
#include <QXmlInputSource>
#include <QXmlSimpleReader>
#include <QTcpSocket>
#include <QTcpServer>

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
#include "kdevgenericfactory.h"

#include "valgrind_part.h"
#include "valgrind_dialog.h"
#include "valgrindmodel.h"

typedef KGenericFactory<ValgrindPart> ValgrindFactory;
K_EXPORT_COMPONENT_FACTORY( kdevvalgrind, ValgrindFactory( "kdevvalgrind" ) )

ValgrindPart::ValgrindPart( QObject *parent, const QStringList& )
    : KDevPlugin( ValgrindFactory::instance(), parent)
  , m_inputSource(0L)
  , m_xmlReader(new QXmlSimpleReader())
  , m_model(new ValgrindModel())
  , m_valgrindServer(0L)
  , m_valgrindConnection(0L)
{
  setXMLFile( "kdevpart_valgrind.rc" );

  proc = new KShellProcess();
  connect( proc, SIGNAL(receivedStdout( KProcess*, char*, int )),
           this, SLOT(receivedStdout( KProcess*, char*, int )) );
  connect( proc, SIGNAL(receivedStderr( KProcess*, char*, int )),
           this, SLOT(receivedStderr( KProcess*, char*, int )) );
  connect( proc, SIGNAL(processExited( KProcess* )),
           this, SLOT(processExited( KProcess* )) );
//   FIXME find replacement
//   connect( KDevApi::self()->core(), SIGNAL(stopButtonClicked(KDevPlugin*)),
//            this, SLOT(slotStopButtonClicked(KDevPlugin*)) );
//   connect( KDevApi::self()->core(), SIGNAL(projectOpened()),
//            this, SLOT(projectOpened()) );

  m_treeView = new QTreeView();
  m_treeView->setWindowIcon(KIcon("fork"));
  m_treeView->setWindowTitle(i18n("Valgrind Output"));
  m_treeView->setModel(m_model);

  m_treeView->setWhatsThis( i18n( "<b>Valgrind</b><p>Shows the output of the valgrind. Valgrind detects<br>"
    "use of uninitialized memory<br>"
    "reading/writing memory after it has been free'd<br>"
    "reading/writing off the end of malloc'd blocks<br>"
    "reading/writing inappropriate areas on the stack<br>"
    "memory leaks -- where pointers to malloc'd blocks are lost forever<br>"
    "passing of uninitialised and/or unaddressable memory to system calls<br>"
    "mismatched use of malloc/new/new [] vs free/delete/delete []<br>"
    "some abuses of the POSIX pthread API." ) );

  KAction* action = new KAction( i18n("&Valgrind Memory Leak Check"), actionCollection(), "tools_valgrind" );
  action->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_V);
  action->setToolTip(i18n("Valgrind memory leak check"));
  action->setWhatsThis(i18n("<b>Valgrind memory leak check</b><p>Runs Valgrind - a tool to help you find memory-management problems in your programs."));
  connect(action, SIGNAL(triggered(bool)), SLOT(slotExecValgrind()));

  action = new KAction( i18n("P&rofile with KCachegrind"), actionCollection(), "tools_calltree" );
  action->setToolTip(i18n("Profile with KCachegrind"));
  action->setWhatsThis(i18n("<b>Profile with KCachegrind</b><p>Runs your program in calltree and then displays profiler information in KCachegrind."));
  connect(action, SIGNAL(triggered(bool)), SLOT(slotExecCalltree()));

  KDevCore::mainWindow()->embedOutputView( m_widget, "Valgrind", i18n("Valgrind memory leak check") );
}


ValgrindPart::~ValgrindPart()
{
  if ( m_widget )
      KDevCore::mainWindow()->removeView( m_widget );
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
    KMessageBox::sorry( 0, i18n("Could not open valgrind output: %1", fName) );
    return;
  }

  clear();

  /*FIXME
  QTextStream stream( &f );
  while ( !stream.atEnd() ) {
    receivedString( stream.readLine() + "\n" );
  }*/
  f.close();
}

void ValgrindPart::slotExecValgrind()
{
    /* FIXME add a mainProgram function or equivalent so this can be ported
  ValgrindDialog* dlg = new ValgrindDialog(ValgrindDialog::Memcheck);
  if ( KDevApi::self()->project() && _lastExec.isEmpty() ) {
    dlg->setExecutable( KDevApi::self()->project()->mainProgram() );
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
    */
}

void ValgrindPart::slotExecCalltree()
{
    /* FIXME add a mainProgram function or equivalent so this can be ported
  ValgrindDialog* dlg = new ValgrindDialog(ValgrindDialog::Calltree);
  if ( KDevApi::self()->project() && _lastExec.isEmpty() ) {
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
    */
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
  m_model->clear();
  currentPid = -1;
}

void ValgrindPart::receivedStdout( KProcess*, char* /* msg */, int /* len */ )
{
  //kDebug() << "got StdOut: " <<QString::fromLocal8Bit( msg, len ) << endl;
}

void ValgrindPart::receivedStderr( KProcess*, char* msg, int len )
{
  //receivedString( QString::fromLocal8Bit( msg, len ) );
}

void ValgrindPart::slotReadFromValgrind( )
{
  kDebug() << k_funcinfo << endl;
  m_xmlReader->parseContinue();
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

void ValgrindPart::newValgrindConnection( )
{
  QTcpSocket* sock = m_valgrindServer->nextPendingConnection();
  kDebug() << k_funcinfo << sock << endl;
  if (sock && !m_valgrindConnection) {
    m_valgrindConnection = sock;
    delete m_inputSource;
    m_inputSource = new QXmlInputSource(sock);
    m_xmlReader->parse(m_inputSource, true);
    connect(sock, SIGNAL(readyRead()), SLOT(slotReadFromValgrind()));
  }
}

#include "valgrind_part.moc"
