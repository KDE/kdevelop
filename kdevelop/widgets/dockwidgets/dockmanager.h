/***************************************************************************
                         DockWidget part of KDEStudio
                             -------------------
    copyright            : (C) 1999 by Judin Max
    email                : novaprint@mtu-net.ru
 ***************************************************************************/

/*  === MEMOS ===
  DockTabCtl and DockSplitter always have parent DockWidget;



*/


#ifndef DOCKMANAGER_H
#define DOCKMANAGER_H

#include <qwidget.h>
#include <qobject.h>
#include <qpoint.h>
#include <qlist.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qstrlist.h>

#include "stabctl.h"

class QObjectList;
class DockSplitter;
class DockMoveManager;
class KTMainWindow;
class QPopupMenu;
class KConfig;

typedef QList<QWidget> WidgetList;

class DockTabCtl : public STabCtl
{ Q_OBJECT
public:
  DockTabCtl( QWidget *parent = 0, const char *name = 0 )
  :STabCtl( parent, name ){};
  ~DockTabCtl(){};
};

enum DockPosition
{
  DockNone   = 0,
  DockTop    = 0x0001,
  DockLeft   = 0x0002,
  DockRight  = 0x0004,
  DockBottom = 0x0008,
  DockCenter = 0x0010,
  DockDesktop= 0x0020
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
  DockWidget( DockManager* dockManager, const char* name, const QPixmap &pixmap, QWidget* parent = 0L );
  virtual ~DockWidget();

  /* if target is null  - dock move to desktop at position pos;
     check - only for internal uses;
     return result GroupDockWidget
  */
  DockWidget* manualDock( DockWidget* target, DockPosition dockPos, int spliPos = 50, QPoint pos = QPoint(0,0), bool check = false );

  void setEnableDocking( int pos );
  int enableDocking(){ return eDocking; }

  void setDockSite( int pos ){ sDocking = pos;}
  int dockSite(){ return sDocking; }

  void setWidget( QWidget* );

  virtual bool event( QEvent * );
  virtual void show();

  void makeDockVisible();
  bool mayBeHide();
  bool mayBeShow();

public slots:
  void changeHideShowState();

protected:
  void updateCaptionButton();
  virtual void paintCaption();

signals:
  //emit for dock when another DockWidget docking in this DockWidget
  void docking( DockWidget*, DockPosition );
  void setDockDefaultPos();

public slots:
  void unDock();

private slots:
  void slotCloseButtonClick();

private:
  void setDockTabName( DockTabCtl* );
  void applyToWidget( QWidget* s, const QPoint& p  = QPoint(0,0) );
  QRect crect();

  QWidget* widget;
  SDockButton* closeButton;
  SDockButton* stayButton;
  DockManager* manager;
  QPixmap* drawBuffer;
  QPixmap* pix;

  int eDocking;
  int sDocking;
  int dockCaptionHeight;

  // GROUP data
  const char* firstName;
  const char* lastName;
  int splitterOrientation;
  bool isGroup;
  bool isTabGroup;
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
	DockManager( QWidget* mainWindow, const char* name = 0L );
	virtual ~DockManager();

  void activate();

  void writeConfig( KConfig* c = 0L, QString group = QString::null );
  void readConfig ( KConfig* c = 0L, QString group = QString::null );

  virtual bool eventFilter( QObject *, QEvent * );

	void startDrag( DockWidget* );
	void dragMove( DockWidget*, QPoint pos );
	void drop();

  DockWidget* findWidgetParentDock( QWidget* );
  void makeWidgetDockVisible( QWidget* w ){ findWidgetParentDock(w)->makeDockVisible(); }

  QPopupMenu* dockMenu(){ return menu; }

  DockWidget* findDockWidgetAt( const QPoint& pos );
  void findChildDockWidget( QWidget*&, const QWidget* p, const QPoint& pos );
  void findChildDockWidget( const QWidget* p, WidgetList*& );
	DockWidget* getDockWidgetFromName( const char* dockName );

signals:
  void change();
  void replaceDock( DockWidget* oldDock, DockWidget* newDock );
  void setDockDefaultPos( DockWidget* );

private slots:
  void slotTabShowPopup( int, QPoint );
  void slotUndockTab();
  void slotHideTab();
  void slotMenuPopup();
  void slotMenuActivated( int );

private:
  QWidget* main;
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
  bool dropCancel;

  /* right mouse button click on the tabbar data: */
  DockWidget* curTabDockWidget;
  QPopupMenu* menu;
  QList<menuDockData> *menuData;
};

#endif
