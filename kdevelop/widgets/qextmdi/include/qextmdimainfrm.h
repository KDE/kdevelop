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

#ifndef NO_KDE2
#include <kmainwindow.h>
#include <kmenubar.h>
#include <kpopupmenu.h>
#include "kparts/dockmainwindow.h"
#else
#include <qmenubar.h>
#include <qpopupmenu.h>
#include "dummykpartsdockmainwindow.h"
#endif

#include <qlist.h>
#include <qrect.h>
#include <qapplication.h>
#include <qdom.h>

#include "qextmditaskbar.h"
#include "qextmdichildarea.h"
#include "qextmdichildview.h"

#include "qextmdiiterator.h"
#include "qextmdilistiterator.h"
#include "qextmdinulliterator.h"

namespace QextMdi {

/**
* During @ref QextMdiMainFrm::addWindow the enum AddWindowFlags is used to determine how the view is initialy being added to the MDI system
*/
typedef enum {
      /**
      * standard is: show normal, attached, visible, document view (not toolview)
      */
      StandardAdd = 0,
      /**
      * add a maximized view
      */
      Maximize    = 1,
      /**
      * add a minimized view
      */
      Minimize    = 2,
      /**
      * add a view that is not visible for the moment but under MDI control
      */
      Hide        = 4,
      /**
      * add a view that appears toplevel
      */
      Detach      = 8,
      /**
      * add the view as tool view. That means it is stay-on-top and toplevel
      */
      ToolWindow  = 16
   } AddWindowFlags;

typedef enum {
      Win95Look = 0,
      KDE1Look  = 1,
      KDE2Look  = 2,
      KDE2LaptopLook = 3
   } QextMdiFrameDecor;

typedef enum {
      ToplevelMode   = 0,
      ChildframeMode = 1,
      TabPageMode    = 2
   } MdiMode;

}; //namespace

/**
 * @short Internal class
 *
 * This special event is needed because the view has to inform the main frame that it`s being closed.
 */
class QextMdiViewCloseEvent : public QCustomEvent
{
public:
   QextMdiViewCloseEvent( QextMdiChildView* pWnd) : QCustomEvent(QEvent::User,pWnd) {};
};

/**
  * @short Base class for all your special main frames.
  *
  * It contains the child frame area and a child view taskbar.
  * Defines some virtual functions for later common use.<br>
  * Provides functionality for docking/undocking view windows and manages the taskbar.<br>
  * Usually you just need to know about this class and the child view class.
  */
class DLL_IMP_EXP_QEXTMDICLASS QextMdiMainFrm : public KParts::DockMainWindow
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
   QPopupMenu              *m_pMdiModeMenu;
   QPopupMenu              *m_pPlacingMenu;
#ifdef NO_KDE2
   QMenuBar                *m_pMainMenuBar;
#else
   KMenuBar                *m_pMainMenuBar;
#endif

   QPixmap                 *m_pUndockButtonPixmap;
   QPixmap                 *m_pMinButtonPixmap;
   QPixmap                 *m_pRestoreButtonPixmap;
   QPixmap                 *m_pCloseButtonPixmap;

   QToolButton             *m_pUndock;
   QToolButton             *m_pMinimize;
   QToolButton             *m_pRestore;
   QToolButton             *m_pClose;
   QPoint                  m_undockPositioningOffset;
   QextMdi::MdiMode        m_mdiMode;
   bool                    m_bMaximizedChildFrmMode;
   int                     m_oldMainFrmHeight;
   int                     m_oldMainFrmMinHeight;
   int                     m_oldMainFrmMaxHeight;
   static QextMdi::QextMdiFrameDecor   m_frameDecoration;
   bool                    m_bSDIApplication;
   KDockWidget*            m_pDockbaseAreaOfDocumentViews;
   KDockWidget*            m_pDockbaseOfTabPage;
   QDomDocument*           m_pTempDockSession;

