//----------------------------------------------------------------------------
//    filename             : kmdimainfrm.h
//----------------------------------------------------------------------------
//    Project              : KDE MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create an
//                           - 06/2000     stand-alone Qt extension set of
//                                         classes and a Qt-based library
//                         : 02/2000       by Massimo Morin (mmorin@schedsys.com)
//                           2000-2003     maintained by the KDevelop project
//    patches              : */2000        by Lars Beikirch (Lars.Beikirch@gmx.net)
//                         : 01/2003       by Jens Zurheide (jens.zurheide@gmx.de)
//
//    copyright            : (C) 1999-2003 by Falk Brettschneider
//                                         and
//                                         Szymon Stefanek (stefanek@tin.it)
//    email                :  falkbr@kdevelop.org (Falk Brettschneider)
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

#ifndef _KMDIMAINFRM_H_
#define _KMDIMAINFRM_H_

#include <kparts/dockmainwindow.h>
#include <kmenubar.h>
#include <kpopupmenu.h>

#include <qptrlist.h>
#include <qrect.h>
#include <qapplication.h>
#include <qdom.h>
#include <qguardedptr.h>

#include "kmditaskbar.h"
#include "kmdichildarea.h"
#include "kmdichildview.h"
#include "kmdiiterator.h"
#include "kmdilistiterator.h"
#include "kmdinulliterator.h"

class QTimer;
class QPopupMenu;
class QMenuBar;
#include "kmditoolviewaccessor.h"

class QToolButton;

namespace KMDIPrivate {
  class KMDIGUIClient;
}

class KMdiDockContainer;
class KMdiMainFrmPrivate;

/**
 * @short Internal class
 *
 * This special event is needed because the view has to inform the main frame that it`s being closed.
 */
class KMdiViewCloseEvent : public QCustomEvent
{
public:
   KMdiViewCloseEvent( KMdiChildView* pWnd) : QCustomEvent(QEvent::User,pWnd) {}
};

