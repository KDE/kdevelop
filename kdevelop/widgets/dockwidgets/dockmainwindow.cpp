/***************************************************************************
                dockmainwindow.cpp  -  Impl. DockMainWindow
                             -------------------
    begin                : Now 21 21:08:00 1999
    copyright            : (C) 2000 by Judin Max (novaprint@mtu-net.ru)
    email                : novaprint@mtu-net.ru

		improved/changed by	 : Falk Brettschneider	(Jan 30 17:52 MET 2000)
													 email: gigafalk@yahoo.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dockmainwindow.h"
#include "docksplitter.h"
#include <qapplication.h>
#include <knewpanner.h>
#include <ktoolbar.h>
#include <stabctl.h>

#include "dock_close_bottom.xpm"
#include "dock_close_left.xpm"
#include "dock_close_right.xpm"
#include "dock_close_top.xpm"

DockMainWindow::DockMainWindow( const char *name )
:KTMainWindow( name )
{
  DockB.dock = 0L;
  DockB.pos  = DockBottom;

  DockT.dock = 0L;
  DockT.pos  = DockTop;

  DockR.dock = 0L;
  DockR.pos  = DockRight;

  DockL.dock = 0L;
  DockL.pos  = DockLeft;

  configName = "lastsavedconfig";
  dockManager = new DockManager( this, QString(name)+"_DockManager");
  mainDockWidget = new DockWidget( dockManager, "MainWindowDockWidget", QPixmap("") );
  mainDockWidget->recreateTo( this );
  setView( mainDockWidget );
	mainDockWidget->setDraggable( false);											//F.B.
	mainDockWidget->setDockSite(DockLeft|DockRight|DockTop|DockBottom);	//F.B.
  
  toolbar = new KToolBar( this );
  toolbar->insertButton( QPixmap(dock_close_top), 1, true, "Close top dock" );
  toolbar->insertButton( QPixmap(dock_close_left), 2, true, "Close left dock" );
  toolbar->insertButton( QPixmap(dock_close_right), 3, true, "Close right dock" );
  toolbar->insertButton( QPixmap(dock_close_bottom), 4, true, "Close bottom dock" );

  toolbar->setToggle(1);
  toolbar->setToggle(2);
  toolbar->setToggle(3);
  toolbar->setToggle(4);

  toolbar->setButton( 1, true );
  toolbar->setButton( 2, true );
  toolbar->setButton( 3, true );
  toolbar->setButton( 4, true );

  toolbar->setFullWidth( false );
  connect( dockManager, SIGNAL(change()), SLOT(slotDockChange()) );
  connect( dockManager, SIGNAL(replaceDock(DockWidget*,DockWidget*)), SLOT(slotReplaceDock(DockWidget*,DockWidget*)) );
  connect( toolbar, SIGNAL(toggled(int)), SLOT(slotToggled(int)) );
}

DockMainWindow::~DockMainWindow()
{
    delete dockManager;
}

void DockMainWindow::setView( QWidget *view )
{
  if ( view->isA("DockWidget") ){
    KTMainWindow::setView(view);
    viewDock = (DockWidget*)view;
  } else {
    view->recreate( mainDockWidget, 0, QPoint(0,0) );
    QApplication::syncX();
  }
}

DockWidget* DockMainWindow::createDockWidget( const char* name, const QPixmap &pixmap )
{
  return new DockWidget( dockManager, name, pixmap );
}

void DockMainWindow::dockIn( DockWidget* w, DockPosition pos )
{
  w->manualDock( mainDockWidget, pos );
}

void DockMainWindow::slotDockChange()
{
  DockWidget* DL = 0L;
  DockWidget* DR = 0L;
  DockWidget* DT = 0L;
  DockWidget* DB = 0L;

  if ( mainDockWidget->parent() == this  || mainDockWidget->parent() == 0L ){
    if ( toolbar->isButtonOn(1) ) toolbar->setItemEnabled( 1, false );
    if ( toolbar->isButtonOn(2) ) toolbar->setItemEnabled( 2, false );
    if ( toolbar->isButtonOn(3) ) toolbar->setItemEnabled( 3, false );
    if ( toolbar->isButtonOn(4) ) toolbar->setItemEnabled( 4, false );
    return;
  }

  DockWidget* base = mainDockWidget;

  while ( base != 0L && base->parent()!= 0L && base->parent()->isA("DockSplitter") )
  {
    DockSplitter* s = (DockSplitter*)base->parent();
    int o = ((DockWidget*)s->parent())->splitterOrientation;
    DockWidget* another = (DockWidget*)s->getAnother( base );
    if (o == (int)KNewPanner::Horizontal ){
      if ( s->getFirst() == base ){
        DB = DB == 0L ? another : DB;
      } else {
        DT = DT == 0L ? another : DT;
      }
    } else { //Vertical
      if ( s->getFirst() == base ){
        DR = DR == 0L ? another : DR;
      } else {
        DL = DL == 0L ? another : DL;
      }
    }
    base = (DockWidget*)s->parent();
  }
  if ( toolbar->isButtonOn(1) ) toolbar->setItemEnabled( 1, DT != 0L );
  if ( toolbar->isButtonOn(2) ) toolbar->setItemEnabled( 2, DL != 0L );
  if ( toolbar->isButtonOn(3) ) toolbar->setItemEnabled( 3, DR != 0L );
  if ( toolbar->isButtonOn(4) ) toolbar->setItemEnabled( 4, DB != 0L );

  /**********************/
  if ( DB != 0L && toolbar->isButtonOn(4) ){
    DockB.dock = DB;
    DockB.dropDock = (DockWidget*)((DockSplitter*)DB->parent())->getAnother( DB );
  }
  if ( DB == 0L && toolbar->isButtonOn(4) ) DockB.dock = 0L;
  /**********************/
  if ( DR != 0L && toolbar->isButtonOn(3) ){
    DockR.dock = DR;
    DockR.dropDock = (DockWidget*)((DockSplitter*)DR->parent())->getAnother( DR );
  }
  if ( DR == 0L && toolbar->isButtonOn(3) ) DockR.dock = 0L;
  /**********************/
  if ( DL != 0L && toolbar->isButtonOn(2) ){
    DockL.dock = DL;
    DockL.dropDock = (DockWidget*)((DockSplitter*)DL->parent())->getAnother( DL );
  }
  if ( DL == 0L && toolbar->isButtonOn(2) ) DockL.dock = 0L;
  /**********************/
  if ( DT != 0L && toolbar->isButtonOn(1) ){
    DockT.dock = DT;
    DockT.dropDock = (DockWidget*)((DockSplitter*)DT->parent())->getAnother( DT );
  }
  if ( DT == 0L && toolbar->isButtonOn(1) ) DockT.dock = 0L;
  /**********************/
}