// methods
public:
   /**
   * Constructor.
   */
   QextMdiMainFrm( QWidget* parentWidget, const char* name = "", WFlags flags = WType_TopLevel | WDestructiveClose);
   /**
   * Destructor.
   */
   virtual ~QextMdiMainFrm();
   /**
   * Returns whether the application's MDI views are in maximized state or not.
   */
   bool isInMaximizedChildFrmMode() { return m_bMaximizedChildFrmMode; };
   /**
   * Returns the MDI mode. This can be one of the enumerations QextMdi::MdiMode.
   */
   bool mdiMode() { return m_mdiMode; };
   /**
   * Returns the focused attached MDI view.
   */
   QextMdiChildView * activeWindow();
   /**
   * Returns a popup menu filled according to the MDI view state. You can override this
   * method to insert additional entries there. The popup menu is usually popuped when the user
   * clicks with the right mouse button on a taskbar entry. The default entries are:
   * Undock/Dock, Restore/Maximize/Minimize, Close and an empty sub-popup ( @ref QextMdiMainFrm::windowPopup )
   * menu called Operations.
   */
   virtual QPopupMenu * taskBarPopup(QextMdiChildView *pWnd,bool bIncludeWindowPopup = FALSE);
   /**
   * Returns a popup menu with only a title "Window". You can fill it with own operations entries
   * on the MDI view. This popup menu is inserted as last menu item in @ref QextMdiMainFrm::taskBarPopup .
   */
   virtual QPopupMenu * windowPopup(QextMdiChildView *pWnd,bool bIncludeTaskbarPopup = TRUE);
   /**
   * Called in the constructor (forces a resize of all MDI views)
   */
   virtual void applyOptions();
   /**
   * Returns the QextMdiChildView belonging to the given caption string.
   */
   QextMdiChildView * findWindow(const QString& caption);
   /**
   * Returns whether this MDI child view is under MDI control (using @ref QextMdiMainFrm::addWindow ) or not.
   */
   bool windowExists(QextMdiChildView *pWnd);
   /**
   * Catches certain Qt events and processes it here.
   * Currently, here this catches only the @ref QextMdiViewCloseEvent (a QextMDI user event) which is sent
   * from itself in @ref QextMdiMainFrm::childWindowCloseRequest right after a @ref QextMdiChildView::closeEvent .
   * The reason for this event to itself is simple: It just wants to break the function call stack.
   * It continues the processing with calling @ref QextMdiMainFrm::closeWindow .
   * You see, a @ref QextMdiChildView::close is translated to a @ref QextMdiMainFrm::closeWindow .
   * It is necessary that the main frame has to start an MDI view close action because it must
   * remove the MDI view from MDI control, additionally.
   *
   * This method calls QMainWindow::event , additionally.
   */
   virtual bool event(QEvent* e);
   /**
   * If there's a main menubar given, it will create the 4 maximize mode buttons there (undock, minimize, restore, close).
   */
   virtual void setSysButtonsAtMenuPosition();
   /**
   * Returns the height of the taskbar.
   */
   virtual int taskBarHeight() { return m_pTaskBar ? m_pTaskBar->height() : 0; };
   /**
   * Sets an offset value that is used on @ref QextMdiMainFrm::detachWindow . The undocked window
   * is visually moved on the desktop by this offset.
   */
   virtual void setUndockPositioningOffset( QPoint offset) { m_undockPositioningOffset = offset; };
   /**
   * If you don't want to know about the inner structure of the QextMDI system, you can use
   * this iterator to handle with the MDI view list in a more abstract way.
   * The iterator hides what special data structure is used in QextMDI.
   */
   QextMdiIterator<QextMdiChildView*>* createIterator() {
      if ( m_pWinList == 0L) {
         return new QextMdiNullIterator<QextMdiChildView*>();
      } else {
         return new QextMdiListIterator<QextMdiChildView>( *m_pWinList);
      }
   }
   /**
   * Returns a popup menu that contains the MDI controlled view list.
   * Additionally, this menu provides some placing actions for these views.
   * Usually, you insert this popup menu in your main menubar as "Window" menu.
   */
   QPopupMenu* windowMenu() { return m_pWindowMenu; };
   /**
   * Sets a background colour for the MDI view area widget.
   */
   virtual void setBackgroundColor( const QColor &c) { m_pMdi->setBackgroundColor( c); };
   /**
   * Sets a background pixmap for the MDI view area widget.
   */
   virtual void setBackgroundPixmap( const QPixmap &pm) { m_pMdi->setBackgroundPixmap( pm); };
   /**
   * Sets a size that is used as the default size for a newly to the MDI system added @ref QextMdiChildView .
   *  By default this size is 600x400. So all non-resized added MDI views appear in that size.
   */
   void setDefaultChildFrmSize( const QSize& sz)
      { m_pMdi->m_defaultChildFrmSize = sz; };
   /**
   * Returns the default size for a newly added QextMdiChildView. See @ref QextMdiMainFrm::setDefaultChildFrmSize .
   */
   QSize defaultChildFrmSize()
      { return m_pMdi->m_defaultChildFrmSize; };
   /**
   * Do nothing when in Toplevel mode
   */
   virtual void setMinimumSize( int minw, int minh);
   /**
   * Returns the Childframe mode height of this. Makes only sense when in Toplevel mode.
   */
   int childFrameModeHeight() { return m_oldMainFrmHeight; };
   /**
   * Tells the MDI system a QMenu where it can insert buttons for
   * the system menu, undock, minimize, restore actions.
   * If no such menu is given, QextMDI simply overlays the buttons
   * at the upper right-hand side of the main widget.
   */
