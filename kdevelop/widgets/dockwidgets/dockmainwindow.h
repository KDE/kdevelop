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

  DockManager* manager(){ return dockManager; }

  void setView( QWidget* );
  DockWidget* getMainViewDockWidget(){ return viewDock; }

  void setMainDockWidget( DockWidget* );
  DockWidget* getMainDockWidget(){ return mainDockWidget; }

  DockWidget* createDockWidget( const char* name, const QPixmap &pixmap, QWidget* parent = 0L );

  void writeDockConfig( KConfig* c = 0L, QString group = QString::null );
  void readDockConfig ( KConfig* c = 0L, QString group = QString::null );

  void activateDock(){ dockManager->activate(); }

  QPopupMenu* dockMenu(){ return dockManager->dockMenu(); }

  void makeDockVisible( DockWidget* dock );
  void makeWidgetDockVisible( QWidget* widget );

  void setDockView( QWidget* );

protected slots:
  void slotDockChange();
  void slotToggled( int );
  void slotReplaceDock( DockWidget* oldDock, DockWidget* newDock );

protected:
  void toolBarManager( bool toggled, dockPosData &data );

  DockWidget* mainDockWidget;
  DockManager* dockManager;

  dockPosData DockL;
  dockPosData DockR;
  dockPosData DockT;
  dockPosData DockB;

  KToolBar* toolbar;
  DockWidget* viewDock;
};

#endif


