//----------------------------------------------------------------------------
//    filename             : qextmdimainfrm.h
//----------------------------------------------------------------------------
//    Project              : Qt MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create an
//                           - 06/2000     stand-alone Qt extension set of
//                                         classes and a Qt-based library
//                         : 02/2000       by Massimo Morin (mmorin@schedsys.com)
//    patches              : */2000        by Lars Beikirch (Lars.Beikirch@gmx.net)
//
//    copyright            : (C) 1999-2000 by Falk Brettschneider
//                                         and
//                                         Szymon Stefanek (stefanek@tin.it)
//    email                :  gigafalk@yahoo.com (Falk Brettschneider)
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Library General Public License as
//    published by the Free Software Foundation; either version 2 of the
//    License, or (at your option) any later version.
//
//----------------------------------------------------------------------------

#ifndef _QEXTMDIMAINFRM_H_
#define _QEXTMDIMAINFRM_H_

#include <qmainwindow.h>
#include <qlist.h>
#include <qrect.h>
#include <qpopupmenu.h>
#include <qapplication.h>
#include <qmenubar.h>

#include "qextmditaskbar.h"
#include "qextmdichildarea.h"
#include "qextmdichildview.h"

#include "qextmdiiterator.h"
#include "qextmdilistiterator.h"
#include "qextmdinulliterator.h"

namespace QextMdi {

typedef enum {
         StandardAdd = 0,
         Maximize    = 1,
         Minimize    = 2,
         Hide        = 4,
         Detach      = 8
      } AddWindowFlags;

}; //namespace

/**
 * @short Internal class
 * This special event is needed because the view has to inform the main frame that it`s being closed.
 */
class QextMdiViewCloseEvent : public QCustomEvent
{
public:
   QextMdiViewCloseEvent( QextMdiChildView* pWnd) : QCustomEvent(QEvent::User,pWnd) {};
};

/**
  * @short Base class for all your special main frames. It contains the child frame area and a child view taskbar.
  * Defines some virtual functions for later common use.<br>
  * Provides functionality for docking/undocking view windows and manages the taskbar.<br>
  * Usually you just need to know about this class and the child view class.
  */
class DLL_IMP_EXP_QEXTMDICLASS QextMdiMainFrm : public QMainWindow
{
   friend class QextMdiChildView;
   friend class QextMdiTaskBar;
   
   Q_OBJECT

// attributes
protected:  
   QextMdiChildArea        *m_pMdi;
   QextMdiTaskBar          *m_pTaskBar;
   QList<QextMdiChildView> *m_pWinList;
   QextMdiChildView        *m_pCurrentWindow;
   QPopupMenu              *m_pWindowPopup;
   QPopupMenu              *m_pTaskBarPopup;
   QPopupMenu              *m_pWindowMenu;
   QPopupMenu              *m_pDockMenu;
   QPopupMenu              *m_pPlacingMenu;
   QMenuBar                *m_pMainMenuBar;

   QPixmap                 *m_pUndockButtonPixmap;
   QPixmap                 *m_pMinButtonPixmap;
   QPixmap                 *m_pRestoreButtonPixmap;
   QPixmap                 *m_pCloseButtonPixmap;

#ifdef _OS_WIN32_
   QPushButton             *m_pUndock;
   QPushButton             *m_pMinimize;
   QPushButton             *m_pRestore;
   QPushButton             *m_pClose;
#else
   QToolButton             *m_pUndock;
   QToolButton             *m_pMinimize;
   QToolButton             *m_pRestore;
   QToolButton             *m_pClose;
#endif
   QPoint                      m_undockPositioningOffset;

// methods
public:
   QextMdiMainFrm( QWidget* parentWidget, const char* name = "", WFlags flags = WType_TopLevel);
   ~QextMdiMainFrm();
   QextMdiChildView * activeWindow();
   virtual QPopupMenu * taskBarPopup(QextMdiChildView *pWnd,bool bIncludeWindowPopup = FALSE);
   virtual QPopupMenu * windowPopup(QextMdiChildView *pWnd,bool bIncludeTaskbarPopup = TRUE);
   virtual void applyOptions();
   QextMdiChildView * findWindow(const QString& caption);
   bool windowExists(QextMdiChildView *pWnd);
   virtual void switchWindows(bool bRight);
   virtual bool event(QEvent* e);
   virtual void setSysButtonsAtMenuPosition();
   virtual int taskBarHeight() { return m_pTaskBar->height(); };
   virtual void setUndockPositioningOffset( QPoint offset) { m_undockPositioningOffset = offset; };

  QextMdiIterator<QextMdiChildView*>* createIterator() {
    if ( m_pWinList == 0L) {
      return new QextMdiNullIterator<QextMdiChildView*>();
    } else {
      return new QextMdiListIterator<QextMdiChildView, QextMdiChildView*>( *m_pWinList);
    }
  }

