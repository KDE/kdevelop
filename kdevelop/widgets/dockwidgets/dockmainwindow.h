/***************************************************************************
                dockmainwindow.h  -  Def. DockMainWindow
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

#ifndef DOCKMAINWINDOW_H
#define DOCKMAINWINDOW_H

#include <ktmainwindow.h>
#include <qpixmap.h>

#include "dockmanager.h"

class QPopupMenu;
class KToolBar;

struct dockPosData
{
  DockWidget* dock;
  DockWidget* dropDock;
  DockPosition pos;
  int sepPos;
};

class DockMainWindow : public KTMainWindow
{Q_OBJECT
public:
  DockMainWindow( const char *name = 0L );
  ~DockMainWindow();

  void setView( QWidget* );
  DockWidget* getMainViewDockWidget(){ return viewDock; }

  void setDockCaptionFont( const QFont &f ){ dockManager->titleFont = new QFont(f);}
  void dockIn( DockWidget* w, DockPosition pos );
  DockWidget* createDockWidget( const char* name, const QPixmap &pixmap );

  void writeDockConfig();
  void readDockConfig();

  void activateDock(){ dockManager->activate(); }

  void setDockSite( int pos ){ mainDockWidget->sDocking = pos/* % DockCenter*/;}
  int dockSite(){ return mainDockWidget->sDocking; }

  DockPosition currentDockPos() { return mainDockWidget->m_curDockPos; }

  QPopupMenu* dockMenu(){ return dockManager->dockMenu(); }

  void makeDockVisible( DockWidget* dock );
  void makeWidgetDockVisible( QWidget* widget );

  void insertMainWindowDockToolBar();
  void setDockView( QWidget* );
  
private slots:
  void slotDockChange();
  void slotToggled( int );
  void slotReplaceDock( DockWidget* oldDock, DockWidget* newDock );

protected:
  DockWidget* mainDockWidget;
  DockWidget* viewDock;

private:
  void toolBarManager( bool toggled, dockPosData &data );

  DockManager* dockManager;
  QString configName;

  dockPosData DockL;
  dockPosData DockR;
  dockPosData DockT;
  dockPosData DockB;

  KToolBar* toolbar;
};

#endif