#ifndef NO_KDE2
   virtual void setMenuForSDIModeSysButtons( KMenuBar* = 0);
#else
   virtual void setMenuForSDIModeSysButtons( QMenuBar* = 0);
#endif
   /**
   * @return the decoration of the window frame of docked (attached) MDI views
   */
   static int frameDecorOfAttachedViews() { return m_frameDecoration; };
   /**
   *
   */
   void fakeSDIApplication();

public slots:
   /**
    * One of the most important methods at all!
    * Adds a QextMdiChildView to the MDI system. The main frame takes it under control.
    * You can specify here whether:
    * <UL><LI>the view should be attached or detached.</LI>
    * <LI>shown or hidden</LI>
    * <LI>maximized, minimized or restored (normalized)</LI>
    * <LI>added as tool view (stay-on-top and toplevel) or added as document-type view.
    */
   virtual void addWindow( QextMdiChildView* pView, int flags = QextMdi::StandardAdd);
   /** 
   * See @ref the method above for more details. Additionally, it moves to point pos.
   */
   virtual void addWindow( QextMdiChildView* pView, QPoint pos, int flags = QextMdi::StandardAdd);
   /** 
   * See @ref the method above for more details. Additionally, it sets the geometry.
   */
   virtual void addWindow( QextMdiChildView* pView, QRect rectNormal, int flags = QextMdi::StandardAdd);
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
   /** 
   * Someone wants that the MDI view to be closed. This method sends a QextMdiViewCloseEvent to itself
   * to break the function call stack. See also @ref QextMdiMainFrm::event .
   */
   virtual void childWindowCloseRequest(QextMdiChildView *pWnd);
   /** 
   * Close all views 
   */
   virtual void closeAllViews();
   /** 
   * Iconfiy all views 
   */
   virtual void iconifyAllViews();
   /** 
   * Closes the view of the active (topchild) window 
   */
   virtual void closeActiveView();
   /** 
   * Undocks all view windows (unix-like) 
   */
   virtual void switchToToplevelMode();
   virtual void finishToplevelMode();
   /**
   * Docks all view windows (Windows-like)
   */
   virtual void switchToChildframeMode();
   virtual void finishChildframeMode();
   /**
   * Docks all view windows (Windows-like)
   */
   virtual void switchToTabPageMode();
   virtual void finishTabPageMode();
   /**
   * Shows the view taskbar. This should be connected with your "View" menu. 
   */
   virtual void showViewTaskBar();
   /** 
   * Hides the view taskbar. This should be connected with your "View" menu. 
   */
   virtual void hideViewTaskBar();
   /** 
   * Update of the window menu contents. 
   */
   virtual void fillWindowMenu();

   /** 
   * Cascades the windows without resizing them. 
   */
   virtual void cascadeWindows() { m_pMdi->cascadeWindows(); }
   /** 
   * Cascades the windows resizing them to the maximum available size. 
   */
   virtual void cascadeMaximized() { m_pMdi->cascadeMaximized(); };
   /** 
   * Maximizes only in vertical direction.
   */
   virtual void expandVertical() { m_pMdi->expandVertical(); };
   /** 
   * Maximizes only in horizontal direction.
   */
   virtual void expandHorizontal() { m_pMdi->expandHorizontal(); };
   /** 
   * Tile Pragma 
   */
   virtual void tilePragma() { m_pMdi->tilePragma(); };
   /** 
   * Tile Anodine 
   */
   virtual void tileAnodine() { m_pMdi->tileAnodine(); };
   /** 
   * Tile Vertically 
   */
   virtual void tileVertically() { m_pMdi->tileVertically(); };
   /**
   * Sets the decoration of the window frame of docked (attached) MDI views
   */
   virtual void setFrameDecorOfAttachedViews( int frameDecor);