void DockMainWindow::insertMainWindowDockToolBar()
{
  addToolBar( toolbar );
}

void DockMainWindow::slotToggled( int id )
{
  switch ( id ){
    case 1:
      toolBarManager( toolbar->isButtonOn(1), DockT );
      break;
    case 2:
      toolBarManager( toolbar->isButtonOn(2), DockL );
      break;
    case 3:
      toolBarManager( toolbar->isButtonOn(3), DockR );
      break;
    case 4:
      toolBarManager( toolbar->isButtonOn(4), DockB );
      break;
    default:
      break;
  }
}

void DockMainWindow::toolBarManager( bool toggled, dockPosData &data )
{
  if ( data.dock == 0L || data.dropDock == 0L ) return;

  if ( toggled ){
    data.dock->manualDock( data.dropDock, data.pos, QPoint(0,0),  data.sepPos );
  } else {
    data.sepPos = ((DockSplitter*)data.dock->parent())->separatorPos();
    data.dock->unDock();
  }
}

void DockMainWindow::slotReplaceDock( DockWidget* oldDock, DockWidget* newDock )
{
  DockB.dock = DockB.dock == oldDock ? newDock:DockB.dock;
  DockT.dock = DockT.dock == oldDock ? newDock:DockT.dock;
  DockR.dock = DockR.dock == oldDock ? newDock:DockR.dock;
  DockL.dock = DockL.dock == oldDock ? newDock:DockL.dock;

  DockB.dropDock = DockB.dropDock == oldDock ? newDock:DockB.dropDock;
  DockT.dropDock = DockT.dropDock == oldDock ? newDock:DockT.dropDock;
  DockR.dropDock = DockR.dropDock == oldDock ? newDock:DockR.dropDock;
  DockL.dropDock = DockL.dropDock == oldDock ? newDock:DockL.dropDock;
}