/**
  * @short Base class for all your special main frames.
  *
  * It contains the child frame area (QMainWindow's central widget) and a child view taskbar
  * for switching the MDI views. Most methods are virtual functions for later overriding.
  *
  * Basically, this class provides functionality for docking/undocking view windows and
  * manages the taskbar. Usually the programmer just need to know about this class and the child view class.
  *
  * Your program mainwidget should inherit KMdiMainFrm. Then usually you'll just need
  * addWindow() and removeWindowFromMdi() to control the views.
  *   \code
  *   class MyMainWindow : public KMdiMainFrm
  *   { .... };
  *   ...
  *   MyMainWindow mainframe;
  *   qApp->setMainWidget(&mainframe);
  *   mainframe->addWindow(view1); // put it under MDI control
  *   mainframe->addWindow(view2);
  *   \endcode
  *
  * Most public and protected methods of this class are for program calls of the actions
  * the user could click.<br>
  *
  * Dynamic switching the MDI mode can be done via switchToChildframeMode(), switchToToplevelMode(),
  * switchToTabPageMode() or fakeSDIApplication() or asked via mdiMode(), isFakingSDIApplication().
  *
  * This class provides already the "Window" menu needed in common MDI applications. Just
  * insert it in your main menu:
  * \code
  * if (!isFakingSDIApplication()) {
  *    menuBar()->insertItem( tr("&Window"), windowMenu());
  * }
  * \endcode
  *
  * Synchronize the positions of the MDI control buttons inserted in your mainmenu:
  * \code
  * void B_MainModuleWidget::initMenuBar()
  * {
  *   setMenuForSDIModeSysButtons( menuBar());
  * ...
  * void B_MainModuleWidget::resizeEvent ( QResizeEvent *e)
  * {
  *   KMdiMainFrm::resizeEvent( e);
  *   setSysButtonsAtMenuPosition();
  * }
  * \endcode
  *
  * You can dynamically change the shape of the attached MDI views using setFrameDecorOfAttachedViews().
  *
  * Additionally, here's a hint how to restore the mainframe's settings from config file:
  * \code
  *
  *    // restore MDI mode (toplevel, childframe, tabpage)
  *    int mdiMode = config->readIntEntry( "mainmodule session", "MDI mode", KMdi::ChildframeMode);
  *    switch (mdiMode) {
  *    case KMdi::ToplevelMode:
  *       {
  *          int childFrmModeHt = config->readIntEntry( "mainmodule session", "Childframe mode height", desktop()->height() - 50);
  *          mainframe->resize( m_pMdiMainFrm->width(), childFrmModeHt);
  *          mainframe->switchToToplevelMode();
  *       }
  *       break;
  *    case KMdi::ChildframeMode:
  *       break;
  *    case KMdi::TabPageMode:
  *       {
  *          int childFrmModeHt = m_pCfgFileManager->readIntEntry( "mainmodule session", "Childframe mode height", desktop()->height() - 50);
  *          mainframe->resize( m_pMdiMainFrm->width(), childFrmModeHt);
  *          mainframe->switchToTabPageMode();
  *       }
  *       break;
  *    default:
  *       break;
  *    }
  *
  *    // restore a possible maximized Childframe mode
  *    bool maxChildFrmMode = config->readBoolEntry( "mainmodule session", "maximized childframes", true);
  *    mainframe->setEnableMaximizedChildFrmMode(maxChildFrmMode);
  * \endcode
  * The maximized-Childframe mode means that currently all views are maximized in Childframe mode's application desktop.
  *
  * This class provides placing algorithms in Childframe mode. Call tilePragma(), tileAnodine(), tileVertically(),
  * cascadeWindows(), cascadeMaximized(), expandVertical(), expandHorizontal() for those actions.
  *
  * activateView(KMdiChildView*) and activateView(int index) set the appropriate MDI child view as the active
  * one. It will be raised, gets an active MDI frame and gets the focus. Call activeView() to find out what the
  * current MDI view is.
  *
  * Use detachWindow() and attachWindow() for docking the MDI views to desktop and back.
  *
  * Connect accels of your program with activatePrevWin(), activateNextWin() and activateView(int index).
  *
  * Note: KMdiChildViews can be added in 2 meanings: Either as a normal child view (usually containing
  * user document views) or as a tool-view (usually containing status, info or control widgets).
  * The tool-views can be added as floating dockwidgets or as stay-on-top desktop windows in tool style.
  *
  * Here's an example how you can suggest things for the adding of views to the MDI control via flags:
  * \code
  * m_mapOfMdiWidgets.insert( pWnd, mh);
  * unsigned int mdiFlags = KMdi::StandardAdd;
  * if( !bShow)
  *    mdiFlags |= KMdi::Hide;
  * if( !bAttach)
  *    mdiFlags |= KMdi::Detach;
  * if( bMinimize)
  *    mdiFlags |= KMdi::Minimize;
  * if( bToolWindow)
  *    mdiFlags |= KMdi::ToolWindow;
  * if (m_pMdiMainFrm->isFakingSDIApplication()) {
  *    if (bAttach) { // fake an SDI app
  *       mdiFlags |= KMdi::Maximize;
  *    }
  *    else {
  *       m_pMdiMainFrm->addWindow( pWnd, QPoint(20, 20), KMdi::AddWindowFlags(mdiFlags));
  *       return;
  *    }
  * }
  * m_pMdiMainFrm->addWindow( pWnd, KMdi::AddWindowFlags(mdiFlags));
  * \endcode
  *
  * Further note: Pay attention to the fact that when you click on the close button of MDI views that their
  * close event should be redirected to closeWindow(). Otherwise the mainframe class will
  * not get noticed about the deleted view and a dangling pointer will remain in the MDI control. The
  * closeWindow() or the removeWindowFromMdi() method is for that issue. The difference is closeWindow()
  * deletes the view object. So if your application wants to control that by itself, call removeWindowFromMdi()
  * and call delete by yourself. See also KMdiChildView::closeEvent() for tat issue.
  */
class KMdiMainFrm : public KParts::DockMainWindow
{
   friend class KMdiChildView;
   friend class KMdiTaskBar;
   Q_OBJECT

