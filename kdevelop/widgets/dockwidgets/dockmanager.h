/***************************************************************************
                dockmanager.h  -  Def. DockWidget,DockManager
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

#ifndef DOCKMANAGER_H
#define DOCKMANAGER_H

#include <qwidget.h>
#include <qobject.h>
#include <qpoint.h>
#include <qlist.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qstrlist.h>

class QObjectList;
class STabCtl;
class DockSplitter;
class DockMoveManager;
class KTMainWindow;
class QPopupMenu;

typedef QList<QWidget> WidgetList;

enum DockPosition
{
  DockNone   = 0,
  DockTop    = 0x0001,
  DockLeft   = 0x0002,
  DockRight  = 0x0004,
  DockBottom = 0x0008,
  DockCenter = 0x0010
};

class SDockButton : public QPushButton
{ Q_OBJECT
public:
  SDockButton( QWidget *parent=0, const char *name=0 );
  ~SDockButton();
protected:
  virtual void drawButton( QPainter * );
  virtual void enterEvent( QEvent * );
  virtual void leaveEvent( QEvent * );

private:
  bool moveMouse;
};

class DockWidget: public QWidget
{ Q_OBJECT

friend class DockManager;
friend class DockSplitter;
friend class DockMainWindow;

public:
  DockWidget( DockManager* dockManager, const char* name, const QPixmap &pixmap );
  virtual ~DockWidget();

  /* if target is null  - dock move to desktop at position pos; */
  void manualDock( DockWidget* target, DockPosition dockPos, QPoint pos = QPoint(0,0), int spliPos = 50 );

  void setEnableDocking( int pos ){ eDocking = pos;}
  int enableDocking(){ return eDocking; }

	void setDraggable( bool bDraggable) { m_bDraggable = bDraggable; }
	bool draggable() { return m_bDraggable; }

  void setDockSite( int pos ){ sDocking = pos;}
  int dockSite(){ return sDocking; }

	DockPosition currentDockPos() { return m_curDockPos; }

  void setKTMainWindow( KTMainWindow* );

  virtual bool event( QEvent * );
  virtual void show();
  
protected:
  virtual void paintCaption();

signals:
  //emit for dock when another DockWidget docking in this DockWidget
  void docking( DockWidget*, DockPosition );
	void iMBeingClosed();

public slots:
  void unDock();

private slots:
  void slotDockBackButtonClick();
	void slotOldBrotherIsLost();

public: // was also private (F.B.)
  void recreateToDesktop( QPoint );
  void recreateTo( QWidget* );

private:
  void setDockTabName( STabCtl* );
  QRect crect();

  DockManager* manager;
  QWidget* widget;
  SDockButton* dockbackButton;
//F.B.  SDockButton* stayButton;
//  DockManager* manager;
  QWidget* Parent;
  QPixmap* drawBuffer;
  QPixmap* pix;

  int eDocking;	// where can this be docked
  int sDocking; // where can other DockWidgets dock in this
  int dockCaptionHeight;
  int buttonY;

  // GROUP data
  const char* firstName;
  const char* lastName;
  int splitterOrientation;
  bool isGroup;
  bool isTabGroup;

  bool m_bDraggable;
	DockWidget* m_oldBrotherDockWidget;
  DockPosition m_curDockPos;
  DockPosition m_oldDockPos;
};

struct menuDockData
{
  menuDockData( QWidget* _dock, bool _hide )
  {
    dock = _dock;
    hide = _hide;
  };
  ~menuDockData(){};

  QWidget* dock;
  bool hide;
};

class DockManager: public QObject
{Q_OBJECT
friend class DockWidget;
friend class DockMainWindow;

public:
	DockManager( DockMainWindow* mainWindow, const char* name );
	virtual ~DockManager();

  void activate();

  void writeConfig( const char* );
  void readConfig( const char* );

  bool eventFilter( QObject *, QEvent * );

	void startDrag( DockWidget* );
	void dragMove( DockWidget*, QPoint pos );
	void drop();

  void makeDockVisible( DockWidget* );
  DockWidget* findWidgetParentDock( QWidget* );
  void makeWidgetDockVisible( QWidget* w ){ makeDockVisible(findWidgetParentDock(w)); }

  bool isDockVisible( DockWidget* );

  QPopupMenu* dockMenu(){ return menu; }

  DockWidget* findDockWidgetAt( const QPoint& pos );
  void findChildDockWidget( QWidget*&, const QWidget* p, const QPoint& pos );
  void findChildDockWidget( const QWidget* p, WidgetList*& );
	DockWidget* getDockWidgetFromName( const char* dockName );

signals:
  void change();
  void replaceDock( DockWidget* oldDock, DockWidget* newDock );

private slots:
  void slotTabShowPopup( int, QPoint );
  void slotUndockTab();
  void slotHideTab();
  void slotMenuPopup();
  void slotMenuActivated( int );
  void slotConfigMenuActivated( int );

private:
  DockMoveManager* mg;
  DockWidget* currentDragWidget;
  DockWidget* currentMoveWidget; // widget where mouse moving
  WidgetList* childDockWidgetList;
  DockPosition curPos;
  QObjectList* childDock;
  QObjectList* autoCreateDock;
  int storeW;
  int storeH;
  bool draging;
  bool undockProcess;

  /* right mouse button click on the tabbar data: */
  DockWidget* curTabDockWidget;
  QFont* titleFont;
  QPopupMenu* menu;
  QPopupMenu* configMenu;
  QList<menuDockData> *menuData;
  QStrList* configMenuData;
};

#endif