void DockMainWindow::makeDockVisible( DockWidget* dock )
{
  if ( dock == 0L ) return;

  toolbar->blockSignals( true );

  QWidget* testWidget = dock;
  bool found = false;

  while ( testWidget != 0L ){
    if ( testWidget->isA("DockWidget") ){
      DockWidget* test = (DockWidget*)testWidget;
      if ( !toolbar->isButtonOn(1) && DockT.dock == test ){
        toolbar->toggleButton(1);
        toolBarManager( true, DockT );
        found = true;
      }
      if ( !toolbar->isButtonOn(2) && DockL.dock == test ){
        toolbar->toggleButton(2);
        toolBarManager( true, DockL );
        found = true;
      }
      if ( !toolbar->isButtonOn(3) && DockR.dock == test ){
        toolbar->toggleButton(3);
        toolBarManager( true, DockR );
        found = true;
      }
      if ( !toolbar->isButtonOn(4) && DockB.dock == test ){
        toolbar->toggleButton(4);
        toolBarManager( true, DockB );
        found = true;
      }
    }
    testWidget = testWidget->parentWidget();
  }

  if ( found && dock->parent() != 0L && dock->parent()->isA("STabCtl") )
      ((STabCtl*)dock->parent())->setVisiblePage( dock );
  else
    dockManager->makeDockVisible( dock );

  toolbar->blockSignals( false );
}

void DockMainWindow::makeWidgetDockVisible( QWidget* widget )
{
  makeDockVisible( dockManager->findWidgetParentDock(widget) );
}

void DockMainWindow::writeDockConfig()
{
  toolbar->blockSignals( true );

  if ( !toolbar->isButtonOn(1) ){
    toolbar->toggleButton(1);
    toolBarManager( true, DockT );
  }
  if ( !toolbar->isButtonOn(2) ){
    toolbar->toggleButton(2);
    toolBarManager( true, DockL );
  }
  if ( !toolbar->isButtonOn(3) ){
    toolbar->toggleButton(3);
    toolBarManager( true, DockR );
  }
  if ( !toolbar->isButtonOn(4) ){
    toolbar->toggleButton(4);
    toolBarManager( true, DockB );
  }
  dockManager->writeConfig( configName );

  toolbar->blockSignals( false );
}

void DockMainWindow::readDockConfig()
{
  toolbar->blockSignals( true );

  if ( !toolbar->isButtonOn(1) ){
    toolbar->toggleButton(1);
    toolBarManager( true, DockT );
  }
  if ( !toolbar->isButtonOn(2) ){
    toolbar->toggleButton(2);
    toolBarManager( true, DockL );
  }
  if ( !toolbar->isButtonOn(3) ){
    toolbar->toggleButton(3);
    toolBarManager( true, DockR );
  }
  if ( !toolbar->isButtonOn(4) ){
    toolbar->toggleButton(4);
    toolBarManager( true, DockB );
  }
  dockManager->readConfig( configName );

  toolbar->blockSignals( false );
}

void DockMainWindow::setDockView( QWidget* widget )
{
  setView( widget );
  updateRects();
}