protected:
//   /**
//   * Calls the destructor by delete.
//   */
//   virtual void closeEvent(QCloseEvent *e);
   /** 
   * Redirect the focus to the MDI area widget ( @ref QextMdiChildArea ).
   */
   virtual void focusInEvent(QFocusEvent *);
   /**
   *
   */
   virtual void resizeEvent(QResizeEvent * );
   /**
   * Creates a new MDI taskbar (showing the MDI views as taskbar entries) and shows it.
   */
   virtual void createTaskBar();
   /**
   * Creates the MDI view area and connects some signals and slots with the QextMdiMainFrm widget.
   */
   virtual void createMdiManager();
   /** 
   * Usually called from @ref QextMdiMainFrm::addWindow when adding a tool view window. It reparents the given widget
   * as toplevel and stay-on-top on the application's main widget.
   */
   virtual void addToolWindow( QWidget* pWnd, KDockWidget::DockPosition pos = KDockWidget::DockNone, QWidget* pTargetWnd = 0L, int percent = 50, const QString& tabToolTip = 0, const QString& tabCaption = 0);

protected slots: // Protected slots
   /** 
   * Sets the focus to this MDI view, raises it, activates its taskbar button and updates
   * the system buttons in the main menubar when in maximized (Maximize mode).
   */
   virtual void activateView(QextMdiChildView *pWnd);
   /** 
   * Activates the MDI view (see @ref QextMdiMainFrm::activateView ) and popups the taskBar popup menu (see @ref QextMdiMainFrm::taskBarPopup ).
   */
   virtual void taskbarButtonRightClicked(QextMdiChildView *pWnd);
   /** 
   * Turns the system buttons for maximize mode (SDI mode) on, and connects them with the current child frame 
   */
   void setMaximizeModeOn();
   /** 
   * Turns the system buttons for maximize mode (SDI mode) off, and disconnects them 
   */
   void setMaximizeModeOff( QextMdiChildFrm* oldChild);
   /** 
   * Reconnects the system buttons form maximize mode (SDI mode) with the new child frame 
   */
   void updateSysButtonConnections( QextMdiChildFrm* oldChild, QextMdiChildFrm* newChild);
   /** 
   * Usually called when the user clicks an MDI view item in the "Window" menu.
   */
   void windowMenuItemActivated(int id);
   /** 
   * Usually called when the user clicks an MDI view item in the sub-popup menu "Docking" of the "Window" menu.
   */
   void dockMenuItemActivated(int id);
   /** 
   * Popups the "Window" menu. See also @ref QextMdiMainFrm::windowPopup .
   */
   void popupWindowMenu(QPoint p);

signals:
   /** 
   * Signals the last attached @ref QextMdiChildView has been closed 
   */
   void lastChildFrmClosed();
   /** 
   * Signals the last QextMdiChildView (that is under MDI control) has been closed 
   */
   void lastChildViewClosed();
   /**
   * Signals that the Toplevel mode has been left
   */
   void leftTopLevelMode();
};

#endif //_QEXTMDIMAINFRM_H_