   /**
    * Returns a popup menu that contains the MDI controlled view list.
    * Additionally, this menu provides some placing actions for these views.
    */
   QPopupMenu* windowMenu() { return m_pWindowMenu; };
   virtual void setBackgroundColor( const QColor &c) { m_pMdi->setBackgroundColor( c); };
   virtual void setBackgroundPixmap( const QPixmap &pm) { m_pMdi->setBackgroundPixmap( pm); };
   void setDefaultChildFrmSize( const QSize& sz)
      { m_pMdi->m_defaultChildFrmSize = sz; };
   QSize defaultChildFrmSize()
      { return m_pMdi->m_defaultChildFrmSize; };

public slots:
   /**
    * One of the most important methods at all!
    * Adds a QextMdiChildView to the MDI system. The main frame takes it under control.
    * You can specify here whether the view should be attached or detached.
    */
// virtual void addWindow(QextMdiChildView *pWnd, bool bShow, bool bAttach = TRUE);
   /**
    * Adds a QextMdiChildView to the MDI system. The main frame takes it under control.
    * You can specify here whether the view should be attached or detached.
    */
   virtual void addWindow( QextMdiChildView* pView, QRect rectNormal, int flags = QextMdi::StandardAdd);
   virtual void addWindow( QextMdiChildView* pView, QPoint pos, int flags = QextMdi::StandardAdd);
   virtual void addWindow( QextMdiChildView* pView, int flags = QextMdi::StandardAdd);
   /**
    * Removes a QextMdiChildView from the MDI system and from the main frame`s control.
    * Note: The view will not be deleted, but it's getting toplevel (reparent to 0)!
    */
   virtual void removeWindowFromMdi(QextMdiChildView *pWnd);
   /**
    * Removes a QextMdiChildView from the MDI system and from the main frame`s control.
    * Note: The view will be deleted!
    */
   virtual void closeWindow(QextMdiChildView *pWnd, bool layoutTaskBar = TRUE);
   /**
    * Switches the QextMdiTaskBar on and off.
    */
   virtual void slot_toggleTaskBar();
   /**
    * Makes a main frame controlled undocked QextMdiChildView docked.
    * Doesn't work on QextMdiChildView which aren't added to the MDI system.
    * Use addWindow() for that.
    */
   virtual void attachWindow(QextMdiChildView *pWnd,bool bShow=TRUE);
   /**
    * Makes a docked QextMdiChildView undocked.
    * The view window still remains under the main frame's MDI control.
    */
   virtual void detachWindow(QextMdiChildView *pWnd,bool bShow=TRUE);
   virtual void childWindowCloseRequest(QextMdiChildView *pWnd);
   /** close all views */
   virtual void closeAllViews();
   /** iconfiy all views */
   virtual void iconifyAllViews();
   /** closes the view of the active (topchild) window */
   virtual void closeActiveView();
   /** undocks all view windows (unix-like) */
   virtual void switchToToplevelMode();
   /** docks all view windows (Windows-like) */
   virtual void switchToChildframeMode();
   /**
     * tells the MDI system a QMenu where it can insert buttons for
     * the system menu, undock, minimize, restore actions.
     * If no such menu is given, QextMDI simply overlays the buttons
     * at the upper right-hand side of the main widget.
     */
   virtual void setMenuForSDIModeSysButtons( QMenuBar* = 0);
   /** Shows the view taskbar. This should be connected with your "View" menu. */
   virtual void showViewTaskBar();
   /** Hides the view taskbar. This should be connected with your "View" menu. */
   virtual void hideViewTaskBar();
   /** Update of the window menu contents. */
   virtual void fillWindowMenu();

   /** Cascades the windows without resizing them. */
   virtual void cascadeWindows() { m_pMdi->cascadeWindows(); }
   /** Cascades the windows resizing them to the maximum available size. */
   virtual void cascadeMaximized() { m_pMdi->cascadeMaximized(); };
   virtual void expandVertical() { m_pMdi->expandVertical(); };
   virtual void expandHorizontal() { m_pMdi->expandHorizontal(); };
   /** Tile Pragma */
   virtual void tilePragma() { m_pMdi->tilePragma(); };
   /** Tile Anodine */
   virtual void tileAnodine() { m_pMdi->tileAnodine(); };
   /** Tile Vertically */
   virtual void tileVertically() { m_pMdi->tileVertically(); };

protected:
   virtual void closeEvent(QCloseEvent *e);
   virtual void focusInEvent(QFocusEvent *);
   virtual void createTaskBar();
   virtual void createMdiManager();

protected slots: // Protected slots
   virtual void activateView(QextMdiChildView *pWnd);
   virtual void taskbarButtonRightClicked(QextMdiChildView *pWnd);
   /** turns the system buttons for maximize mode (SDI mode) on, and connects them with the current child frame */
   void setMaximizeModeOn();
   /** turns the system buttons for maximize mode (SDI mode) off, and disconnects them */
   void setMaximizeModeOff( QextMdiChildFrm* oldChild);
   /** reconnects the system buttons form maximize mode (SDI mode) with the new child frame */
   void updateSysButtonConnections( QextMdiChildFrm* oldChild, QextMdiChildFrm* newChild);
   void windowMenuItemActivated(int id);
   void dockMenuItemActivated(int id);
   void popupWindowMenu(QPoint p);
};

#endif //_QEXTMDIMAINFRM_H_