   friend class KMdiToolViewAccessor;
// attributes
protected:
   KMdi::MdiMode           m_mdiMode;
   KMdiChildArea           *m_pMdi;
   KMdiTaskBar             *m_pTaskBar;
   QPtrList<KMdiChildView> *m_pDocumentViews;
   QMap<QWidget*,KMdiToolViewAccessor*> *m_pToolViews;
   KMdiChildView           *m_pCurrentWindow;
   QPopupMenu              *m_pWindowPopup;
   QPopupMenu              *m_pTaskBarPopup;
   QPopupMenu              *m_pWindowMenu;
   QPopupMenu              *m_pDockMenu;
   QPopupMenu              *m_pMdiModeMenu;
   QPopupMenu              *m_pPlacingMenu;
   KMenuBar                *m_pMainMenuBar;

   QPixmap                 *m_pUndockButtonPixmap;
   QPixmap                 *m_pMinButtonPixmap;
   QPixmap                 *m_pRestoreButtonPixmap;
   QPixmap                 *m_pCloseButtonPixmap;

   QToolButton             *m_pUndock;
   QToolButton             *m_pMinimize;
   QToolButton             *m_pRestore;
   QToolButton             *m_pClose;
   QPoint                  m_undockPositioningOffset;
   bool                    m_bMaximizedChildFrmMode;
   int                     m_oldMainFrmHeight;
   int                     m_oldMainFrmMinHeight;
   int                     m_oldMainFrmMaxHeight;
   static KMdi::FrameDecor m_frameDecoration;
   bool                    m_bSDIApplication;
   KDockWidget*         m_pDockbaseAreaOfDocumentViews;
   QDomDocument*           m_pTempDockSession;
   bool                    m_bClearingOfWindowMenuBlocked;

   QTimer*                 m_pDragEndTimer;

   bool                    m_bSwitching;

   KDockWidget*         m_leftContainer;
   KDockWidget*         m_rightContainer;
   KDockWidget*         m_topContainer;
   KDockWidget*         m_bottomContainer;


private:
   KMdiMainFrmPrivate*     d;
   KMDIPrivate::KMDIGUIClient*     m_mdiGUIClient;
   bool m_managedDockPositionMode;

// methods
public:
   /**
   * Constructor.
   */
   KMdiMainFrm( QWidget* parentWidget, const char* name = "", KMdi::MdiMode mdiMode=KMdi::ChildframeMode,WFlags flags = WType_TopLevel | WDestructiveClose);
   /**
   * Destructor.
   */
   virtual ~KMdiMainFrm();

   void setStandardMDIMenuEnabled(bool showModeMenu=true);

   void setManagedDockPositionModeEnabled(bool enabled);

   /**
   * Returns whether the application's MDI views are in maximized state or not.
   */
   bool isInMaximizedChildFrmMode() { return m_bMaximizedChildFrmMode; };
   /**
   * Returns the MDI mode. This can be one of the enumerations KMdi::MdiMode.
   */
   KMdi::MdiMode mdiMode() { return m_mdiMode; };
   /**
   * Returns the focused attached MDI view.
   */
   KMdiChildView * activeWindow();
   /**
   * Returns a popup menu filled according to the MDI view state. You can override this
   * method to insert additional entries there. The popup menu is usually popuped when the user
   * clicks with the right mouse button on a taskbar entry. The default entries are:
   * Undock/Dock, Restore/Maximize/Minimize, Close and an empty sub-popup ( windowPopup() )
   * menu called Operations.
   */
   virtual QPopupMenu * taskBarPopup(KMdiChildView *pWnd,bool bIncludeWindowPopup = false);
   /**
   * Returns a popup menu with only a title "Window". You can fill it with own operations entries
   * on the MDI view. This popup menu is inserted as last menu item in taskBarPopup() .
   */
   virtual QPopupMenu * windowPopup(KMdiChildView *pWnd,bool bIncludeTaskbarPopup = true);
   /**
   * Called in the constructor (forces a resize of all MDI views)
   */
   virtual void applyOptions();


