/* This file is part of the KDE libraries
   Copyright (C) 2001 Harald Fernengel <harry@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <stdlib.h>

#include <qapplication.h>
#include <qprocess.h>
#include <qtimer.h>
#include <qvariant.h>

#include <kdebug.h>
#include <kwin.h>
#include <kwinmodule.h>
#include <kprocess.h>

#include "document.h"
#include "view.h"
#include "kneditfactory.h"

namespace KNEdit
{

// NEdit Highlightmodes are hard-coded, so we hard-code them here, too ;)
// Took from NEdit 5.2
static const char* hlModes[] = { "Plain", "Ada", "Awk", "C++", "C", "CSS", "Csh",
        "Fortran", "Java", "JavaScript", "LaTeX", "Lex", "Makefile",
        "Matlab", "NEdit Macro", "Pascal", "Perl", "PostScript",
        "Python", "Regex", "SGML HTML", "SQL", "Sh Ksh Bash", "Tcl",
        "VHDL", "Verilog", "XML", "X Resources", "Yacc", 0 };
static const uint hlMCount = 29;

Document::Document(bool bReadOnly, bool bSingleView, QWidget *parentWidget, const char *widgetName,
                   QObject *parent, const char *name) : KTextEditor::Document (parent, name)
{
  m_isReady = false;
  activeView = 0;
  m_wid = 0;
  m_singleView = bSingleView;
  m_readOnly = bReadOnly;
  m_kwm = new KWinModule( this );
  m_timer = new QTimer( this, "NEdit Timer" );
  connect( m_timer, SIGNAL(timeout()), this, SLOT(checkForNEditServer()) );

  setInstance( KNEditFactory::instance() );

  // unique NEdit server name
  // TODO: make this really unique...
  m_serverName = QString::fromLatin1( "ktexteditor_nedit_" + QString::number( documentNumber() ) );

  // start one NEdit server per document
  proc = new QProcess( QString::fromLatin1( "nedit" ), this, "nedit process" );
  proc->addArgument( "-server" );
  proc->addArgument( "-svrname" );
  proc->addArgument( m_serverName );
  connect(proc, SIGNAL(processExited()), this, SLOT(processExited()));
  connect(m_kwm, SIGNAL(windowAdded(WId)), this, SLOT(launchFinished(WId)) );
  if ( !proc->launch( QByteArray() ) ) {
    kdDebug() << "cannot start NEdit server" << endl;
    delete proc;
    proc = 0;
  }

  if ( bSingleView ) {
    KTextEditor::View *v = createView( parentWidget, widgetName );
    v->show();
    setWidget( v );
  }
}

Document::~Document()
{
  kdDebug() << "Document::~Document" << endl;
}

KTextEditor::View *Document::createView( QWidget *parent, const char *name )
{
  return new View( this, parent, name );
}

bool Document::openFile()
{
  invokeNC( QString::null, true );
  return true;
}

bool Document::saveFile()
{
  invokeNC("-do \"save()\"", true);
  return true;
}

void Document::addView(KTextEditor::View *view)
{
  _views.append( view );
  activeView = view;
}

void Document::removeView(KTextEditor::View *view)
{
  if (activeView == view)
    activeView = 0;

  _views.removeRef( view  );
}

void Document::invokeNC( const QString& command, bool appendFilename )
{
  QString cmd = command;
  if ( appendFilename )
#if (KDE_VERSION > 305)
    cmd += " " + KProcess::quote(m_file);
#else
    cmd += " " + KShellProcess::quote(m_file);
#endif

  if ( !m_isReady ) {
    // if NEdit server is not ready yet we store the commands for later
    m_commands += cmd;
  } else {
#if (KDE_VERSION > 305)
    cmd = "nc -noask -svrname " + KProcess::quote(serverName()) + " " + cmd;
#else
    cmd = "nc -noask -svrname " + KShellProcess::quote(serverName()) + " " + cmd;
#endif
    system( cmd );
  }
}

// processes all commands in the queue
void Document::processCommands()
{
  while ( !m_commands.isEmpty() ) {
    invokeNC( m_commands.first(), false );
    m_commands.pop_front();
  }
}

// call this function when the NEdit Server is up and running
// and ready to receive commands
void Document::serverReady( WId wid )
{
  if ( m_isReady )
    return;

  disconnect( m_kwm, SIGNAL(windowAdded(WId)), this, SLOT(launchFinished(WId)) );

  m_wid = wid;
  m_isReady = true;

  if ( activeView ) {
    View* aView = (View*)activeView->qt_cast( "KNEdit::View" );
    if ( aView ) {
        aView->embedNEdit( wid );
    }
  }

  processCommands();
}


void Document::launchFinished( WId wid )
{
  if ( m_isReady ) {
    return;
  }

  KWin::Info inf = KWin::info( wid );
  kdDebug() << "launchFinished: " << wid << " " << inf.pid << " " << inf.name << " || " << inf.visibleName << endl;

  if ( inf.name.startsWith( "-" + m_serverName + "-" ) ) {
    // yepp - this is the Process we wanted...
    serverReady( wid );
  } else {
    // NEdit server is launched, but not ready to receive commands yet
    // so we need a stupid timer to check when it is ready...
    m_widCache += wid;
    if ( !m_timer->isActive() ) {
      m_timer->start( 200, false );
    }
  }
}

// checks whether our NEdit server is ready to process commands...
void Document::checkForNEditServer()
{
  QValueList<WId>::iterator it;
  for ( it = m_widCache.begin(); it != m_widCache.end(); ++it ) {
    KWin::Info inf = KWin::info( *it );
    if ( inf.name.startsWith( "-" + m_serverName + "-" ) ) {
      // yepp - this is the Process we wanted...
      m_timer->stop();
      disconnect( m_timer, SIGNAL(timeout()), this, SLOT(checkForNEditServer()) );

      serverReady( *it );
      m_widCache.clear();
      return;
    }
  }
}


void Document::processExited()
{
  // NEdit has been closed by user, so the document gets inactive
  m_isReady = false;
  delete proc;
  proc = 0;

// TODO - notify views

  kdDebug() << "Process Exited" << endl;

  // life is useless without an NEdit server...
  deleteLater();
}

uint Document::hlMode ()
{
  return 0; // TODO: get hlMode  
}

bool Document::setHlMode (uint mode)
{
  if ( ( mode < hlMCount ) ) {
    invokeNC("-do \"set_language_mode(\\\"" + QString::fromLatin1(hlModes[mode]) + "\\\")\"", true);
  }
  return false;
}

uint Document::hlModeCount ()
{
  return hlMCount;
}

QString Document::hlModeName (uint mode)
{
  if ( mode < hlMCount )
    return QString::fromLatin1( hlModes[mode] );
  return QString::null;
}

QString Document::hlModeSectionName (uint mode)
{
  return QString::null;
}

};

#include "document.moc"

