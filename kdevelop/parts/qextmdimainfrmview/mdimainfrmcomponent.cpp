/***************************************************************************
  mdimainfrmcomponent.cpp  -  a viewhandler (kdevcomponent) based on QextMDI
                             -------------------
    begin                : Thu Jul 27 2000
    copyright            : (C) 2000 by Falk Brettschneider
    email                : <Falk Brettschneider> falk@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kaction.h>
#include <kmainwindow.h>
#include <kparts/event.h>

#include "mdimainfrmcomponent.h"
#include "mdiframe.h"
#include "mdiview.h"
#include "main.h"

//---------------------------------------------------------------------------

MdiMainFrmComponent::MdiMainFrmComponent(QObject *parent, const char *name)
    : KDevViewHandler(parent, name)
  ,m_mainframe(0L)
{
  setInstance(MdiMainFrmFactory::instance());
  setXMLFile("kdevmdimainfrmcomponent.rc");
}

//---------------------------------------------------------------------------

MdiMainFrmComponent::~MdiMainFrmComponent()
{
}

//---------------------------------------------------------------------------

void MdiMainFrmComponent::setupGUI()
{
  kdDebug(9005) << "running MdiMainFrmComponent::setupGUI..." << endl;

  // embed the MDI mainframe in KDevelop
  m_mainframe = new MdiFrame(0L);
  emit embedWidget(m_mainframe, AreaOfDocumentViews, i18n("QextMdi-MainFrm"), i18n("source file views area"));

  // give the application menubar to QextMDI
  KMainWindow* tlw = (KMainWindow*) m_mainframe->topLevelWidget();
  m_mainframe->setMenuForSDIModeSysButtons( (QMenuBar*) tlw->menuBar());

  // construct 'Window' popupmenu of KDevelop by actions
  KAction *action;
  action = new KAction( i18n("&Close"), 0, m_mainframe, SLOT(closeActiveView()),
                        actionCollection(), "close_view");
  action->setStatusText( i18n("Closes the focused document view") );
  action = new KAction( i18n("Close &All"), 0, m_mainframe, SLOT(closeAllViews()),
                        actionCollection(), "close_all_views");
  action->setStatusText( i18n("Closes all document views") );
  action = new KAction( i18n("&Iconify All"), 0, m_mainframe, SLOT(iconifyAllViews()),
                        actionCollection(), "iconify_all_views");
  action->setStatusText( i18n("Iconifies all document views") );

  action = new KAction( i18n("Ca&scade windows"), 0, m_mainframe, SLOT(cascadeWindows()),
                        actionCollection(), "cascade_windows");
  action->setStatusText( i18n("Repositiones all document views in a cascading order") );
  action = new KAction( i18n("Cascade &maximized"), 0, m_mainframe, SLOT(cascadeMaximized()),
                        actionCollection(), "cascade_maximized");
  action->setStatusText( i18n("Repositiones all document views maximized and in a cascading order") );
  action = new KAction( i18n("Expand &vertical"), 0, m_mainframe, SLOT(expandVertical()),
                        actionCollection(), "expand_vertical");
  action->setStatusText( i18n("Maximizes all document views in vertical direction, only") );
  action = new KAction( i18n("Expand &horizontal"), 0, m_mainframe, SLOT(expandHorizontal()),
                        actionCollection(), "expand_horizontal");
  action->setStatusText( i18n("Maximizes all document views in horizontal direction, only") );
  action = new KAction( i18n("A&nodine's tile"), 0, m_mainframe, SLOT(tileAnodine()),
                        actionCollection(), "anodines_tile");
  action->setStatusText( i18n("") );
  action = new KAction( i18n("&Pragma's tile"), 0, m_mainframe, SLOT(tilePragma()),
                        actionCollection(), "pragmas_tile");
  action->setStatusText( i18n("") );
  action = new KAction( i18n("Tile v&ertically"), 0, m_mainframe, SLOT(tileVertically()),
                        actionCollection(), "tile_vertically");
  action->setStatusText( i18n("Tiles all document views vertically") );

  action = new KAction( i18n("&Toplevel mode"), 0, m_mainframe, SLOT(switchToToplevelMode()),
                        actionCollection(), "toplevel_mode");
  action->setStatusText( i18n("Undocks all document views to toplevel windows and drop the views area of KDevelop") );
  action = new KAction( i18n("C&hildframe mode"), 0, m_mainframe, SLOT(switchToChildframeMode()),
                        actionCollection(), "childframe_mode");
  action->setStatusText( i18n("Docks all document views to the views area of KDevelop") );

//  action = new KAction( i18n("Test"), 0, 0, SLOT(test()),
//                        actionCollection());
//  m_viewList.append(action);
//  action = new KAction( i18n("Test2"), 0, m_mainframe, SLOT(switchToToplevelMode()),
//                        actionCollection());
//  m_viewsDockList.append(action);
//  plugActionList( QString::fromLatin1("viewlist"), m_viewList );
//  plugActionList( QString::fromLatin1("views_docklist"), m_viewsDockList );
//
//  action = new KAction( i18n("xxx"), 0, m_mainframe, SLOT(switchToToplevelMode()),
//                        actionCollection(), "window");
}

//---------------------------------------------------------------------------

void MdiMainFrmComponent::projectClosed()
{
  m_mainframe->closeAllViews();
}

//---------------------------------------------------------------------------

void MdiMainFrmComponent::addView( QWidget* w)
{
  kdDebug(9005) << "running MdiMainFrmComponent::addView : " << w << endl;

  MdiView* wrapper = new MdiView( w, w->name());
  int flags = QextMdi::StandardAdd;
  if (m_mainframe->isInMaximizedChildFrmMode())
    flags |= QextMdi::Maximize;
  if (m_mainframe->isInTopLevelMode())
    flags |= QextMdi::Detach;

  m_mainframe->addWindow( wrapper, flags);
}

//---------------------------------------------------------------------------

void MdiMainFrmComponent::removeView( QWidget* w)
{
  kdDebug(9005) << "running MdiMainFrmComponent::removeView..." << endl;

  if (!w->parentWidget())
    return;
  if (!w->parentWidget()->inherits("QextMdiChildView"))
    return;

  QextMdiChildView* pParent = (QextMdiChildView*) w->parentWidget();
  pParent->hide();

  m_mainframe->removeWindowFromMdi( pParent);  // Note: the inner widget w keeps still alive!
}

//---------------------------------------------------------------------------

bool MdiMainFrmComponent::event( QEvent *e)
{
  kdDebug(9005) << "running MdiMainFrmComponent::event..." << endl;

  bool ret = KDevViewHandler::event( e);
  // ### Are you sure that you are sending that event from KDevelop?
  //     See also http://lists.kde.org/?l=kde-devel&m=96469534324495&w=2
  //     (Simon)
  if (KParts::GUIActivateEvent::test( e)) {
    KParts::GUIActivateEvent* ae = (KParts::GUIActivateEvent*) e;
    if (ae->activated()) {
      // rebuild the action lists
    }
  }
  return ret;
}

