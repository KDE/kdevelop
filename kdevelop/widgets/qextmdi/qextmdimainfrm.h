//----------------------------------------------------------------------------
//    $Id$
//----------------------------------------------------------------------------
//    Project              : Qt MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create an
//                                         stand-alone Qt extension set of
//                                         classes and a Qt-based library
//
//    copyright            : (C) 1999-2000 by Falk Brettschneider
//                                         and
//                                         Szymon Stefanek (stefanek@tin.it)
//    email                :  gigafalk@geocities.com (Falk Brettschneider)
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
public:
	QextMdiChildArea        *m_pMdi;

protected:	
	QextMdiTaskBar          *m_pTaskBar;
	QList<QextMdiChildView> *m_pWinList;
	QextMdiChildView        *m_pCurrentWindow;
   QPopupMenu              *m_pWindowPopup;
   QPopupMenu              *m_pTaskBarPopup;
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

// methods
public:
	QextMdiMainFrm( QWidget* parentWidget, const char* name = "", WFlags flags = WType_TopLevel);
	~QextMdiMainFrm();
	QextMdiChildView * activeWindow();
	virtual QPopupMenu * taskBarPopup(QextMdiChildView *pWnd,bool bIncludeWindowPopup = false);
	virtual QPopupMenu * windowPopup(QextMdiChildView *pWnd,bool bIncludeTaskbarPopup = true);
	virtual void applyOptions();
	QextMdiChildView * findWindow(const QString& caption);
	bool windowExists(QextMdiChildView *pWnd);
	virtual void switchWindows(bool bRight);
   virtual bool event(QEvent* e);
   virtual void setSysButtonsAtMenuPosition();

public slots:
   /**
    * One of the most important methods at all!
    * Adds a QextMdiChildView to the MDI system. The main frame takes it under control.
    * You can specify here whether the view should be attached or detached.
    */
	virtual void addWindow(QextMdiChildView *pWnd, bool bShow, bool bAttach, bool bMaximized, QRect* pNormalSizeRect = 0);
   /**
    * Removes a QextMdiChildView from the MDI system and from the main frame`s control.
    * Note: The view will not be deleted, but it's getting toplevel (reparent to 0)!
    */
   virtual void removeWindowFromMdi(QextMdiChildView *pWnd);
   /**
    * Removes a QextMdiChildView from the MDI system and from the main frame`s control.
    * Note: The view will be deleted!
    */
	virtual void closeWindow(QextMdiChildView *pWnd, bool layoutTaskBar = true);
   /**
    * Switches the QextMdiTaskBar on and off.
    */
	virtual void slot_toggleTaskBar();
   /**
    * Makes a main frame controlled undocked QextMdiChildView docked.
    * Doesn't work on QextMdiChildView which aren't added to the MDI system.
    * Use addWindow() for that.
    */
	virtual void attachWindow(QextMdiChildView *pWnd,bool bShow,bool overrideGeometry,QRect *r);
   /**
    * Makes a docked QextMdiChildView undocked.
    * The view window still remains under the main frame's MDI control.
    */
	virtual void detachWindow(QextMdiChildView *pWnd);
	virtual void childWindowCloseRequest(QextMdiChildView *pWnd);
	/** ... */
///	virtual void childWindowGainFocus(QextMdiChildView *pWnd);
   /** close all views */
   virtual void closeAllViews();
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
  void showViewTaskBar();
  /** Hides the view taskbar. This should be connected with your "View" menu. */
  void hideViewTaskBar();

protected:
	virtual void closeEvent(QCloseEvent *e);
	virtual void focusInEvent(QFocusEvent *);
	virtual void createTaskBar();
	virtual void createMdiManager();
   virtual void raiseTopLevelWidget(QWidget * ptr);
   /**
    * Returns a popup menu that contains the MDI controlled view list.
    * Additionally, this menu provides some placing actions for these views.
    */
   QPopupMenu* windowMenu() { return m_pMdi->m_pWindowMenu; };

protected slots: // Protected slots
	virtual void activateView(QextMdiChildView *pWnd);
	virtual void taskbarButtonRightClicked(QextMdiChildView *pWnd);
   /**
    * For internal use. Called when a taskbar button must be pushed.
    * Usually, if its view raises.
    */
	//virtual void pushNewTaskBarButton(QextMdiChildView* pWnd);
   /** turns the system buttons for maximize mode (SDI mode) on, and connects them with the current child frame */
   void setMaximizeModeOn();
   /** turns the system buttons for maximize mode (SDI mode) off, and disconnects them */
   void setMaximizeModeOff( QextMdiChildFrm* oldChild);
   /** reconnects the system buttons form maximize mode (SDI mode) with the new child frame */
   void updateSysButtonConnections( QextMdiChildFrm* oldChild, QextMdiChildFrm* newChild);
};

#endif //_QEXTMDIMAINFRM_H_
