/***************************************************************************
                         DockWidget part of KDEStudio
                             -------------------
    copyright            : (C) 1999 by Judin Max
    email                : novaprint@mtu-net.ru
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
  QPopupMenu* dockMenu(){ return dockManager->dockMenu(); }

  void makeDockVisible( DockWidget* dock );
  void makeWidgetDockVisible( QWidget* widget );

  void insertMainWindowDockToolBar();
  void setDockView( QWidget* );
  
private slots:
  void slotDockChange();
  void slotToggled( int );
  void slotReplaceDock( DockWidget* oldDock, DockWidget* newDock );

private:
  void toolBarManager( bool toggled, dockPosData &data );

  DockWidget* mainDockWidget;
  DockManager* dockManager;
  QString configName;

  dockPosData DockL;
  dockPosData DockR;
  dockPosData DockT;
  dockPosData DockB;

  KToolBar* toolbar;
  DockWidget* viewDock;
};

#endif