   /**
   * Returns the KMdiChildView belonging to the given caption string.
   */
   KMdiChildView * findWindow(const QString& caption);
   /**
   * Returns whether this MDI child view is under MDI control (using addWindow() ) or not.
   */
   enum ExistsAs {DocumentView,ToolView,AnyView};
   bool windowExists(KMdiChildView *pWnd,ExistsAs as);
   /**
   * Catches certain Qt events and processes it here.
   * Currently, here this catches only the KMdiViewCloseEvent (a KMdi user event) which is sent
   * from itself in childWindowCloseRequest() right after a KMdiChildView::closeEvent() .
   * The reason for this event to itself is simple: It just wants to break the function call stack.
   * It continues the processing with calling closeWindow() .
   * You see, a close() is translated to a closeWindow() .
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
   * Sets an offset value that is used on detachWindow() . The undocked window
   * is visually moved on the desktop by this offset.
   */
   virtual void setUndockPositioningOffset( QPoint offset) { m_undockPositioningOffset = offset; };
   /**
   * If you don't want to know about the inner structure of the KMdi system, you can use
   * this iterator to handle with the MDI view list in a more abstract way.
   * The iterator hides what special data structure is used in KMdi.
   */
   // FIXME
   KMdiIterator<KMdiChildView*>* createIterator() {
      if ( m_pDocumentViews == 0L) {
         return new KMdiNullIterator<KMdiChildView*>();
      } else {
         return new KMdiListIterator<KMdiChildView>( *m_pDocumentViews);
      }
   }
   /**
   * Deletes an KMdiIterator created in the KMdi library (needed for the windows dll problem).
   */
   void deleteIterator(KMdiIterator<KMdiChildView*>* pIt) { delete pIt; }
   /**
   * Returns a popup menu that contains the MDI controlled view list.
   * Additionally, this menu provides some placing actions for these views.
   * Usually, you insert this popup menu in your main menubar as "Window" menu.
   */
   QPopupMenu* windowMenu() const { return m_pWindowMenu; };
   /**
   * Sets a background color for the MDI view area widget.
   */
   virtual void setBackgroundColor( const QColor &c) { m_pMdi->setBackgroundColor( c); };
   /**
   * Sets a background pixmap for the MDI view area widget.
   */
   virtual void setBackgroundPixmap( const QPixmap &pm) { m_pMdi->setBackgroundPixmap( pm); };
   /**
   * Sets a size that is used as the default size for a newly to the MDI system added KMdiChildView .
   *  By default this size is 600x400. So all non-resized added MDI views appear in that size.
   */
   void setDefaultChildFrmSize( const QSize& sz)
      { m_pMdi->m_defaultChildFrmSize = sz; };
   /**
   * Returns the default size for a newly added KMdiChildView. See setDefaultChildFrmSize() .
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
   * If no such menu is given, KMdi simply overlays the buttons
   * at the upper right-hand side of the main widget.
   */
   virtual void setMenuForSDIModeSysButtons( KMenuBar* = 0);

  /**
   * @return the decoration of the window frame of docked (attached) MDI views
   */
   static int frameDecorOfAttachedViews() { return m_frameDecoration; };
   /**
   * An SDI application user interface is faked:
   * @li an opened view is always maximized
   * @li buttons for maximized childframe mode aren't inserted in the main menubar
   * @li taskbar and windowmenu are not created/updated
   */
   void fakeSDIApplication();
   /**
   * @returns if we are faking an SDI application (fakeSDIApplication())
   */
   bool isFakingSDIApplication() const { return m_bSDIApplication; };
   /**
   *
   */
   virtual bool eventFilter(QObject *obj, QEvent *e );
   /**
   *
   */
   void findRootDockWidgets(QPtrList<KDockWidget>* pRootDockWidgetList, QValueList<QRect>* pPositionList);

