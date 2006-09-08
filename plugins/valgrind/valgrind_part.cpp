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
#include <kgenericfactory.h>
#include <kaction.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <kicon.h>

#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevproject.h"

#include "valgrind_part.h"
#include "valgrind_dialog.h"
#include "valgrindmodel.h"
#include "valgrindcontrol.h"

typedef KGenericFactory<ValgrindPart> ValgrindFactory;
K_EXPORT_COMPONENT_FACTORY( kdevvalgrind, ValgrindFactory( "kdevvalgrind" ) )

ValgrindPart::ValgrindPart( QObject *parent, const QStringList& )
  : KDevPlugin( ValgrindFactory::instance(), parent)
  , m_model(new ValgrindModel())
  , m_control(new ValgrindControl(this))
{
  setXMLFile( "kdevpart_valgrind.rc" );

  m_treeView = new QTreeView();
  m_treeView->setWindowIcon(KIcon("fork"));
  m_treeView->setObjectName("valgrind");
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
}


ValgrindPart::~ValgrindPart()
{
  delete m_treeView;
}

QWidget *ValgrindPart::pluginView() const
{
    return m_treeView;
}

Qt::DockWidgetArea ValgrindPart::dockWidgetAreaHint() const
{
    return Qt::BottomDockWidgetArea;
}

void ValgrindPart::projectOpened()
{
  m_lastExec.truncate( 0 );
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
  ValgrindDialog* dlg = new ValgrindDialog(ValgrindDialog::Memcheck);
  /*if ( KDevApi::self()->project() && m_lastExec.isEmpty() ) {
    dlg->setExecutable( KDevApi::self()->project()->mainProgram() );
  } else {*/
    dlg->setExecutable( m_lastExec );
  //}
  dlg->setParameters( m_lastParams );
  dlg->setValExecutable( m_lastValExec );
  dlg->setValParams( m_lastValParams );
  m_lastValExec = dlg->valExecutable();
  m_lastValParams = dlg->valParams();
  if ( dlg->exec() == QDialog::Accepted ) {
    m_control->run( m_model, dlg->executableName(), dlg->parameters(), dlg->valExecutable(), dlg->valParams() );
  }
}

void ValgrindPart::slotExecCalltree()
{
    /* FIXME add a mainProgram function or equivalent so this can be ported
  ValgrindDialog* dlg = new ValgrindDialog(ValgrindDialog::Calltree);
  if ( KDevApi::self()->project() && m_lastExec.isEmpty() ) {
    dlg->setExecutable( project()->mainProgram() );
  } else {
    dlg->setExecutable( m_lastExec );
  }
  dlg->setParameters( m_lastParams );
  dlg->setCtExecutable( m_lastCtExec );
  dlg->setKcExecutable( m_lastKcExec );
  dlg->setCtParams( m_lastCtParams );
  kcInfo.runKc = true;
  kcInfo.kcPath = dlg->kcExecutable();
//  kcInfo.kcWorkDir = KUrl(dlg->executableName()).directory();
  if ( dlg->exec() == QDialog::Accepted ) {
    runValgrind( dlg->executableName(), dlg->parameters(), dlg->ctExecutable(), dlg->ctParams() );
  }
  m_lastKcExec = dlg->kcExecutable();
  m_lastCtExec = dlg->ctExecutable();
  m_lastCtParams = dlg->ctParams();
    */
}

void ValgrindPart::slotKillValgrind()
{
  m_control->stop();
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
}

void ValgrindPart::restorePartialProjectSession( const QDomElement* el )
{
  QDomElement execElem = el->namedItem( "executable" ).toElement();
  m_lastExec = execElem.attribute( "path", "" );
  m_lastParams = execElem.attribute( "params", "" );

  QDomElement valElem = el->namedItem( "valgrind" ).toElement();
  m_lastValExec = valElem.attribute( "path", "" );
  m_lastValParams = valElem.attribute( "params", "" );

  QDomElement ctElem = el->namedItem( "calltree" ).toElement();
  m_lastCtExec = ctElem.attribute( "path", "" );
  m_lastCtParams = ctElem.attribute( "params", "" );

  QDomElement kcElem = el->namedItem( "kcachegrind" ).toElement();
  m_lastKcExec = kcElem.attribute( "path", "" );
}

void ValgrindPart::savePartialProjectSession( QDomElement* el )
{
  QDomDocument domDoc = el->ownerDocument();
  if ( domDoc.isNull() )
    return;

  QDomElement execElem = domDoc.createElement( "executable" );
  execElem.setAttribute( "path", m_lastExec );
  execElem.setAttribute( "params", m_lastParams );

  QDomElement valElem = domDoc.createElement( "valgrind" );
  valElem.setAttribute( "path", m_lastValExec );
  valElem.setAttribute( "params", m_lastValParams );

  QDomElement ctElem = domDoc.createElement( "calltree" );
  ctElem.setAttribute( "path", m_lastCtExec );
  ctElem.setAttribute( "params", m_lastCtParams );

  QDomElement kcElem = domDoc.createElement( "kcachegrind" );
  kcElem.setAttribute( "path", m_lastKcExec );

  el->appendChild( execElem );
  el->appendChild( valElem );
  el->appendChild( ctElem );
  el->appendChild( kcElem );
}

#include "valgrind_part.moc"
