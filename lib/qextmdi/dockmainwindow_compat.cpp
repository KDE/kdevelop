
/* This file is part of the KDE libraries
   Copyright (C) 2000 Max Judin <novaprint@mtu-net.ru>
   Copyright (C) 2002,2003 Joseph Wenninger <jowenn@kde.org>

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

#include <qapplication.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qobjectlist.h>
#include <qstrlist.h>
#include <qcursor.h>
#include <qwidgetlist.h>
#include <qtabwidget.h>
#include <qtooltip.h>
#include <qstyle.h>
#include <qpushbutton.h>
#include <qguardedptr.h>
#include <qapplication.h>
#include <qnamespace.h>

#ifndef NO_KDE2
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <ktoolbar.h>
#include <kpopupmenu.h>
#include <kwin.h>
#include <kdebug.h>
#include <kparts/event.h>
#include <kparts/part.h>
#include <kaccel.h>
#include <kparts/plugin.h>
#include <kstatusbar.h>
#include <kinstance.h>
#include <khelpmenu.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kxmlguifactory.h>
#include <kdeversion.h>
#include <assert.h>
#ifdef Q_WS_X11
#include <X11/X.h>
#include <X11/Xlib.h>
#endif
#else
#include <qtoolbar.h>
#include <qpopupmenu.h>
#endif

#include <stdlib.h>

#include "kdockwidget_compat.h"

#ifndef NO_KDE2
#include <netwm_def.h>
#include "dockmainwindow_compat.h"

using namespace KDockWidget_Compat;
using namespace KParts;

class DockMainWindowPrivate
{
public:
  DockMainWindowPrivate()
  {
    m_activePart = 0;
    m_bShellGUIActivated = false;
    m_helpMenu = 0;
  }
  ~DockMainWindowPrivate()
  {
  }

  QGuardedPtr<Part> m_activePart;
  bool m_bShellGUIActivated;
  KHelpMenu *m_helpMenu;
};

DockMainWindow::DockMainWindow( QWidget* parent, const char *name, Qt::WFlags f )
  : KDockMainWindow( parent, name, f )
{
  d = new DockMainWindowPrivate();
  PartBase::setPartObject( this );
}

DockMainWindow::~DockMainWindow()
{
  delete d;
}

void DockMainWindow::createGUI( Part * part )
{
  kdDebug(1000) << QString("DockMainWindow::createGUI for %1").arg(part?part->name():"0L") << endl;

  KXMLGUIFactory *factory = guiFactory();

  setUpdatesEnabled( false );

  QPtrList<Plugin> plugins;

  if ( d->m_activePart )
  {
    kdDebug(1000) << QString("deactivating GUI for %1").arg(d->m_activePart->name()) << endl;

    GUIActivateEvent ev( false );
    QApplication::sendEvent( d->m_activePart, &ev );

#if KDE_VERSION < 310    
    plugins = Plugin::pluginObjects( d->m_activePart );
    Plugin *plugin = plugins.last();
    for (; plugin; plugin = plugins.prev() )
      factory->removeClient( plugin );
#endif

    factory->removeClient( d->m_activePart );

    disconnect( d->m_activePart, SIGNAL( setWindowCaption( const QString & ) ),
             this, SLOT( setCaption( const QString & ) ) );
    disconnect( d->m_activePart, SIGNAL( setStatusBarText( const QString & ) ),
             this, SLOT( slotSetStatusBarText( const QString & ) ) );
  }

  if ( !d->m_bShellGUIActivated )
  {
#if KDE_VERSION >= 310    
    loadPlugins( this, this, KGlobal::instance() );
#endif    
    createShellGUI();
    d->m_bShellGUIActivated = true;
  }

  if ( part )
  {
    // do this before sending the activate event
    connect( part, SIGNAL( setWindowCaption( const QString & ) ),
             this, SLOT( setCaption( const QString & ) ) );
    connect( part, SIGNAL( setStatusBarText( const QString & ) ),
             this, SLOT( slotSetStatusBarText( const QString & ) ) );

    factory->addClient( part );

    GUIActivateEvent ev( true );
    QApplication::sendEvent( part, &ev );

#if KDE_VERSION < 310
    plugins = Plugin::pluginObjects( part );
    QPtrListIterator<Plugin> pIt( plugins );

    for (; pIt.current(); ++pIt )
      factory->addClient( pIt.current() );
#endif
  }

  setUpdatesEnabled( true );

  d->m_activePart = part;
}

void DockMainWindow::slotSetStatusBarText( const QString & text )
{
  statusBar()->message( text );
}

void DockMainWindow::createShellGUI( bool create )
{
#if KDE_VERSION >= 310
    bool bAccelAutoUpdate = accel()->setAutoUpdate( false );
    assert( d->m_bShellGUIActivated != create );
    d->m_bShellGUIActivated = create;
#endif    
    if ( create )
    {
        if ( isHelpMenuEnabled() )
            d->m_helpMenu = new KHelpMenu( this, instance()->aboutData(), true, actionCollection() );

        QString f = xmlFile();
        setXMLFile( locate( "config", "ui/ui_standards.rc", instance() ) );
        if ( !f.isEmpty() )
            setXMLFile( f, true );
        else
        {
            QString auto_file( instance()->instanceName() + "ui.rc" );
            setXMLFile( auto_file, true );
        }

        GUIActivateEvent ev( true );
        QApplication::sendEvent( this, &ev );

        guiFactory()->addClient( this );

#if KDE_VERSION < 310
        QPtrList<Plugin> plugins = Plugin::pluginObjects( this );
        QPtrListIterator<Plugin> pIt( plugins );
        for (; pIt.current(); ++pIt )
            guiFactory()->addClient( pIt.current() );
#endif
    }
    else
    {
        GUIActivateEvent ev( false );
        QApplication::sendEvent( this, &ev );

#if KDE_VERSION < 310
        QPtrList<Plugin> plugins = Plugin::pluginObjects( this );
        Plugin *plugin = plugins.last();
        for (; plugin; plugin = plugins.prev() )
            guiFactory()->removeClient( plugin );
#endif
        guiFactory()->removeClient( this );
    }
#if KDE_VERSION >= 310
    accel()->setAutoUpdate( bAccelAutoUpdate );
#endif
}

#endif // NO_KDE2

#ifndef NO_INCLUDE_MOCFILES // for Qt-only projects, because tmake doesn't take this name
#include "dockmainwindow_compat.moc"
#endif