   void setSwitching( const bool switching ) { m_bSwitching = switching; }
   bool switching(void) const { return m_bSwitching; }

public slots:
   /** addWindow demands a KMdiChildView. This method wraps every QWidget in such an object and
       this way you can every widget put under MDI control.
    */
   KMdiChildView* createWrapper(QWidget *view, const QString& name, const QString& shortName);
   /**
    * One of the most important methods at all!
    * Adds a KMdiChildView to the MDI system. The main frame takes it under control.
    * You can specify here whether:
    * <UL><LI>the view should be attached or detached.</LI>
    * <LI>shown or hidden</LI>
    * <LI>maximized, minimized or restored (normalized)</LI>
    * <LI>added as tool view (stay-on-top and toplevel) or added as document-type view.</LI?
    * </UL>
    */
   virtual void addWindow( KMdiChildView* pView, int flags = KMdi::StandardAdd);
   /**
   * See the method above for more details. Additionally, it moves to point pos.
   */
   virtual void addWindow( KMdiChildView* pView, QPoint pos, int flags = KMdi::StandardAdd);
   /**
   * See the method above for more details. Additionally, it sets the geometry.
   */
   virtual void addWindow( KMdiChildView* pView, QRect rectNormal, int flags = KMdi::StandardAdd);
   /**
   * Usually called from addWindow() when adding a tool view window. It reparents the given widget
   * as toplevel and stay-on-top on the application's main widget.
   */
   virtual KMdiToolViewAccessor *addToolWindow( QWidget* pWnd, KDockWidget::DockPosition pos = KDockWidget::DockNone, QWidget* pTargetWnd = 0L, int percent = 50, const QString& tabToolTip = 0, const QString& tabCaption = 0);
   virtual void deleteToolWindow( QWidget* pWnd);
   virtual void deleteToolWindow( KMdiToolViewAccessor *accessor);
   /**
    * Using this method you have to use the setWidget method of the access object, and it is very recommendet, that you use
    * the widgetContainer() method for the parent of your newly created widget
    */
   KMdiToolViewAccessor *createToolWindow();
   /**
   * Removes a KMdiChildView from the MDI system and from the main frame`s control.
   * Note: The view will not be deleted, but it's getting toplevel (reparent to 0)!
   */
   virtual void removeWindowFromMdi(KMdiChildView *pWnd);
   /**
   * Removes a KMdiChildView from the MDI system and from the main frame`s control.
   * Note: The view will be deleted!
   */
   virtual void closeWindow(KMdiChildView *pWnd, bool layoutTaskBar = true);
   /**
   * Switches the KMdiTaskBar on and off.
   */
   virtual void slot_toggleTaskBar();
   /**
    * Makes a main frame controlled undocked KMdiChildView docked.
    * Doesn't work on KMdiChildView which aren't added to the MDI system.
    * Use addWindow() for that.
    */
   virtual void attachWindow(KMdiChildView *pWnd,bool bShow=true,bool bAutomaticResize=false);
   /**
    * Makes a docked KMdiChildView undocked.
    * The view window still remains under the main frame's MDI control.
    */
   virtual void detachWindow(KMdiChildView *pWnd,bool bShow=true);
   /**
   * Someone wants that the MDI view to be closed. This method sends a KMdiViewCloseEvent to itself
   * to break the function call stack. See also event() .
   */
   virtual void childWindowCloseRequest(KMdiChildView *pWnd);
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
    * Docks all view windows. Toolviews use dockcontainers
    */
   virtual void switchToIDEAlMode();
   virtual void finishIDEAlMode(bool full=true);
   /**
    * Sets the appearance of the IDEAl mode. See KMultiTabBar styles for the first 3 bits.
    */
   void setIDEAlModeStyle(int flags);
   /*
   * @return if the view taskbar should be shown if there are MDI views
   */
   bool isViewTaskBarOn();
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
   /**
   * If in Childframe mode, we can switch between maximized or restored shown MDI views
   */
   virtual void setEnableMaximizedChildFrmMode(bool bEnable);
   /**
   * Activates the next open view
   */
   virtual void activateNextWin();
   /**
   * Activates the previous open view
   */
   virtual void activatePrevWin();
   /**
   * Activates the view first viewed concerning to the access time.
   */
   virtual void activateFirstWin();
   /**
   * Activates the view last viewed concerning to the access time.
   */
   virtual void activateLastWin();
   /**
   * Activates the view with the tab page index (TabPage mode only)
   */
   virtual void activateView(int index);

private:
   void setupToolViewsForIDEALMode();
   void setupTabbedDocumentViewSpace();
   class KMdiDocumentViewTabWidget * m_documentTabWidget;

protected:
   /**
   *
   */
   virtual void resizeEvent(QResizeEvent * );
   /**
   * Creates a new MDI taskbar (showing the MDI views as taskbar entries) and shows it.
   */
   virtual void createTaskBar();
   /**
   * Creates the MDI view area and connects some signals and slots with the KMdiMainFrm widget.
   */
   virtual void createMdiManager();
   /**
   * prevents fillWindowMenu() from m_pWindowMenu->clear(). You have to care for it by yourself.
   * This is useful if you want to add some actions in your overridden fillWindowMenu() method.
   */
   void blockClearingOfWindowMenu( bool bBlocked) { m_bClearingOfWindowMenuBlocked = bBlocked; };

   void findToolViewsDockedToMain(QPtrList<KDockWidget>* list,KDockWidget::DockPosition dprtmw);
   void dockToolViewsIntoContainers(QPtrList<KDockWidget>& widgetsToReparent,KDockWidget *container);
   QStringList prepareIdealToTabs(KDockWidget* container);
   void idealToolViewsToStandardTabs(QStringList widgetNames,KDockWidget::DockPosition pos,int sizee);

   /**
   * Get tabwidget visibility
   */
   KMdi::TabWidgetVisibility tabWidgetVisibility();
   /**
   * Set tabwidget visibility
   */
   void setTabWidgetVisibility( KMdi::TabWidgetVisibility );
   /**
   * Returns the tabwidget used in IDEAl and Tabbed modes. Returns 0 in other modes.
   */
   class KTabWidget * tabWidget() const;


protected slots: // Protected slots
   /**
   * Sets the focus to this MDI view, raises it, activates its taskbar button and updates
   * the system buttons in the main menubar when in maximized (Maximize mode).
   */
   virtual void activateView(KMdiChildView *pWnd);
   /**
   * Activates the MDI view (see activateView() ) and popups the taskBar popup menu (see taskBarPopup() ).
   */
   virtual void taskbarButtonRightClicked(KMdiChildView *pWnd);
   /**
   * Turns the system buttons for maximize mode (SDI mode) off, and disconnects them
   */
   void switchOffMaximizeModeForMenu(KMdiChildFrm* oldChild);
   /**
   * Reconnects the system buttons form maximize mode (SDI mode) with the new child frame
   */
   void updateSysButtonConnections( KMdiChildFrm* oldChild, KMdiChildFrm* newChild);
   /**
   * Usually called when the user clicks an MDI view item in the "Window" menu.
   */
   void windowMenuItemActivated(int id);
   /**
   * Usually called when the user clicks an MDI view item in the sub-popup menu "Docking" of the "Window" menu.
   */
   void dockMenuItemActivated(int id);
   /**
   * Popups the "Window" menu. See also windowPopup() .
   */
   void popupWindowMenu(QPoint p);
   /**
   * The timer for main widget moving has elapsed -> send drag end to all concerned views.
   */
   void dragEndTimeOut();
   /**
   * internally used to handle click on view close button (TabPage mode, only)
   */
   void closeViewButtonPressed();

   signals:
   /**
   * Signals the last attached KMdiChildView has been closed
   */
   void lastChildFrmClosed();
   /**
   * Signals the last KMdiChildView (that is under MDI control) has been closed
   */
   void lastChildViewClosed();
   /**
   * Signals that the Toplevel mode has been left
   */
   void leftTopLevelMode();
   /**
   * Signals that a child view has been detached (undocked to desktop)
   */
   void childViewIsDetachedNow(QWidget*);

   void collapseOverlapContainers();

   void mdiModeHasBeenChangedTo(KMdi::MdiMode);

   void viewActivated(KMdiChildView*);
   void viewDeactivated(KMdiChildView*);

public slots:
   void prevToolViewInDock();
   void nextToolViewInDock();

private slots:
   void setActiveToolDock(KMdiDockContainer*);
   void removeFromActiveDockList(KMdiDockContainer*);
   void slotDocCurrentChanged(QWidget*);
#define protected public
signals:
#undef protected
    void toggleTop();
    void toggleLeft();
    void toggleRight();
    void toggleBottom();
};

#endif //_KMDIMAINFRM_H_

// kate: space-indent on; indent-width 2; replace-tabs on;
